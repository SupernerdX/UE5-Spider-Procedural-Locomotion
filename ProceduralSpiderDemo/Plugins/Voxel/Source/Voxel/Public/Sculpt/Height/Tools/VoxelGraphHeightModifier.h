// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNodeEvaluator.h"
#include "Sculpt/Height/VoxelHeightModifier.h"
#include "Sculpt/Height/VoxelHeightSculptGraphWrapper.h"
#include "VoxelGraphHeightModifier.generated.h"

struct FVoxelOutputNode_OutputSculptHeight;

USTRUCT()
struct VOXEL_API FVoxelGraphHeightModifier : public FVoxelHeightModifier
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY()
	FVector2D Center = FVector2D(ForceInit);

	UPROPERTY()
	float Radius = 0;

	UPROPERTY()
	FVoxelHeightSculptGraphWrapper Graph;

public:
	//~ Begin FVoxelHeightModifier Interface
	virtual TSharedPtr<IVoxelHeightModifierRuntime> GetRuntime() const override;
	//~ End FVoxelHeightModifier Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelGraphHeightModifierRuntime : public IVoxelHeightModifierRuntime
{
public:
	const FVector2D Center;
	const float Radius;
	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptHeight> Evaluator;

	FVoxelGraphHeightModifierRuntime(
		const FVector2D& Center,
		const float Radius,
		const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptHeight>& Evaluator)
		: Center(Center)
		, Radius(Radius)
		, Evaluator(Evaluator)
	{
	}

public:
	//~ Begin IVoxelHeightModifierRuntime Interface
	virtual FVoxelBox2D GetBounds() const override;
	virtual void Apply(FVoxelHeightSculptCanvasWriter& Writer) const override;
	//~ End IVoxelHeightModifierRuntime Interface
};