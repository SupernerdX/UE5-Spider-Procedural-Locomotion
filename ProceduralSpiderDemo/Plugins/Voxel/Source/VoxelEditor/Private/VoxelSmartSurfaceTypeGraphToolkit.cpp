// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSmartSurfaceTypeGraphToolkit.h"
#include "VoxelWorld.h"
#include "VoxelStampActor.h"
#include "VoxelGraphToolkit.h"
#include "Surface/VoxelSmartSurfaceType.h"
#include "Surface/VoxelSmartSurfaceTypeGraph.h"
#include "SVoxelMaterialGraphShapeToolbar.h"

void FVoxelSmartSurfaceTypeGraphViewportInterface::PopulateOverlay(const TSharedRef<SOverlay>& Overlay)
{
	FVoxelStampGraphViewportInterface::PopulateOverlay(Overlay);

	UVoxelSmartSurfaceTypeGraph* MaterialGraph = Cast<UVoxelSmartSurfaceTypeGraph>(GetGraph());
	if (!ensure(MaterialGraph))
	{
		return;
	}

	Overlay->AddSlot()
	.VAlign(VAlign_Bottom)
	[
		SNew(SVoxelMaterialGraphShapeToolbar)
		.Type_Lambda(MakeWeakObjectPtrLambda(MaterialGraph, [MaterialGraph]
		{
			return MaterialGraph->PreviewShape;
		}))
		.OnTypeChanged_Lambda(MakeWeakObjectPtrLambda(MaterialGraph, [this, MaterialGraph](const EVoxelSmartSurfacePreviewShape NewType)
		{
			FVoxelTransaction Transaction(MaterialGraph);

			MaterialGraph->PreviewShape = NewType;

			AVoxelStampActor* StampActor = WeakStampActor.Resolve();
			if (!StampActor)
			{
				return;
			}

			StampActor->SetStamp(FVoxelSmartSurfacePreviewShapeUtilities::MakeStamp(
				MaterialGraph->PreviewShape,
				MaterialGraph->GetPreviewSurface()));
		}))
	];
}

void FVoxelSmartSurfaceTypeGraphViewportInterface::SetupVoxelWorld(AVoxelWorld& VoxelWorld) const
{
	VoxelWorld.VoxelSize = 5;
}

FVoxelStampRef FVoxelSmartSurfaceTypeGraphViewportInterface::MakeStampRef(UVoxelGraph& Graph) const
{
	UVoxelSmartSurfaceTypeGraph& MaterialGraph = CastChecked<UVoxelSmartSurfaceTypeGraph>(Graph);

	return FVoxelSmartSurfacePreviewShapeUtilities::MakeStamp(
		MaterialGraph.PreviewShape,
		MaterialGraph.GetPreviewSurface());
}

TOptional<float> FVoxelSmartSurfaceTypeGraphViewportInterface::GetBoundsSize(const FVoxelStampRef& StampRef) const
{
	return 200.f;
}

#if VOXEL_ENGINE_VERSION >= 506
void FVoxelSmartSurfaceTypeGraphViewportInterface::ExtendToolbar(UToolMenu& ToolMenu)
{
	UVoxelSmartSurfaceTypeGraph* MaterialGraph = Cast<UVoxelSmartSurfaceTypeGraph>(GetGraph());
	if (!ensure(MaterialGraph))
	{
		return;
	}

	SVoxelMaterialGraphShapeToolbar::ExtendToolbar(
		ToolMenu,
		MakeWeakObjectPtrDelegate(MaterialGraph, [this, MaterialGraph](const EVoxelSmartSurfacePreviewShape NewType)
		{
			FVoxelTransaction Transaction(MaterialGraph);

			MaterialGraph->PreviewShape = NewType;

			AVoxelStampActor* StampActor = WeakStampActor.Resolve();
			if (!StampActor)
			{
				return;
			}

			StampActor->SetStamp(FVoxelSmartSurfacePreviewShapeUtilities::MakeStamp(
				MaterialGraph->PreviewShape,
				MaterialGraph->GetPreviewSurface()));
		}),
		MakeWeakObjectPtrDelegate(MaterialGraph, [MaterialGraph]
		{
			return MaterialGraph->PreviewShape;
		}));
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TArray<FVoxelToolkit::FMode> FVoxelSmartSurfaceTypeGraphToolkit::GetModes() const
{
	TArray<FMode> Modes;
	{
		FMode Mode;
		Mode.Struct = FVoxelSmartSurfaceTypeGraphPreviewToolkit::StaticStruct();
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
		Mode.ConfigureToolkit = [WeakAsset = MakeVoxelObjectPtr(Asset)](FVoxelToolkit& Toolkit)
		{
			CastStructChecked<FVoxelGraphToolkit>(Toolkit).ViewportInterface = MakeShared<FVoxelSmartSurfaceTypeGraphViewportInterface>();
		};
		Modes.Add(Mode);
	}
	return Modes;
}

UScriptStruct* FVoxelSmartSurfaceTypeGraphToolkit::GetDefaultMode() const
{
	return FVoxelGraphToolkit::StaticStruct();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSmartSurfaceTypeGraphPreviewToolkit::Initialize()
{
	Super::Initialize();

	FVoxelDetailsViewCustomData* CustomData = FVoxelDetailsViewCustomData::GetCustomData(&GetDetailsView());
	if (!CustomData)
	{
		return;
	}

	CustomData->SetMetadata("ShowTooltipColumn");
}

void FVoxelSmartSurfaceTypeGraphPreviewToolkit::SetupPreview()
{
	ViewportInterface = MakeShared<FVoxelSmartSurfaceTypeGraphViewportInterface>();
	ViewportInterface->SetupPreview(*Asset, GetSharedViewport());
}

FRotator FVoxelSmartSurfaceTypeGraphPreviewToolkit::GetInitialViewRotation() const
{
	return ViewportInterface->GetInitialViewRotation();
}

TOptional<float> FVoxelSmartSurfaceTypeGraphPreviewToolkit::GetInitialViewDistance() const
{
	return ViewportInterface->GetInitialViewDistance();
}

void FVoxelSmartSurfaceTypeGraphPreviewToolkit::PostEditChange(const FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChange(PropertyChangedEvent);

	ViewportInterface->UpdateStamp();
}

TSharedPtr<SWidget> FVoxelSmartSurfaceTypeGraphPreviewToolkit::GetMenuOverlay() const
{
	return FVoxelGraphToolkit::MakeMenuOverlay(Asset);
}