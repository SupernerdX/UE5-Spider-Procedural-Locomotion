// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadata.h"
#include "VoxelStaticMeshMetadata.generated.h"

UENUM()
enum class EVoxelStaticMeshMetadataAttribute : uint8
{
	BaseColor,
	EmissiveColor,
	Normal,
	Roughness,
	Metallic
};

USTRUCT()
struct VOXEL_API FVoxelStaticMeshMetadataData
{
	GENERATED_BODY()

	TSharedPtr<FVoxelBuffer> Buffer;

	//~ Begin TStructOpsTypeTraits Interface
	bool Serialize(FArchive& Ar);
	//~ End TStructOpsTypeTraits Interface
};

template<>
struct TStructOpsTypeTraits<FVoxelStaticMeshMetadataData> : TStructOpsTypeTraitsBase2<FVoxelStaticMeshMetadataData>
{
	enum
	{
		WithSerializer = true,
	};
};

USTRUCT()
struct VOXEL_API FVoxelStaticMeshMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditAnywhere, Category = "Config")
	EVoxelStaticMeshMetadataAttribute Attribute = EVoxelStaticMeshMetadataAttribute::BaseColor;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (AllowedClasses = "/Script/Voxel.VoxelFloatMetadata, /Script/Voxel.VoxelLinearColorMetadata, /Script/Voxel.VoxelNormalMetadata"))
	TObjectPtr<UVoxelMetadata> Metadata;

	UPROPERTY()
	FVoxelStaticMeshMetadataData Data;
};