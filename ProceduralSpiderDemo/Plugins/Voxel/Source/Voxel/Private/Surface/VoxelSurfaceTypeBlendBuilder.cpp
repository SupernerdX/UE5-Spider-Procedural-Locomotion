// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSurfaceTypeBlendBuilder.h"

void FVoxelSurfaceTypeBlendBuilder::Build(
	FVoxelSurfaceTypeBlend& OutBlend,
	const bool bSkipSortByType)
{
	if (Layers.Num() == 0)
	{
		OutBlend.InitializeNull();
		return;
	}

	if (Layers.Num() > FVoxelSurfaceTypeBlend::MaxLayers)
	{
		PopLayers();
	}
	checkVoxelSlow(Layers.Num() <= FVoxelSurfaceTypeBlend::MaxLayers);

	double Multiplier;
	{
		double WeightSum = 0;
		for (const FLayer& Layer : Layers)
		{
			WeightSum += Layer.Weight;
		}

		if (WeightSum == 0)
		{
			OutBlend.InitializeNull();
			return;
		}

		Multiplier = MAX_uint16 / WeightSum;
	}

	int32 WeightSum = 0;
	{
		int32 WriteIndex = 0;

		for (int32 Index = 0; Index < Layers.Num(); Index++)
		{
			const FLayer& Layer = Layers[Index];

			const int32 Weight = FMath::FloorToInt32(Layer.Weight * Multiplier);
			if (Weight == 0)
			{
				continue;
			}

			WeightSum += Weight;

			FVoxelSurfaceTypeBlendLayer& BlendLayer = OutBlend.Layers[WriteIndex++];
			BlendLayer.Type = Layer.Type;
			BlendLayer.Weight = FVoxelSurfaceTypeBlendWeight::MakeUnsafe(Weight);
		}

		OutBlend.NumLayers = WriteIndex;
	}

	// Fixup precision errors
	{
		checkVoxelSlow(WeightSum <= MAX_uint16);
		checkVoxelSlow(WeightSum >= MAX_uint16 - 100);

		FVoxelSurfaceTypeBlendWeight& LastWeight = OutBlend.Layers[OutBlend.NumLayers - 1].Weight;
		LastWeight = FVoxelSurfaceTypeBlendWeight::MakeUnsafe(LastWeight.ToInt32() + MAX_uint16 - WeightSum);
	}

	if (!bSkipSortByType)
	{
		OutBlend.SortByType();
	}

	OutBlend.Check();
}

void FVoxelSurfaceTypeBlendBuilder::PopLayers()
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(Layers.Num() > FVoxelSurfaceTypeBlend::MaxLayers);

	while (Layers.Num() > FVoxelSurfaceTypeBlend::MaxLayers)
	{
		float LowestWeight = Layers[0].Weight;
		int32 LowestIndex = 0;

		for (int32 Index = 1; Index < Layers.Num(); Index++)
		{
			const FLayer& Layer = Layers[Index];
			if (Layer.Weight > LowestWeight)
			{
				continue;
			}

			LowestWeight = Layer.Weight;
			LowestIndex = Index;
		}

		checkVoxelSlow(LowestWeight > 0);

		// Move data
		for (int32 Index = LowestIndex + 1; Index < Layers.Num(); Index++)
		{
			Layers[Index - 1] = Layers[Index];
		}

		Layers.Pop();
	}

	checkVoxelSlow(Layers.Num() == FVoxelSurfaceTypeBlend::MaxLayers);
}