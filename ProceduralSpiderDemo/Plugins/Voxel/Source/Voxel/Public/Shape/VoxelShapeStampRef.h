// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeStampRef.h"
#include "Shape/VoxelShapeStamp.h"
#include "VoxelShapeStampRef.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

USTRUCT(BlueprintType, DisplayName = "Voxel Shape Stamp", Category = "Voxel|Stamp|Shape", meta = (HasNativeMake = "/Script/Voxel.VoxelShapeStamp_K2.Make", HasNativeBreak = "/Script/Voxel.VoxelShapeStamp_K2.Break"))
struct VOXEL_API FVoxelShapeStampRef final : public FVoxelVolumeStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelShapeStampRef, FVoxelShapeStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelShapeStampRef> : public TStructOpsTypeTraits<FVoxelStampRef>
{
};

template<>
struct TVoxelStampRefImpl<FVoxelShapeStamp>
{
	using Type = FVoxelShapeStampRef;
};

USTRUCT()
struct VOXEL_API FVoxelShapeInstancedStampRef final : public FVoxelVolumeInstancedStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelShapeInstancedStampRef, FVoxelShapeStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelShapeInstancedStampRef> : public TStructOpsTypeTraits<FVoxelInstancedStampRef>
{
};

template<>
struct TVoxelInstancedStampRefImpl<FVoxelShapeStamp>
{
	using Type = FVoxelShapeInstancedStampRef;
};