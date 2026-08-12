// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightmap.h"
#include "VoxelHeightmap_HeightData.h"
#include "VoxelHeightmap_Height.generated.h"

UCLASS(Within=VoxelHeightmap)
class VOXEL_API UVoxelHeightmap_Height : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Source")
	TSoftObjectPtr<UTexture2D> Texture;

	UPROPERTY(EditAnywhere, Category = "Source")
	EVoxelTextureChannel TextureChannel = EVoxelTextureChannel::R;

	// If true data will be stored in uint16 instead of float
	UPROPERTY(EditAnywhere, Category = "Source", AdvancedDisplay)
	bool bCompressTo16Bits = false;

	UPROPERTY(EditAnywhere, Category = "Source|Min Height")
	bool bEnableMinHeight = false;

	UPROPERTY(EditAnywhere, Category = "Source|Min Height", meta = (EditCondition = bEnableMinHeight, UIMin = 0, UIMax = 1))
	float MinHeight = 0;

	UPROPERTY(EditAnywhere, Category = "Source|Min Height", meta = (EditCondition = bEnableMinHeight))
	float MinHeightSlope = 1;
#endif

public:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (Units = cm))
	float ScaleZ = 64000;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (Units = cm))
	float OffsetZ = 0;

	// Bicubic interpolation is slower but better looking
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bUseBicubic = true;

public:
	// Heights are normalized between 0-1: ie, MAX_uint16 is 1.f
	void SetHeights(
		int32 SizeX,
		int32 SizeY,
		TConstVoxelArrayView<uint16> Heights,
		float InternalScaleZ = 1.f,
		float InternalOffsetZ = 0.f);

	void SetHeights(
		int32 SizeX,
		int32 SizeY,
		TConstVoxelArrayView<float> Heights,
		float InternalScaleZ = 1.f,
		float InternalOffsetZ = 0.f);

#if WITH_EDITOR
	void OnReimport();
#endif

public:
	TSharedPtr<const FVoxelHeightmap_HeightData> GetData() const;

	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

#if WITH_EDITOR
	bool bAllowUpdate = true;
#endif

private:
	mutable TSharedPtr<const FVoxelHeightmap_HeightData> Data;
};