// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBuffer.h"
#include "VoxelMetadataRef.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelVolumeMetadata_Memory, "Voxel Volume Metadata Memory");

struct FVoxelVolumeMetadataChunk
	: FVoxelVolumeChunkDefinitions
	, TVoxelRefCountThis<FVoxelVolumeMetadataChunk>
{
public:
	TVoxelStaticArray<uint8, ChunkCount> Alphas{ NoInit };
	TSharedPtr<FVoxelBuffer> Buffer;

	FVoxelVolumeMetadataChunk();
	UE_NONCOPYABLE(FVoxelVolumeMetadataChunk)

	VOXEL_COUNT_INSTANCES();
	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelVolumeMetadata_Memory);

public:
	FORCEINLINE float GetAlpha(const int32 Index) const
	{
		return FVoxelUtilities::UINT8ToFloat(Alphas[Index]);
	}
	template<typename Type>
	FORCEINLINE Type GetValue(const int32 Index) const
	{
		checkVoxelSlow(Buffer.IsValid());
		return Buffer.Get()->AsChecked<TVoxelBufferType<Type>>()[Index];
	}

public:
	template<typename LambdaType>
	static void SwitchType(
		const FVoxelMetadataRef& MetadataRef,
		LambdaType Lambda)
	{
		if (MetadataRef.GetInnerType().Is<float>())
		{
			(void)sizeof(FVoxelFloatBuffer);
			Lambda.template operator()<float>();
		}
		else if (MetadataRef.GetInnerType().Is<FLinearColor>())
		{
			(void)sizeof(FVoxelLinearColorBuffer);
			Lambda.template operator()<FLinearColor>();
		}
		else
		{
			ensureVoxelSlow(false);
			VOXEL_MESSAGE(Error, "Unsupport metadata type for sculpting: {0}", MetadataRef.GetInnerType().ToString());
		}
	}

public:
	void Serialize(
		FArchive& Ar,
		int32 Version);

	void GetAverageValue(
		const FVoxelMetadataRef& MetadataRef,
		float& OutAlpha,
		FVoxelBuffer& OutBuffer,
		int32 IndexInBuffer) const;

	int64 GetAllocatedSize() const;
	TVoxelRefCountPtr<FVoxelVolumeMetadataChunk> Clone() const;

public:
	static TVoxelRefCountPtr<FVoxelVolumeMetadataChunk> Create(
		TConstVoxelArrayView<float> Alphas,
		const FVoxelBuffer& Buffer,
		const FIntVector& Offset,
		const FIntVector& Size);
};