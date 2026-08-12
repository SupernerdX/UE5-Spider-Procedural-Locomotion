// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Preview/VoxelEnumPreviewHandler.h"

void FVoxelPreviewHandler_Enum::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<uint8>())
	{
		Buffer = MakeShared<FVoxelByteBuffer>(Value.Get<uint8>());
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelByteBuffer>();
	}

	EnumType = Value.GetType().GetInnerType();
}

void FVoxelPreviewHandler_Enum::BuildStats(const FAddStat& AddStat)
{
	auto ValueLambda = [this, Enum = EnumType.GetEnum()]() -> FString
	{
		const FIntPoint Position = FVoxelUtilities::FloorToInt(MousePosition);
		const int32 Index = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, FVoxelUtilities::Clamp(Position, 0, PreviewSize - 1));

		if (!Buffer->IsValidIndex(Index))
		{
			return "Invalid";
		}

		if (!Enum)
		{
			return "Invalid";
		}

		return Enum->GetDisplayNameTextByValue((*Buffer)[Index]).ToString();
	};

	AddStat(
		"Value",
		true,
		MakeWeakPtrLambda(this, ValueLambda),
		MakeWeakPtrLambda(this, [ValueLambda]() -> TArray<FString>
		{
			return { ValueLambda() };
		}));

	Super::BuildStats(AddStat);
}

void FVoxelPreviewHandler_Enum::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	const UEnum* Enum = EnumType.GetEnum();
	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const uint8 EnumValue = (*Buffer)[Index];

		if (!Enum ||
			!Enum->IsValidEnumValue(EnumValue))
		{
			OutColor = FColor::Magenta;
			return;
		}

		OutColor = FLinearColor::IntToDistinctColor(EnumValue, 1.f, 0.75f, 90.f);
	});
}