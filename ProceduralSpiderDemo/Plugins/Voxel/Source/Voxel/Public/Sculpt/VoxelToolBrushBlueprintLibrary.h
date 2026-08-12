// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelToolBrushBlueprintLibrary.generated.h"

UCLASS()
class VOXEL_API UVoxelToolBrushBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Make a simple circular brush with a uniform falloff based on radius
	 * @param FalloffType	Falloff type
	 * @param FalloffAmount	Falloff amount, relative to radius, between 0 and 1
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Sculpt")
	static FVoxelToolBrush MakeCircularBrush(
		EVoxelFalloffType FalloffType = EVoxelFalloffType::Smooth,
		float FalloffAmount = 0.5f);

	/**
	 * Make a brush following an alpha texture. The texture will follow mouse movement and can be optionally aligned to it.
	 * @param FalloffType		Falloff type
	 * @param FalloffAmount		Falloff amount, relative to radius, between 0 and 1
	 * @param HitNormal			Surface normal, used to project the texture downwards. Ignored if Use2DProjection is true.
	 * @param StrokeDirection	Mouse direction, used to align the texture. Ignored if AutoRotateMask is false.
	 * @param Texture			The texture to use
	 * @param TextureChannel	The channel of the texture to use
	 * @param bAutoRotateMask	If true the texture will follow the mouse direction, if false FixedRotation will be used
	 * @param FixedRotation		Fixed rotation to apply if AutoRotateMask is false
	 * @param bUse2DProjection	If true HitNormal will be ignored and the texture will be applied downward instead
	 * @return
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Sculpt")
	static FVoxelToolBrush MakeAlphaBrush(
		EVoxelFalloffType FalloffType = EVoxelFalloffType::Smooth,
		float FalloffAmount = 0.5f,
		FVector HitNormal = FVector::UpVector,
		FVector StrokeDirection = FVector::ForwardVector,
		UVoxelTexture* Texture = nullptr,
		EVoxelTextureChannel TextureChannel = EVoxelTextureChannel::R,
		bool bAutoRotateMask = true,
		float FixedRotation = 0.f,
		bool bUse2DProjection = false);

	/**
	 * Make a brush following a tiled alpha texture. The texture will be tiled and projected down.
	 * @param FalloffType				Falloff type
	 * @param FalloffAmount				Falloff amount, relative to radius, between 0 and 1
	 * @param Texture					The texture to use
	 * @param TextureChannel			The channel of the texture to use
	 * @param Origin					Use to offset the texture
	 * @param TextureRotation			Use to rotate the texture
	 * @param bCenterTextureOnOrigin	If true, the texture used for the pattern is centered on the PatternOrigin.
	 *									If false, the corner of the texture is placed at the PatternOrigin
	 * @param RepeatSize				Scale of the texture
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Sculpt")
	static FVoxelToolBrush MakePatternBrush(
		EVoxelFalloffType FalloffType = EVoxelFalloffType::Smooth,
		float FalloffAmount = 0.5f,
		UVoxelTexture* Texture = nullptr,
		EVoxelTextureChannel TextureChannel = EVoxelTextureChannel::R,
		FVector2D Origin = FVector2D::ZeroVector,
		float TextureRotation = 0.f,
		bool bCenterTextureOnOrigin = false,
		float RepeatSize = 1000.f);
};