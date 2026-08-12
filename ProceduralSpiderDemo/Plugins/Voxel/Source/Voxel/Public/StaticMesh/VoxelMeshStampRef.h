// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeStampRef.h"
#include "StaticMesh/VoxelMeshStamp.h"
#include "VoxelMeshStampRef.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

USTRUCT(BlueprintType, DisplayName = "Voxel Mesh Stamp", Category = "Voxel|Stamp|Mesh", meta = (HasNativeMake = "/Script/Voxel.VoxelMeshStamp_K2.Make", HasNativeBreak = "/Script/Voxel.VoxelMeshStamp_K2.Break"))
struct VOXEL_API FVoxelMeshStampRef final : public FVoxelVolumeStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelMeshStampRef, FVoxelMeshStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelMeshStampRef> : public TStructOpsTypeTraits<FVoxelStampRef>
{
};

template<>
struct TVoxelStampRefImpl<FVoxelMeshStamp>
{
	using Type = FVoxelMeshStampRef;
};

USTRUCT()
struct VOXEL_API FVoxelMeshInstancedStampRef final : public FVoxelVolumeInstancedStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelMeshInstancedStampRef, FVoxelMeshStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelMeshInstancedStampRef> : public TStructOpsTypeTraits<FVoxelInstancedStampRef>
{
};

template<>
struct TVoxelInstancedStampRefImpl<FVoxelMeshStamp>
{
	using Type = FVoxelMeshInstancedStampRef;
};