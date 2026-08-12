// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelSimpleAssetToolkit.h"
#include "VoxelStampGraphViewportInterface.h"
#include "VoxelHeightGraphToolkit.generated.h"

class UVoxelHeightGraph;

class FVoxelHeightGraphViewportInterface : public FVoxelStampGraphViewportInterface
{
public:
	//~ Begin FVoxelStampGraphViewportInterface Interface
	virtual FVoxelStampRef MakeStampRef(UVoxelGraph& Graph) const override;
	virtual TOptional<float> GetBoundsSize(const FVoxelStampRef& StampRef) const override;
	//~ End FVoxelStampGraphViewportInterface Interface
};

USTRUCT()
struct FVoxelHeightGraphToolkit : public FVoxelToolkit
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelHeightGraph> Asset;

public:
	//~ Begin FVoxelToolkit Interface
	virtual TArray<FMode> GetModes() const override;
	virtual UScriptStruct* GetDefaultMode() const override;
	//~ End FVoxelToolkit Interface
};

USTRUCT(meta = (Internal))
struct FVoxelHeightGraphPreviewToolkit : public FVoxelSimpleAssetToolkit
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelHeightGraph> Asset;

public:
	//~ Begin FVoxelSimpleAssetToolkit Interface
	virtual void Initialize() override;
	virtual bool ShowFloor() const override { return false; }
	virtual void SetupPreview() override;
	virtual FRotator GetInitialViewRotation() const override;
	virtual TOptional<float> GetInitialViewDistance() const override;
	virtual TOptional<float> GetMaxFocusDistance() const override;
	virtual void PostEditChange(const FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual TSharedPtr<SWidget> GetMenuOverlay() const override;
	//~ End FVoxelSimpleAssetToolkit Interface

private:
	TSharedPtr<FVoxelHeightGraphViewportInterface> ViewportInterface;
};