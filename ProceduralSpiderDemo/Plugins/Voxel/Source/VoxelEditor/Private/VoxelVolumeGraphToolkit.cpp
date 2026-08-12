// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelVolumeGraphToolkit.h"
#include "VoxelWorld.h"
#include "VoxelStampActor.h"
#include "VoxelGraphContext.h"
#include "VoxelNodeEvaluator.h"
#include "Graphs/VoxelVolumeGraph.h"
#include "Graphs/VoxelVolumeGraphStamp.h"
#include "Graphs/VoxelOutputNode_OutputVolume.h"

FVoxelStampRef FVoxelVolumeGraphViewportInterface::MakeStampRef(UVoxelGraph& Graph) const
{
	FVoxelVolumeGraphStamp Stamp;
	Stamp.Graph = CastChecked<UVoxelVolumeGraph>(&Graph);
	// Some nodes are override only
	Stamp.BlendMode = EVoxelVolumeBlendMode::Override;
	return FVoxelStampRef::New(Stamp);
}

TOptional<float> FVoxelVolumeGraphViewportInterface::GetBoundsSize(const FVoxelStampRef& StampRef) const
{
	const TSharedPtr<FVoxelVolumeGraphStamp> Stamp = StampRef.ToSharedPtr<FVoxelVolumeGraphStamp>();
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

	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputVolume> Evaluator = FVoxelNodeEvaluator::Create<FVoxelOutputNode_OutputVolume>(Environment.ToSharedRef());
	if (!Evaluator)
	{
		return {};
	}

	FVoxelGraphContext Context = Evaluator.MakeContext(FVoxelDependencyCollector::Null);

	return Evaluator->BoundsPin.GetSynchronous(Context.MakeQuery()).Size().Length();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TArray<FVoxelToolkit::FMode> FVoxelVolumeGraphToolkit::GetModes() const
{
	TArray<FMode> Modes;
	{
		FMode Mode;
		Mode.Struct = FVoxelVolumeGraphPreviewToolkit::StaticStruct();
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
			CastStructChecked<FVoxelGraphToolkit>(Toolkit).ViewportInterface = MakeShared<FVoxelVolumeGraphViewportInterface>();
		};
		Modes.Add(Mode);
	}
	return Modes;
}

UScriptStruct* FVoxelVolumeGraphToolkit::GetDefaultMode() const
{
	return FVoxelGraphToolkit::StaticStruct();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeGraphPreviewToolkit::Initialize()
{
	Super::Initialize();

	FVoxelDetailsViewCustomData* CustomData = FVoxelDetailsViewCustomData::GetCustomData(&GetDetailsView());
	if (!CustomData)
	{
		return;
	}

	CustomData->SetMetadata("ShowTooltipColumn");
}

void FVoxelVolumeGraphPreviewToolkit::SetupPreview()
{
	ViewportInterface = MakeShared<FVoxelVolumeGraphViewportInterface>();
	ViewportInterface->SetupPreview(*Asset, GetSharedViewport());
}

FRotator FVoxelVolumeGraphPreviewToolkit::GetInitialViewRotation() const
{
	return ViewportInterface->GetInitialViewRotation();
}

TOptional<float> FVoxelVolumeGraphPreviewToolkit::GetInitialViewDistance() const
{
	return ViewportInterface->GetInitialViewDistance();
}

TOptional<float> FVoxelVolumeGraphPreviewToolkit::GetMaxFocusDistance() const
{
	return ViewportInterface->GetMaxFocusDistance();
}

void FVoxelVolumeGraphPreviewToolkit::PostEditChange(const FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChange(PropertyChangedEvent);

	ViewportInterface->UpdateStamp();
}

TSharedPtr<SWidget> FVoxelVolumeGraphPreviewToolkit::GetMenuOverlay() const
{
	return FVoxelGraphToolkit::MakeMenuOverlay(Asset);
}