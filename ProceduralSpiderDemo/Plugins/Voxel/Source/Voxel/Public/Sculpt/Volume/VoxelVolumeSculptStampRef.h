// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeStampRef.h"
#include "Sculpt/Volume/VoxelVolumeSculptStamp.h"
#include "VoxelVolumeSculptStampRef.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

USTRUCT(BlueprintType, DisplayName = "Voxel Volume Sculpt Stamp", Category = "Voxel|Stamp|Volume Sculpt", meta = (HasNativeMake = "/Script/Voxel.VoxelVolumeSculptStamp_K2.Make", HasNativeBreak = "/Script/Voxel.VoxelVolumeSculptStamp_K2.Break"))
struct VOXEL_API FVoxelVolumeSculptStampRef final : public FVoxelVolumeStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelVolumeSculptStampRef, FVoxelVolumeSculptStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelVolumeSculptStampRef> : public TStructOpsTypeTraits<FVoxelStampRef>
{
};

template<>
struct TVoxelStampRefImpl<FVoxelVolumeSculptStamp>
{
	using Type = FVoxelVolumeSculptStampRef;
};

USTRUCT()
struct VOXEL_API FVoxelVolumeSculptInstancedStampRef final : public FVoxelVolumeInstancedStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelVolumeSculptInstancedStampRef, FVoxelVolumeSculptStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelVolumeSculptInstancedStampRef> : public TStructOpsTypeTraits<FVoxelInstancedStampRef>
{
};

template<>
struct TVoxelInstancedStampRefImpl<FVoxelVolumeSculptStamp>
{
	using Type = FVoxelVolumeSculptInstancedStampRef;
};