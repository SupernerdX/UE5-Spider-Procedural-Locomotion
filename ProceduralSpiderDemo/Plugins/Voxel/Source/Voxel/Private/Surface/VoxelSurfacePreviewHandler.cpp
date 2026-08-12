// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSurfacePreviewHandler.h"

void FVoxelPreviewHandler_Surface::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.Is<FVoxelSurfaceTypeBlend>())
	{
		Buffer = MakeShared<FVoxelSurfaceTypeBlendBuffer>(Value.Get<FVoxelSurfaceTypeBlend>());
	}
	else
	{
		Buffer = Value.GetSharedStruct<FVoxelSurfaceTypeBlendBuffer>();
	}
}

void FVoxelPreviewHandler_Surface::BuildStats(const FAddStat& AddStat)
{
	AddStat(
		"Material",
		true,
		MakeWeakPtrLambda(this, [this]() -> FString
		{
			return "The material at the position being previewed";
		}),
		MakeWeakPtrLambda(this, [this]() -> TArray<FString>
		{
			const FIntPoint Position = FVoxelUtilities::FloorToInt(MousePosition);
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, FVoxelUtilities::Clamp(Position, 0, PreviewSize - 1));

			if (!Buffer->IsValidIndex(Index))
			{
				return { "Invalid" };
			}

			const TVoxelArray<FVoxelSurfaceTypeBlendLayer> Layers = (*Buffer)[Index].GetLayersSortedByWeight();
			if (Layers.Num() == 0)
			{
				return { "null" };
			}

			TArray<FString> Result;
			for (const FVoxelSurfaceTypeBlendLayer& Layer : Layers)
			{
				Result.Add(Layer.GetSurfaceName());
				Result.Add(Layer.GetWeightString());
			}
			return Result;
		}));

	Super::BuildStats(AddStat);
}

void FVoxelPreviewHandler_Surface::GetColors(const TVoxelArrayView<FLinearColor> Colors) const
{
	VOXEL_FUNCTION_COUNTER();

	Voxel::ParallelFor(Colors, [&](FLinearColor& OutColor, const int32 Index)
	{
		const FVoxelSurfaceTypeBlend Value = (*Buffer)[Index];

		OutColor = Value.GetTopLayer().Type.GetDebugColor();
	});
}