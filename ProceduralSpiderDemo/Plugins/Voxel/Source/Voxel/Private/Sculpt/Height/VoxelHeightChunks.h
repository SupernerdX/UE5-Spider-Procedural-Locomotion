// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkPtr.h"
#include "Bulk/VoxelBulkHint.h"
#include "Sculpt/Height/VoxelHeightChunkKey.h"
#include "Sculpt/Height/VoxelHeightChunkData.h"
#include "VoxelHeightChunks.generated.h"

USTRUCT()
struct FVoxelHeightNearChunkHint final : public FVoxelBulkHintData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	FVoxelBulkHash RootHash;
	FIntPoint FarChunkKey;
	FIntPoint MidChunkKey;
	FIntPoint NearChunkKey;

	//~ Begin FVoxelBulkHintData Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelBulkHintData Interface
};

USTRUCT()
struct FVoxelHeightNearChunk final : public FVoxelBulkData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	FVoxelHeightChunkData ChunkData;

	//~ Begin FVoxelBulkData Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const override;
	//~ End FVoxelBulkData Interface
};

USTRUCT()
struct FVoxelHeightMidChunkHint final : public FVoxelBulkHintData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	FVoxelBulkHash RootHash;
	FIntPoint FarChunkKey;
	FIntPoint MidChunkKey;

	//~ Begin FVoxelBulkHintData Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelBulkHintData Interface
};

USTRUCT()
struct FVoxelHeightMidChunk final : public FVoxelBulkData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	FVoxelHeightChunkData ChunkData;
	TVoxelMap<FVoxelHeightChunkKey, TVoxelBulkPtr<FVoxelHeightNearChunk>> KeyToNearChunk;

	static TVoxelBulkRef<FVoxelHeightMidChunk> Create(
		TVoxelMap<FVoxelHeightChunkKey, TVoxelBulkPtr<FVoxelHeightNearChunk>> KeyToNearChunk,
		float TargetPrecision);

	//~ Begin FVoxelBulkData Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const override;
	//~ End FVoxelBulkData Interface
};

USTRUCT()
struct FVoxelHeightFarChunkHint final : public FVoxelBulkHintData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	FVoxelBulkHash RootHash;
	FIntPoint FarChunkKey;

	//~ Begin FVoxelBulkHintData Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelBulkHintData Interface
};

USTRUCT()
struct FVoxelHeightFarChunk final : public FVoxelBulkData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	FVoxelHeightChunkData ChunkData;
	TVoxelMap<FVoxelHeightChunkKey, TVoxelBulkPtr<FVoxelHeightMidChunk>> KeyToMidChunk;

	static TVoxelBulkRef<FVoxelHeightFarChunk> Create(
		TVoxelMap<FVoxelHeightChunkKey, TVoxelBulkPtr<FVoxelHeightMidChunk>> KeyToMidChunk,
		float TargetPrecision);

	//~ Begin FVoxelBulkData Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const override;
	//~ End FVoxelBulkData Interface
};