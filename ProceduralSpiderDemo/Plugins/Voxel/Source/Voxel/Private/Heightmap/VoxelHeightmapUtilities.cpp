// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Heightmap/VoxelHeightmapUtilities.h"
#include "Surface/VoxelSurfaceTypeBlendBuilder.h"

TVoxelInlineArray<FVoxelHeightmapUtilities::FWeightData, 16> FVoxelHeightmapUtilities::BuildWeightmaps(
	const TVoxelArray<FWeightmap>& Weightmaps,
	const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery,
	const int32 Num,
	const bool bComputeSurfaceTypes,
	const bool bComputeMetadatas,
	const TVoxelFunctionRef<void(const FWeightmap&, FWeightData&)> OnSampleWeightmap)
{
	if (!bComputeSurfaceTypes &&
		!bComputeMetadatas)
	{
		return {};
	}

	TVoxelInlineArray<FWeightData, 16> WeightDatas;
	WeightDatas.Reserve(Weightmaps.Num());

	for (const FWeightmap& Weightmap : Weightmaps)
	{
		const bool bShouldCompute = INLINE_LAMBDA
		{
			if (bComputeSurfaceTypes &&
				!Weightmap.SurfaceType.IsNull())
			{
				return true;
			}

			if (bComputeMetadatas &&
				MetadatasToQuery.Contains(Weightmap.MetadataRef))
			{
				return true;
			}

			return false;
		};

		if (!bShouldCompute)
		{
			continue;
		}

		FWeightData& WeightData = WeightDatas.Emplace_GetRef_EnsureNoGrow();
		WeightData.Weightmap = &Weightmap;
		WeightData.Weights.Allocate(Num);

		VOXEL_SCOPE_COUNTER("SampleWeightmap");

		OnSampleWeightmap(Weightmap, WeightData);
	}

	return WeightDatas;
}

void FVoxelHeightmapUtilities::ComputeSurfaceTypes(
	const TVoxelInlineArray<FWeightData, 16>& WeightDatas,
	const FVoxelSurfaceType& DefaultSurfaceType,
	const FVoxelFloatBuffer& Alphas,
	const int32 Num,
	const TVoxelFunctionRef<void(int32, const FVoxelSurfaceTypeBlend&, float)> OnSetSurfaceType)
{
	VOXEL_FUNCTION_COUNTER();

	struct FBlend
	{
		FVoxelSurfaceType SurfaceType;
		TVoxelInlineArray<TConstVoxelArrayView<float>, 1> AllWeights;
	};
	TVoxelInlineArray<FBlend, 16> WeightBlends;
	TVoxelInlineArray<FBlend, 16> AlphaBlends;

	{
		VOXEL_SCOPE_COUNTER("Setup blends");

		WeightBlends.Reserve(WeightDatas.Num());
		AlphaBlends.Reserve(WeightDatas.Num());

		for (const FWeightData& WeightData : WeightDatas)
		{
			const FVoxelSurfaceType SurfaceType = WeightData.Weightmap->SurfaceType;
			if (SurfaceType.IsNull())
			{
				continue;
			}

			TVoxelInlineArray<FBlend, 16>& Blends =
				WeightData.Weightmap->Type == EVoxelHeightmapWeightType::AlphaBlended
				? AlphaBlends
				: WeightBlends;

			const bool bAdded = INLINE_LAMBDA
			{
				for (FBlend& Blend : Blends)
				{
					if (Blend.SurfaceType == SurfaceType)
					{
						Blend.AllWeights.Add(WeightData.Weights.View());
						return true;
					}
				}

				return false;
			};
			if (bAdded)
			{
				continue;
			}

			FBlend& Blend = Blends.Emplace_GetRef_EnsureNoGrow(FBlend
			{
				WeightData.Weightmap->SurfaceType,
			});

			Blend.AllWeights.Add_EnsureNoGrow(WeightData.Weights.View());
		}
	}

	FVoxelSurfaceTypeBlendBuilder Builder;

	for (int32 Index = 0; Index < Num; Index++)
	{
		const float Alpha = Alphas[Index];
		if (Alpha == 0.f)
		{
			continue;
		}
		checkVoxelSlow(FVoxelUtilities::IsFinite(Alpha));
		checkVoxelSlow(0.f <= Alpha && Alpha <= 1.f);

		FVoxelSurfaceTypeBlend SurfaceType;

		// Process weight-blended weightmaps
		INLINE_LAMBDA
		{
			if (WeightBlends.Num() == 0)
			{
				return;
			}

			Builder.Reset();

			for (const FBlend& Blend : WeightBlends)
			{
				float Weight = Blend.AllWeights[0][Index];
				if (Blend.AllWeights.Num() > 1)
				{
					for (int32 WeightmapIndex = 1; WeightmapIndex < Blend.AllWeights.Num(); WeightmapIndex++)
					{
						Weight += Blend.AllWeights[WeightmapIndex][Index];
					}
				}
				checkVoxelSlow(FVoxelUtilities::IsFinite(Weight));

				if (Weight == 0.f)
				{
					continue;
				}

				Builder.AddLayer_CheckNew(Blend.SurfaceType, Weight);
			}

			Builder.Build(SurfaceType);
		};

		if (SurfaceType.IsNull())
		{
			SurfaceType.InitializeFromType(DefaultSurfaceType);
		}

		// Process alpha-blended weightmaps
		for (const FBlend& Blend : AlphaBlends)
		{
			float Weight = Blend.AllWeights[0][Index];
			if (Blend.AllWeights.Num() > 1)
			{
				for (int32 WeightmapIndex = 1; WeightmapIndex < Blend.AllWeights.Num(); WeightmapIndex++)
				{
					Weight += Blend.AllWeights[WeightmapIndex][Index];
				}
			}
			checkVoxelSlow(FVoxelUtilities::IsFinite(Weight));

			if (Weight == 0.f)
			{
				continue;
			}

			FVoxelSurfaceTypeBlend::Lerp(
				SurfaceType,
				SurfaceType,
				Blend.SurfaceType,
				FMath::Clamp(Weight, 0.f, 1.f));
		}

		OnSetSurfaceType(Index, SurfaceType, Alpha);
	}
}