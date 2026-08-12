// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelLevelToolType.h"
#include "Sculpt/Volume/VoxelVolumeModifier.h"
#include "VoxelFlattenVolumeModifier.generated.h"

USTRUCT()
struct VOXEL_API FVoxelFlattenVolumeModifier : public FVoxelVolumeModifier
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	FVector Center = FVector(ForceInit);

	UPROPERTY()
	FVector Normal = FVector(ForceInit);

	UPROPERTY()
	float Radius = 0.f;

	UPROPERTY()
	float Height = 0.f;

	UPROPERTY()
	float Falloff = 0.f;

	UPROPERTY()
	EVoxelLevelToolType Type = {};

	//~ Begin FVoxelVolumeModifier Interface
	virtual TSharedPtr<IVoxelVolumeModifierRuntime> GetRuntime() const override;
	//~ End FVoxelVolumeModifier Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelFlattenVolumeModifierRuntime : public IVoxelVolumeModifierRuntime
{
public:
	const FVector Center;
	const FVector Normal;
	const float Radius;
	const float Height;
	const float Falloff;
	const EVoxelLevelToolType Type;

	FVoxelFlattenVolumeModifierRuntime(
		const FVector& Center,
		const FVector& Normal,
		const float Radius,
		const float Height,
		const float Falloff,
		const EVoxelLevelToolType Type)
		: Center(Center)
		, Normal(Normal)
		, Radius(Radius)
		, Height(Height)
		, Falloff(Falloff)
		, Type(Type)
	{
	}

public:
	//~ Begin IVoxelVolumeModifierRuntime Interface
	virtual FVoxelBox GetBounds() const override;
	virtual void Apply(FVoxelVolumeSculptCanvasWriter& Writer) const override;
	//~ End IVoxelVolumeModifierRuntime Interface
};