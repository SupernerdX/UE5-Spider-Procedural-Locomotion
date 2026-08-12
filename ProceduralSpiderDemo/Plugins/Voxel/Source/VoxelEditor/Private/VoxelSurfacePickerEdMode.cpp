// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSurfacePickerEdMode.h"
#include "VoxelWorld.h"
#include "VoxelQuery.h"
#include "EditorModes.h"
#include "LevelEditor.h"
#include "VoxelLayers.h"
#include "EditorModeManager.h"
#include "LevelEditorViewport.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Collision/VoxelCollisionChannels.h"
#include "Surface/VoxelSurfaceTypeInterface.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"
#include "Utilities/VoxelBufferGradientUtilities.h"

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FEditorModeRegistry::Get().RegisterMode<FVoxelSurfacePickerEdMode>("EM_VoxelSurfaceTypePicker");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSurfacePickerEdMode::BeginSurfaceTypePicker(const TDelegate<void(UVoxelSurfaceTypeInterface*)>& OnSurfaceTypeSelected)
{
	const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	const TSharedPtr<ILevelEditor> FirstLevelEditor = LevelEditorModule.GetFirstLevelEditor();
	if (!FirstLevelEditor)
	{
		return;
	}

	FEditorModeTools& ModeTools = FirstLevelEditor->GetEditorModeManager();

	ModeTools.ActivateMode("EM_VoxelSurfaceTypePicker");

	FVoxelSurfacePickerEdMode* SurfacePickerMode = ModeTools.GetActiveModeTyped<FVoxelSurfacePickerEdMode>("EM_VoxelSurfaceTypePicker");
	if (!ensure(SurfacePickerMode))
	{
		return;
	}
	SurfacePickerMode->OnSurfaceTypeSelected = OnSurfaceTypeSelected;
}

void FVoxelSurfacePickerEdMode::EndSurfaceTypePicker()
{
	const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	const TSharedPtr<ILevelEditor> FirstLevelEditor = LevelEditorModule.GetFirstLevelEditor();
	if (!FirstLevelEditor)
	{
		return;
	}

	FEditorModeTools& ModeTools = FirstLevelEditor->GetEditorModeManager();
	ModeTools.DeactivateMode("EM_VoxelSurfaceTypePicker");
}

bool FVoxelSurfacePickerEdMode::IsSurfaceTypePickerActive()
{
	const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	const TSharedPtr<ILevelEditor> FirstLevelEditor = LevelEditorModule.GetFirstLevelEditor();
	if (!FirstLevelEditor)
	{
		return false;
	}

	const FEditorModeTools& ModeTools = FirstLevelEditor->GetEditorModeManager();
	return ModeTools.IsModeActive("EM_VoxelSurfaceTypePicker");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSurfacePickerEdMode::Enter()
{
	FEdMode::Enter();

	PickState = EVoxelSurfacePickerState::NotOverViewport;
	WeakHoveredActor = {};
	WeakHoveredSurfaceType = {};

	CursorDecoratorWindow = SWindow::MakeCursorDecorator();
	FSlateApplication::Get().AddWindow(CursorDecoratorWindow.ToSharedRef(), true);
	CursorDecoratorWindow->SetContent(
		SNew(SToolTip)
		.Text(this, &FVoxelSurfacePickerEdMode::GetCursorDecoratorText));
}

void FVoxelSurfacePickerEdMode::Exit()
{
	OnSurfaceTypeSelected = {};

	if (CursorDecoratorWindow)
	{
		CursorDecoratorWindow->RequestDestroyWindow();
		CursorDecoratorWindow.Reset();
	}

	WeakHoveredActor = {};
	WeakHoveredSurfaceType = {};
	PickState = EVoxelSurfacePickerState::NotOverViewport;

	UpdateWidgetVisibility(nullptr);

	FEdMode::Exit();
}

void FVoxelSurfacePickerEdMode::Tick(FEditorViewportClient* ViewportClient, const float DeltaTime)
{
	if (CursorDecoratorWindow)
	{
		CursorDecoratorWindow->MoveWindowTo(FSlateApplication::Get().GetCursorPos() + FSlateApplication::Get().GetCursorSize());
	}

	FEdMode::Tick(ViewportClient, DeltaTime);
}

bool FVoxelSurfacePickerEdMode::MouseEnter(
	FEditorViewportClient* ViewportClient,
	FViewport* Viewport,
	const int32 X,
	const int32 Y)
{
	PickState = EVoxelSurfacePickerState::OverViewport;
	WeakHoveredActor = {};
	WeakHoveredSurfaceType = {};
	UpdateWidgetVisibility(ViewportClient);
	return FEdMode::MouseEnter(ViewportClient, Viewport, X, Y);
}

bool FVoxelSurfacePickerEdMode::MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	PickState = EVoxelSurfacePickerState::NotOverViewport;
	WeakHoveredActor = {};
	WeakHoveredSurfaceType = {};
	UpdateWidgetVisibility(nullptr);
	return FEdMode::MouseLeave(ViewportClient, Viewport);
}

