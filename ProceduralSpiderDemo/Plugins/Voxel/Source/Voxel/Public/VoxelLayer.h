// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelAssetIcon.h"
#include "VoxelLayer.generated.h"

enum class EVoxelLayerType : uint8;

UCLASS(BlueprintType, Abstract)
class VOXEL_API UVoxelLayer : public UVoxelAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Editor")
	FVoxelAssetIcon AssetIcon;
#endif

public:
	EVoxelLayerType GetType() const;

public:
	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface
};

UCLASS(meta = (VoxelAssetType, AssetColor=Grey))
class VOXEL_API UVoxelHeightLayer : public UVoxelLayer
{
	GENERATED_BODY()

public:
	static UVoxelHeightLayer* Default();
};

UCLASS(meta = (VoxelAssetType, AssetColor=Grey))
class VOXEL_API UVoxelVolumeLayer : public UVoxelLayer
{
	GENERATED_BODY()

public:
	static UVoxelVolumeLayer* Default();
};