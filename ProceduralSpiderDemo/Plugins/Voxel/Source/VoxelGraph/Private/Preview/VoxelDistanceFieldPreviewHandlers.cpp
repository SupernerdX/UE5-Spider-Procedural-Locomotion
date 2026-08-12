// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Preview/VoxelDistanceFieldPreviewHandlers.h"

void FVoxelPreviewHandler_DistanceField_Base::BuildStats(const FAddStat& AddStat)
{
	AddStat(
		"Value",
		true,
		MakeWeakPtrLambda(this, [this]() -> FString
		{
			const FIntPoint Position = FVoxelUtilities::FloorToInt(MousePosition);
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, FVoxelUtilities::Clamp(Position, 0, PreviewSize - 1));

			return GetValueAt(Index, true);
		}),
		MakeWeakPtrLambda(this, [this]() -> TArray<FString>
		{
			const FIntPoint Position = FVoxelUtilities::FloorToInt(MousePosition);
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, FVoxelUtilities::Clamp(Position, 0, PreviewSize - 1));

			return { GetValueAt(Index, bShowFullValue) };
		}));

	AddStat(
		"Min Value",
		true,
		MakeWeakPtrLambda(this, [this]
		{
			return GetMinValue(true);
		}),
		MakeWeakPtrLambda(this, [this]() -> TArray<FString>
		{
			return { GetMinValue(bShowFullValue) };
		}));

	AddStat(
		"Max Value",
		true,
		MakeWeakPtrLambda(this, [this]
		{
			return GetMaxValue(true);
		}),
		MakeWeakPtrLambda(this, [this]() -> TArray<FString>
		{
			return { GetMaxValue(bShowFullValue) };
		}));

	Super::BuildStats(AddStat);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_DistanceField_Float::Initialize(const FVoxelRuntimePinValue& Value)
{
	Buffer = Value.GetSharedStruct<FVoxelFloatBuffer>();

	const FFloatInterval MinMax = FVoxelUtilities::GetMinMaxSafe(Buffer->View());
	MinValue = MinMax.Min;
	MaxValue = MinMax.Max;
}

void FVoxelPreviewHandler_DistanceField_Float::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	const float Divisor = FMath::Max(FMath::Abs(MinValue), FMath::Abs(MaxValue));

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const float Scalar = (*Buffer)[Index];

		if (!FMath::IsFinite(Scalar))
		{
			OutColor = FColor::Magenta;
			return;
		}

		const float ScaledValue = Scalar / Divisor;
		OutColor = FVoxelUtilities::GetDistanceFieldColor(ScaledValue);
	});
}

FString FVoxelPreviewHandler_DistanceField_Float::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return "Invalid";
	}

	if (bFullValue)
	{
		return LexToSanitizedString((*Buffer)[Index]);
	}

	return FVoxelUtilities::NumberToString((*Buffer)[Index]);
}

FString FVoxelPreviewHandler_DistanceField_Float::GetMinValue(const bool bFullValue) const
{
	if (bFullValue)
	{
		return LexToSanitizedString(MinValue);
	}

	return FVoxelUtilities::NumberToString(MinValue);
}

FString FVoxelPreviewHandler_DistanceField_Float::GetMaxValue(const bool bFullValue) const
{
	if (bFullValue)
	{
		return LexToSanitizedString(MaxValue);
	}

	return FVoxelUtilities::NumberToString(MaxValue);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler_DistanceField_Double::Initialize(const FVoxelRuntimePinValue& Value)
{
	Buffer = Value.GetSharedStruct<FVoxelDoubleBuffer>();

	const FDoubleInterval MinMax = FVoxelUtilities::GetMinMaxSafe(Buffer->View());
	MinValue = MinMax.Min;
	MaxValue = MinMax.Max;
}

void FVoxelPreviewHandler_DistanceField_Double::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	const float Divisor = FMath::Max(FMath::Abs(MinValue), FMath::Abs(MaxValue));

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const double Scalar = (*Buffer)[Index];

		if (!FMath::IsFinite(Scalar))
		{
			OutColor = FColor::Magenta;
			return;
		}

		const double ScaledValue = Scalar / Divisor;
		OutColor = FVoxelUtilities::GetDistanceFieldColor(ScaledValue);
	});
}

FString FVoxelPreviewHandler_DistanceField_Double::GetValueAt(const int32 Index, const bool bFullValue) const
{
	if (!Buffer->IsValidIndex(Index))
	{
		return "Invalid";
	}

	if (bFullValue)
	{
		return LexToSanitizedString((*Buffer)[Index]);
	}

	return FVoxelUtilities::NumberToString((*Buffer)[Index]);
}

FString FVoxelPreviewHandler_DistanceField_Double::GetMinValue(const bool bFullValue) const
{
	if (bFullValue)
	{
		return LexToSanitizedString(MinValue);
	}

	return FVoxelUtilities::NumberToString(MinValue);
}

FString FVoxelPreviewHandler_DistanceField_Double::GetMaxValue(const bool bFullValue) const
{
	if (bFullValue)
	{
		return LexToSanitizedString(MaxValue);
	}

	return FVoxelUtilities::NumberToString(MaxValue);
}