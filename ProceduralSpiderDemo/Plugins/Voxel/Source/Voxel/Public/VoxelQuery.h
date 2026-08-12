// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelMetadataRef.h"
#include "VoxelStampBehavior.h"
#include "Buffer/VoxelDoubleBuffers.h"

class UVoxelLayer;
class FVoxelLayers;
class FVoxelSurfaceTypeTable;
struct FVoxelBreadcrumbs;
struct FVoxelStampRuntime;
struct FVoxelSurfaceTypeBlend;
struct FVoxelHeightBulkQuery;
struct FVoxelVolumeBulkQuery;
struct FVoxelHeightSparseQuery;
struct FVoxelVolumeSparseQuery;

class VOXEL_API FVoxelQuery
{
public:
	const int32 LOD;
	const FVoxelLayers& Layers;
	const FVoxelSurfaceTypeTable& SurfaceTypeTable;
	FVoxelDependencyCollector& DependencyCollector;
	FVoxelBreadcrumbs* const Breadcrumbs;

	// Used by sculpting
	const TVoxelUniqueFunction<bool(const FVoxelStampRuntime& NextStamp)>* ShouldStopTraversal_BeforeStamp = nullptr;
	const TVoxelUniqueFunction<bool(const FVoxelStampRuntime& LastStamp)>* ShouldStopTraversal_AfterStamp = nullptr;

	FVoxelQuery(
		const int32 LOD,
		const FVoxelLayers& Layers,
		const FVoxelSurfaceTypeTable& SurfaceTypeTable,
		FVoxelDependencyCollector& DependencyCollector,
		FVoxelBreadcrumbs* Breadcrumbs = nullptr)
		: LOD(LOD)
		, Layers(Layers)
		, SurfaceTypeTable(SurfaceTypeTable)
		, DependencyCollector(DependencyCollector)
		, Breadcrumbs(Breadcrumbs)
	{
	}

public:
	bool HasQueriedLayer(const FVoxelWeakStackLayer& WeakLayer) const
	{
		return QueriedLayers.Contains(WeakLayer);
	}
	TConstVoxelArrayView<FVoxelWeakStackLayer> GetQueriedLayers() const
	{
		return QueriedLayers;
	}

	bool CheckNoRecursion(const FVoxelWeakStackLayer& WeakLayer) const;

	FVoxelQuery MakeChild_Layer(const FVoxelWeakStackLayer& WeakLayer) const;

public:
	bool HasStamps(
		const FVoxelWeakStackLayer& WeakLayer,
		const FVoxelBox& Bounds,
		EVoxelStampBehavior BehaviorMask) const;

	TVoxelOptional<FVoxelWeakStackLayer> GetFirstHeightLayer(const FVoxelWeakStackLayer& WeakLayer) const;

public:
	FVoxelFloatBuffer SampleHeightLayer(
		const FVoxelWeakStackLayer& WeakLayer,
		const FVector2D& Start,
		const FIntPoint& Size,
		float Step) const;

	FVoxelFloatBuffer SampleHeightLayer(
		const FVoxelWeakStackLayer& WeakLayer,
		const FVoxelDoubleVector2DBuffer& Positions,
		TVoxelArrayView<FVoxelSurfaceTypeBlend> OutSurfaceTypes,
		const TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>>& OutMetadataToBuffer) const;

	FVoxelFloatBuffer SampleHeightLayer(
		const FVoxelWeakStackLayer& WeakLayer,
		const FVoxelDoubleVector2DBuffer& Positions) const;

public:
	FVoxelFloatBuffer SampleVolumeLayer(
		const FVoxelWeakStackLayer& WeakLayer,
		const FVector& Start,
		const FIntVector& Size,
		float Step) const;

	FVoxelFloatBuffer SampleVolumeLayer(
		const FVoxelWeakStackLayer& WeakLayer,
		const FVoxelDoubleVectorBuffer& Positions,
		TVoxelArrayView<FVoxelSurfaceTypeBlend> OutSurfaceTypes,
		const TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>>& OutMetadataToBuffer) const;

	FVoxelFloatBuffer SampleVolumeLayer(
		const FVoxelWeakStackLayer& WeakLayer,
		const FVoxelDoubleVectorBuffer& Positions) const;

private:
	TVoxelArray<FVoxelWeakStackLayer> QueriedLayers;
};