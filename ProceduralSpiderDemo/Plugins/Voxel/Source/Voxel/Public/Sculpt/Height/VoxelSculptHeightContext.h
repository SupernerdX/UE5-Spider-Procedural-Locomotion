// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class FVoxelLayers;
class FVoxelSurfaceTypeTable;
class FVoxelSculptHeightCache;
class IVoxelHeightSculptPreviousHeightProvider;

struct FVoxelSculptHeightContext
{
	const FTransform2d SculptToWorld;
	const float ScaleZ;
	const float OffsetZ;
	const bool bStoreRelativeHeights;
	const TSharedRef<IVoxelHeightSculptPreviousHeightProvider> PreviousHeightProvider;
	const float TargetPrecision;

	FVoxelSculptHeightContext(
		const FTransform2d& SculptToWorld,
		const float ScaleZ,
		const float OffsetZ,
		const bool bStoreRelativeHeights,
		const TSharedRef<IVoxelHeightSculptPreviousHeightProvider>& PreviousHeightProvider,
		const float TargetPrecision = 0.f)
		: SculptToWorld(SculptToWorld)
		, ScaleZ(ScaleZ)
		, OffsetZ(OffsetZ)
		, bStoreRelativeHeights(bStoreRelativeHeights)
		, PreviousHeightProvider(PreviousHeightProvider)
		, TargetPrecision(TargetPrecision)
	{
	}
};
