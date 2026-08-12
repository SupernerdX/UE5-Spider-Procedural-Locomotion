// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Sculpt/VoxelLevelToolType.h"
#include "Sculpt/Height/VoxelHeightModifier.h"
#include "VoxelFlattenHeightModifier.generated.h"

USTRUCT()
struct VOXEL_API FVoxelFlattenHeightModifier : public FVoxelHeightModifier
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY()
	FVector2D Center = FVector2D(ForceInit);

	UPROPERTY()
	float Radius = 0.f;

	UPROPERTY()
	float Falloff = 0.f;

	UPROPERTY()
	EVoxelLevelToolType Type = {};

	UPROPERTY()
	float TargetHeight = 0;

	UPROPERTY()
	FVoxelToolBrush Brush;

public:
	//~ Begin FVoxelHeightModifier Interface
	virtual TSharedPtr<IVoxelHeightModifierRuntime> GetRuntime() const override;
	//~ End FVoxelHeightModifier Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelFlattenHeightModifierRuntime : public IVoxelHeightModifierRuntime
{
public:
	const FVector2D Center;
	const float Radius;
	const float Falloff;
	const EVoxelLevelToolType Type;
	const float TargetHeight;
	const TSharedRef<const FVoxelToolRuntimeBrush> Brush;

	FVoxelFlattenHeightModifierRuntime(
		const FVector2D& Center,
		const float Radius,
		const float Falloff,
		const EVoxelLevelToolType Type,
		const float TargetHeight,
		const TSharedRef<const FVoxelToolRuntimeBrush>& Brush)
		: Center(Center)
		, Radius(Radius)
		, Falloff(Falloff)
		, Type(Type)
		, TargetHeight(TargetHeight)
		, Brush(Brush)
	{
	}

public:
	//~ Begin IVoxelHeightModifierRuntime Interface
	virtual FVoxelBox2D GetBounds() const override;
	virtual void Apply(FVoxelHeightSculptCanvasWriter& Writer) const override;
	//~ End IVoxelHeightModifierRuntime Interface
};