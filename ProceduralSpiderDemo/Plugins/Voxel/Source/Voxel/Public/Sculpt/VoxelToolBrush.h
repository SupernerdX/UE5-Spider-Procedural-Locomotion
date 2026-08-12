// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFalloff.h"
#include "VoxelToolBrush.generated.h"

struct FVoxelConfig;
struct FVoxelToolBrush;
class UVoxelTexture;
class FVoxelTextureData;
class FVoxelToolRuntimeBrush;

UENUM(BlueprintType, meta = (VoxelSegmentedEnum))
enum class EVoxelBrushType : uint8
{
	Circular UMETA(ToolTip = "Simple circular brush", Icon = "LandscapeEditor.CircleBrush"),
	Alpha UMETA(ToolTip = "Alpha brush, orients a mask image with the brush stroke", Icon = "LandscapeEditor.AlphaBrush"),
	Pattern UMETA(ToolTip = "Pattern brush, tiles a mask image across the landscape", Icon = "LandscapeEditor.AlphaBrush_Pattern")
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FVoxelCircularBrush
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelFalloff Falloff;

	void UpdateMaterial(
		UMaterialInstanceDynamic*& Material,
		bool bHeightTool) const;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FVoxelAlphaBrush
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelTexture> Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EVoxelTextureChannel TextureChannel = EVoxelTextureChannel::R;

	// Rotate the brush texture to follow the mouse movement (ie, StrideDirection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Auto-Rotate", Category = "Config")
	bool bAutoRotateMask = true;

	// Used to rotate the mask if bAutoRotateMask is false
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (EditCondition = "!bAutoRotateMask", UIMin = "-180", UIMax = "180", ClampMin = "-360", ClampMax = "360"))
	float FixedRotation = 0.f;

	// If true will always be projected down and won't use the surface normal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bUse2DProjection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelFalloff Falloff;

	void UpdateMaterial(
		UMaterialInstanceDynamic*& Material,
		bool bHeightTool) const;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FVoxelPatternBrush
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelTexture> Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EVoxelTextureChannel TextureChannel = EVoxelTextureChannel::R;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVector2D Origin = FVector2D::ZeroVector;

	// Rotates the brush mask texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (UIMin = "-180", UIMax = "180", ClampMin = "-360", ClampMax = "360"))
	float TextureRotation = 0.f;

	// if true, the texture used for the pattern is centered on the PatternOrigin.
	// if false, the corner of the texture is placed at the PatternOrigin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bCenterTextureOnOrigin = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float RepeatSize = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelFalloff Falloff;

	void UpdateMaterial(
		UMaterialInstanceDynamic*& Material,
		bool bHeightTool) const;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FVoxelToolBrushBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EVoxelBrushType BrushType = EVoxelBrushType::Circular;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ShowOnlyInnerProperties, EditCondition = "BrushType == EVoxelBrushType::Circular", EditConditionHides))
	FVoxelCircularBrush CircularBrushData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ShowOnlyInnerProperties, EditCondition = "BrushType == EVoxelBrushType::Alpha", EditConditionHides))
	FVoxelAlphaBrush AlphaBrushData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ShowOnlyInnerProperties, EditCondition = "BrushType == EVoxelBrushType::Pattern", EditConditionHides))
	FVoxelPatternBrush PatternBrushData;

public:
	FVoxelToolBrush GetBrush(
		const FVector& Normal,
		const FVector& Tangent) const;

	void UpdateMaterial(
		UMaterialInstanceDynamic*& Material,
		float Radius,
		const FVector& Normal,
		const FVector& Tangent,
		bool bHeightTool) const;

protected:
	void GetBrushPlane(
		FVector Normal,
		FVector Tangent,
		FVector& OutPlaneX,
		FVector& OutPlaneY) const;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FVoxelToolBrush : public FVoxelToolBrushBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVector StrokeDirection = FVector::ForwardVector;

public:
	TSharedRef<const FVoxelToolRuntimeBrush> GetRuntimeBrush() const;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelToolRuntimeBrush
{
public:
	FORCEINLINE float GetFalloff(
		const float Distance,
		const float Radius) const
	{
		return FVoxelFalloff::GetFalloff(
			Falloff.Type,
			Distance,
			Radius,
			Falloff.Amount);
	}

	float GetMaskStrength(
		const FVector& MaskCenter,
		const FVector& Position,
		float Radius) const;

	float GetMaskStrength(
		const FVector2D& MaskCenter,
		const FVector2D& Position,
		float Radius) const;

private:
	float GetAlphaMaskStrength(float X, float Y) const;
	float GetPatternMaskStrength(const FVector2D& Position) const;

private:
	EVoxelBrushType BrushType = {};
	FVector3f PlaneX = FVector3f(ForceInit);
	FVector3f PlaneY = FVector3f(ForceInit);

	FVoxelFalloff Falloff;
	FVoxelPatternBrush PatternBrush;
	TSharedPtr<const FVoxelTextureData> TextureData;
	EVoxelTextureChannel TextureChannel = {};

	FVoxelToolRuntimeBrush() = default;

	friend FVoxelToolBrush;
};