// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_MakeStruct.h"
#include "VoxelRuntimeStruct.h"
#include "VoxelRuntimeStructBuffer.h"
#include "StructUtils/UserDefinedStruct.h"

void FVoxelNode_MakeStruct::Initialize(FInitializer& Initializer)
{
	for (FPinRef_Input& PinRef : InputPins)
	{
		Initializer.InitializePinRef(PinRef);
	}
}

void FVoxelNode_MakeStruct::Compute(const FVoxelGraphQuery Query) const
{
	TVoxelArray<FValue> Values;
	Values.Reserve(InputPins.Num());

	for (const FPinRef_Input& Pin : InputPins)
	{
		Values.Add_EnsureNoGrow(Pin.Get(Query));
	}

	VOXEL_GRAPH_WAIT(Values)
	{
		const FVoxelPinType Type = StructPin.GetType_RuntimeOnly();
		if (!ensure(Type.GetInnerType().IsUserDefinedStruct()))
		{
			return;
		}

		if (Type.IsBuffer())
		{
			FVoxelRuntimeStructBuffer Result = FVoxelRuntimeStructBuffer::MakeEmpty(*Type.GetInnerType().GetStruct());

			for (int32 Index = 0; Index < InputPins.Num(); Index++)
			{
				Result.SetBuffer(
					InputPins[Index].GetName(),
					ConstCast(Values[Index].GetSharedStruct<FVoxelBuffer>()));
			}

			StructPin.Set(Query, FVoxelRuntimePinValue::Make(MakeSharedCopy(MoveTemp(Result))));
		}
		else
		{
			const TSharedRef<FVoxelRuntimeStruct> Result = MakeShared<FVoxelRuntimeStruct>();
			Result->UserStruct = CastChecked<UUserDefinedStruct>(Type.GetStruct());
			Result->PropertyNameToValue.Reserve(InputPins.Num());

			for (int32 Index = 0; Index < InputPins.Num(); Index++)
			{
				Result->PropertyNameToValue.Add_EnsureNew(
					InputPins[Index].GetName(),
					Values[Index]);
			}

			StructPin.Set(Query, FVoxelRuntimePinValue::MakeRuntimeStruct(Result));
		}
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_MakeStruct::GetPromotionTypes(const FVoxelPin& Pin) const
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

void FVoxelNode_MakeStruct::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Pin.SetType(NewType);

	for (FVoxelPin& OtherPin : GetPins())
	{
		OtherPin.SetType(
			NewType.IsBuffer()
			? OtherPin.GetType().GetBufferType()
			: OtherPin.GetType().GetInnerType());
	}

	FixupInputPins();
}
#endif

void FVoxelNode_MakeStruct::PostSerialize()
{
	Super::PostSerialize();

	FixupInputPins();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_MakeStruct::FixupInputPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (const FPinRef& Pin : InputPins)
	{
		RemovePin(Pin.GetName());
	}
	InputPins.Reset();

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

		const FPinRef_Input Pin = CreateInputPin(
			FVoxelPinType::MakeWildcard(),
			Property.GetFName(),
			VOXEL_PIN_METADATA(void, nullptr, DisplayName(DisplayName)));

		FVoxelPinType Type = FVoxelRuntimeStruct::GetRuntimeType(FVoxelPinType(Property));
		if (StructType.IsBuffer())
		{
			Type = Type.GetBufferType();
		}

		GetPin(Pin).SetType(Type);

		InputPins.Add(Pin);
	}
}