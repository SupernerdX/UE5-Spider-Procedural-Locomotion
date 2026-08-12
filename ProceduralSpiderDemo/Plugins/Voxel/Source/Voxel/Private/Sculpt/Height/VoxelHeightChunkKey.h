// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Height/VoxelHeightChunkDefinitions.h"

struct FVoxelHeightChunkKey : public FVoxelHeightChunkDefinitions
{
	union
	{
		uint8 RawData = 0;

		struct
		{
			uint8 X : 4;
			uint8 Y : 4;
		};
	};

	FVoxelHeightChunkKey() = default;

	FORCEINLINE FVoxelHeightChunkKey(const FIntPoint& Key)
	{
		checkVoxelSlow(0 <= Key.X && Key.X < ChunkSize);
		checkVoxelSlow(0 <= Key.Y && Key.Y < ChunkSize);

		RawData = 0;
		X = Key.X;
		Y = Key.Y;
	}

	FORCEINLINE FIntPoint ToPoint() const
	{
		return FIntPoint(X, Y);
	}
	FORCEINLINE int32 ToIndex() const
	{
		checkVoxelSlow(RawData == FVoxelUtilities::Get2DIndex<int32>(ChunkSize, ToPoint()));
		return int32(RawData);
	}

	FORCEINLINE bool operator<(const FVoxelHeightChunkKey& Other) const
	{
		return RawData < Other.RawData;
	}
	FORCEINLINE bool operator==(const FVoxelHeightChunkKey& Other) const
	{
		return RawData == Other.RawData;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FVoxelHeightChunkKey& Key)
	{
		return Ar << Key.RawData;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelHeightChunkKey& Key)
	{
		return FVoxelUtilities::MurmurHash32(Key.RawData);
	}
};