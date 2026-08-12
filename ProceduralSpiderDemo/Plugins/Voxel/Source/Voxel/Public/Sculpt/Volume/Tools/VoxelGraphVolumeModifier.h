// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNodeEvaluator.h"
#include "Sculpt/Volume/VoxelVolumeModifier.h"
#include "Sculpt/Volume/VoxelVolumeSculptGraphWrapper.h"
#include "VoxelGraphVolumeModifier.generated.h"

struct FVoxelOutputNode_OutputSculptDistance;

USTRUCT(NotBlueprintType)
struct VOXEL_API FVoxelGraphVolumeModifier : public FVoxelVolumeModifier
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	float Radius = 0;

	UPROPERTY()
	FVoxelVolumeSculptGraphWrapper Graph;

	//~ Begin FVoxelVolumeModifier Interface
	virtual TSharedPtr<IVoxelVolumeModifierRuntime> GetRuntime() const override;
	//~ End FVoxelVolumeModifier Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelGraphVolumeModifierRuntime : public IVoxelVolumeModifierRuntime
{
public:
	const FTransform Transform;
	const float Radius;
	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptDistance> Evaluator;

	FVoxelGraphVolumeModifierRuntime(
		const FTransform& Transform,
		const float Radius,
		const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptDistance>& Evaluator)
		: Transform(Transform)
		, Radius(Radius)
		, Evaluator(Evaluator)
	{
	}

public:
	//~ Begin IVoxelVolumeModifierRuntime Interface
	virtual FVoxelBox GetBounds() const override;
	virtual void Apply(FVoxelVolumeSculptCanvasWriter& Writer) const override;
	//~ End IVoxelVolumeModifierRuntime Interface
};