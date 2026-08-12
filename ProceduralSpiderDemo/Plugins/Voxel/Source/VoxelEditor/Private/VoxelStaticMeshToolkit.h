// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelSimpleAssetToolkit.h"
#include "StaticMesh/VoxelStaticMesh.h"
#include "VoxelStaticMeshToolkit.generated.h"

class AVoxelWorld;
class AVoxelStampActor;

USTRUCT()
struct FVoxelStaticMeshToolkit : public FVoxelSimpleAssetToolkit
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelStaticMesh> Asset;

public:
	//~ Begin FVoxelSimpleAssetToolkit Interface
	virtual void Tick() override;
	virtual bool ShowFloor() const override { return false; }
	virtual void SetupPreview() override;
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual TOptional<float> GetInitialViewDistance() const override;
	virtual void PopulateToolBar(const TSharedRef<SHorizontalBox>& ToolbarBox, const TSharedPtr<SViewportToolBar>& ParentToolBarPtr) override;
	virtual void PostEditChange(const FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostUndo() override;
	//~ End FVoxelSimpleAssetToolkit Interface

private:
	UPROPERTY()
	TObjectPtr<AVoxelWorld> VoxelWorld;

	UPROPERTY()
	TObjectPtr<AVoxelStampActor> StampActor;
};