// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightStampRef.h"
#include "Spline/VoxelHeightSplineStamp.h"
#include "VoxelHeightSplineStampRef.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

USTRUCT(BlueprintType, DisplayName = "Voxel Height Spline Stamp", Category = "Voxel|Stamp|Height Spline", meta = (HasNativeMake = "/Script/Voxel.VoxelHeightSplineStamp_K2.Make", HasNativeBreak = "/Script/Voxel.VoxelHeightSplineStamp_K2.Break"))
struct VOXEL_API FVoxelHeightSplineStampRef final : public FVoxelHeightStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelHeightSplineStampRef, FVoxelHeightSplineStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelHeightSplineStampRef> : public TStructOpsTypeTraits<FVoxelStampRef>
{
};

template<>
struct TVoxelStampRefImpl<FVoxelHeightSplineStamp>
{
	using Type = FVoxelHeightSplineStampRef;
};

USTRUCT()
struct VOXEL_API FVoxelHeightSplineInstancedStampRef final : public FVoxelHeightInstancedStampRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_STAMP_REF_BODY(FVoxelHeightSplineInstancedStampRef, FVoxelHeightSplineStamp)
};

template<>
struct TStructOpsTypeTraits<FVoxelHeightSplineInstancedStampRef> : public TStructOpsTypeTraits<FVoxelInstancedStampRef>
{
};

template<>
struct TVoxelInstancedStampRefImpl<FVoxelHeightSplineStamp>
{
	using Type = FVoxelHeightSplineInstancedStampRef;
};