// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightStampRef.h"
#include "Graphs/VoxelHeightGraphStamp.h"
#include "VoxelHeightGraphStampRef.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

USTRUCT(BlueprintType, DisplayName = "Voxel Height Graph Stamp", Category = "Voxel|Stamp|Height Graph", meta = (HasNativeMake = "/Script/Voxel.VoxelHeightGraphStamp_K2.Make", HasNativeBreak = "/Script/Voxel.VoxelHeightGraphStamp_K2.Break"))
struct VOXEL_API FVoxelHeightGraphStampRef final : public FVoxelHeightStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelHeightGraphStampRef, FVoxelHeightGraphStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelHeightGraphStampRef> : public TStructOpsTypeTraits<FVoxelStampRef>
{
};

template<>
struct TVoxelStampRefImpl<FVoxelHeightGraphStamp>
{
	using Type = FVoxelHeightGraphStampRef;
};

USTRUCT()
struct VOXEL_API FVoxelHeightGraphInstancedStampRef final : public FVoxelHeightInstancedStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelHeightGraphInstancedStampRef, FVoxelHeightGraphStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelHeightGraphInstancedStampRef> : public TStructOpsTypeTraits<FVoxelInstancedStampRef>
{
};

template<>
struct TVoxelInstancedStampRefImpl<FVoxelHeightGraphStamp>
{
	using Type = FVoxelHeightGraphInstancedStampRef;
};