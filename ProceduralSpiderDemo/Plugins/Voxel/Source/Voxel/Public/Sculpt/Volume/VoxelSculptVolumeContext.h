// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class IVoxelVolumeSculptPreviousDistanceProvider;

struct FVoxelSculptVolumeContext
{
	const FMatrix SculptToWorld;
	const bool bStoreMovableDistances;
	const float MaxErrorPercentage;
	const TSharedRef<IVoxelVolumeSculptPreviousDistanceProvider> PreviousDistanceProvider;

	FVoxelSculptVolumeContext(
		const FMatrix& SculptToWorld,
		const bool bStoreMovableDistances,
		const float MaxErrorPercentage,
		const TSharedRef<IVoxelVolumeSculptPreviousDistanceProvider>& PreviousDistanceProvider)
		: SculptToWorld(SculptToWorld)
		, bStoreMovableDistances(bStoreMovableDistances)
		, MaxErrorPercentage(MaxErrorPercentage)
		, PreviousDistanceProvider(PreviousDistanceProvider)
	{
	}
};