bool FVoxelSurfacePickerEdMode::MouseMove(
	FEditorViewportClient* ViewportClient,
	FViewport* Viewport,
	const int32 X,
	const int32 Y)
{
	WeakHoveredActor = {};
	WeakHoveredSurfaceType = {};

	if (ViewportClient != GCurrentLevelEditingViewportClient)
	{
		PickState = EVoxelSurfacePickerState::NotOverViewport;
		return FEdMode::MouseMove(ViewportClient, Viewport, X, Y);
	}

	PickState = EVoxelSurfacePickerState::OverViewport;

	const int32 HitX = Viewport->GetMouseX();
	const int32 HitY = Viewport->GetMouseY();
	const HActor* ActorProxy = HitProxyCast<HActor>(Viewport->GetHitProxy(HitX, HitY));
	if (!ActorProxy ||
		!ActorProxy->Actor)
	{
		return FEdMode::MouseMove(ViewportClient, Viewport, X, Y);
	}

	if (!ActorProxy->Actor->IsA<AVoxelWorld>())
	{
		PickState = EVoxelSurfacePickerState::OverIncompatibleActor;
		WeakHoveredActor = ActorProxy->Actor;
		return FEdMode::MouseMove(ViewportClient, Viewport, X, Y);
	}

	WeakHoveredSurfaceType = GetHoveredSurfaceType(ActorProxy, ViewportClient);
	if (!WeakHoveredSurfaceType.IsExplicitlyNull())
	{
		PickState = EVoxelSurfacePickerState::OverSurfaceType;
	}

	return FEdMode::MouseMove(ViewportClient, Viewport, X, Y);
}

bool FVoxelSurfacePickerEdMode::LostFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	if (ViewportClient != GCurrentLevelEditingViewportClient)
	{
		return false;
	}

	UpdateWidgetVisibility(nullptr);
	RequestDeletion();
	return true;
}

bool FVoxelSurfacePickerEdMode::InputKey(
	FEditorViewportClient* ViewportClient,
	FViewport* Viewport,
	const FKey Key,
	const EInputEvent Event)
{
	if (ViewportClient != GCurrentLevelEditingViewportClient)
	{
		UpdateWidgetVisibility(nullptr);
		RequestDeletion();
		return true;
	}

	if (Event != IE_Pressed)
	{
		return true;
	}

	if (Key == EKeys::Escape)
	{
		UpdateWidgetVisibility(nullptr);
		RequestDeletion();
		return true;
	}

	const int32 HitX = Viewport->GetMouseX();
	const int32 HitY = Viewport->GetMouseY();
	const HActor* ActorProxy = HitProxyCast<HActor>(Viewport->GetHitProxy(HitX, HitY));
	if (!ActorProxy ||
		!ActorProxy->Actor ||
		!ActorProxy->Actor->IsA<AVoxelWorld>())
	{
		return true;
	}

	UVoxelSurfaceTypeInterface* SurfaceType = GetHoveredSurfaceType(ActorProxy, ViewportClient);
	if (!SurfaceType)
	{
		return true;
	}

	OnSurfaceTypeSelected.ExecuteIfBound(SurfaceType);
	UpdateWidgetVisibility(nullptr);
	RequestDeletion();
	return true;
}

bool FVoxelSurfacePickerEdMode::GetCursor(EMouseCursor::Type& OutCursor) const
{
	if (WeakHoveredSurfaceType.IsValid() &&
		PickState == EVoxelSurfacePickerState::OverSurfaceType)
	{
		OutCursor = EMouseCursor::EyeDropper;
	}
	else
	{
		OutCursor = EMouseCursor::SlashedCircle;
	}

	return true;
}

bool FVoxelSurfacePickerEdMode::UsesToolkits() const
{
	return false;
}

