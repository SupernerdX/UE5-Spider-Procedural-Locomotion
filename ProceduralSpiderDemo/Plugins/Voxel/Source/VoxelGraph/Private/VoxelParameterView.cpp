// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelParameterView.h"
#include "VoxelGraph.h"
#include "VoxelGraphParametersViewContext.h"

FVoxelParameterView::FVoxelParameterView(
	FVoxelGraphParametersViewContext& Context,
	const FGuid& Guid,
	const FVoxelParameter& Parameter)
	: Context(Context)
	, Guid(Guid)
	, Parameter(Parameter)
{
}

FVoxelPinValue FVoxelParameterView::GetValue() const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelPinType ExposedType = GetType().GetExposedType();

	const FVoxelParameterValueOverride* ValueOverride = Context.FindValue(Guid);
	if (!ValueOverride)
	{
		return FVoxelPinValue(ExposedType);
	}

	ensure(ValueOverride->Value.CanBeCastedTo(ExposedType));
	return ValueOverride->Value;
}

FVoxelPinValue* FVoxelParameterView::GetOverrideValue() const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelPinType ExposedType = GetType().GetExposedType();

	FVoxelParameterValueOverride* ValueOverride = Context.FindValueOverride(Guid);
	if (!ValueOverride)
	{
		return nullptr;
	}

	ensure(ValueOverride->Value.CanBeCastedTo(ExposedType));
	return &ValueOverride->Value;
}

FVoxelPinValue FVoxelParameterView::GetDefaultValue() const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelPinType ExposedType = GetType().GetExposedType();

	const FVoxelParameterValueOverride* ValueOverride = Context.FindDefaultValue(Guid);
	if (!ValueOverride)
	{
		return FVoxelPinValue(ExposedType);
	}

	ensure(ValueOverride->Value.CanBeCastedTo(ExposedType));
	return ValueOverride->Value;
}