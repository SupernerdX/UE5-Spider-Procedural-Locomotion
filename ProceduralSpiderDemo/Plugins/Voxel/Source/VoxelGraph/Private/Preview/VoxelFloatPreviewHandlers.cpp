// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Preview/VoxelFloatPreviewHandlers.h"

void FVoxelPreviewHandler_Grayscale_Float::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<float>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelFloatBuffer>(Value.Get<float>());
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelFloatBuffer>();
	}

	const FFloatInterval MinMax = FVoxelUtilities::GetMinMaxSafe(Buffer->View());
	MinValue = MinMax.Min;
	MaxValue = MinMax.Max;
}

void FVoxelPreviewHandler_Grayscale_Float::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const float Scalar = (*Buffer)[Index];

		if (!FMath::IsFinite(Scalar))
		{
			OutColor = FColor::Magenta;
			return;
		}

		const float ScaledValue = GetNormalizedValue(Scalar, MinValue, MaxValue);
		OutColor = FLinearColor(ScaledValue, ScaledValue, ScaledValue);
	});
}

TArray<FString> FVoxelPreviewHandler_Grayscale_Float::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	return { ValueToString(bFullValue, "", (*Buffer)[Index]) };
}

TArray<FString> FVoxelPreviewHandler_Grayscale_Float::GetMinValue(const bool bFullValue) const
{
	return { ValueToString(bFullValue, "", MinValue) };
}

TArray<FString> FVoxelPreviewHandler_Grayscale_Float::GetMaxValue(const bool bFullValue) const
{
	return { ValueToString(bFullValue, "", MaxValue) };
}

FString FVoxelPreviewHandler_Grayscale_Float::ValueToString(const bool bFullValue, const FString& Prefix, const float Value)
{
	FString Result;
	if (bFullValue)
	{
		Result = LexToSanitizedString(Value);
	}
	else
	{
		Result = FVoxelUtilities::NumberToString(Value);
	}

	if (Value >= 0)
	{
		Result = " " + Result;
	}

	return Prefix + Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_Vector2D::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<FVector2D>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelVector2DBuffer>(FVector2f(Value.Get<FVector2D>()));
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelVector2DBuffer>();
	}

	const FFloatInterval MinMaxX = FVoxelUtilities::GetMinMaxSafe(Buffer->X.View());
	const FFloatInterval MinMaxY = FVoxelUtilities::GetMinMaxSafe(Buffer->Y.View());

	MinValue.X = MinMaxX.Min;
	MinValue.Y = MinMaxY.Min;

	MaxValue.X = MinMaxX.Max;
	MaxValue.Y = MinMaxY.Max;
}

void FVoxelPreviewHandler_Vector2D::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const FVector2f Vector2D = (*Buffer)[Index];

		if (Vector2D.ContainsNaN())
		{
			OutColor = FColor::Magenta;
			return;
		}

		OutColor = FLinearColor(
			GetNormalizedValue(Vector2D.X, MinValue.X, MaxValue.X),
			GetNormalizedValue(Vector2D.Y, MinValue.Y, MaxValue.Y),
			0);
	});
}

TArray<FString> FVoxelPreviewHandler_Vector2D::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	const FVector2f& Value = (*Buffer)[Index];
	return {
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "X=", Value.X),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "Y=", Value.Y),
	};
}

TArray<FString> FVoxelPreviewHandler_Vector2D::GetMinValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "X=", MinValue.X),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "Y=", MinValue.Y),
	};
}

TArray<FString> FVoxelPreviewHandler_Vector2D::GetMaxValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "X=", MaxValue.X),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "Y=", MaxValue.Y),
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_Vector::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<FVector>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelVectorBuffer>(FVector3f(Value.Get<FVector>()));
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelVectorBuffer>();
	}

	const FFloatInterval MinMaxX = FVoxelUtilities::GetMinMaxSafe(Buffer->X.View());
	const FFloatInterval MinMaxY = FVoxelUtilities::GetMinMaxSafe(Buffer->Y.View());
	const FFloatInterval MinMaxZ = FVoxelUtilities::GetMinMaxSafe(Buffer->Z.View());

	MinValue.X = MinMaxX.Min;
	MinValue.Y = MinMaxY.Min;
	MinValue.Z = MinMaxZ.Min;

	MaxValue.X = MinMaxX.Max;
	MaxValue.Y = MinMaxY.Max;
	MaxValue.Z = MinMaxZ.Max;
}

