// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSmartSurfaceTypeToolkit.h"
#include "VoxelWorld.h"
#include "VoxelStampActor.h"
#include "Shape/VoxelSphereShape.h"
#include "SVoxelMaterialGraphShapeToolbar.h"

void FVoxelSmartSurfaceTypeToolkit::Initialize()
{
	FVoxelSimpleAssetToolkit::Initialize();

	FVoxelDetailsViewCustomData* CustomData = FVoxelDetailsViewCustomData::GetCustomData(&GetDetailsView());
	if (!CustomData)
	{
		return;
	}

	CustomData->SetMetadata("ShowTooltipColumn");
}

void FVoxelSmartSurfaceTypeToolkit::SetupPreview()
{
	Super::SetupPreview();

	VoxelWorld = GetViewport().SpawnActor<AVoxelWorld>();
	VoxelWorld->SetupForPreview();
	VoxelWorld->VoxelSize = 5;

	FVoxelSphereShape Shape;
	Shape.Radius = 100.f;

	StampActor = GetViewport().SpawnActor<AVoxelStampActor>();

	StampActor->SetStamp(FVoxelSmartSurfacePreviewShapeUtilities::MakeStamp(
		Asset->PreviewShape,
		Asset));
}

void FVoxelSmartSurfaceTypeToolkit::PopulateOverlay(const TSharedRef<SOverlay>& Overlay)
{
	Super::PopulateOverlay(Overlay);

	Overlay->AddSlot()
	.VAlign(VAlign_Bottom)
	[
		SNew(SVoxelMaterialGraphShapeToolbar)
		.Type_Lambda([this]
		{
			return Asset->PreviewShape;
		})
		.OnTypeChanged_Lambda([this](const EVoxelSmartSurfacePreviewShape NewType)
		{
			FVoxelTransaction Transaction(Asset);
			Asset->PreviewShape = NewType;

			StampActor->SetStamp(FVoxelSmartSurfacePreviewShapeUtilities::MakeStamp(
				Asset->PreviewShape,
				Asset));
		})
	];
}

#if VOXEL_ENGINE_VERSION >= 506
void FVoxelSmartSurfaceTypeToolkit::ExtendToolbar(UToolMenu& ToolMenu)
{
	SVoxelMaterialGraphShapeToolbar::ExtendToolbar(
		ToolMenu,
		MakeWeakPtrDelegate(this, [this](const EVoxelSmartSurfacePreviewShape NewType)
		{
			FVoxelTransaction Transaction(Asset);
			Asset->PreviewShape = NewType;

			StampActor->SetStamp(FVoxelSmartSurfacePreviewShapeUtilities::MakeStamp(
				Asset->PreviewShape,
				Asset));
		}),
		MakeWeakPtrDelegate(this, [this]
		{
			return Asset->PreviewShape;
		}));
}
#endif