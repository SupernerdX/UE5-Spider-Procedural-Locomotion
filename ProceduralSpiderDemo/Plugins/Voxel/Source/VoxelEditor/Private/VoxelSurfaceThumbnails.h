// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelThumbnailRenderers.h"
#include "VoxelSurfaceThumbnails.generated.h"

class FMaterialThumbnailScene;

UCLASS()
class UVoxelSurfaceAssetThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()

public:
	//~ Begin UDefaultSizedThumbnailRenderer Interface
	virtual void BeginDestroy() override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	virtual bool CanVisualizeAsset(UObject* Object) override;
	//~ End UDefaultSizedThumbnailRenderer Interface

private:
	TSharedPtr<FMaterialThumbnailScene> ThumbnailScene;
};