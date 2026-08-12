// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkPtr.h"
#include "VoxelStampBehavior.h"
#include "VoxelHeightBlendMode.h"
#include "Sculpt/Height/VoxelHeightChunkDefinitions.h"
#include "VoxelSculptHeightData.generated.h"

class FVoxelAABBTree2D;
class IVoxelHeightModifierRuntime;
struct FVoxelHeightFarChunk;
struct FVoxelHeightMidChunk;
struct FVoxelHeightNearChunk;
struct FVoxelHeightChunkData;
struct FVoxelHeightTransform;
struct FVoxelHeightSparseQuery;
struct FVoxelSculptHeightContext;

enum class EVoxelHeightChunkQuality
{
	Far,
	Mid,
	Near
};

USTRUCT()
struct VOXEL_API FVoxelSculptHeightData final
	: public FVoxelBulkData
#if CPP
	, public FVoxelHeightChunkDefinitions
#endif
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	FVoxelSculptHeightData() = default;

	static TSharedRef<FVoxelSculptHeightData> Create(TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>> KeyToFarChunk);

	//~ Begin FVoxelBulkData Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const override;
	//~ End FVoxelBulkData Interface

public:
	FORCEINLINE bool IsEmpty() const
	{
		return KeyToFarChunk.Num() == 0;
	}
	FORCEINLINE const TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>>& GetKeyToFarChunk() const
	{
		return KeyToFarChunk;
	}
	FORCEINLINE const FVoxelBox& GetBounds() const
	{
		return PrivateBounds;
	}
	FORCEINLINE void SetLegacyChunksType(const bool bRelative)
	{
		bLegacyHeightChunksRelative = bRelative;
	}

public:
	void Apply(
		IVoxelBulkLoader& Loader,
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery,
		float ScaleXY,
		EVoxelStampBehavior Behavior,
		EVoxelHeightBlendMode BlendMode,
		bool bApplyOnVoid,
		EVoxelHeightChunkQuality Quality,
		const FVoxelBulkHash& RootHash) const;

	TVoxelFuture<TVoxelMap<FIntPoint, TSharedPtr<const FVoxelHeightNearChunk>>> LoadNearChunks(
		const TSharedRef<IVoxelBulkLoader>& Loader,
		const FVoxelIntBox2D& NearBounds,
		const FVoxelBulkHash& RootHash) const;

	TVoxelFuture<const FVoxelSculptHeightData> ApplyModifier(
		const TSharedRef<IVoxelBulkLoader>& Loader,
		const FVoxelSculptHeightContext& Context,
		const FVoxelBulkHash& RootHash,
		const TSharedRef<const IVoxelHeightModifierRuntime>& Modifier) const;

private:
	TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>> KeyToFarChunk;
	FVoxelBox PrivateBounds = FVoxelBox();

	TOptional<bool> bLegacyHeightChunksRelative;

	void ComputeBounds();
};
