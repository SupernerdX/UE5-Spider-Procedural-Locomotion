// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Toolkits/BaseToolkit.h"
#include "VoxelToolToolkit.generated.h"

class UVoxelTool;
class UVoxelToolEdMode;
class UVoxelToolInteractiveTool;
class AVoxelSculptActorBase;
struct FVoxelToolEdMode;

USTRUCT()
struct FVoxelToolSculptActorSelector
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TObjectPtr<AVoxelSculptActorBase> Actor;
};

class FVoxelToolToolkit : public FModeToolkit
{
public:
	explicit FVoxelToolToolkit(const TSharedPtr<FVoxelToolEdMode>& EdMode)
		: FModeToolkit()
		, WeakEdMode(EdMode)
	{}

	//~ Begin FModeToolkit Interface
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override;

	virtual FEdMode* GetEditorMode() const override;

	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;

	virtual FText GetActiveToolDisplayName() const override;
	virtual FText GetActiveToolMessage() const override;

	virtual void GetToolPaletteNames(TArray<FName>& PaletteNames) const override;
	virtual FText GetToolPaletteDisplayName(FName Palette) const override;
	virtual void OnToolPaletteChanged(FName PaletteName) override;
	virtual TSharedRef<SWidget> CreatePaletteWidget(TSharedPtr<FUICommandList> InCommandList, FName InToolbarCustomizationName, FName InPaletteName) override;

	virtual void RequestModeUITabs() override;
	virtual TSharedPtr<SWidget> GetInlineContent() const override;

	virtual void OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool) override;
	virtual void OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool) override;
	//~ End FModeToolkit Interface

public:
	void SwitchSculptActor(AVoxelSculptActorBase* NewStampComponent) const;

private:
	bool IsToolActive(const TSubclassOf<UVoxelTool>& Class) const;
	void OnChangeTool(const TSubclassOf<UVoxelTool>& Class) const;
	void OnStampComponentChanged(const FPropertyChangedEvent& PropertyChangedEvent);

	TSharedPtr<FVoxelToolEdMode> GetEdMode() const;

private:
	TWeakPtr<FVoxelToolEdMode> WeakEdMode;
	TSharedPtr<IStructureDetailsView> ActorDetailsView;
	TSharedPtr<TStructOnScope<FVoxelToolSculptActorSelector>> StructOnScope;
};
