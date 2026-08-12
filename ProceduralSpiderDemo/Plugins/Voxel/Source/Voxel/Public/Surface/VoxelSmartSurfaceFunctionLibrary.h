// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelFunctionLibrary.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "VoxelSmartSurfaceFunctionLibrary.generated.h"

namespace FVoxelGraphParameters
{
	struct VOXEL_API FSmartSurfaceUniform : FUniformParameter
	{
		FVoxelWeakStackLayer WeakLayer;
	};
	struct VOXEL_API FSmartSurface : FBufferParameter
	{
		FVoxelVectorBuffer Normals;

		void Split(
			const FVoxelBufferSplitter& Splitter,
			TConstVoxelArrayView<FSmartSurface*> OutResult) const;
	};
}

UCLASS()
class VOXEL_API UVoxelSmartSurfaceFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Surface Type", meta = (AllowList = "SmartSurfaceType"))
	FVoxelVectorBuffer GetVertexNormal() const;

	/**
	 * Resolve smart surface types to their final surface types
	 * @param SurfaceTypes	The surface types to resolve
	 * @param Position		The position to feed into the smart surface graphs
	 * @param Normal		The normal to feed into the smart surface graphs. This is queried using GetVertexNormal.
	 * @param LOD			The LOD to pass to smart surface graphs
	 * @param Layer			Layer to pass to smart surface graphs, used in Sample Previous Stamps
	 */
	UFUNCTION(Category = "Surface Type")
	FVoxelSurfaceTypeBlendBuffer ResolveSmartSurfaces(
		const FVoxelSurfaceTypeBlendBuffer& SurfaceTypes,
		UPARAM(meta = (PositionPin)) const FVoxelDoubleVectorBuffer& Position,
		const FVoxelVectorBuffer& Normal = FVector3f::UpVector,
		int32 LOD = 0,
		const FVoxelWeakStackLayer& Layer = {}) const;
};