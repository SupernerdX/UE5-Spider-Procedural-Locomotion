// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Sculpt/VoxelSculptMode.h"
#include "Sculpt/Height/VoxelHeightModifier.h"
#include "VoxelSculptHeightModifier.generated.h"

USTRUCT()
struct VOXEL_API FVoxelSculptHeightModifier : public FVoxelHeightModifier
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
	EVoxelSculptMode Mode = {};

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

class VOXEL_API FVoxelSculptHeightModifierRuntime : public IVoxelHeightModifierRuntime
{
public:
	const FVector2D Center;
	const float Radius;
	const float Strength;
	const EVoxelSculptMode Mode;
	const TSharedRef<const FVoxelToolRuntimeBrush> Brush;

	FVoxelSculptHeightModifierRuntime(
		const FVector2D& Center,
		const float Radius,
		const float Strength,
		const EVoxelSculptMode Mode,
		const TSharedRef<const FVoxelToolRuntimeBrush>& Brush)
		: Center(Center)
		, Radius(Radius)
		, Strength(Strength)
		, Mode(Mode)
		, Brush(Brush)
	{
	}

public:
	//~ Begin IVoxelHeightModifierRuntime Interface
	virtual FVoxelBox2D GetBounds() const override;
	virtual void Apply(FVoxelHeightSculptCanvasWriter& Writer) const override;
	//~ End IVoxelHeightModifierRuntime Interface
};
