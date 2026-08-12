// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadata.h"
#include "VoxelFloatMetadataRef.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelFloatMetadata.generated.h"

UENUM()
enum class EVoxelFloatMetadataPacking
{
	OneByte UMETA(DisplayName = "1 Byte", ToolTip = "Values will be clamped between 0-1 and stored in 8 bits, when passed to materials"),
	TwoBytes UMETA(DisplayName = "2 Bytes", ToolTip = "Values will be clamped between -65504 to +65504 and stored in 16 bits, when passed to materials"),
	FourBytes UMETA(DisplayName = "4 Bytes", ToolTip = "Full float values will be passed to materials (most expensive)")
};

UCLASS()
class VOXEL_API UVoxelFloatMetadata : public UVoxelMetadata
{
	GENERATED_BODY()
	GENERATED_VOXEL_METADATA_BODY(FVoxelFloatMetadataRef)

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	float DefaultValue = 0.f;

	// Values packing type when passed to materials
	UPROPERTY(EditAnywhere, Category = "Config", meta = (FullRefresh))
	EVoxelFloatMetadataPacking GPUPacking = EVoxelFloatMetadataPacking::FourBytes;

public:
	//~ Begin UVoxelMetadata Interface
	virtual FVoxelPinValue GetDefaultValue() const override;
	virtual TVoxelOptional<EVoxelMetadataMaterialType> GetMaterialType() const override;
	//~ End UVoxelMetadata Interface

private:
	UPROPERTY()
	FName LegacyName;

	friend UVoxelMetadata;
};