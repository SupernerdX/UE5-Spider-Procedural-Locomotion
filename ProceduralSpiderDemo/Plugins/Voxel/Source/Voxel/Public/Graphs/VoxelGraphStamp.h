// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValueOps.h"
#include "VoxelVolumeStamp.h"
#include "VoxelHeightStamp.h"
#include "VoxelFunctionLibrary.h"
#include "VoxelGraphStamp.generated.h"

struct FVoxelStampRuntime;
struct FVoxelSurfaceTypeBlendBuffer;

USTRUCT()
struct VOXEL_API FVoxelHeightGraphStampWrapper
{
	GENERATED_BODY()

	TSharedPtr<const FVoxelHeightStampRuntime> Stamp;
};

USTRUCT()
struct VOXEL_API FVoxelVolumeGraphStampWrapper
{
	GENERATED_BODY()

	TSharedPtr<const FVoxelVolumeStampRuntime> Stamp;
};

USTRUCT()
struct FVoxelPinValueOps_FVoxelHeightGraphStampWrapper : public FVoxelPinValueOps
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	//~ Begin FVoxelPinValueOps Interface
	virtual EVoxelPinValueOpsUsage GetUsage() const override;
	virtual FVoxelPinType GetExposedType() const override;

	virtual FVoxelRuntimePinValue MakeRuntimeValue(
		const FVoxelPinValue& Value,
		const FVoxelPinType::FRuntimeValueContext& Context) const override;

	virtual bool HasPinDefaultValue() const override;

#if WITH_EDITOR
	virtual TMap<FName, FString> GetMetaData() const override;
#endif
	//~ End FVoxelPinValueOps Interface
};

USTRUCT()
struct FVoxelPinValueOps_FVoxelVolumeGraphStampWrapper : public FVoxelPinValueOps
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	//~ Begin FVoxelPinValueOps Interface
	virtual EVoxelPinValueOpsUsage GetUsage() const override;
	virtual FVoxelPinType GetExposedType() const override;

	virtual FVoxelRuntimePinValue MakeRuntimeValue(
		const FVoxelPinValue& Value,
		const FVoxelPinType::FRuntimeValueContext& Context) const override;

	virtual bool HasPinDefaultValue() const override;

#if WITH_EDITOR
	virtual TMap<FName, FString> GetMetaData() const override;
#endif
	//~ End FVoxelPinValueOps Interface
};