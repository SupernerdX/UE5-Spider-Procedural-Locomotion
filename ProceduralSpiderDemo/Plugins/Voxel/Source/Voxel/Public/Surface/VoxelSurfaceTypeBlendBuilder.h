// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Surface/VoxelSurfaceTypeBlend.h"

class VOXEL_API FVoxelSurfaceTypeBlendBuilder
{
public:
	void Build(
		FVoxelSurfaceTypeBlend& OutBlend,
		bool bSkipSortByType = false);

public:
	FORCEINLINE void Reset()
	{
		Layers.Reset();
	}
	FORCEINLINE void AddLayer(
		const FVoxelSurfaceType Type,
		const float Weight)
	{
		checkVoxelSlow(Weight >= 0);

		for (FLayer& Layer : Layers)
		{
			if (Layer.Type == Type)
			{
				Layer.Weight += Weight;
				return;
			}
		}

		Layers.Add(FLayer
		{
			Type,
			Weight
		});
	}
	FORCEINLINE void AddLayer_CheckNew(
		const FVoxelSurfaceType Type,
		const float Weight)
	{
		checkVoxelSlow(Weight >= 0);

#if VOXEL_DEBUG
		for (const FLayer& Layer : Layers)
		{
			checkVoxelSlow(Layer.Type != Type);
		}
#endif

		Layers.Add(FLayer
		{
			Type,
			Weight
		});
	}

private:
	struct FLayer
	{
		FVoxelSurfaceType Type;
		float Weight = 0.f;
	};
	TVoxelInlineArray<FLayer, 16> Layers;

	void PopLayers();
};