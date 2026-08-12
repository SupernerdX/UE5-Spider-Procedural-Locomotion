// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkPtr.h"
#include "VoxelStampBehavior.h"
#include "VoxelVolumeBlendMode.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"
#include "VoxelSculptVolumeData.generated.h"

class FVoxelAABBTree;
class IVoxelVolumeModifierRuntime;
struct FVoxelVolumeFarChunk;
struct FVoxelVolumeMidChunk;
struct FVoxelVolumeNearChunk;
struct FVoxelVolumeChunkData;
struct FVoxelVolumeTransform;
struct FVoxelVolumeSparseQuery;
struct FVoxelSculptVolumeContext;

enum class EVoxelVolumeChunkQuality
{
	Far,
	Mid,
	Near
};

USTRUCT()
struct VOXEL_API FVoxelSculptVolumeData final
	: public FVoxelBulkData
#if CPP
	, public FVoxelVolumeChunkDefinitions
#endif
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	FVoxelSculptVolumeData() = default;

	static TSharedRef<FVoxelSculptVolumeData> Create(TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>> KeyToFarChunk);

	//~ Begin FVoxelBulkData Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const override;
	//~ End FVoxelBulkData Interface

public:
	FORCEINLINE bool IsEmpty() const
	{
		return KeyToFarChunk.Num() == 0;
	}
	FORCEINLINE const TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>>& GetKeyToFarChunk() const
	{
		return KeyToFarChunk;
	}
	FORCEINLINE const FVoxelIntBox& GetBounds() const
	{
		return PrivateBounds;
	}

public:
	void Apply(
		IVoxelBulkLoader& Loader,
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery,
		float Scale,
		EVoxelStampBehavior Behavior,
		EVoxelVolumeBlendMode BlendMode,
		bool bApplyOnVoid,
		EVoxelVolumeChunkQuality Quality,
		const FVoxelBulkHash& RootHash) const;

	TVoxelFuture<TVoxelMap<FIntVector, TSharedPtr<const FVoxelVolumeNearChunk>>> LoadNearChunks(
		const TSharedRef<IVoxelBulkLoader>& Loader,
		const FVoxelIntBox& NearBounds,
		const FVoxelBulkHash& RootHash) const;

	TVoxelFuture<const FVoxelSculptVolumeData> ApplyModifier(
		const TSharedRef<IVoxelBulkLoader>& Loader,
		const FVoxelSculptVolumeContext& Context,
		const FVoxelBulkHash& RootHash,
		const TSharedRef<const IVoxelVolumeModifierRuntime>& Modifier) const;

private:
	TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>> KeyToFarChunk;
	FVoxelIntBox PrivateBounds;

	void ComputeBounds();
};