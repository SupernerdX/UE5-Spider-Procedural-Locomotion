// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Sculpt/Volume/VoxelVolumeModifier.h"
#include "VoxelAngleVolumeModifier.generated.h"

UENUM(BlueprintType)
enum class EVoxelSDFMergeMode : uint8
{
	// Additive mode: will only grow the surface
	Union,
	// Destructive mode: will only shrink the surface
	Intersection,
	// Will add and remove at the same time
	Override
};

USTRUCT()
struct VOXEL_API FVoxelAngleVolumeModifier : public FVoxelVolumeModifier
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
	FPlane Plane = FPlane(ForceInit);

	UPROPERTY()
	EVoxelSDFMergeMode MergeMode = {};

	UPROPERTY()
	FVoxelToolBrush Brush;

	//~ Begin FVoxelVolumeModifier Interface
	virtual TSharedPtr<IVoxelVolumeModifierRuntime> GetRuntime() const override;
	//~ End FVoxelVolumeModifier Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelAngleVolumeModifierRuntime : public IVoxelVolumeModifierRuntime
{
public:
	const FVector Center;
	const float Radius;
	const float Strength;
	const FPlane Plane;
	const EVoxelSDFMergeMode MergeMode;
	const TSharedRef<const FVoxelToolRuntimeBrush> Brush;

	FVoxelAngleVolumeModifierRuntime(
		const FVector& Center,
		const float Radius,
		const float Strength,
		const FPlane& Plane,
		const EVoxelSDFMergeMode MergeMode,
		const TSharedRef<const FVoxelToolRuntimeBrush>& Brush)
		: Center(Center)
		, Radius(Radius)
		, Strength(Strength)
		, Plane(Plane)
		, MergeMode(MergeMode)
		, Brush(Brush)
	{
	}

public:
	//~ Begin IVoxelVolumeModifierRuntime Interface
	virtual FVoxelBox GetBounds() const override;
	virtual void Apply(FVoxelVolumeSculptCanvasWriter& Writer) const override;
	//~ End IVoxelVolumeModifierRuntime Interface
};