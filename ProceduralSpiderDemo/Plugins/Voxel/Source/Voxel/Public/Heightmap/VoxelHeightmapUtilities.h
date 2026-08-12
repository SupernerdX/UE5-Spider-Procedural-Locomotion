// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFloatMetadataRef.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Heightmap/VoxelHeightmap_Weight.h"
#include "Surface/VoxelSurfaceType.h"

struct FVoxelSurfaceTypeBlend;

struct VOXEL_API FVoxelHeightmapUtilities
{
	struct FWeightmap
	{
		FVoxelSurfaceType SurfaceType;
		EVoxelHeightmapWeightType Type;
		float Strength = 0;
		FVoxelFloatMetadataRef MetadataRef;
		bool bUseBicubic = true;
		TSharedPtr<const FVoxelHeightmap_WeightData> Data;
	};

	struct FWeightData
	{
		const FWeightmap* Weightmap;
		FVoxelFloatBuffer Weights;
	};

	static TVoxelInlineArray<FWeightData, 16> BuildWeightmaps(
		const TVoxelArray<FWeightmap>& Weightmaps,
		const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery,
		int32 Num,
		bool bComputeSurfaceTypes,
		bool bComputeMetadatas,
		TVoxelFunctionRef<void(const FWeightmap&, FWeightData&)> OnSampleWeightmap);

	static void ComputeSurfaceTypes(
		const TVoxelInlineArray<FWeightData, 16>& WeightDatas,
		const FVoxelSurfaceType& DefaultSurfaceType,
		const FVoxelFloatBuffer& Alphas,
		int32 Num,
		TVoxelFunctionRef<void(int32, const FVoxelSurfaceTypeBlend&, float)> OnSetSurfaceType);
};