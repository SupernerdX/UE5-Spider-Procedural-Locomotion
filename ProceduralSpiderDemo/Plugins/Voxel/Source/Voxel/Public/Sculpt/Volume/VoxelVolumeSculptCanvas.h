// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "Surface/VoxelSurfaceTypeBlend.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"

class IVoxelBulkLoader;
class IVoxelVolumeSculptPreviousDistanceProvider;
struct FVoxelBulkHash;
struct FVoxelVolumeNearChunk;
struct FVoxelSculptVolumeData;
struct FVoxelVolumeDistanceChunk;
struct FVoxelVolumeSculptPreviousChunk;

class VOXEL_API FVoxelVolumeSculptCanvas : public FVoxelVolumeChunkDefinitions
{
public:
	static TVoxelFuture<FVoxelVolumeSculptCanvas> CreateCanvas(
		const TSharedRef<IVoxelBulkLoader>& BulkLoader,
		const TSharedRef<const FVoxelSculptVolumeData>& SculptData,
		const TSharedRef<IVoxelVolumeSculptPreviousDistanceProvider>& PreviousDistanceProvider,
		// TRICKY: Only chunks within ChunkKeyBounds.Extend(-1) will be written to
		// We keep a one chunk border around for jump flooding
		const FVoxelIntBox& ChunkKeyBounds,
		const FVoxelBulkHash& RootHash);

	FVoxelVolumeSculptCanvas(
		const FVoxelIntBox& ChunkKeyBounds,
		const TSharedRef<const FVoxelSculptVolumeData>& PreviousSculptData,
		const TSharedRef<IVoxelVolumeSculptPreviousDistanceProvider>& PreviousDistanceProvider,
		const TVoxelMap<FIntVector, TSharedPtr<const FVoxelVolumeNearChunk>>& KeyToPreviousChunk);

	TSharedRef<const FVoxelSculptVolumeData> Finalize(
		bool bStoreMovableDistances,
		float MaxErrorPercentage);

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
	const FIntVector SizeInChunks;
	const FIntVector Size;
	const int32 NumVoxels;
	const FIntVector ChunkKeyOffset;

	const TSharedRef<const FVoxelSculptVolumeData> PreviousSculptData;
	const TSharedRef<IVoxelVolumeSculptPreviousDistanceProvider> PreviousDistanceProvider;
	const TVoxelMap<FIntVector, TSharedPtr<const FVoxelVolumeNearChunk>> KeyToPreviousChunk;

	bool bWriteDistances = false;
	bool bWriteSurfaceTypes = false;
	TVoxelArray<FVoxelMetadataRef> MetadataRefsToWrite;

	TVoxelArray<float> Distances;
	TVoxelOptional<FSurfaceTypes> SurfaceTypes;
	TVoxelMap<FVoxelMetadataRef, FMetadata> MetadataRefToMetadata;

	void Propagate(
		TConstVoxelArrayView<float> ClosestX,
		TConstVoxelArrayView<float> ClosestY,
		TConstVoxelArrayView<float> ClosestZ);

	void QueryChunkDistances(const FIntVector& ChunkKey);
	TVoxelArray<float> QueryPreviousDistances() const;

	friend class FVoxelVolumeSculptCanvasWriter;
};