// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Sculpt/Volume/VoxelVolumeModifier.h"
#include "VoxelSmoothVolumeModifier.generated.h"

class FVoxelToolRuntimeBrush;

USTRUCT()
struct VOXEL_API FVoxelSmoothVolumeModifier : public FVoxelVolumeModifier
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	FVector Center = FVector(ForceInit);

	UPROPERTY()
	float Radius = 0.f;

	UPROPERTY()
	float Strength = 0.f;

	UPROPERTY()
	FVoxelToolBrush Brush;

	//~ Begin FVoxelVolumeModifier Interface
	virtual TSharedPtr<IVoxelVolumeModifierRuntime> GetRuntime() const override;
	//~ End FVoxelVolumeModifier Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelSmoothVolumeModifierRuntime : public IVoxelVolumeModifierRuntime
{
public:
	const FVector Center;
	const float Radius;
	const float Strength;
	const TSharedRef<const FVoxelToolRuntimeBrush> Brush;

	FVoxelSmoothVolumeModifierRuntime(
		const FVector& Center,
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
	//~ Begin IVoxelVolumeModifierRuntime Interface
	virtual FVoxelBox GetBounds() const override;
	virtual void Apply(FVoxelVolumeSculptCanvasWriter& Writer) const override;
	//~ End IVoxelVolumeModifierRuntime Interface
};