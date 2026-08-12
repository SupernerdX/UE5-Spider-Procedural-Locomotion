// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadata.h"
#include "VoxelNormalMetadataRef.h"
#include "Buffer/VoxelNormalBuffer.h"
#include "VoxelNormalMetadata.generated.h"

UCLASS()
class VOXEL_API UVoxelNormalMetadata : public UVoxelMetadata
{
	GENERATED_BODY()
	GENERATED_VOXEL_METADATA_BODY(FVoxelNormalMetadataRef)

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	FVector DefaultValue = FVector::UpVector;

public:
	//~ Begin UVoxelMetadata Interface
	virtual FVoxelPinValue GetDefaultValue() const override;
	virtual TVoxelOptional<EVoxelMetadataMaterialType> GetMaterialType() const override;
	//~ End UVoxelMetadata Interface
};