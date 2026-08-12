// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/VoxelToolBrushBlueprintLibrary.h"

FVoxelToolBrush UVoxelToolBrushBlueprintLibrary::MakeCircularBrush(
	const EVoxelFalloffType FalloffType,
	const float FalloffAmount)
{
	FVoxelToolBrush Result;
	Result.BrushType = EVoxelBrushType::Circular;
	Result.CircularBrushData.Falloff.Type = FalloffType;
	Result.CircularBrushData.Falloff.Amount = FalloffAmount;
	return Result;
}

FVoxelToolBrush UVoxelToolBrushBlueprintLibrary::MakeAlphaBrush(
	const EVoxelFalloffType FalloffType,
	const float FalloffAmount,
	const FVector HitNormal,
	const FVector StrokeDirection,
	UVoxelTexture* Texture,
	const EVoxelTextureChannel TextureChannel,
	const bool bAutoRotateMask,
	const float FixedRotation,
	const bool bUse2DProjection)
{
	FVoxelToolBrush Result;
	Result.BrushType = EVoxelBrushType::Alpha;
	Result.CircularBrushData.Falloff.Type = FalloffType;
	Result.CircularBrushData.Falloff.Amount = FalloffAmount;
	Result.HitNormal = HitNormal;
	Result.StrokeDirection = StrokeDirection;
	Result.AlphaBrushData.Texture = Texture;
	Result.AlphaBrushData.TextureChannel = TextureChannel;
	Result.AlphaBrushData.bAutoRotateMask = bAutoRotateMask;
	Result.AlphaBrushData.FixedRotation = FixedRotation;
	Result.AlphaBrushData.bUse2DProjection = bUse2DProjection;
	return Result;
}

FVoxelToolBrush UVoxelToolBrushBlueprintLibrary::MakePatternBrush(
	const EVoxelFalloffType FalloffType,
	const float FalloffAmount,
	UVoxelTexture* Texture,
	const EVoxelTextureChannel TextureChannel,
	const FVector2D Origin,
	const float TextureRotation,
	const bool bCenterTextureOnOrigin,
	const float RepeatSize)
{
	FVoxelToolBrush Result;
	Result.BrushType = EVoxelBrushType::Pattern;
	Result.CircularBrushData.Falloff.Type = FalloffType;
	Result.CircularBrushData.Falloff.Amount = FalloffAmount;
	Result.PatternBrushData.Texture = Texture;
	Result.PatternBrushData.TextureChannel = TextureChannel;
	Result.PatternBrushData.Origin = Origin;
	Result.PatternBrushData.TextureRotation = TextureRotation;
	Result.PatternBrushData.bCenterTextureOnOrigin = bCenterTextureOnOrigin;
	Result.PatternBrushData.RepeatSize = RepeatSize;
	return Result;
}