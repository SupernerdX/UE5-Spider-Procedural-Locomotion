// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Preview/VoxelDoublePreviewHandlers.h"

void FVoxelPreviewHandler_Grayscale_Double::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<double>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelDoubleBuffer>(Value.Get<double>());
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelDoubleBuffer>();
	}

	const FDoubleInterval MinMax = FVoxelUtilities::GetMinMaxSafe(Buffer->View());
	MinValue = MinMax.Min;
	MaxValue = MinMax.Max;
}

void FVoxelPreviewHandler_Grayscale_Double::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const double Scalar = (*Buffer)[Index];

		if (!FMath::IsFinite(Scalar))
		{
			OutColor = FColor::Magenta;
			return;
		}

		const float ScaledValue = GetNormalizedValue(Scalar, MinValue, MaxValue);
		OutColor = FLinearColor(ScaledValue, ScaledValue, ScaledValue);
	});
}

TArray<FString> FVoxelPreviewHandler_Grayscale_Double::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	return { ValueToString(bFullValue, "", (*Buffer)[Index]) };
}

TArray<FString> FVoxelPreviewHandler_Grayscale_Double::GetMinValue(const bool bFullValue) const
{
	return { ValueToString(bFullValue, "", MinValue) };
}

TArray<FString> FVoxelPreviewHandler_Grayscale_Double::GetMaxValue(const bool bFullValue) const
{
	return { ValueToString(bFullValue, "", MaxValue) };
}

FString FVoxelPreviewHandler_Grayscale_Double::ValueToString(const bool bFullValue, const FString& Prefix, const double Value)
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

void FVoxelPreviewHandler_DoubleVector2D::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<FVoxelDoubleVector2D>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelDoubleVector2DBuffer>(Value.Get<FVoxelDoubleVector2D>());
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelDoubleVector2DBuffer>();
	}

	const FDoubleInterval MinMaxX = FVoxelUtilities::GetMinMaxSafe(Buffer->X.View());
	const FDoubleInterval MinMaxY = FVoxelUtilities::GetMinMaxSafe(Buffer->Y.View());

	MinValue.X = MinMaxX.Min;
	MinValue.Y = MinMaxY.Min;

	MaxValue.X = MinMaxX.Max;
	MaxValue.Y = MinMaxY.Max;
}

void FVoxelPreviewHandler_DoubleVector2D::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const FVoxelDoubleVector2D Vector2D = (*Buffer)[Index];

		if (!FMath::IsFinite(Vector2D.X) ||
			!FMath::IsFinite(Vector2D.Y))
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

TArray<FString> FVoxelPreviewHandler_DoubleVector2D::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	const FVoxelDoubleVector2D& Value = (*Buffer)[Index];
	return {
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "X=", Value.X),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "Y=", Value.Y),
	};
}

TArray<FString> FVoxelPreviewHandler_DoubleVector2D::GetMinValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "X=", MinValue.X),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "Y=", MinValue.Y),
	};
}

TArray<FString> FVoxelPreviewHandler_DoubleVector2D::GetMaxValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "X=", MaxValue.X),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "Y=", MaxValue.Y),
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_DoubleVector::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<FVoxelDoubleVector>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelDoubleVectorBuffer>(Value.Get<FVoxelDoubleVector>());
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelDoubleVectorBuffer>();
	}

	const FDoubleInterval MinMaxX = FVoxelUtilities::GetMinMaxSafe(Buffer->X.View());
	const FDoubleInterval MinMaxY = FVoxelUtilities::GetMinMaxSafe(Buffer->Y.View());
	const FDoubleInterval MinMaxZ = FVoxelUtilities::GetMinMaxSafe(Buffer->Z.View());

	MinValue.X = MinMaxX.Min;
	MinValue.Y = MinMaxY.Min;
	MinValue.Z = MinMaxZ.Min;

	MaxValue.X = MinMaxX.Max;
	MaxValue.Y = MinMaxY.Max;
	MaxValue.Z = MinMaxZ.Max;
}

void FVoxelPreviewHandler_DoubleVector::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const FVoxelDoubleVector Vector = (*Buffer)[Index];

		if (!FMath::IsFinite(Vector.X) ||
			!FMath::IsFinite(Vector.Y) ||
			!FMath::IsFinite(Vector.Z))
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

TArray<FString> FVoxelPreviewHandler_DoubleVector::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	const FVoxelDoubleVector& Value = (*Buffer)[Index];
	return {
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "X=", Value.X),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "Y=", Value.Y),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "Z=", Value.Z),
	};
}

TArray<FString> FVoxelPreviewHandler_DoubleVector::GetMinValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "X=", MinValue.X),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "Y=", MinValue.Y),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "Z=", MinValue.Z),
	};
}

TArray<FString> FVoxelPreviewHandler_DoubleVector::GetMaxValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "X=", MaxValue.X),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "Y=", MaxValue.Y),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "Z=", MaxValue.Z),
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_DoubleColor::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<FVoxelDoubleLinearColor>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelDoubleLinearColorBuffer>(Value.Get<FVoxelDoubleLinearColor>());
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelDoubleLinearColorBuffer>();
	}

	const FDoubleInterval MinMaxR = FVoxelUtilities::GetMinMaxSafe(Buffer->R.View());
	const FDoubleInterval MinMaxG = FVoxelUtilities::GetMinMaxSafe(Buffer->G.View());
	const FDoubleInterval MinMaxB = FVoxelUtilities::GetMinMaxSafe(Buffer->B.View());
	const FDoubleInterval MinMaxA = FVoxelUtilities::GetMinMaxSafe(Buffer->A.View());

	MinValue.R = MinMaxR.Min;
	MinValue.G = MinMaxG.Min;
	MinValue.B = MinMaxB.Min;
	MinValue.A = MinMaxA.Min;

	MaxValue.R = MinMaxR.Max;
	MaxValue.G = MinMaxG.Max;
	MaxValue.B = MinMaxB.Max;
	MaxValue.A = MinMaxA.Max;
}

void FVoxelPreviewHandler_DoubleColor::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const FVoxelDoubleLinearColor Color = (*Buffer)[Index];

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

TArray<FString> FVoxelPreviewHandler_DoubleColor::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	const FVoxelDoubleLinearColor& Value = (*Buffer)[Index];
	return {
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "R=", Value.R),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "G=", Value.G),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "B=", Value.B),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "A=", Value.A),
	};
}

TArray<FString> FVoxelPreviewHandler_DoubleColor::GetMinValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "R=", MinValue.R),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "G=", MinValue.G),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "B=", MinValue.B),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "A=", MinValue.A),
	};
}

TArray<FString> FVoxelPreviewHandler_DoubleColor::GetMaxValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "R=", MaxValue.R),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "G=", MaxValue.G),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "B=", MaxValue.B),
		FVoxelPreviewHandler_Grayscale_Double::ValueToString(bFullValue, "A=", MaxValue.A),
	};
}