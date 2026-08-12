// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Preview/VoxelGenericPreviewHandler.h"

void FVoxelGenericPreviewHandler::Initialize(const FVoxelRuntimePinValue& Value)
{
	if (Value.IsBuffer())
	{
		Buffer = Value.GetSharedStruct<FVoxelBuffer>();
	}
	else
	{
		Buffer = FVoxelBuffer::MakeConstant(Value);
	}
}

bool FVoxelGenericPreviewHandler::SupportsType(const FVoxelPinType& Type) const
{
	for (const TSharedRef<const FVoxelPreviewHandler>& Handler : GetHandlers())
	{
		if (Handler->GetStruct() == StaticStruct())
		{
			continue;
		}

		if (Handler->SupportsType(Type))
		{
			return false;
		}
	}

	return true;
}

void FVoxelGenericPreviewHandler::BuildStats(const FAddStat& AddStat)
{
	ON_SCOPE_EXIT
	{
		Super::BuildStats(AddStat);
	};

	AddStat(
		"Value",
		true,
		MakeWeakPtrLambda(this, [this]() -> FString
		{
			const FIntPoint Position = FVoxelUtilities::FloorToInt(MousePosition);
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, FVoxelUtilities::Clamp(Position, 0, PreviewSize - 1));

			return GetValue(Index);
		}),
		MakeWeakPtrLambda(this, [this]() -> TArray<FString>
		{
			const FIntPoint Position = FVoxelUtilities::FloorToInt(MousePosition);
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, FVoxelUtilities::Clamp(Position, 0, PreviewSize - 1));

			return { GetValue(Index) };
		}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FString FVoxelGenericPreviewHandler::GetValue(const int32 Index) const
{
	if (!Buffer ||
		!Buffer->IsValidIndex_Slow(Index))
	{
		return "Invalid";
	}

	const FVoxelRuntimePinValue Value = Buffer->GetGeneric(Index);
	return Value.ToDebugString(bShowFullValue, true);
}