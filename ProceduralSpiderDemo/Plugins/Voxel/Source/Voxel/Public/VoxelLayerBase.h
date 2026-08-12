// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelQuery.h"
#include "VoxelStampTree.h"

class FVoxelLayerBase
{
public:
	const FVoxelWeakStackLayer WeakLayer;
	const TVoxelArray<TSharedRef<FVoxelFutureStampTree>> LODToTree;

	FVoxelLayerBase(
		const FVoxelWeakStackLayer& WeakLayer,
		const TVoxelArray<TSharedRef<FVoxelFutureStampTree>>& LODToTree)
		: WeakLayer(WeakLayer)
		, LODToTree(LODToTree)
	{
	}

public:
	FVoxelStampTree& GetTree(int32 LOD) const
	{
		LOD = FMath::Clamp(LOD, 0, LODToTree.Num() - 1);
		return LODToTree[LOD]->GetTree();
	}
};