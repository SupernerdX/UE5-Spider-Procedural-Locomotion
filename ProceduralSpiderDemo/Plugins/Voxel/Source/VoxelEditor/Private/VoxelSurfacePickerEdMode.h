// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "EdMode.h"
#include "Surface/VoxelSurfaceTypeInterface.h"

class AVoxelWorld;
class UVoxelSurfaceTypeInterface;

enum class EVoxelSurfacePickerState
{
	NotOverViewport,
	OverViewport,
	OverIncompatibleActor,
	OverSurfaceType,
};

class FVoxelSurfacePickerEdMode : public FEdMode
{
public:
	static void BeginSurfaceTypePicker(const TDelegate<void(UVoxelSurfaceTypeInterface*)>& OnSurfaceTypeSelected);
	static void EndSurfaceTypePicker();
	static bool IsSurfaceTypePickerActive();

public:
	//~ Begin FSurfaceTypePickerEdMode Interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual bool MouseEnter(
		FEditorViewportClient* ViewportClient,
		FViewport* Viewport,
		int32 X,
		int32 Y) override;
	virtual bool MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport) override;
	virtual bool MouseMove(
		FEditorViewportClient* ViewportClient,
		FViewport* Viewport,
		int32 X,
		int32 Y) override;
	virtual bool LostFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport) override;
	virtual bool InputKey(
		FEditorViewportClient* ViewportClient,
		FViewport* Viewport,
		FKey Key,
		EInputEvent Event) override;
	virtual bool GetCursor(EMouseCursor::Type& OutCursor) const override;
	virtual bool UsesToolkits() const override;
	virtual bool IsCompatibleWith(FEditorModeID OtherModeID) const override;
	//~ End FSurfaceTypePickerEdMode Interface

private:
	FText GetCursorDecoratorText() const;
	UVoxelSurfaceTypeInterface* GetHoveredSurfaceType(
		const HActor* ActorProxy,
		FEditorViewportClient* ViewportClient) const;
	void UpdateWidgetVisibility(FEditorViewportClient* ViewportClient);

	static UVoxelSurfaceTypeInterface* QuerySurfaceType(
		AVoxelWorld* VoxelWorld,
		const FHitResult& HitResult);

private:
	TDelegate<void(UVoxelSurfaceTypeInterface*)> OnSurfaceTypeSelected;

	TWeakObjectPtr<AActor> WeakHoveredActor;
	TWeakObjectPtr<UVoxelSurfaceTypeInterface> WeakHoveredSurfaceType;

	EVoxelSurfacePickerState PickState = EVoxelSurfacePickerState::NotOverViewport;

private:
	TSharedPtr<SWindow> CursorDecoratorWindow;
	TFunction<void()> WidgetVisibilityFunction;
};