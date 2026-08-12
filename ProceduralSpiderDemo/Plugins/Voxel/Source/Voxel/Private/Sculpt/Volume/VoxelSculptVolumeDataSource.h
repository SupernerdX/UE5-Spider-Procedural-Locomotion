// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkPtr.h"
#include "Sculpt/Volume/VoxelSculptVolumeContext.h"

class AVoxelSculptVolume;
class FVoxelSculptVolumeLocalData;
struct FVoxelVolumeModifier;
struct FVoxelSculptVolumeData;

class IVoxelSculptVolumeDataSource
{
public:
	explicit IVoxelSculptVolumeDataSource(AVoxelSculptVolume& SculptActor);
	virtual ~IVoxelSculptVolumeDataSource() = default;

	TVoxelBulkRef<FVoxelSculptVolumeData> GetData() const;

public:
	virtual TSharedRef<IVoxelBulkLoader> GetBulkLoader() const = 0;

	virtual void SetSculptData(
		const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData,
		const TSharedRef<IVoxelBulkLoader>& NewBulkLoader) = 0;

	virtual FVoxelFuture ApplyModifier(const TSharedRef<FVoxelVolumeModifier>& Modifier) = 0;

	virtual TSharedPtr<FVoxelSculptVolumeLocalData> AsLocalData() { return {}; }

protected:
	bool IsEditor() const;
	TVoxelOptional<FVoxelSculptVolumeContext> GetContext() const;
	virtual void SetData(const TVoxelBulkRef<FVoxelSculptVolumeData>& Data);

private:
	const TVoxelObjectPtr<AVoxelSculptVolume> WeakSculptActor;
	TVoxelBulkRef<FVoxelSculptVolumeData> PrivateData;
};