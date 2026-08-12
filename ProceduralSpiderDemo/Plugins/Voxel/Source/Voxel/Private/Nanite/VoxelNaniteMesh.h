// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelChunkKey.h"
#include "VoxelSubsystemGCObject.h"

struct FVoxelRenderSubsystem;
struct FVoxelMegaMaterialRenderData;
class FVoxelMesh;
class FVoxelTexturePoolRef;
class FStaticMeshRenderData;
class FVoxelNaniteRendererMeshRef;

DECLARE_VOXEL_COUNTER(VOXEL_API, STAT_VoxelNumNaniteMeshes, "Num Nanite Meshes");
DECLARE_VOXEL_COUNTER(VOXEL_API, STAT_VoxelNumNanitePages, "Num Nanite Pages");
DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelNaniteMemory, "Nanite Memory (GPU)");

class VOXEL_API FVoxelNaniteMesh
	: public IVoxelSubsystemGCObject
	, public TSharedFromThis<FVoxelNaniteMesh>
{
public:
	const TSharedRef<FVoxelMesh> Mesh;
	const TSharedRef<const FVoxelMegaMaterialRenderData> MegaMaterialRenderData;
	const FVoxelChunkNeighborInfo NeighborInfo;

	static TVoxelFuture<TSharedPtr<FVoxelNaniteMesh>> Create(
		const FVoxelRenderSubsystem& Subsystem,
		const TSharedRef<FVoxelMesh>& Mesh,
		const TSharedRef<const FVoxelMegaMaterialRenderData>& MegaMaterialRenderData,
		const FVoxelChunkNeighborInfo& NeighborInfo);
	virtual ~FVoxelNaniteMesh() override;
	UE_NONCOPYABLE(FVoxelNaniteMesh);

	VOXEL_COUNT_INSTANCES();

	//~ Begin IVoxelSubsystemGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End IVoxelSubsystemGCObject Interface

private:
	TObjectPtr<UStaticMesh> StaticMesh;
	TVoxelObjectPtr<UStaticMesh> WeakStaticMesh;

	struct FPage
	{
		int32 Index = 0;
		int32 VertexOffset = 0;
	};
	TVoxelArray<FPage> Pages;

	VOXEL_COUNTER_HELPER(STAT_VoxelNumNaniteMeshes, NumNaniteMeshes);
	VOXEL_COUNTER_HELPER(STAT_VoxelNumNanitePages, NumNanitePages);
	VOXEL_ALLOCATED_SIZE_TRACKER_CUSTOM(STAT_VoxelNaniteMemory, NaniteMemory);

	FVoxelNaniteMesh(
		const TSharedRef<FVoxelMesh>& Mesh,
		const TSharedRef<const FVoxelMegaMaterialRenderData>& MegaMaterialRenderData,
		const FVoxelChunkNeighborInfo& NeighborInfo);

	TVoxelFuture<TSharedPtr<FVoxelNaniteMesh>> Initialize(const FVoxelRenderSubsystem& Subsystem);

	friend class UVoxelNaniteComponent;
	friend class FVoxelNaniteMaterialRenderer;
};