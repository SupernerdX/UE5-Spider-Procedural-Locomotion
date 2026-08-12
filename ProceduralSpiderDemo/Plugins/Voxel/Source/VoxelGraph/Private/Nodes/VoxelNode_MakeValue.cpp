// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_MakeValue.h"

void FVoxelNode_MakeValue::Initialize(FInitializer& Initializer)
{
	RuntimeValue = FVoxelPinType::MakeRuntimeValue(
		GetPin(ValuePin).GetType(),
		Value,
		{});
}

void FVoxelNode_MakeValue::Compute(const FVoxelGraphQuery Query) const
{
	ValuePin.Set(Query, RuntimeValue);
}

void FVoxelNode_MakeValue::PostSerialize()
{
	Super::PostSerialize();
	FixupValue();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_MakeValue::GetPromotionTypes(const FVoxelPin& Pin) const
{
	return FVoxelPinTypeSet::All();
}

void FVoxelNode_MakeValue::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Super::PromotePin(Pin, NewType);
	FixupValue();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_MakeValue::FixupValue()
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelPinType Type = GetPin(ValuePin).GetType().GetExposedType();
	if (Type.IsWildcard())
	{
		Value = {};
		return;
	}

	Value.Fixup(Type);
}