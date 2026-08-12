// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelStampRef;
class FVoxelStampRefInner;

struct VOXEL_API FVoxelWeakStampRef
{
public:
	FVoxelWeakStampRef() = default;
	FVoxelWeakStampRef(const FVoxelStampRef& StampRef);

	FVoxelStampRef Pin() const;

	FORCEINLINE bool IsExplicitlyNull() const
	{
		return ::IsExplicitlyNull(WeakInner);
	}
	FORCEINLINE bool operator==(const FVoxelWeakStampRef& Other) const
	{
		return WeakInner == Other.WeakInner;
	}

private:
	TWeakPtr<FVoxelStampRefInner> WeakInner;
};