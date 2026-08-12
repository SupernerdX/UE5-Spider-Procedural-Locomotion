// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSubsystem.h"
#include "MegaMaterial/VoxelMegaMaterialTarget.h"
#include "VoxelRenderSubsystem.generated.h"

class FVoxelRenderTree;
class FVoxelRenderChunk;
class FVoxelTextureManager;
class FVoxelRenderInvokersView;
class FVoxelNaniteMaterialRenderer;
struct FVoxelRenderChunkData;
struct FVoxelChunkNeighborInfo;

USTRUCT()
struct VOXEL_API FVoxelRenderSubsystem : public FVoxelSubsystem
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_BODY()

public:
	bool HasChunksToSubdivide() const
	{
		return bHasChunksToSubdivide;
	}
	const FVoxelOptionalBox& GetBoundsToGenerate() const
	{
		return BoundsToGenerate;
	}
	const FVoxelTextureManager& GetTextureManager() const
	{
		return *TextureManager;
	}
	FVoxelNaniteMaterialRenderer& GetNaniteMaterialRenderer() const
	{
		return *NaniteMaterialRenderer;
	}
	TSharedRef<FVoxelMaterialInstanceRef> GetMaterialInstanceRef(EVoxelMegaMaterialTarget Target) const;

	//~ Begin FVoxelSubsystem Interface
	virtual bool ShouldCreateOnServer() const override { return false; }
	virtual void LoadFromPrevious(FVoxelSubsystem& InPreviousSubsystem) override;
	virtual void Initialize() override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual void Compute() override;
	virtual void Render(FVoxelRuntime& Runtime) override;
	//~ End FVoxelSubsystem Interface

private:
	FVoxelOptionalBox BoundsToGenerate;

	bool bHasChunksToSubdivide = false;
	TSharedPtr<FVoxelRenderTree> RenderTree;

	TSharedPtr<FVoxelDependencyTracker> BoundsToGenerateDependencyTracker;

	TVoxelStaticArray<TSharedPtr<FVoxelMaterialInstanceRef>, int32(EVoxelMegaMaterialTarget::Max)> MaterialInstanceRefs;

	// Keep PreviousTextureManager & PreviousNaniteMaterialRenderer alive until we've rendered
	TSharedPtr<FVoxelTextureManager> TextureManager;
	TSharedPtr<FVoxelTextureManager> PreviousTextureManager;
	TSharedPtr<FVoxelNaniteMaterialRenderer> NaniteMaterialRenderer;
	TSharedPtr<FVoxelNaniteMaterialRenderer> PreviousNaniteMaterialRenderer;
	TSharedPtr<FVoxelRenderInvokersView> InvokerView;
	TSharedPtr<FVoxelDependencyTracker> InvokersDependencyTracker;

private:
	TVoxelChunkedArray<TSharedPtr<FVoxelRenderChunkData>> RenderDatasToDestroy;
	TVoxelChunkedArray<TSharedPtr<FVoxelRenderChunkData>> RenderDatasToRender;

	FORCEINLINE void DestroyRenderData(TSharedPtr<FVoxelRenderChunkData>& RenderData)
	{
		if (RenderData)
		{
			RenderDatasToDestroy.Add(RenderData);
			RenderData.Reset();
		}
	}

	friend class FVoxelRenderTree;

private:
	bool TryInitializeRootChunks();

private:
	FVoxelFuture StartMeshingTasks();
	FVoxelFuture StartRenderTasks();
	void FinalizeRender_Nanite() const;

private:
	FVoxelFuture ProcessChunk(
		const TSharedRef<FVoxelRenderChunk>& Chunk,
		const FVoxelChunkNeighborInfo& NeighborInfo) const;
};