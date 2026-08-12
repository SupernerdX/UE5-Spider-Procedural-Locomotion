// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelStampRef;
class FVoxelStampManager;

struct VOXEL_API FVoxelStampIndex
{
public:
	FVoxelStampIndex() = default;

	FORCEINLINE bool IsValid() const
	{
		if (SerialNumber == 0)
		{
			checkVoxelSlow(Index == 0);
		}
		checkVoxelSlow((SerialNumber == 0) == IsExplicitlyNull(WeakStampManager));

		return SerialNumber != 0;
	}
	FORCEINLINE const TWeakPtr<FVoxelStampManager>& GetWeakStampManager() const
	{
		return WeakStampManager;
	}

	FORCEINLINE bool operator==(const FVoxelStampIndex& Other) const
	{
		if (SerialNumber == Other.SerialNumber &&
			WeakStampManager == Other.WeakStampManager)
		{
			checkVoxelSlow(Index == Other.Index);
			return true;
		}

		return false;
	}

private:
	int32 SerialNumber = 0;
	int32 Index = 0;
	TWeakPtr<FVoxelStampManager> WeakStampManager;

	friend FVoxelStampManager;
};