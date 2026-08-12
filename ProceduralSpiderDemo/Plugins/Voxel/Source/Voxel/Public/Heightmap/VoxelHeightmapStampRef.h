// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightStampRef.h"
#include "Heightmap/VoxelHeightmapStamp.h"
#include "VoxelHeightmapStampRef.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

USTRUCT(BlueprintType, DisplayName = "Voxel Heightmap Stamp", Category = "Voxel|Stamp|Heightmap", meta = (HasNativeMake = "/Script/Voxel.VoxelHeightmapStamp_K2.Make", HasNativeBreak = "/Script/Voxel.VoxelHeightmapStamp_K2.Break"))
struct VOXEL_API FVoxelHeightmapStampRef final : public FVoxelHeightStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelHeightmapStampRef, FVoxelHeightmapStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelHeightmapStampRef> : public TStructOpsTypeTraits<FVoxelStampRef>
{
};

template<>
struct TVoxelStampRefImpl<FVoxelHeightmapStamp>
{
	using Type = FVoxelHeightmapStampRef;
};

USTRUCT()
struct VOXEL_API FVoxelHeightmapInstancedStampRef final : public FVoxelHeightInstancedStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelHeightmapInstancedStampRef, FVoxelHeightmapStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelHeightmapInstancedStampRef> : public TStructOpsTypeTraits<FVoxelInstancedStampRef>
{
};

template<>
struct TVoxelInstancedStampRefImpl<FVoxelHeightmapStamp>
{
	using Type = FVoxelHeightmapInstancedStampRef;
};