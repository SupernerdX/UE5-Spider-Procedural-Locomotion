// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Preview/VoxelIntegerPreviewHandlers.h"
#include "Utilities/VoxelBufferConversionUtilities.h"

void FVoxelPreviewHandler_Byte::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<uint8>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelByteBuffer>(Value.Get<uint8>());
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelByteBuffer>();
	}

	const FInt32Interval MinMax = FVoxelUtilities::GetMinMax(Buffer->View());
	MinValue = MinMax.Min;
	MaxValue = MinMax.Max;
}

void FVoxelPreviewHandler_Byte::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const uint8 Scalar = (*Buffer)[Index];

		const float ScaledValue = GetNormalizedValue(Scalar, MinValue, MaxValue);
		OutColor = FLinearColor(ScaledValue, ScaledValue, ScaledValue);
	});
}

TArray<FString> FVoxelPreviewHandler_Byte::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	return { LexToString((*Buffer)[Index]) };
}

TArray<FString> FVoxelPreviewHandler_Byte::GetMinValue(const bool bFullValue) const
{
	return { LexToString(MinValue) };
}

TArray<FString> FVoxelPreviewHandler_Byte::GetMaxValue(const bool bFullValue) const
{
	return { LexToString(MaxValue) };
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_Integer::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<int32>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelInt64Buffer>(Value.Get<int32>());
	}
	else if (Value.Is<int64>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelInt64Buffer>(Value.Get<int64>());
	}
	else if (Value.Is<FVoxelInt32Buffer>())
	{
		Buffer = MakeSharedCopy(FVoxelBufferConversionUtilities::Int32ToInt64(Value.Get<FVoxelInt32Buffer>()));
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelInt64Buffer>();
	}

	const TInterval<int64> MinMax = FVoxelUtilities::GetMinMax(Buffer->View());
	MinValue = MinMax.Min;
	MaxValue = MinMax.Max;
}

void FVoxelPreviewHandler_Integer::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const int32 Scalar = (*Buffer)[Index];

		const float ScaledValue = GetNormalizedValue(Scalar, MinValue, MaxValue);
		OutColor = FLinearColor(ScaledValue, ScaledValue, ScaledValue);
	});
}

TArray<FString> FVoxelPreviewHandler_Integer::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	return { ValueToString("", (*Buffer)[Index]) };
}

TArray<FString> FVoxelPreviewHandler_Integer::GetMinValue(const bool bFullValue) const
{
	return { ValueToString("", MinValue) };
}

TArray<FString> FVoxelPreviewHandler_Integer::GetMaxValue(const bool bFullValue) const
{
	return { ValueToString("", MaxValue) };
}

FString FVoxelPreviewHandler_Integer::ValueToString(const FString& Prefix, const int64 Value)
{
	FString Result = LexToString(Value);

	if (Value >= 0)
	{
		Result = " " + Result;
	}

	return Prefix + Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_IntPoint::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<FIntPoint>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelInt64PointBuffer>(FInt64Point(Value.Get<FIntPoint>()));
	}
	else if (Value.Is<FInt64Point>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelInt64PointBuffer>(Value.Get<FInt64Point>());
	}
	else if (Value.Is<FVoxelIntPointBuffer>())
	{
		Buffer = MakeSharedCopy(FVoxelBufferConversionUtilities::Int32ToInt64(Value.Get<FVoxelIntPointBuffer>()));
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelInt64PointBuffer>();
	}

	const TInterval<int64> MinMaxX = FVoxelUtilities::GetMinMax(Buffer->X.View());
	const TInterval<int64> MinMaxY = FVoxelUtilities::GetMinMax(Buffer->Y.View());

	MinValue.X = MinMaxX.Min;
	MinValue.Y = MinMaxY.Min;

	MaxValue.X = MinMaxX.Max;
	MaxValue.Y = MinMaxY.Max;
}

void FVoxelPreviewHandler_IntPoint::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const FInt64Point IntPoint = (*Buffer)[Index];

		OutColor = FLinearColor(
			GetNormalizedValue(IntPoint.X, MinValue.X, MaxValue.X),
			GetNormalizedValue(IntPoint.Y, MinValue.Y, MaxValue.Y),
			0);
	});
}

TArray<FString> FVoxelPreviewHandler_IntPoint::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	const FInt64Point Value = (*Buffer)[Index];

	return {
		FVoxelPreviewHandler_Integer::ValueToString("X=", Value.X),
		FVoxelPreviewHandler_Integer::ValueToString("Y=", Value.Y),
	};
}

TArray<FString> FVoxelPreviewHandler_IntPoint::GetMinValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Integer::ValueToString("X=", MinValue.X),
		FVoxelPreviewHandler_Integer::ValueToString("Y=", MinValue.Y),
	};
}

TArray<FString> FVoxelPreviewHandler_IntPoint::GetMaxValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Integer::ValueToString("X=", MaxValue.X),
		FVoxelPreviewHandler_Integer::ValueToString("Y=", MaxValue.Y),
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_IntVector::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<FIntVector>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelInt64VectorBuffer>(FInt64Vector(Value.Get<FIntVector>()));
	}
	else if (Value.Is<FInt64Vector>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelInt64VectorBuffer>(Value.Get<FInt64Vector>());
	}
	else if (Value.Is<FVoxelIntVectorBuffer>())
	{
		Buffer = MakeSharedCopy(FVoxelBufferConversionUtilities::Int32ToInt64(Value.Get<FVoxelIntVectorBuffer>()));
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelInt64VectorBuffer>();
	}

	const TInterval<int64> MinMaxX = FVoxelUtilities::GetMinMax(Buffer->X.View());
	const TInterval<int64> MinMaxY = FVoxelUtilities::GetMinMax(Buffer->Y.View());
	const TInterval<int64> MinMaxZ = FVoxelUtilities::GetMinMax(Buffer->Z.View());

	MinValue.X = MinMaxX.Min;
	MinValue.Y = MinMaxY.Min;
	MinValue.Z = MinMaxZ.Min;

	MaxValue.X = MinMaxX.Max;
	MaxValue.Y = MinMaxY.Max;
	MaxValue.Z = MinMaxZ.Max;
}

