// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightmap.h"
#include "VoxelHeightmap_WeightData.h"
#include "VoxelHeightmap_Weight.generated.h"

class UVoxelFloatMetadata;
class UVoxelSurfaceTypeInterface;

UENUM()
enum class EVoxelHeightmapWeightType
{
	// Weightmaps will be linearly interpolated one after the other
	AlphaBlended,
	// All weight-blended weightmap are applied first
	// Their weights are normalized
	WeightBlended
};

UCLASS(Within=VoxelHeightmap)
class VOXEL_API UVoxelHeightmap_Weight : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Source")
	TSoftObjectPtr<UTexture2D> Texture;

	UPROPERTY(EditAnywhere, Category = "Source")
	EVoxelTextureChannel TextureChannel = EVoxelTextureChannel::R;
#endif

public:
	UPROPERTY()
	TObjectPtr<UObject> Material;

	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelSurfaceTypeInterface> SurfaceType;

	UPROPERTY(EditAnywhere, Category = "Config")
	EVoxelHeightmapWeightType Type = EVoxelHeightmapWeightType::AlphaBlended;

	UPROPERTY(EditAnywhere, Category = "Config")
	float Strength = 1.f;

	// If set this metadata will be set to the clamped strength from 0 to 1 of the weightmap
	// The value will be maxed with the values from previous stamps
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelFloatMetadata> Metadata;

	// Bicubic interpolation is slower but better looking
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	bool bUseBicubic = false;

public:
	void SetWeights(
		int32 SizeX,
		int32 SizeY,
		TConstVoxelArrayView<uint8> Weights);

	void SetWeights(
		int32 SizeX,
		int32 SizeY,
		TConstVoxelArrayView<uint16> Weights);

	void SetWeights(
		int32 SizeX,
		int32 SizeY,
		TConstVoxelArrayView<float> Weights);

#if WITH_EDITOR
	void OnReimport();
#endif

public:
	TSharedPtr<const FVoxelHeightmap_WeightData> GetData() const;

	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

private:
	mutable TSharedPtr<const FVoxelHeightmap_WeightData> Data;
};