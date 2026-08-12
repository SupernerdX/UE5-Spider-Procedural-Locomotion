// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphRootToolkit.h"
#include "VoxelGraphToolkit.h"

TArray<FVoxelToolkit::FMode> FVoxelGraphRootToolkit::GetModes() const
{
	TArray<FMode> Modes;
	{
		FMode Mode;
		Mode.Struct = FVoxelGraphPreviewToolkit::StaticStruct();
		Mode.Object = Asset;
		Mode.DisplayName = INVTEXT("Preview");
		Mode.Icon = FAppStyle::GetBrush("UMGEditor.SwitchToDesigner");
		Modes.Add(Mode);
	}
	{
		FMode Mode;
		Mode.Struct = FVoxelGraphToolkit::StaticStruct();
		Mode.Object = Asset;
		Mode.DisplayName = INVTEXT("Graph");
		Mode.Icon = FAppStyle::GetBrush("FullBlueprintEditor.SwitchToScriptingMode");
		Modes.Add(Mode);
	}
	return Modes;
}

UScriptStruct* FVoxelGraphRootToolkit::GetDefaultMode() const
{
	return FVoxelGraphToolkit::StaticStruct();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphPreviewToolkit::Initialize()
{
	Super::Initialize();

	FVoxelDetailsViewCustomData* CustomData = FVoxelDetailsViewCustomData::GetCustomData(&GetDetailsView());
	if (!CustomData)
	{
		return;
	}

	CustomData->SetMetadata("ShowTooltipColumn");
}

void FVoxelGraphPreviewToolkit::SetupPreview()
{
	VOXEL_FUNCTION_COUNTER();

	Super::SetupPreview();
}

TOptional<float> FVoxelGraphPreviewToolkit::GetInitialViewDistance() const
{
	return {};
}

TOptional<float> FVoxelGraphPreviewToolkit::GetMaxFocusDistance() const
{
	return 10000.f;
}

TSharedPtr<SWidget> FVoxelGraphPreviewToolkit::GetMenuOverlay() const
{
	return FVoxelGraphToolkit::MakeMenuOverlay(Asset);
}