void FVoxelPreviewHandler_IntVector::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const FInt64Vector Vector = (*Buffer)[Index];

		OutColor = FLinearColor(
			GetNormalizedValue(Vector.X, MinValue.X, MaxValue.X),
			GetNormalizedValue(Vector.Y, MinValue.Y, MaxValue.Y),
			GetNormalizedValue(Vector.Z, MinValue.Z, MaxValue.Z));
	});
}

TArray<FString> FVoxelPreviewHandler_IntVector::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	const FInt64Vector Value = (*Buffer)[Index];

	return {
		FVoxelPreviewHandler_Integer::ValueToString("X=", Value.X),
		FVoxelPreviewHandler_Integer::ValueToString("Y=", Value.Y),
		FVoxelPreviewHandler_Integer::ValueToString("Z=", Value.Z),
	};
}

TArray<FString> FVoxelPreviewHandler_IntVector::GetMinValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Integer::ValueToString("X=", MinValue.X),
		FVoxelPreviewHandler_Integer::ValueToString("Y=", MinValue.Y),
		FVoxelPreviewHandler_Integer::ValueToString("Z=", MinValue.Z),
	};
}

TArray<FString> FVoxelPreviewHandler_IntVector::GetMaxValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Integer::ValueToString("X=", MaxValue.X),
		FVoxelPreviewHandler_Integer::ValueToString("Y=", MaxValue.Y),
		FVoxelPreviewHandler_Integer::ValueToString("Z=", MaxValue.Z),
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_IntVector4::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<FIntVector4>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelInt64Vector4Buffer>(FInt64Vector4(Value.Get<FIntVector4>()));
	}
	else if (Value.Is<FInt64Vector4>())
	{
		bUniform = true;
		Buffer = MakeShared<FVoxelInt64Vector4Buffer>(Value.Get<FInt64Vector4>());
	}
	else if (Value.Is<FVoxelIntVector4Buffer>())
	{
		Buffer = MakeSharedCopy(FVoxelBufferConversionUtilities::Int32ToInt64(Value.Get<FVoxelIntVector4Buffer>()));
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelInt64Vector4Buffer>();
	}

	const TInterval<int64> MinMaxR = FVoxelUtilities::GetMinMax(Buffer->X.View());
	const TInterval<int64> MinMaxG = FVoxelUtilities::GetMinMax(Buffer->Y.View());
	const TInterval<int64> MinMaxB = FVoxelUtilities::GetMinMax(Buffer->Z.View());
	const TInterval<int64> MinMaxA = FVoxelUtilities::GetMinMax(Buffer->W.View());

	MinValue.X = MinMaxR.Min;
	MinValue.Y = MinMaxG.Min;
	MinValue.Z = MinMaxB.Min;
	MinValue.W = MinMaxA.Min;

	MaxValue.X = MinMaxR.Max;
	MaxValue.Y = MinMaxG.Max;
	MaxValue.Z = MinMaxB.Max;
	MaxValue.W = MinMaxA.Max;
}

void FVoxelPreviewHandler_IntVector4::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const FInt64Vector4 Vector4 = (*Buffer)[Index];

		OutColor = FLinearColor(
			GetNormalizedValue(Vector4.X, MinValue.X, MaxValue.X),
			GetNormalizedValue(Vector4.Y, MinValue.Y, MaxValue.Y),
			GetNormalizedValue(Vector4.Z, MinValue.Z, MaxValue.Z));
	});
}

TArray<FString> FVoxelPreviewHandler_IntVector4::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return { "Invalid" };
	}

	const FInt64Vector4 Value = (*Buffer)[Index];

	return {
		FVoxelPreviewHandler_Integer::ValueToString("X=", Value.X),
		FVoxelPreviewHandler_Integer::ValueToString("Y=", Value.Y),
		FVoxelPreviewHandler_Integer::ValueToString("Z=", Value.Z),
		FVoxelPreviewHandler_Integer::ValueToString("W=", Value.W),
	};
}

TArray<FString> FVoxelPreviewHandler_IntVector4::GetMinValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Integer::ValueToString("X=", MinValue.X),
		FVoxelPreviewHandler_Integer::ValueToString("Y=", MinValue.Y),
		FVoxelPreviewHandler_Integer::ValueToString("Z=", MinValue.Z),
		FVoxelPreviewHandler_Integer::ValueToString("W=", MinValue.W),
	};
}

TArray<FString> FVoxelPreviewHandler_IntVector4::GetMaxValue(const bool bFullValue) const
{
	return {
		FVoxelPreviewHandler_Integer::ValueToString("X=", MaxValue.X),
		FVoxelPreviewHandler_Integer::ValueToString("Y=", MaxValue.Y),
		FVoxelPreviewHandler_Integer::ValueToString("Z=", MaxValue.Z),
		FVoxelPreviewHandler_Integer::ValueToString("W=", MaxValue.W),
	};
}