bool FVoxelSurfacePickerEdMode::IsCompatibleWith(const FEditorModeID OtherModeID) const
{
	return OtherModeID != FBuiltinEditorModes::EM_None;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FText FVoxelSurfacePickerEdMode::GetCursorDecoratorText() const
{
	switch(PickState)
	{
	default:
	case EVoxelSurfacePickerState::NotOverViewport:
	{
		return INVTEXT("Pick a surface type by clicking on it in the active level viewport");
	}
	case EVoxelSurfacePickerState::OverViewport:
	{
		return INVTEXT("Pick a surface type by clicking on it");
	}
	case EVoxelSurfacePickerState::OverIncompatibleActor:
	{
		const AActor* HoveredActor = WeakHoveredActor.Get();
		if (!HoveredActor)
		{
			return INVTEXT("Pick a surface type by clicking on it");
		}

		return FText::FromString(HoveredActor->GetActorNameOrLabel() + " does not support surface types");
	}
	case EVoxelSurfacePickerState::OverSurfaceType:
	{
		const UVoxelSurfaceTypeInterface* HoveredSurfaceType = WeakHoveredSurfaceType.Get();
		if (!HoveredSurfaceType)
		{
			return INVTEXT("Pick an actor by clicking on it");
		}

		return FText::FromString("Pick " + HoveredSurfaceType->GetName());
	}
	}
}

UVoxelSurfaceTypeInterface* FVoxelSurfacePickerEdMode::GetHoveredSurfaceType(const HActor* ActorProxy, FEditorViewportClient* ViewportClient) const
{
	FVector Start;
	FVector End;
	if (!ensure(FVoxelEditorUtilities::GetRayInfo(ViewportClient, Start, End)))
	{
		return nullptr;
	}

	AVoxelWorld* VoxelWorld = CastChecked<AVoxelWorld>(ActorProxy->Actor);
	FHitResult HitResult;
	{
		FCollisionQueryParams QueryParams;
		while (true)
		{
			if (!GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_VoxelEditor, QueryParams))
			{
				return nullptr;
			}

			AActor* Actor = HitResult.GetActor();
			if (!Actor)
			{
				return nullptr;
			}

			if (Actor == VoxelWorld)
			{
				break;
			}

			QueryParams.AddIgnoredActor(Actor);
		}
	}

	return QuerySurfaceType(VoxelWorld, HitResult);
}

void FVoxelSurfacePickerEdMode::UpdateWidgetVisibility(FEditorViewportClient* ViewportClient)
{
	if (WidgetVisibilityFunction)
	{
		WidgetVisibilityFunction();
		WidgetVisibilityFunction.Reset();
	}

	if (!ViewportClient)
	{
		return;
	}

	const bool bPreviousModeWidgets = ViewportClient->EngineShowFlags.ModeWidgets;
	WidgetVisibilityFunction = [ViewportClient, bPreviousModeWidgets]
	{
		if (ViewportClient)
		{
			ViewportClient->EngineShowFlags.SetModeWidgets(bPreviousModeWidgets);
			ViewportClient->Invalidate(false, false);
		}
	};

	ViewportClient->EngineShowFlags.SetModeWidgets(false);
	ViewportClient->Invalidate(false, false);
}

UVoxelSurfaceTypeInterface* FVoxelSurfacePickerEdMode::QuerySurfaceType(
	AVoxelWorld* VoxelWorld,
	const FHitResult& HitResult)
{
	const TSharedRef<FVoxelLayers> Layers = FVoxelLayers::Get(VoxelWorld->GetWorld());
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable = FVoxelSurfaceTypeTable::Get();

	FVoxelDoubleVectorBuffer PositionsBuffer;
	PositionsBuffer.Allocate(1);
	PositionsBuffer.Set(0, HitResult.Location);

	FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
	SurfaceTypes.AllocateZeroed(1);

	const FVoxelQuery Query(
		0,
		*Layers,
		*SurfaceTypeTable,
		FVoxelDependencyCollector::Null);

	const FVoxelWeakStackLayer WeakLayer = FVoxelStackLayer(VoxelWorld->LayerStack, nullptr);

	FVoxelVectorBuffer PositionNormals;
	if (WeakLayer.Type == EVoxelLayerType::Height)
	{
		FVoxelDoubleVector2DBuffer Positions2D;
		Positions2D.X = PositionsBuffer.X;
		Positions2D.Y = PositionsBuffer.Y;

		Query.SampleHeightLayer(
			WeakLayer,
			Positions2D,
			SurfaceTypes.View(),
			{});

		FVoxelDoubleVector2DBuffer GradientPositions = FVoxelBufferGradientUtilities::SplitPositions2D(Positions2D, VoxelWorld->VoxelSize);

		const FVoxelFloatBuffer GradientHeights = Query.SampleHeightLayer(WeakLayer, GradientPositions);

		PositionNormals = FVoxelBufferGradientUtilities::CollapseGradient2DToNormal(GradientHeights, 1, VoxelWorld->VoxelSize);
	}
	else
	{
		Query.SampleVolumeLayer(
			WeakLayer,
			PositionsBuffer,
			SurfaceTypes.View(),
			{});

		FVoxelDoubleVectorBuffer GradientPositions = FVoxelBufferGradientUtilities::SplitPositions3D(PositionsBuffer, VoxelWorld->VoxelSize);

		const FVoxelFloatBuffer GradientHeights = Query.SampleVolumeLayer(WeakLayer, GradientPositions);

		PositionNormals = FVoxelBufferGradientUtilities::CollapseGradient3D(GradientHeights, 1, VoxelWorld->VoxelSize);
	}

	FVoxelSmartSurfaceTypeResolver Resolver(
		0,
		WeakLayer,
		*Layers,
		*SurfaceTypeTable,
		FVoxelDependencyCollector::Null,
		PositionsBuffer,
		PositionNormals,
		SurfaceTypes.View());

	Resolver.Resolve();

	return SurfaceTypes[0].GetTopLayer().Type.GetSurfaceTypeInterface().Resolve_Ensured();
}