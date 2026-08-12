// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightStampRef.h"
#include "Sculpt/Height/VoxelHeightSculptStamp.h"
#include "VoxelHeightSculptStampRef.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

USTRUCT(BlueprintType, DisplayName = "Voxel Height Sculpt Stamp", Category = "Voxel|Stamp|Height Sculpt", meta = (HasNativeMake = "/Script/Voxel.VoxelHeightSculptStamp_K2.Make", HasNativeBreak = "/Script/Voxel.VoxelHeightSculptStamp_K2.Break"))
struct VOXEL_API FVoxelHeightSculptStampRef final : public FVoxelHeightStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelHeightSculptStampRef, FVoxelHeightSculptStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelHeightSculptStampRef> : public TStructOpsTypeTraits<FVoxelStampRef>
{
};

template<>
struct TVoxelStampRefImpl<FVoxelHeightSculptStamp>
{
	using Type = FVoxelHeightSculptStampRef;
};

USTRUCT()
struct VOXEL_API FVoxelHeightSculptInstancedStampRef final : public FVoxelHeightInstancedStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelHeightSculptInstancedStampRef, FVoxelHeightSculptStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelHeightSculptInstancedStampRef> : public TStructOpsTypeTraits<FVoxelInstancedStampRef>
{
};

template<>
struct TVoxelInstancedStampRefImpl<FVoxelHeightSculptStamp>
{
	using Type = FVoxelHeightSculptInstancedStampRef;
};