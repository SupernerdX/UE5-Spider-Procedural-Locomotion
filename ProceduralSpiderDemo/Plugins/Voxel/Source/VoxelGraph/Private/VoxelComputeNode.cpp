// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelComputeNode.h"
#include "VoxelComputeNodeImpl.h"
#include "VoxelBuffer.h"
#include "VoxelBufferStruct.h"
#include "VoxelTerminalBuffer.h"

void FVoxelComputeNode::Initialize(FInitializer& Initializer)
{
	ensure(!CachedPtr);
	CachedPtr = GVoxelNodeISPCPtrs.FindRef(GetStruct()->GetFName());

	ensureMsgf(CachedPtr, TEXT("Missing ISPC compute for %s. If you added a custom FVoxelComputeNode, you need to start the editor with -VoxelDevWorkflow"),
		*GetStruct()->GetName());

	ensure(InputPins.Num() == 0);
	ensure(OutputPins.Num() == 0);
	InputPins.Reserve(GetPins().Num());
	OutputPins.Reserve(GetPins().Num());

	Initializer.ClearPinRefs();

	for (const FVoxelPin& Pin : GetPins())
	{
		if (Pin.bIsInput)
		{
			checkf(OutputPins.Num() == 0, TEXT("Output pins should always be after input pins in ISPC nodes"));

			FPinRef_Input& PinRef = InputPins.Emplace_GetRef_EnsureNoGrow(Pin.Name);
			Initializer.InitializePinRef(PinRef);
		}
		else
		{
			FPinRef_Output& PinRef = OutputPins.Emplace_GetRef_EnsureNoGrow(Pin.Name);
			Initializer.InitializePinRef(PinRef);
		}
	}
}

void FVoxelComputeNode::Compute(const FVoxelGraphQuery Query) const
{
	if (!ensure(CachedPtr))
	{
		return;
	}

	TVoxelInlineArray<FValue, 16> InputValues;
	for (const FPinRef_Input& InputPin : InputPins)
	{
		InputValues.Add(InputPin.Get(Query));
	}

	Query.AddTask([this, Query, InputValues = MoveTemp(InputValues)]
	{
		int32 Num = 1;
		for (const FValue& InputValue : InputValues)
		{
			const FVoxelRuntimePinValue& Value = InputValue.GetValue();
			if (!Value.IsBuffer())
			{
				// Uniform input
				// We might still have some buffer inputs though, eg FVoxelTemplateNode_LerpBase
				continue;
			}

			const int32 NewNum = Value.Get<FVoxelBuffer>().Num_Slow();
			if (Num == 1)
			{
				Num = NewNum;
				continue;
			}
			if (NewNum == 1 ||
				NewNum == Num)
			{
				continue;
			}

			RaiseBufferError();
			return;
		}

		if (Num == 0)
		{
			for (const FPinRef_Output& OutputPin : OutputPins)
			{
				OutputPin.Set(Query, FVoxelRuntimePinValue::Make(FVoxelBuffer::MakeEmpty(OutputPin.GetInnerType_RuntimeOnly())));
			}

			return;
		}

		TVoxelInlineArray<TSharedRef<FVoxelBuffer>, 16> ConstantInputBufferRefs;
		ConstantInputBufferRefs.Reserve(InputPins.Num());

		TVoxelInlineArray<const FVoxelBuffer*, 16> InputBuffers;
		InputBuffers.Reserve(InputPins.Num());

		for (const FValue& FutureValue : InputValues)
		{
			const FVoxelRuntimePinValue& Value = FutureValue.GetValue();
			if (Value.IsBuffer())
			{
				InputBuffers.Add(&Value.Get<FVoxelBuffer>());
			}
			else
			{
				const TSharedRef<FVoxelBuffer> Buffer = FVoxelBuffer::MakeConstant(Value);
				ConstantInputBufferRefs.Add_EnsureNoGrow(Buffer);
				InputBuffers.Add_EnsureNoGrow(&Buffer.Get());
			}
		}

		TVoxelInlineArray<TSharedRef<FVoxelBuffer>, 16> OutputBuffers;
		OutputBuffers.Reserve(OutputPins.Num());

		for (const FPinRef_Output& OutputPin : OutputPins)
		{
			const TSharedRef<FVoxelBuffer> Buffer = FVoxelBuffer::MakeEmpty(OutputPin.GetInnerType_RuntimeOnly());
			Buffer->Allocate(Num);
			OutputBuffers.Add_EnsureNoGrow(Buffer);
		}

		{
			TVoxelInlineArray<ispc::FVoxelInputBuffer, 64> ISPCInputBuffers;
			for (const FVoxelBuffer* Buffer : InputBuffers)
			{
				if (const FVoxelTerminalBuffer* TerminalBuffer = Buffer->As<FVoxelTerminalBuffer>())
				{
					checkVoxelSlow(TerminalBuffer->IsConstant() || TerminalBuffer->Num() == Num);

					ispc::FVoxelInputBuffer& ISPCBuffer = ISPCInputBuffers.Emplace_GetRef();
					ISPCBuffer.Data = TerminalBuffer->GetRawData();
					ISPCBuffer.bIsConstant = TerminalBuffer->IsConstant();

					continue;
				}

				for (const FVoxelTerminalBuffer& TerminalBuffer : Buffer->AsChecked<FVoxelBufferStruct>().GetTerminalBuffers())
				{
					checkVoxelSlow(TerminalBuffer.IsConstant() || TerminalBuffer.Num() == Num);

					ispc::FVoxelInputBuffer& ISPCBuffer = ISPCInputBuffers.Emplace_GetRef();
					ISPCBuffer.Data = TerminalBuffer.GetRawData();
					ISPCBuffer.bIsConstant = TerminalBuffer.IsConstant();
				}
			}

			TVoxelInlineArray<void*, 64> ISPCOutputBuffers;
			for (const TSharedRef<FVoxelBuffer>& Buffer : OutputBuffers)
			{
				if (FVoxelTerminalBuffer* TerminalBuffer = Buffer->As<FVoxelTerminalBuffer>())
				{
					checkVoxelSlow(TerminalBuffer->Num() == Num);
					ISPCOutputBuffers.Add(TerminalBuffer->GetRawData());
					continue;
				}

				for (FVoxelTerminalBuffer& TerminalBuffer : Buffer->AsChecked<FVoxelBufferStruct>().GetTerminalBuffers())
				{
					checkVoxelSlow(TerminalBuffer.Num() == Num);
					ISPCOutputBuffers.Add(TerminalBuffer.GetRawData());
				}
			}

			VOXEL_SCOPE_COUNTER_FORMAT("%s Num=%d", *GetStruct()->GetName(), Num);
			FVoxelNodeStatScope StatScope(*this, Num);

			checkVoxelSlow(CachedPtr);
			(*CachedPtr)(ISPCInputBuffers.GetData(), ISPCOutputBuffers.GetData(), Num);
		}

		checkVoxelSlow(OutputBuffers.Num() == OutputPins.Num());
		for (int32 Index = 0; Index < OutputPins.Num(); Index++)
		{
			const FPinRef_Output& Pin = OutputPins[Index];
			const TSharedRef<FVoxelBuffer>& Buffer = OutputBuffers[Index];

			if (Pin.GetType_RuntimeOnly().IsBuffer())
			{
				Pin.Set(Query, FVoxelRuntimePinValue::Make(Buffer));
			}
			else
			{
				Pin.Set(Query, Buffer->GetGenericConstant());
			}
		}
	});
}