// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPointSet.h"
#include "VoxelStackLayer.h"
#include "VoxelMetadataRef.h"
#include "VoxelFunctionLibrary.h"
#include "VoxelScatterUtilities.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelScatterFunctionLibrary.generated.h"

namespace FVoxelGraphParameters
{
	struct VOXEL_API FPointSetChunkBounds : FUniformParameter
	{
		explicit FPointSetChunkBounds(
			const FVoxelBox& Bounds,
			const bool bCanExtendBounds);
		explicit FPointSetChunkBounds(const FPointSetChunkBounds& Other);
		void ExtendBounds(const FVoxelBox& NewBounds);
		FORCEINLINE const FVoxelBox& GetInitialBounds() const
		{
			return Bounds;
		}
		bool HasExtendedBounds() const
		{
			return ExtendedBounds.IsSet();
		}
		FVoxelBox GetBounds() const;

	private:
		FVoxelBox Bounds;
		TOptional<FVoxelBox> ExtendedBounds;
		bool bCanExtendBounds = false;
	};
}

UCLASS(Category = "Scatter")
class VOXEL_API UVoxelScatterFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(meta = (AllowList = "Scatter", ShowInPinShortList))
	FVoxelBox GetScatterBounds(bool bExtendedBounds = false) const;

	/**
	 * Generate points on a volume layer
	 * @param Layer					The layer to generate on
	 * @param DistanceBetweenPoints				
	 * @param HeightScatterType		Scatter type, when sampling height layer
	 * @param Looseness				Used with Height Grid and Volume scatter types
	 * @param MinCellArea			Increasing this will cause cells with little 'surface' inside them to be ignored. Can help prevent visible lines in point density. Used only when sampling volume layers.
	 * @param Seed					
	 * @param bQuerySurfaceTypes	
	 * @param bResolveSurfaceTypes	If false smart surface types won't be resolved
	 * @param MetadatasToQuery		
	 */
	UFUNCTION(meta = (AllowList = "Scatter, PCG", ShowInPinShortList))
	FVoxelPointSet GeneratePoints(
		const FVoxelWeakStackLayer& Layer = {},
		float DistanceBetweenPoints = 100.f,
		EVoxelHeightScatterType HeightScatterType = EVoxelHeightScatterType::Grid,
		float Looseness = 1.f,
		UPARAM(meta = (AdvancedDisplay)) float MinCellArea = 0.03f,
		const FVoxelSeed& Seed = {},
		bool bQuerySurfaceTypes = true,
		bool bResolveSurfaceTypes = false,
		UPARAM(meta = (ArrayPin)) const FVoxelMetadataRefBuffer& MetadatasToQuery = {});

	/**
	 * Generate points at the center of each visible cubic mesh face that neighbors air
	 * Does NOT generate points on faces between solid blocks with different surface types
	 * @param Layer					The layer to generate on
	 * @param VoxelSize				Size of each voxel in world units
	 * @param ChunkSize				Number of voxels per chunk side
	 * @param Seed
	 * @param bQuerySurfaceTypes
	 * @param bResolveSurfaceTypes	If false smart surface types won't be resolved
	 * @param bSkipNaNFaces			If true, skip faces where the neighbor cell is NaN (unsampled)
	 * @param MetadatasToQuery
	 */
	UFUNCTION(meta = (AllowList = "Scatter, PCG", ShowInPinShortList))
	FVoxelPointSet CubicPointGenerator(
		const FVoxelWeakStackLayer& Layer = {},
		int32 VoxelSize = 100,
		int32 ChunkSize = 32,
		const FVoxelSeed& Seed = {},
		bool bQuerySurfaceTypes = true,
		bool bResolveSurfaceTypes = false,
		UPARAM(meta = (AdvancedDisplay)) bool bSkipNaNFaces = false,
		UPARAM(meta = (ArrayPin)) const FVoxelMetadataRefBuffer& MetadatasToQuery = {});
};