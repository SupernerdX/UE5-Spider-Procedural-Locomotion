// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelCoreMinimal.h"

struct VOXELCORE_API FVoxelSerializationGuard
{
public:
	explicit FVoxelSerializationGuard(FArchive& Ar);
	~FVoxelSerializationGuard();

private:
	FArchive& Ar;
	const int64 Offset;
	int32 SerializedSize = 0;
};