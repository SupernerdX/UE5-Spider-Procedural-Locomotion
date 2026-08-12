// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelSculptMode.h"
#include "Sculpt/Volume/VoxelVolumeModifier.h"
#include "VoxelCubeVolumeModifier.generated.h"

USTRUCT()
struct VOXEL_API FVoxelCubeVolumeModifier : public FVoxelVolumeModifier
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	FVector Center = FVector(ForceInit);

	UPROPERTY()
	FVector Size = FVector(ForceInit);

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	float Roundness = 0.f;

	UPROPERTY()
	float Smoothness = 0.f;

	UPROPERTY()
	EVoxelSculptMode Mode = {};

	//~ Begin FVoxelVolumeTransaction_Modifier Interface
	virtual TSharedPtr<IVoxelVolumeModifierRuntime> GetRuntime() const override;
	//~ End FVoxelVolumeTransaction_Modifier Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelCubeVolumeModifierRuntime : public IVoxelVolumeModifierRuntime
{
public:
	const FVector Center;
	const FVector Size;
	const FRotator Rotation;
	const float Roundness;
	const float Smoothness;
	const EVoxelSculptMode Mode;

	FVoxelCubeVolumeModifierRuntime(
		const FVector& Center,
		const FVector& Size,
		const FRotator& Rotation,
		const float Roundness,
		const float Smoothness,
		const EVoxelSculptMode Mode)
		: Center(Center)
		, Size(Size)
		, Rotation(Rotation)
		, Roundness(Roundness)
		, Smoothness(Smoothness)
		, Mode(Mode)
	{
	}

public:
	//~ Begin FVoxelCubeVolumeModifierRuntime Interface
	virtual FVoxelBox GetBounds() const override;
	virtual void Apply(FVoxelVolumeSculptCanvasWriter& Writer) const override;
	//~ End FVoxelCubeVolumeModifierRuntime Interface
};