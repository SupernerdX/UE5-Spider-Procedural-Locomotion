// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelHeightGraphToolkit.h"
#include "VoxelWorld.h"
#include "VoxelStampActor.h"
#include "VoxelGraphContext.h"
#include "VoxelGraphToolkit.h"
#include "VoxelNodeEvaluator.h"
#include "Graphs/VoxelHeightGraph.h"
#include "Graphs/VoxelHeightGraphStamp.h"
#include "Graphs/VoxelOutputNode_OutputHeight.h"

FVoxelStampRef FVoxelHeightGraphViewportInterface::MakeStampRef(UVoxelGraph& Graph) const
{
	FVoxelHeightGraphStamp Stamp;
	Stamp.Graph = CastChecked<UVoxelHeightGraph>(&Graph);
	// Some nodes are override only
	Stamp.BlendMode = EVoxelHeightBlendMode::Override;
	return FVoxelStampRef::New(Stamp);
}

TOptional<float> FVoxelHeightGraphViewportInterface::GetBoundsSize(const FVoxelStampRef& StampRef) const
{
	const TSharedPtr<FVoxelHeightGraphStamp> Stamp = StampRef.ToSharedPtr<FVoxelHeightGraphStamp>();
	if (!ensureVoxelSlow(Stamp))
	{
		return {};
	}

	const TSharedPtr<const FVoxelStampRuntime> Runtime = Stamp->ResolveStampRuntime();
	if (!ensureVoxelSlow(Runtime))
	{
		return {};
	}

	const TSharedPtr<const FVoxelGraphEnvironment> Environment = Stamp->CreateEnvironment(
		*Runtime,
		FVoxelDependencyCollector::Null);

	if (!Environment)
	{
		return {};
	}

	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputHeight> Evaluator = FVoxelNodeEvaluator::Create<FVoxelOutputNode_OutputHeight>(Environment.ToSharedRef());
	if (!Evaluator)
	{
		return {};
	}

	FVoxelGraphContext Context = Evaluator.MakeContext(FVoxelDependencyCollector::Null);

	return Evaluator->BoundsPin.GetSynchronous(Context.MakeQuery()).Size().Length() * 0.25f;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TArray<FVoxelToolkit::FMode> FVoxelHeightGraphToolkit::GetModes() const
{
	TArray<FMode> Modes;
	{
		FMode Mode;
		Mode.Struct = FVoxelHeightGraphPreviewToolkit::StaticStruct();
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
			CastStructChecked<FVoxelGraphToolkit>(Toolkit).ViewportInterface = MakeShared<FVoxelHeightGraphViewportInterface>();
		};
		Modes.Add(Mode);
	}
	return Modes;
}

UScriptStruct* FVoxelHeightGraphToolkit::GetDefaultMode() const
{
	return FVoxelGraphToolkit::StaticStruct();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightGraphPreviewToolkit::Initialize()
{
	Super::Initialize();

	FVoxelDetailsViewCustomData* CustomData = FVoxelDetailsViewCustomData::GetCustomData(&GetDetailsView());
	if (!CustomData)
	{
		return;
	}

	CustomData->SetMetadata("ShowTooltipColumn");
}

void FVoxelHeightGraphPreviewToolkit::SetupPreview()
{
	ViewportInterface = MakeShared<FVoxelHeightGraphViewportInterface>();
	ViewportInterface->SetupPreview(*Asset, GetSharedViewport());
}

FRotator FVoxelHeightGraphPreviewToolkit::GetInitialViewRotation() const
{
	return ViewportInterface->GetInitialViewRotation();
}

TOptional<float> FVoxelHeightGraphPreviewToolkit::GetInitialViewDistance() const
{
	return ViewportInterface->GetInitialViewDistance();
}

TOptional<float> FVoxelHeightGraphPreviewToolkit::GetMaxFocusDistance() const
{
	return ViewportInterface->GetMaxFocusDistance();
}

void FVoxelHeightGraphPreviewToolkit::PostEditChange(const FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChange(PropertyChangedEvent);

	ViewportInterface->UpdateStamp();
}

TSharedPtr<SWidget> FVoxelHeightGraphPreviewToolkit::GetMenuOverlay() const
{
	return FVoxelGraphToolkit::MakeMenuOverlay(Asset);
}