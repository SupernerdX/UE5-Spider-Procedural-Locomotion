// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "SVoxelGraphPreview.generated.h"

class UVoxelTerminalGraph;
class SVoxelGraphPreviewImage;
class SVoxelGraphPreviewScale;
class SVoxelGraphPreviewRuler;
class SVoxelGraphPreviewStats;
class SVoxelGraphPreviewDepthSlider;
class FDeferredCleanupSlateBrush;
struct FToolMenuEntry;
struct FVoxelGraphToolkit;
struct FVoxelPreviewHandler;

class SVoxelGraphPreview : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ARGUMENT(TSharedPtr<FVoxelGraphToolkit>, Toolkit);
	};

	void Construct(const FArguments& Args);

	void SetTerminalGraph(UVoxelTerminalGraph* NewTerminalGraph);

	TSharedRef<SWidget> GetPreviewStats() const;
	void AddReferencedObjects(FReferenceCollector& Collector);

	//~ Begin SWidget Interface
	virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	//~ End SWidget Interface

private:
	TSharedRef<SWidget> CreateToolbar(const TSharedPtr<FVoxelGraphToolkit>& Toolkit, bool bNewToolbar);
	FToolMenuEntry MakeAxisEntry(EVoxelAxis Axis);
	FToolMenuEntry MakeResetViewEntry();
	FToolMenuEntry MakePreviewPropertiesEntry(const TSharedPtr<FVoxelGraphToolkit>& Toolkit);
	FToolMenuEntry MakeNormalizeEntry();
	FToolMenuEntry MakeGrayscaleEntry();
	FToolMenuEntry MakeResolutionEntry();

private:
	FTransform LocalToWorld;

	TVoxelObjectPtr<UVoxelTerminalGraph> WeakTerminalGraph;
	FSharedVoidPtr OnChangedPtr;

	bool bUpdateQueued = false;
	bool bMousePositionUpdateQueued = false;
	double ProcessingStartTime = 0;
	FString Message;

	TSharedPtr<FVoxelPreviewHandler> PreviewHandler;
	FVoxelFuture FuturePreviewHandler;
	FVoxelFuture FutureMousePositionComputeHandler;

	TObjectPtr<UTexture2D> Texture;
	TSharedPtr<FDeferredCleanupSlateBrush> Brush;

	TSharedPtr<SVoxelGraphPreviewStats> PreviewStats;
	TSharedPtr<SVoxelGraphPreviewStats> ExternalPreviewStats;
	TSharedPtr<SVoxelGraphPreviewImage> PreviewImage;
	TSharedPtr<SVoxelGraphPreviewRuler> PreviewRuler;

	TSharedPtr<SBox> DepthSliderContainer;
	TSharedPtr<SVoxelGraphPreviewDepthSlider> DepthSlider;

	FVector2D MousePosition = FVector2D::ZeroVector;

	bool bLockCoordinatePending = false;
	bool bIsCoordinateLocked = false;
	FVector LockedCoordinate_WorldSpace = FVector::ZeroVector;

	void Update();
	void UpdateStats();
	void ComputeMousePositionStats();
	FMatrix GetPixelToWorld() const;
};

UCLASS()
class UVoxelGraphPreviewContext : public UObject
{
	GENERATED_BODY()

public:
	TWeakPtr<FVoxelGraphToolkit> WeakToolkit;
	TWeakPtr<SVoxelGraphPreview> WeakPreview;
};