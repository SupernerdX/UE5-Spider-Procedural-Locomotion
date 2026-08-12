// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/VoxelToolBrush.h"
#include "Engine/Texture2D.h"
#include "VoxelConfig.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Texture/VoxelTexture.h"
#include "Texture/VoxelTextureData.h"

void FVoxelCircularBrush::UpdateMaterial(
	UMaterialInstanceDynamic*& Material,
	const bool bHeightTool) const
{
	VOXEL_FUNCTION_COUNTER();

	UMaterialInterface* TargetMaterial;
	if (bHeightTool)
	{
		TargetMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Tools/MI_Height_ToolCircularBrush.MI_Height_ToolCircularBrush"));
	}
	else
	{
		TargetMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Tools/MI_ToolCircularBrush.MI_ToolCircularBrush"));
	}
	ensure(TargetMaterial);

	if (!Material ||
		Material->Parent != TargetMaterial)
	{
		Material = UMaterialInstanceDynamic::Create(TargetMaterial, nullptr);
	}

	Material->SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(Falloff.Type));
	Material->SetScalarParameterValue(STATIC_FNAME("Falloff"), Falloff.Type == EVoxelFalloffType::None ? 0.f : Falloff.Amount);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelAlphaBrush::UpdateMaterial(
	UMaterialInstanceDynamic*& Material,
	const bool bHeightTool) const
{
	VOXEL_FUNCTION_COUNTER();

	UMaterialInterface* TargetMaterial;
	if (bHeightTool)
	{
		TargetMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Tools/MI_Height_ToolAlphaBrush.MI_Height_ToolAlphaBrush"));
	}
	else
	{
		TargetMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Tools/MI_ToolAlphaBrush.MI_ToolAlphaBrush"));
	}
	ensure(TargetMaterial);

	if (!Material ||
		Material->Parent != TargetMaterial)
	{
		Material = UMaterialInstanceDynamic::Create(TargetMaterial, nullptr);
	}

	const FLinearColor Channel = INLINE_LAMBDA
	{
		switch (TextureChannel)
		{
		case EVoxelTextureChannel::R: return FLinearColor(1.f, 0.f, 0.f, 0.f);
		case EVoxelTextureChannel::G: return FLinearColor(0.f, 1.f, 0.f, 0.f);
		case EVoxelTextureChannel::B: return FLinearColor(0.f, 0.f, 1.f, 0.f);
		case EVoxelTextureChannel::A: return FLinearColor(0.f, 0.f, 0.f, 1.f);
		default: ensure(false); return FLinearColor::Black;
		}
	};

#if WITH_EDITOR
	Material->SetTextureParameterValue(STATIC_FNAME("Texture"), Texture ? Texture->Texture.LoadSynchronous() : nullptr);
#else
	ensure(false);
#endif
	Material->SetVectorParameterValue(STATIC_FNAME("TextureChannel"), Channel);
	Material->SetScalarParameterValue(STATIC_FNAME("Rotation"), FixedRotation);
	Material->SetScalarParameterValue(STATIC_FNAME("Use2DProjection"), bUse2DProjection ? 1.f : 0.f);
	Material->SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(Falloff.Type));
	Material->SetScalarParameterValue(STATIC_FNAME("Falloff"), Falloff.Type == EVoxelFalloffType::None ? 0.f : Falloff.Amount);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPatternBrush::UpdateMaterial(
	UMaterialInstanceDynamic*& Material,
	const bool bHeightTool) const
{
	VOXEL_FUNCTION_COUNTER();

	UMaterialInterface* TargetMaterial;
	if (bHeightTool)
	{
		TargetMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Tools/MI_Height_ToolPatternBrush.MI_Height_ToolPatternBrush"));
	}
	else
	{
		TargetMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Tools/MI_ToolPatternBrush.MI_ToolPatternBrush"));
	}
	ensure(TargetMaterial);

	if (!Material ||
		Material->Parent != TargetMaterial)
	{
		Material = UMaterialInstanceDynamic::Create(TargetMaterial, nullptr);
	}

	const FLinearColor Channel = INLINE_LAMBDA
	{
		switch (TextureChannel)
		{
		case EVoxelTextureChannel::R: return FLinearColor(1.f, 0.f, 0.f, 0.f);
		case EVoxelTextureChannel::G: return FLinearColor(0.f, 1.f, 0.f, 0.f);
		case EVoxelTextureChannel::B: return FLinearColor(0.f, 0.f, 1.f, 0.f);
		case EVoxelTextureChannel::A: return FLinearColor(0.f, 0.f, 0.f, 1.f);
		default: ensure(false); return FLinearColor::Black;
		}
	};

#if WITH_EDITOR
	Material->SetTextureParameterValue(STATIC_FNAME("Texture"), Texture ? Texture->Texture.LoadSynchronous() : nullptr);
#else
	ensure(false);
#endif
	Material->SetVectorParameterValue(STATIC_FNAME("TextureChannel"), Channel);
	Material->SetVectorParameterValue(STATIC_FNAME("Origin"), FVector(Origin, 0.f));
	Material->SetScalarParameterValue(STATIC_FNAME("CenterTextureOnOrigin"), bCenterTextureOnOrigin ? 1.f : 0.f);
	Material->SetScalarParameterValue(STATIC_FNAME("RepeatSize"), RepeatSize);
	Material->SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(Falloff.Type));
	Material->SetScalarParameterValue(STATIC_FNAME("Falloff"), Falloff.Type == EVoxelFalloffType::None ? 0.f : Falloff.Amount);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolBrush FVoxelToolBrushBase::GetBrush(
	const FVector& Normal,
	const FVector& Tangent) const
{
	FVoxelToolBrush Result;
	static_cast<FVoxelToolBrushBase&>(Result) = *this;
	Result.HitNormal = Normal;
	Result.StrokeDirection = Tangent;
	return Result;
}

