// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "Surface/VoxelSurfaceTypeBlend.h"
#include "Sculpt/Height/VoxelHeightChunkDefinitions.h"

struct FVoxelBulkHash;
class IVoxelBulkLoader;
class IVoxelHeightSculptPreviousHeightProvider;
struct FVoxelHeightNearChunk;
struct FVoxelSculptHeightData;
struct FVoxelHeightHeightChunk;
struct FVoxelSculptHeightContext;
struct FVoxelHeightSurfaceTypeChunk;
struct FVoxelHeightSculptPreviousChunk;

class VOXEL_API FVoxelHeightSculptCanvas : public FVoxelHeightChunkDefinitions
{
public:
	static TVoxelFuture<FVoxelHeightSculptCanvas> CreateCanvas(
		const TSharedRef<IVoxelBulkLoader>& BulkLoader,
		const TSharedRef<const FVoxelSculptHeightData>& SculptData,
		const FVoxelSculptHeightContext& Context,
		// TRICKY: Only chunks within ChunkKeyBounds.Extend(-1) will be written to
		// We keep a one chunk border around for jump flooding
		const FVoxelIntBox2D& ChunkKeyBounds,
		const FVoxelBulkHash& RootHash);

	FVoxelHeightSculptCanvas(
		const FVoxelIntBox2D& ChunkKeyBounds,
		const TSharedRef<const FVoxelSculptHeightData>& PreviousSculptData,
		const FVoxelSculptHeightContext& Context,
		const TVoxelMap<FIntPoint, TSharedPtr<const FVoxelHeightNearChunk>>& KeyToPreviousChunk);

	TSharedRef<const FVoxelSculptHeightData> Finalize(float TargetPrecision);

private:
	struct FSurfaceTypes
	{
		TVoxelArray<float> Alphas;
		TVoxelArray<FVoxelSurfaceTypeBlend> Blends;
	};
	struct FMetadata
	{
		TVoxelArray<float> Alphas;
		TSharedPtr<FVoxelBuffer> Buffer;
	};

private:
	const FIntPoint SizeInChunks;
	const FIntPoint Size;
	const int32 NumVoxels;
	const FIntPoint ChunkKeyOffset;

	const TSharedRef<const FVoxelSculptHeightData> PreviousSculptData;
	const TSharedRef<IVoxelHeightSculptPreviousHeightProvider> PreviousHeightProvider;
	const TVoxelMap<FIntPoint, TSharedPtr<const FVoxelHeightNearChunk>> KeyToPreviousChunk;

	const float ScaleZ;
	const float OffsetZ;
	const bool bRelativeHeights;

	bool bWriteHeights = false;
	bool bWriteSurfaceTypes = false;
	TVoxelArray<FVoxelMetadataRef> MetadataRefsToWrite;

	TVoxelArray<float> Heights;
	TVoxelOptional<FSurfaceTypes> SurfaceTypes;
	TVoxelMap<FVoxelMetadataRef, FMetadata> MetadataRefToMetadata;

	void QueryChunkHeights(const FIntPoint& ChunkKey);
	TVoxelArray<float> QueryPreviousHeights() const;
	void QuerySurfaceTypesMetadatas();

	friend class FVoxelHeightSculptCanvasWriter;
};
