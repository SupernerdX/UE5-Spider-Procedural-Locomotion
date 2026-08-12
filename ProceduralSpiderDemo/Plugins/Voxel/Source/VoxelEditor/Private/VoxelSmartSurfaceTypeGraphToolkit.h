// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelSimpleAssetToolkit.h"
#include "VoxelStampGraphViewportInterface.h"
#include "VoxelSmartSurfaceTypeGraphToolkit.generated.h"

class UVoxelSmartSurfaceTypeGraph;

class FVoxelSmartSurfaceTypeGraphViewportInterface : public FVoxelStampGraphViewportInterface
{
public:
	//~ Begin FVoxelStampGraphViewportInterface Interface
	virtual void PopulateOverlay(const TSharedRef<SOverlay>& Overlay) override;
	virtual void SetupVoxelWorld(AVoxelWorld& VoxelWorld) const override;
	virtual FVoxelStampRef MakeStampRef(UVoxelGraph& Graph) const override;
	virtual TOptional<float> GetBoundsSize(const FVoxelStampRef& StampRef) const override;
#if VOXEL_ENGINE_VERSION >= 506
	virtual void ExtendToolbar(UToolMenu& ToolMenu) override;
#endif
	//~ End FVoxelStampGraphViewportInterface Interface
};

USTRUCT()
struct FVoxelSmartSurfaceTypeGraphToolkit : public FVoxelToolkit
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelSmartSurfaceTypeGraph> Asset;

public:
	//~ Begin FVoxelToolkit Interface
	virtual TArray<FMode> GetModes() const override;
	virtual UScriptStruct* GetDefaultMode() const override;
	//~ End FVoxelToolkit Interface
};

USTRUCT(meta = (Internal))
struct FVoxelSmartSurfaceTypeGraphPreviewToolkit : public FVoxelSimpleAssetToolkit
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelSmartSurfaceTypeGraph> Asset;

public:
	//~ Begin FVoxelSimpleAssetToolkit Interface
	virtual void Initialize() override;
	virtual bool ShowFloor() const override { return false; }
	virtual void SetupPreview() override;
	virtual FRotator GetInitialViewRotation() const override;
	virtual TOptional<float> GetInitialViewDistance() const override;
	virtual void PostEditChange(const FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual TSharedPtr<SWidget> GetMenuOverlay() const override;
	//~ End FVoxelSimpleAssetToolkit Interface

private:
	TSharedPtr<FVoxelSmartSurfaceTypeGraphViewportInterface> ViewportInterface;
};