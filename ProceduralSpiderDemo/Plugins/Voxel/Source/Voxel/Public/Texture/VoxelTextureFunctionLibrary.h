// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFunctionLibrary.h"
#include "Texture/VoxelTextureRef.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelTextureFunctionLibrary.generated.h"

UENUM()
enum class EVoxelTextureInterpolation : uint8
{
	NearestNeighbor,
	Bilinear,
	Bicubic
};

UCLASS()
class VOXEL_API UVoxelTextureFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Texture")
	bool IsValid(const FVoxelTextureRef& Texture) const;

	UFUNCTION(Category = "Texture")
	FIntPoint GetSize(const FVoxelTextureRef& Texture) const;

public:
	// Sample a voxel texture
	//	@param	bsRGB	If true will convert from sRGB space if the texture is a color texture
	UFUNCTION(Category = "Texture", meta = (AdvancedDisplay = "Interpolation, bsRGB"))
	FVoxelLinearColorBuffer SampleTexture(
		const FVoxelTextureRef& Texture,
		UPARAM(meta = (PositionPin)) const FVoxelVector2DBuffer& Position,
		float Scale = 100.f,
		EVoxelTextureInterpolation Interpolation = EVoxelTextureInterpolation::Bilinear,
		UPARAM(DisplayName = "sRGB") bool bSRGB = true) const;
};