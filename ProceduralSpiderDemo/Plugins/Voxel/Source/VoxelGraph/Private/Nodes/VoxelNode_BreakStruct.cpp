// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_BreakStruct.h"
#include "VoxelRuntimeStruct.h"
#include "VoxelRuntimeStructBuffer.h"

void FVoxelNode_BreakStruct::Initialize(FInitializer& Initializer)
{
	for (FPinRef_Output& PinRef : OutputPins)
	{
		Initializer.InitializePinRef(PinRef);
	}
}

void FVoxelNode_BreakStruct::Compute(const FVoxelGraphQuery Query) const
{
	const FValue StructValue = StructPin.Get(Query);

	VOXEL_GRAPH_WAIT(StructValue)
	{
		const FVoxelPinType Type = StructPin.GetType_RuntimeOnly();
		if (!ensure(Type.GetInnerType().IsUserDefinedStruct()))
		{
			return;
		}

		if (Type.IsBuffer())
		{
			const FVoxelRuntimeStructBuffer& Struct = StructValue.GetStructView().Get<FVoxelRuntimeStructBuffer>();

			for (const FPinRef_Output& OutputPin : OutputPins)
			{
				if (!OutputPin.ShouldCompute())
				{
					continue;
				}

				const TSharedPtr<const FVoxelBuffer> Buffer = Struct.GetBuffer(OutputPin.GetName());
				if (!ensureVoxelSlow(Buffer))
				{
					continue;
				}

				OutputPin.Set(Query, FVoxelRuntimePinValue::Make(Buffer.ToSharedRef()));
			}
		}
		else
		{
			const FVoxelRuntimeStruct& Struct = StructValue.GetStructView().Get<FVoxelRuntimeStruct>();

			for (const FPinRef_Output& OutputPin : OutputPins)
			{
				if (!OutputPin.ShouldCompute())
				{
					continue;
				}

				const FVoxelRuntimePinValue* Value = Struct.PropertyNameToValue.Find(OutputPin.GetName());
				if (!ensureVoxelSlow(Value))
				{
					continue;
				}

				OutputPin.Set(Query, *Value);
			}
		}
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_BreakStruct::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == StructPin)
	{
		FVoxelPinTypeSet Result;
		Result.Add(FVoxelPinTypeSet::AllUserStructs());
		Result.Add(FVoxelPinTypeSet::AllUserStructs().GetBufferTypes());
		return Result;
	}

	FVoxelPinTypeSet Result;
	Result.Add(Pin.GetType().GetInnerType());
	Result.Add(Pin.GetType().GetBufferType());
	return Result;
}

void FVoxelNode_BreakStruct::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Pin.SetType(NewType);

	for (FVoxelPin& OtherPin : GetPins())
	{
		OtherPin.SetType(
			NewType.IsBuffer()
			? OtherPin.GetType().GetBufferType()
			: OtherPin.GetType().GetInnerType());
	}

	FixupOutputPins();
}
#endif

void FVoxelNode_BreakStruct::PostSerialize()
{
	Super::PostSerialize();

	FixupOutputPins();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_BreakStruct::FixupOutputPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (const FPinRef& Pin : OutputPins)
	{
		RemovePin(Pin.GetName());
	}
	OutputPins.Reset();

	const FVoxelPinType StructType = GetPin(StructPin).GetType();
	const FVoxelPinType InnerType = StructType.GetInnerType();

	if (InnerType.IsWildcard() ||
		!ensure(InnerType.IsUserDefinedStruct()))
	{
		return;
	}

	for (const FProperty& Property : GetStructProperties(InnerType.GetStruct()))
	{
		if (!FVoxelPinType::IsSupported(Property))
		{
			continue;
		}

		FString DisplayName;
#if WITH_EDITOR
		DisplayName = Property.GetDisplayNameText().ToString();
#endif

		const FPinRef_Output Pin = CreateOutputPin(
			FVoxelPinType::MakeWildcard(),
			Property.GetFName(),
			VOXEL_PIN_METADATA(void, nullptr, DisplayName(DisplayName)));

		FVoxelPinType Type = FVoxelRuntimeStruct::GetRuntimeType(FVoxelPinType(Property));
		if (StructType.IsBuffer())
		{
			Type = Type.GetBufferType();
		}

		GetPin(Pin).SetType(Type);

		OutputPins.Add(Pin);
	}
}