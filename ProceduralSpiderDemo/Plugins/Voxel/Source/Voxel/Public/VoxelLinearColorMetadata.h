// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadata.h"
#include "VoxelFloatMetadata.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelLinearColorMetadataRef.h"
#include "VoxelLinearColorMetadata.generated.h"

UCLASS()
class VOXEL_API UVoxelLinearColorMetadata : public UVoxelMetadata
{
	GENERATED_BODY()
	GENERATED_VOXEL_METADATA_BODY(FVoxelLinearColorMetadataRef)

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	FLinearColor DefaultValue;

	// Values packing type when passed to materials
	UPROPERTY(EditAnywhere, Category = "Config", meta = (FullRefresh))
	EVoxelFloatMetadataPacking GPUPacking = EVoxelFloatMetadataPacking::FourBytes;

public:
	//~ Begin UVoxelMetadata Interface
	virtual FVoxelPinValue GetDefaultValue() const override;
	virtual TVoxelOptional<EVoxelMetadataMaterialType> GetMaterialType() const override;
	//~ End UVoxelMetadata Interface
};