// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct alignas(8) VOXEL_API FVoxelChunkKey
{
public:
	int32 LOD = 0;
	// ChunkPosition = ChunkKey * ChunkSize
	FIntVector ChunkKey = FIntVector(ForceInit);

	FVoxelChunkKey() = default;
	FVoxelChunkKey(
		const int32 LOD,
		const FIntVector& ChunkKey)
		: LOD(LOD)
		, ChunkKey(ChunkKey)
	{
	}

	FORCEINLINE FVoxelChunkKey GetChild(const int32 ChildIndex) const
	{
		checkVoxelSlow(ChunkKey % (1 << LOD) == 0);
		checkVoxelSlow(LOD > 0);

		return FVoxelChunkKey
		{
			LOD - 1,
			ChunkKey + FIntVector(
				bool(ChildIndex & 0x1) << (LOD - 1),
				bool(ChildIndex & 0x2) << (LOD - 1),
				bool(ChildIndex & 0x4) << (LOD - 1))
		};
	}
	FORCEINLINE FVoxelChunkKey GetParent() const
	{
		checkVoxelSlow(ChunkKey % (1 << LOD) == 0);

		FIntVector ParentChunkKey = ChunkKey >> LOD;
		ParentChunkKey = FVoxelUtilities::DivideFloor_FastLog2(ParentChunkKey, 1);
		ParentChunkKey = ParentChunkKey << (LOD + 1);

		checkVoxelSlow(ParentChunkKey == FVoxelUtilities::DivideFloor(ChunkKey, 1 << (LOD + 1)) * (1 << (LOD + 1)));

		return FVoxelChunkKey(LOD + 1, ParentChunkKey);
	}

	FORCEINLINE FVoxelBox GetBounds(
		const int32 RenderChunkSize,
		const int32 VoxelSize) const
	{
		return GetChunkKeyBounds().ToVoxelBox().Scale(RenderChunkSize * VoxelSize);
	}
	FORCEINLINE FVoxelIntBox GetChunkKeyBounds() const
	{
		return FVoxelIntBox(ChunkKey, ChunkKey + (1 << LOD));
	}

public:
	FORCEINLINE bool operator==(const FVoxelChunkKey& Other) const
	{
		return
			LOD == Other.LOD &&
			ChunkKey == Other.ChunkKey;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelChunkKey& Key)
	{
		struct FKey
		{
			uint64 A;
			uint64 B;
		};
		return uint32(
			FVoxelUtilities::MurmurHash64(ReinterpretCastRef<FKey>(Key).A) ^
			FVoxelUtilities::MurmurHash64(ReinterpretCastRef<FKey>(Key).B));
	}
};
checkStatic(sizeof(FVoxelChunkKey) == 16);

struct FVoxelChunkNeighborInfo
{
	TVoxelStaticArray<uint8, 27> NeighborLODs{ ForceInit };

	FORCEINLINE int32 GetLOD(
		const int32 NeighborX,
		const int32 NeighborY,
		const int32 NeighborZ) const
	{
		checkVoxelSlow(-1 <= NeighborX && NeighborX <= 1);
		checkVoxelSlow(-1 <= NeighborY && NeighborY <= 1);
		checkVoxelSlow(-1 <= NeighborZ && NeighborZ <= 1);

		return NeighborLODs[(1 + NeighborX) + (1 + NeighborY) * 3 + (1 + NeighborZ) * 3 * 3];
	}
	FORCEINLINE void SetLOD(
		const int32 NeighborX,
		const int32 NeighborY,
		const int32 NeighborZ,
		const int32 LOD)
	{
		checkVoxelSlow(-1 <= NeighborX && NeighborX <= 1);
		checkVoxelSlow(-1 <= NeighborY && NeighborY <= 1);
		checkVoxelSlow(-1 <= NeighborZ && NeighborZ <= 1);

		NeighborLODs[(1 + NeighborX) + (1 + NeighborY) * 3 + (1 + NeighborZ) * 3 * 3] = LOD;
	}

	FORCEINLINE bool operator==(const FVoxelChunkNeighborInfo& Other) const
	{
		return NeighborLODs == Other.NeighborLODs;
	}

	int32 GetVertexLOD(
		int32 ChunkLOD,
		int32 ChunkSize,
		int32 CellX,
		int32 CellY,
		int32 CellZ) const;
};