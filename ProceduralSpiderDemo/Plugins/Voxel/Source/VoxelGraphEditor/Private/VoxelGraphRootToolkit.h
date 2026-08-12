// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraph.h"
#include "VoxelSimpleAssetToolkit.h"
#include "VoxelGraphRootToolkit.generated.h"

USTRUCT()
struct FVoxelGraphRootToolkit : public FVoxelToolkit
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelGraph> Asset;

public:
	//~ Begin FVoxelToolkit Interface
	virtual TArray<FMode> GetModes() const override;
	virtual UScriptStruct* GetDefaultMode() const override;
	//~ End FVoxelToolkit Interface
};

USTRUCT(meta = (Internal))
struct FVoxelGraphPreviewToolkit : public FVoxelSimpleAssetToolkit
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelGraph> Asset;

public:
	//~ Begin FVoxelSimpleAssetToolkit Interface
	virtual void Initialize() override;
	virtual bool ShowFloor() const override { return false; }
	virtual void SetupPreview() override;
	virtual TOptional<float> GetInitialViewDistance() const override;
	virtual TOptional<float> GetMaxFocusDistance() const override;
	virtual TSharedPtr<SWidget> GetMenuOverlay() const override;
	//~ End FVoxelSimpleAssetToolkit Interface
};