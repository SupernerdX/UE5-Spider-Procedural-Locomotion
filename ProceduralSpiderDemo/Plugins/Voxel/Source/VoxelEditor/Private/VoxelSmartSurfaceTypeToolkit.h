// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelSimpleAssetToolkit.h"
#include "Surface/VoxelSmartSurfaceType.h"
#include "VoxelSmartSurfaceTypeToolkit.generated.h"

class AVoxelWorld;
class AVoxelStampActor;

USTRUCT()
struct FVoxelSmartSurfaceTypeToolkit : public FVoxelSimpleAssetToolkit
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY()
	TObjectPtr<UVoxelSmartSurfaceType> Asset;

public:
	//~ Begin FVoxelSimpleAssetToolkit Interface
	virtual void Initialize() override;
	virtual bool ShowFloor() const override { return false; }
	virtual void SetupPreview() override;
	virtual void PopulateOverlay(const TSharedRef<SOverlay>& Overlay) override;
#if VOXEL_ENGINE_VERSION >= 506
	virtual void ExtendToolbar(UToolMenu& ToolMenu) override;
#endif
	//~ End FVoxelSimpleAssetToolkit Interface

private:
	UPROPERTY()
	TObjectPtr<AVoxelWorld> VoxelWorld;

	UPROPERTY()
	TObjectPtr<AVoxelStampActor> StampActor;
};