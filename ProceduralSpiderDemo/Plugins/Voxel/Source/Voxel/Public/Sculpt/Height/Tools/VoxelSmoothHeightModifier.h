// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Sculpt/Height/VoxelHeightModifier.h"
#include "VoxelSmoothHeightModifier.generated.h"

USTRUCT()
struct VOXEL_API FVoxelSmoothHeightModifier : public FVoxelHeightModifier
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY()
	FVector2D Center = FVector2D(ForceInit);

	UPROPERTY()
	float Radius = 0.f;

	UPROPERTY()
	float Strength = 0.f;

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

class VOXEL_API FVoxelSmoothHeightModifierRuntime : public IVoxelHeightModifierRuntime
{
public:
	const FVector2D Center;
	const float Radius;
	const float Strength;
	const TSharedRef<const FVoxelToolRuntimeBrush> Brush;

	FVoxelSmoothHeightModifierRuntime(
		const FVector2D& Center,
		const float Radius,
		const float Strength,
		const TSharedRef<const FVoxelToolRuntimeBrush>& Brush)
		: Center(Center)
		, Radius(Radius)
		, Strength(Strength)
		, Brush(Brush)
	{
	}

public:
	//~ Begin IVoxelHeightModifierRuntime Interface
	virtual FVoxelBox2D GetBounds() const override;
	virtual void Apply(FVoxelHeightSculptCanvasWriter& Writer) const override;
	//~ End IVoxelHeightModifierRuntime Interface
};
