// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelParameter.h"

class FVoxelGraphParametersViewBase;
class FVoxelGraphParametersViewContext;

class VOXELGRAPH_API FVoxelParameterView
{
public:
	FVoxelGraphParametersViewContext& Context;
	const FGuid Guid;

	FVoxelParameterView(
		FVoxelGraphParametersViewContext& Context,
		const FGuid& Guid,
		const FVoxelParameter& Parameter);
	UE_NONCOPYABLE(FVoxelParameterView)

public:
	FORCEINLINE const FVoxelParameter& GetParameter() const
	{
		return Parameter;
	}
	FORCEINLINE FName GetName() const
	{
		return Parameter.Name;
	}
#if WITH_EDITOR
	FORCEINLINE const FString& GetDescription() const
	{
		return Parameter.Description;
	}
#endif
	FORCEINLINE const FVoxelPinType& GetType() const
	{
		return Parameter.Type;
	}

	// Value.Type is GetType().GetExposedType()
	FVoxelPinValue GetValue() const;
	FVoxelPinValue* GetOverrideValue() const;
	FVoxelPinValue GetDefaultValue() const;

private:
	const FVoxelParameter Parameter;
};