// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Preview/VoxelScalarPreviewHandler.h"

void FVoxelScalarPreviewHandler::BuildStats(const FAddStat& AddStat)
{
	ON_SCOPE_EXIT
	{
		Super::BuildStats(AddStat);
	};

	AddStat(
		"Value",
		true,
		MakeWeakPtrLambda(this, [this]
		{
			const FIntPoint Position = FVoxelUtilities::FloorToInt(MousePosition);
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, FVoxelUtilities::Clamp(Position, 0, PreviewSize - 1));

			FString Result;
			const TArray<FString> Values = GetValueAt(Index, true);
			for (const FString& Value : Values)
			{
				if (!Result.IsEmpty())
				{
					Result += ", ";
				}
				Result += Value;
			}
			return Result;
		}),
		MakeWeakPtrLambda(this, [this]() -> TArray<FString>
		{
			const FIntPoint Position = FVoxelUtilities::FloorToInt(MousePosition);
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, FVoxelUtilities::Clamp(Position, 0, PreviewSize - 1));

			return GetValueAt(Index, bShowFullValue);
		}));

	if (bUniform)
	{
		return;
	}

	AddStat(
		"Min Value",
		true,
		MakeWeakPtrLambda(this, [this]
		{
			FString Result;
			const TArray<FString> Values = GetMinValue(true);
			for (const FString& Value : Values)
			{
				if (!Result.IsEmpty())
				{
					Result += ", ";
				}
				Result += Value;
			}
			return Result;
		}),
		MakeWeakPtrLambda(this, [this]
		{
			return GetMinValue(bShowFullValue);
		}));

	AddStat(
		"Max Value",
		true,
		MakeWeakPtrLambda(this, [this]
		{
			FString Result;
			const TArray<FString> Values = GetMaxValue(true);
			for (const FString& Value : Values)
			{
				if (!Result.IsEmpty())
				{
					Result += ", ";
				}
				Result += Value;
			}
			return Result;
		}),
		MakeWeakPtrLambda(this, [this]
		{
			return GetMaxValue(bShowFullValue);
		}));
}