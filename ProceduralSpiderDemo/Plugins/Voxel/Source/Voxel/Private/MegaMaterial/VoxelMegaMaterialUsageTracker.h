// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Surface/VoxelSurfaceType.h"

class UVoxelMegaMaterial;
class UVoxelSurfaceTypeInterface;

#if WITH_EDITOR
class FVoxelMegaMaterialUsageTracker : public TSharedFromThis<FVoxelMegaMaterialUsageTracker>
{
public:
	const TVoxelObjectPtr<UVoxelMegaMaterial> WeakMegaMaterial;
	TVoxelMap<TVoxelObjectPtr<UVoxelSurfaceTypeInterface>, TWeakPtr<FVoxelNotification>> SurfaceTypeToWeakNotification;

	explicit FVoxelMegaMaterialUsageTracker(const TVoxelObjectPtr<UVoxelMegaMaterial>& WeakMegaMaterial)
		: WeakMegaMaterial(WeakMegaMaterial)
	{
	}

public:
	FVoxelCriticalSection CriticalSection;
	TVoxelSet<FVoxelSurfaceType> SurfaceTypesToAdd_RequiresLock;

	void Sync();
	void NotifyMissingSurfaceType(FVoxelSurfaceType SurfaceType);
};
#endif