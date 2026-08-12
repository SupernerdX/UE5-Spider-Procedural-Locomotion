// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Preview/VoxelBoolPreviewHandler.h"

void FVoxelPreviewHandler_Bool::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<bool>())
	{
		Buffer = MakeShared<FVoxelBoolBuffer>(Value.Get<bool>());
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelBoolBuffer>();
	}
}

void FVoxelPreviewHandler_Bool::BuildStats(const FAddStat& AddStat)
{
	const auto GetValue = [this]() -> FString
	{
		const FIntPoint Position = FVoxelUtilities::FloorToInt(MousePosition);
		const int32 Index = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, FVoxelUtilities::Clamp(Position, 0, PreviewSize - 1));

		if (!Buffer->IsValidIndex(Index))
		{
			return "Invalid";
		}
		return (*Buffer)[Index] ? "true" : "false";
	};

	AddStat(
		"Value",
		true,
		MakeWeakPtrLambda(this, GetValue),
		MakeWeakPtrLambda(this, [this, GetValue]() -> TArray<FString>
		{
			return { GetValue() };
		}));

	Super::BuildStats(AddStat);
}

void FVoxelPreviewHandler_Bool::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const bool bValue = (*Buffer)[Index];

		const float ScaledValue = bValue ? 1.f : 0.f;
		OutColor = FLinearColor(ScaledValue, ScaledValue, ScaledValue);
	});
}