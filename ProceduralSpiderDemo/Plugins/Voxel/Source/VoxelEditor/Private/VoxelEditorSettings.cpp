// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorSettings.h"
#include "VoxelGlobalActionsExtender.h"
#include "PlaceStamps/VoxelPlaceStampsSubsystem.h"

void UVoxelEditorSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property == &FindFPropertyChecked(UVoxelEditorSettings, bEnableToolbarActions))
	{
		if (bEnableToolbarActions)
		{
			FVoxelGlobalActionsExtender::RegisterMenu();
		}
		else
		{
			FVoxelGlobalActionsExtender::UnregisterMenu();
		}
	}
	else if (PropertyChangedEvent.Property == &FindFPropertyChecked(UVoxelEditorSettings, bEnablePlaceStampsDrawer))
	{
		if (bEnablePlaceStampsDrawer)
		{
			UVoxelPlaceStampsSubsystem::RegisterDrawer();
		}
		else
		{
			UVoxelPlaceStampsSubsystem::UnregisterDrawer();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FLinearColor UVoxelEditorSettings::GetRandomLayerColor()
{
	const UVoxelEditorSettings* Settings = GetDefault<UVoxelEditorSettings>();
	if (Settings->DefaultLayerColors.Num() == 0)
	{
		return FVoxelAssetIcon().Color;
	}

	return Settings->DefaultLayerColors[FMath::RandRange(0, Settings->DefaultLayerColors.Num() - 1)];
}

FLinearColor UVoxelEditorSettings::GetRandomStackColor()
{
	const UVoxelEditorSettings* Settings = GetDefault<UVoxelEditorSettings>();
	if (Settings->DefaultStackColors.Num() == 0)
	{
		return FVoxelAssetIcon().Color;
	}

	return Settings->DefaultStackColors[FMath::RandRange(0, Settings->DefaultStackColors.Num() - 1)];
}