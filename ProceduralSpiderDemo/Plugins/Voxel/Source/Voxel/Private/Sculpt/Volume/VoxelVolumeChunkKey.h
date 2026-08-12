// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"

struct FVoxelVolumeChunkKey : public FVoxelVolumeChunkDefinitions
{
	union
	{
		uint16 RawData = 0;

		struct
		{
			uint16 X : 3;
			uint16 Y : 3;
			uint16 Z : 3;
		};
	};

	FVoxelVolumeChunkKey() = default;

	FORCEINLINE FVoxelVolumeChunkKey(const FIntVector& Key)
	{
		checkVoxelSlow(0 <= Key.X && Key.X < ChunkSize);
		checkVoxelSlow(0 <= Key.Y && Key.Y < ChunkSize);
		checkVoxelSlow(0 <= Key.Z && Key.Z < ChunkSize);

		RawData = 0;
		X = Key.X;
		Y = Key.Y;
		Z = Key.Z;
	}

	FORCEINLINE FIntVector ToVector() const
	{
		return FIntVector(X, Y, Z);
	}
	FORCEINLINE int32 ToIndex() const
	{
		checkVoxelSlow(RawData == FVoxelUtilities::Get3DIndex<int32>(ChunkSize, ToVector()));
		return int32(RawData);
	}

	FORCEINLINE bool operator<(const FVoxelVolumeChunkKey& Other) const
	{
		return RawData < Other.RawData;
	}
	FORCEINLINE bool operator==(const FVoxelVolumeChunkKey& Other) const
	{
		return RawData == Other.RawData;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FVoxelVolumeChunkKey& Key)
	{
		return Ar << Key.RawData;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelVolumeChunkKey& Key)
	{
		return FVoxelUtilities::MurmurHash32(Key.RawData);
	}
};