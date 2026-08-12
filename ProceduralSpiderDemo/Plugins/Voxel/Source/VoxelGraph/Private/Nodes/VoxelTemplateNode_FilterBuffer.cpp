// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelTemplateNode_FilterBuffer.h"
#include "VoxelBufferAccessor.h"
#include "VoxelCompilationGraph.h"

void FVoxelNode_FilterBuffer::Compute(const FVoxelGraphQuery Query) const
{
	const FValue ValueBuffer = ValuePin.Get(Query);
	const TValue<FVoxelBoolBuffer> ConditionBuffer = ConditionPin.Get(Query);

	VOXEL_GRAPH_WAIT(ValueBuffer, ConditionBuffer)
	{
		if (ConditionBuffer->IsConstant())
		{
			if (ConditionBuffer->GetConstant())
			{
				OutValuePin.Set(Query, ValueBuffer);
				return;
			}
			else
			{
				OutValuePin.Set(Query, FVoxelRuntimePinValue::Make(FVoxelBuffer::MakeEmpty(OutValuePin.GetInnerType_RuntimeOnly())));
				return;
			}
		}

		CheckVoxelBuffersNum(ValueBuffer.Get<FVoxelBuffer>(), ConditionBuffer);

		FVoxelInt32Buffer Indices;
		Indices.Allocate(ConditionBuffer->Num());

		int32 WriteIndex = 0;
		for (int32 Index = 0; Index < ConditionBuffer->Num(); Index++)
		{
			if ((*ConditionBuffer)[Index])
			{
				Indices.Set(WriteIndex++, Index);
			}
		}
		Indices.ShrinkTo(WriteIndex);

		const TSharedRef<const FVoxelBuffer> NewBuffer = ValueBuffer.Get<FVoxelBuffer>().Gather(Indices.View());

		OutValuePin.Set(Query, FVoxelRuntimePinValue::Make(NewBuffer));
	};
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_FilterBuffer::GetPromotionTypes(const FVoxelPin& Pin) const
{
	return FVoxelPinTypeSet::AllBuffers();
}

void FVoxelNode_FilterBuffer::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	GetPin(ValuePin).SetType(NewType);
	GetPin(OutValuePin).SetType(NewType);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTemplateNode_FilterBuffer::FVoxelTemplateNode_FilterBuffer()
{
	FixupBufferPins();
}

void FVoxelTemplateNode_FilterBuffer::ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins, TArray<FPin*>& OutPins) const
{
	for (int32 Index = 0; Index < NumBufferPins; Index++)
	{
		OutPins.Add(Call_Single<FVoxelNode_FilterBuffer>(Pins[Index], Pins[NumBufferPins]));
	}
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelTemplateNode_FilterBuffer::GetPromotionTypes(const FVoxelPin& Pin) const
{
	return FVoxelPinTypeSet::AllBuffers();
}

void FVoxelTemplateNode_FilterBuffer::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	const int32 PinIndex = (Pin.bIsInput ? BufferInputPins : BufferOutputPins).IndexOfByPredicate([&](const FPinRef& TargetPin)
	{
		return Pin == TargetPin;
	});

	if (!ensure(PinIndex != -1))
	{
		return;
	}

	if (!ensure(
		BufferInputPins.Num() == NumBufferPins &&
		BufferOutputPins.Num() == NumBufferPins))
	{
		FixupBufferPins();
		return;
	}

	GetPin(BufferInputPins[PinIndex]).SetType(NewType);
	GetPin(BufferOutputPins[PinIndex]).SetType(NewType);

	for (int32 Index = 0; Index < NumBufferPins; Index++)
	{
		if (!GetPin(BufferInputPins[Index]).GetType().IsWildcard() ||
			!GetPin(BufferOutputPins[Index]).GetType().IsWildcard())
		{
			continue;
		}

		GetPin(BufferInputPins[Index]).SetType(NewType);
		GetPin(BufferOutputPins[Index]).SetType(NewType);
	}
}
#endif

void FVoxelTemplateNode_FilterBuffer::PostSerialize()
{
	FixupBufferPins();

	Super::PostSerialize();
}

void FVoxelTemplateNode_FilterBuffer::FixupBufferPins()
{
	const int32 OldNumBufferPins = BufferInputPins.Num();
	if (!ensure(OldNumBufferPins == BufferOutputPins.Num()))
	{
		return;
	}

	TArray<FVoxelPinType> Types;
	for (int32 Index = 0; Index < OldNumBufferPins; Index++)
	{
		const FPinRef InputPin = BufferInputPins[Index];
		const FPinRef OutputPin = BufferOutputPins[Index];

		const FVoxelPinType Type = GetPin(InputPin).GetType();
		ensure(Type == GetPin(OutputPin).GetType());
		Types.Add(Type);

		RemovePin(InputPin.GetName());
		RemovePin(OutputPin.GetName());
	}

	BufferInputPins.Reset();
	BufferOutputPins.Reset();

	for (int32 Index = 0; Index < NumBufferPins; Index++)
	{
		BufferInputPins.Add(CreateInputPin(
			FVoxelPinType::MakeWildcardBuffer(),
			FName("Value", Index + 2),
			VOXEL_PIN_METADATA(void, nullptr, DisplayName("Value " + LexToString(Index + 1)))));

		BufferOutputPins.Add(CreateOutputPin(
			FVoxelPinType::MakeWildcardBuffer(),
			FName("FilteredValue", Index + 2),
			VOXEL_PIN_METADATA(void, nullptr, DisplayName("Filtered Value " + LexToString(Index + 1)))));

		if (Types.IsValidIndex(Index))
		{
			GetPin(BufferInputPins[Index]).SetType(Types[Index]);
			GetPin(BufferOutputPins[Index]).SetType(Types[Index]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString FVoxelTemplateNode_FilterBuffer::FDefinition::GetAddPinLabel() const
{
	return "Add Value";
}

FString FVoxelTemplateNode_FilterBuffer::FDefinition::GetAddPinTooltip() const
{
	return "Adds an additional buffer which will be filtered by same condition";
}

void FVoxelTemplateNode_FilterBuffer::FDefinition::AddInputPin()
{
	Node.NumBufferPins++;
	Node.FixupBufferPins();
}

bool FVoxelTemplateNode_FilterBuffer::FDefinition::CanRemoveInputPin() const
{
	return Node.NumBufferPins > 1;
}

void FVoxelTemplateNode_FilterBuffer::FDefinition::RemoveInputPin()
{
	Node.NumBufferPins--;
	Node.FixupBufferPins();
}
#endif