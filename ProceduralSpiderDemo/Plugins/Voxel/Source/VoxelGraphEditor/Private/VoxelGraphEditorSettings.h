// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelDeveloperSettings.h"
#include "VoxelGraphEditorSettings.generated.h"

class UVoxelGraph;
class UVoxelFunctionLibraryAsset;

UENUM()
enum class EVoxelGraphEditorInputType : uint8
{
	None,
	Struct,
	Function,
	FunctionLibrary
};

USTRUCT()
struct FVoxelGraphEditorInputBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FInputChord InputChord;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelGraphEditorInputType Type = EVoxelGraphEditorInputType::None;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSoftObjectPtr<UScriptStruct> Struct;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSoftObjectPtr<UFunction> Function;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSoftObjectPtr<UVoxelFunctionLibraryAsset> FunctionLibrary;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FGuid FunctionGuid;

	FString GetActionName() const;
	bool IsActionValid(bool bNoneValid) const;
	TSharedPtr<FEdGraphSchemaAction> CreateAction() const;
};

UCLASS(config=EditorPerProjectUserSettings)
class UVoxelGraphEditorSettings : public UVoxelDeveloperSettings
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelDeveloperSettings Interface
	virtual FName GetContainerName() const override;
	//~ End UVoxelDeveloperSettings Interface

public:
	UPROPERTY(Config, EditAnywhere, Category = "Graph")
	FLinearColor PointSetPinColor = FLinearColor(0.607717f, 0.224984f, 1.f, 1.f);

	UPROPERTY(Config, EditAnywhere, Category = "Graph")
	FLinearColor SeedPinColor = FLinearColor(0.607717f, 0.224984f, 1.f, 1.f);

	UPROPERTY(Config, EditAnywhere, Category = "Graph")
	FLinearColor Vector2DPinColor = FLinearColor(0.3f, 0.65f, 1.f, 1.f);

	UPROPERTY(Config, EditAnywhere, Category = "Graph")
	FLinearColor DoubleVectorPinColor = FLinearColor(1.f, 0.9f, 0.2f, 1.f);

	UPROPERTY(Config, EditAnywhere, Category = "Graph")
	FLinearColor DoubleVector2DPinColor = FLinearColor(0.5f, 1.f, 1.f, 1.f);

	// When the Voxel graph context menu is invoked (e.g. by right-clicking in the graph or dragging off a pin), do not block the UI while populating the available actions list.
	UPROPERTY(Config, EditAnywhere, Category = "Graph", meta = (DisplayName = "Enable Non-Blocking Context Menu"))
	bool bEnableContextMenuTimeSlicing = true;

	// The maximum amount of time (in milliseconds) allowed per frame for Voxel graph context menu building when the non-blocking option is enabled.
	// Larger values will complete the menu build in fewer frames, but will also make the UI feel less responsive.
	// Smaller values will make the UI feel more responsive, but will also take longer to fully populate the menu.
	UPROPERTY(Config, EditAnywhere, Category = "Graph", AdvancedDisplay, meta = (EditCondition = "bEnableContextMenuTimeSlicing", DisplayName = "Context Menu: Non-Blocking Per-Frame Threshold (ms)", ClampMin = "1"))
	int32 ContextMenuTimeSlicingThresholdMs = 15;

	UPROPERTY(Config, EditAnywhere, Category = "Hotkeys")
	TArray<FVoxelGraphEditorInputBinding> Hotkeys;
};