// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelChunkKey.h"

int32 FVoxelChunkNeighborInfo::GetVertexLOD(
	const int32 ChunkLOD,
	const int32 ChunkSize,
	const int32 CellX,
	const int32 CellY,
	const int32 CellZ) const
{
	int32 LOD = ChunkLOD;

#define PROCESS_NEIGHBOR(NeighborX, NeighborY, NeighborZ) \
	{ \
		const int32 NeighborLOD = GetLOD(NeighborX, NeighborY, NeighborZ); \
		if (NeighborLOD > LOD) \
		{ \
			const int32 NeighborStep = 1 << (NeighborLOD - ChunkLOD - 1); \
			\
			const bool bPassesX = \
				NeighborX == 0 || \
				(NeighborX == -1 && CellX < NeighborStep) || \
				(NeighborX == +1 && CellX > ChunkSize - NeighborStep); \
			\
			const bool bPassesY = \
				NeighborY == 0 || \
				(NeighborY == -1 && CellY < NeighborStep) || \
				(NeighborY == +1 && CellY > ChunkSize - NeighborStep); \
			\
			const bool bPassesZ = \
				NeighborZ == 0 || \
				(NeighborZ == -1 && CellZ < NeighborStep) || \
				(NeighborZ == +1 && CellZ > ChunkSize - NeighborStep); \
			\
			if (bPassesX && \
				bPassesY && \
				bPassesZ) \
			{ \
				LOD = NeighborLOD; \
			} \
		} \
	}

	PROCESS_NEIGHBOR(-1, -1, -1);
	PROCESS_NEIGHBOR(+0, -1, -1);
	PROCESS_NEIGHBOR(+1, -1, -1);
	PROCESS_NEIGHBOR(-1, +0, -1);
	PROCESS_NEIGHBOR(+0, +0, -1);
	PROCESS_NEIGHBOR(+1, +0, -1);
	PROCESS_NEIGHBOR(-1, +1, -1);
	PROCESS_NEIGHBOR(+0, +1, -1);
	PROCESS_NEIGHBOR(+1, +1, -1);

	PROCESS_NEIGHBOR(-1, -1, +0);
	PROCESS_NEIGHBOR(+0, -1, +0);
	PROCESS_NEIGHBOR(+1, -1, +0);
	PROCESS_NEIGHBOR(-1, +0, +0);
	//PROCESS_NEIGHBOR(+0, +0, +0);
	PROCESS_NEIGHBOR(+1, +0, +0);
	PROCESS_NEIGHBOR(-1, +1, +0);
	PROCESS_NEIGHBOR(+0, +1, +0);
	PROCESS_NEIGHBOR(+1, +1, +0);

	PROCESS_NEIGHBOR(-1, -1, +1);
	PROCESS_NEIGHBOR(+0, -1, +1);
	PROCESS_NEIGHBOR(+1, -1, +1);
	PROCESS_NEIGHBOR(-1, +0, +1);
	PROCESS_NEIGHBOR(+0, +0, +1);
	PROCESS_NEIGHBOR(+1, +0, +1);
	PROCESS_NEIGHBOR(-1, +1, +1);
	PROCESS_NEIGHBOR(+0, +1, +1);
	PROCESS_NEIGHBOR(+1, +1, +1);

#undef PROCESS_NEIGHBOR

	return LOD;
}