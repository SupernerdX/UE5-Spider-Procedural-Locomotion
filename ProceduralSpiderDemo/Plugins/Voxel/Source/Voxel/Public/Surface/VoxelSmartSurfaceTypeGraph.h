// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelGraph.h"
#include "VoxelSmartSurfacePreviewShape.h"
#include "VoxelSmartSurfaceTypeGraph.generated.h"

class UVoxelSmartSurfaceType;

UCLASS(BlueprintType, meta = (AssetSubMenu = "Graph", CustomAssetTypeActions))
class VOXEL_API UVoxelSmartSurfaceTypeGraph : public UVoxelGraph
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelGraph Interface
#if WITH_EDITOR
	virtual FFactoryInfo GetFactoryInfo() override;
#endif
	virtual UScriptStruct* GetOutputNodeStruct() const override;
	//~ End UVoxelGraph Interface

	UVoxelSmartSurfaceType* GetPreviewSurface();

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	EVoxelSmartSurfacePreviewShape PreviewShape = EVoxelSmartSurfacePreviewShape::Sphere;
#endif

private:
	UPROPERTY(Transient)
	TObjectPtr<UVoxelSmartSurfaceType> PreviewSurfaceType;
};