void FVoxelToolBrushBase::UpdateMaterial(
	UMaterialInstanceDynamic*& Material,
	const float Radius,
	const FVector& Normal,
	const FVector& Tangent,
	const bool bHeightTool) const
{
	switch (BrushType)
	{
	case EVoxelBrushType::Circular: CircularBrushData.UpdateMaterial(Material, bHeightTool); break;
	case EVoxelBrushType::Alpha: AlphaBrushData.UpdateMaterial(Material, bHeightTool); break;
	case EVoxelBrushType::Pattern: PatternBrushData.UpdateMaterial(Material, bHeightTool); break;
	}

	if (!Material)
	{
		return;
	}

	FVector PlaneX;
	FVector PlaneY;
	GetBrushPlane(
		Normal,
		Tangent,
		PlaneX,
		PlaneY);

	Material->SetScalarParameterValue(STATIC_FNAME("Radius"), Radius);
	Material->SetVectorParameterValue(STATIC_FNAME("PlaneX"), PlaneX);
	Material->SetVectorParameterValue(STATIC_FNAME("PlaneY"), PlaneY);
}

void FVoxelToolBrushBase::GetBrushPlane(
	FVector Normal,
	FVector Tangent,
	FVector& OutPlaneX,
	FVector& OutPlaneY) const
{
	const auto ComputeTangent = [&]
	{
		const FVector ParallelVector = FMath::Abs(Normal.Z) >= 1.f ? FVector::ForwardVector : FVector::UpVector;
		const FVector InitialTangent = FVector::CrossProduct(Normal, ParallelVector).GetSafeNormal();

		const float RotationRads = FMath::DegreesToRadians(AlphaBrushData.FixedRotation);
		Tangent = FQuat(Normal, RotationRads).RotateVector(InitialTangent);
	};

	if (BrushType == EVoxelBrushType::Alpha)
	{
		if (AlphaBrushData.bUse2DProjection)
		{
			Normal = FVector::UpVector;
		}

		if (!AlphaBrushData.bAutoRotateMask)
		{
			ComputeTangent();
		}
	}
	else if (BrushType == EVoxelBrushType::Pattern)
	{
		Normal = FVector::UpVector;
		ComputeTangent();
	}

	OutPlaneY = FVector::CrossProduct(Normal, Tangent).GetSafeNormal();
	OutPlaneX = FVector::CrossProduct(OutPlaneY, Normal).GetSafeNormal();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<const FVoxelToolRuntimeBrush> FVoxelToolBrush::GetRuntimeBrush() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedRef<FVoxelToolRuntimeBrush> Brush = MakeShareable(new FVoxelToolRuntimeBrush());
	Brush->BrushType = BrushType;

	FVector PlaneX;
	FVector PlaneY;
	GetBrushPlane(
		HitNormal,
		StrokeDirection,
		PlaneX,
		PlaneY);

	Brush->PlaneX = FVector3f(PlaneX);
	Brush->PlaneY = FVector3f(PlaneY);

	switch (BrushType)
	{
	default: ensure(false);
	case EVoxelBrushType::Circular:
	{
		Brush->Falloff = CircularBrushData.Falloff;
	}
	break;
	case EVoxelBrushType::Alpha:
	{
		Brush->Falloff = AlphaBrushData.Falloff;
		Brush->TextureData = AlphaBrushData.Texture ? AlphaBrushData.Texture->GetData() : nullptr;
		Brush->TextureChannel = AlphaBrushData.TextureChannel;
	}
	break;
	case EVoxelBrushType::Pattern:
	{
		Brush->Falloff = AlphaBrushData.Falloff;
		Brush->TextureData = AlphaBrushData.Texture ? AlphaBrushData.Texture->GetData() : nullptr;
		Brush->TextureChannel = AlphaBrushData.TextureChannel;

		Brush->PatternBrush = PatternBrushData;
		Brush->PatternBrush.Texture = nullptr;
	}
	break;
	}

	return Brush;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float FVoxelToolRuntimeBrush::GetMaskStrength(
	const FVector& MaskCenter,
	const FVector& Position,
	const float Radius) const
{
	switch (BrushType)
	{
	default: VOXEL_ASSUME(false);
	case EVoxelBrushType::Circular:
	{
		return 1.f;
	}
	case EVoxelBrushType::Alpha:
	{
		if (!TextureData)
		{
			return 1.f;
		}

		const FVector3f RelativePosition = FVector3f(Position - MaskCenter);
		const FVector2f MaskScale = FVector2f(TextureData->SizeX, TextureData->SizeY) / Radius / 2.f;

		const float X = FVector3f::DotProduct(RelativePosition, PlaneX) * MaskScale.X + TextureData->SizeX / 2.f;
		const float Y = FVector3f::DotProduct(RelativePosition, PlaneY) * MaskScale.Y + TextureData->SizeY / 2.f;

		return GetAlphaMaskStrength(X, Y);
	}
	case EVoxelBrushType::Pattern:
	{
		if (!TextureData)
		{
			return 1.f;
		}

		return GetPatternMaskStrength(FVector2D(Position));
	}
	}
}

float FVoxelToolRuntimeBrush::GetMaskStrength(
	const FVector2D& MaskCenter,
	const FVector2D& Position,
	const float Radius) const
{
	switch (BrushType)
	{
	default: VOXEL_ASSUME(false);
	case EVoxelBrushType::Circular:
	{
		return 1.f;
	}
	case EVoxelBrushType::Alpha:
	{
		if (!TextureData)
		{
			return 1.f;
		}

		const FVector2f RelativePosition = FVector2f(Position - MaskCenter);
		const FVector2f MaskScale = FVector2f(TextureData->SizeX, TextureData->SizeY) / Radius / 2.f;

		const float X = FVector2f::DotProduct(RelativePosition, FVector2f(PlaneX).GetSafeNormal()) * MaskScale.X + TextureData->SizeX / 2.f;
		const float Y = FVector2f::DotProduct(RelativePosition, FVector2f(PlaneY).GetSafeNormal()) * MaskScale.Y + TextureData->SizeY / 2.f;

		return GetAlphaMaskStrength(X, Y);
	}
	case EVoxelBrushType::Pattern:
	{
		if (!TextureData)
		{
			return 1.f;
		}

		return GetPatternMaskStrength(Position);
	}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float FVoxelToolRuntimeBrush::GetAlphaMaskStrength(const float X, const float Y) const
{
	if (X < 0.f ||
		Y < 0.f ||
		X >= TextureData->SizeX - 1 ||
		Y >= TextureData->SizeY - 1)
	{
		return 0.f;
	}

	return TextureData->Sample(FVector2D(X, Y), TextureChannel);
}

float FVoxelToolRuntimeBrush::GetPatternMaskStrength(const FVector2D& Position) const
{
	const FIntPoint MaskSize(
		TextureData->SizeX,
		TextureData->SizeY);

	FVector2D LocalOrigin = -PatternBrush.Origin;

	const FVector2D LocalScale = FVector2D(
		1.f / (PatternBrush.RepeatSize * (float(MaskSize.X) / float(MaskSize.Y))),
		1.f / PatternBrush.RepeatSize) / 2.f;

	LocalOrigin *= LocalScale;

	const float Angle = -PatternBrush.TextureRotation;

	if (PatternBrush.bCenterTextureOnOrigin)
	{
		LocalOrigin += FVector2D(0.5f, 0.5f).GetRotated(-Angle);
	}

	const FVector2D Scale = LocalScale * MaskSize;
	const FVector2D Bias = LocalOrigin * MaskSize;

	FVector2D SamplePos = Position * Scale + Bias;
	SamplePos = SamplePos.GetRotated(Angle);
	return TextureData->Sample(SamplePos, TextureChannel);
}