void FVoxelPreviewHandler_Vector::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const FVector3f Vector = (*Buffer)[Index];

		if (Vector.ContainsNaN())
		{
			OutColor = FColor::Magenta;
			return;
		}

		OutColor = FLinearColor(
			GetNormalizedValue(Vector.X, MinValue.X, MaxValue.X),
			GetNormalizedValue(Vector.Y, MinValue.Y, MaxValue.Y),
			GetNormalizedValue(Vector.Z, MinValue.Z, MaxValue.Z));
	});
}

TArray<FString> FVoxelPreviewHandler_Vector::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	const FVector3f& Value = (*Buffer)[Index];
	return {
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "X=", Value.X),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "Y=", Value.Y),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "Z=", Value.Z),
	};
}

TArray<FString> FVoxelPreviewHandler_Vector::GetMinValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "X=", MinValue.X),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "Y=", MinValue.Y),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "Z=", MinValue.Z),
	};
}

TArray<FString> FVoxelPreviewHandler_Vector::GetMaxValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "X=", MaxValue.X),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "Y=", MaxValue.Y),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "Z=", MaxValue.Z),
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_Color::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<FLinearColor>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelLinearColorBuffer>(Value.Get<FLinearColor>());
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelLinearColorBuffer>();
	}

	const FFloatInterval MinMaxR = FVoxelUtilities::GetMinMaxSafe(Buffer->R.View());
	const FFloatInterval MinMaxG = FVoxelUtilities::GetMinMaxSafe(Buffer->G.View());
	const FFloatInterval MinMaxB = FVoxelUtilities::GetMinMaxSafe(Buffer->B.View());
	const FFloatInterval MinMaxA = FVoxelUtilities::GetMinMaxSafe(Buffer->A.View());

	MinValue.R = MinMaxR.Min;
	MinValue.G = MinMaxG.Min;
	MinValue.B = MinMaxB.Min;
	MinValue.A = MinMaxA.Min;

	MaxValue.R = MinMaxR.Max;
	MaxValue.G = MinMaxG.Max;
	MaxValue.B = MinMaxB.Max;
	MaxValue.A = MinMaxA.Max;
}

void FVoxelPreviewHandler_Color::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const FLinearColor Color = (*Buffer)[Index];

		if (!FMath::IsFinite(Color.R) ||
			!FMath::IsFinite(Color.G) ||
			!FMath::IsFinite(Color.B) ||
			!FMath::IsFinite(Color.A))
		{
			OutColor = FColor::Magenta;
			return;
		}

		OutColor = FLinearColor(
			GetNormalizedValue(Color.R, MinValue.R, MaxValue.R),
			GetNormalizedValue(Color.G, MinValue.G, MaxValue.G),
			GetNormalizedValue(Color.B, MinValue.B, MaxValue.B));
	});
}

TArray<FString> FVoxelPreviewHandler_Color::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	const FLinearColor& Value = (*Buffer)[Index];
	return {
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "R=", Value.R),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "G=", Value.G),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "B=", Value.B),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "A=", Value.A),
	};
}

TArray<FString> FVoxelPreviewHandler_Color::GetMinValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "R=", MinValue.R),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "G=", MinValue.G),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "B=", MinValue.B),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "A=", MinValue.A),
	};
}

TArray<FString> FVoxelPreviewHandler_Color::GetMaxValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "R=", MaxValue.R),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "G=", MaxValue.G),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "B=", MaxValue.B),
		FVoxelPreviewHandler_Grayscale_Float::ValueToString(bFullValue, "A=", MaxValue.A),
	};
}