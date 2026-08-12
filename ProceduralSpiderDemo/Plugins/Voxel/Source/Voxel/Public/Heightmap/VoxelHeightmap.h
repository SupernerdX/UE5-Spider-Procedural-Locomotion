// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightmap.generated.h"

class UVoxelHeightmap_Height;
class UVoxelHeightmap_Weight;
class UVoxelSurfaceTypeInterface;

UCLASS(BlueprintType, meta = (VoxelAssetType, AssetColor=Red))
class VOXEL_API UVoxelHeightmap : public UVoxelAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (Units = cm))
	float ScaleXY = 100;

	UPROPERTY()
	TObjectPtr<UObject> DefaultMaterial;

	// Default surface used as base when no weightmap is applied
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelSurfaceTypeInterface> DefaultSurfaceType;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Config", AssetRegistrySearchable)
	TSet<FName> Tags;
#endif

public:
	UPROPERTY()
	TObjectPtr<UVoxelHeightmap_Height> Height;

	UPROPERTY()
	TArray<TObjectPtr<UVoxelHeightmap_Weight>> Weights;

public:
#if WITH_EDITOR
	FSimpleMulticastDelegate OnChanged_EditorOnly;
#endif

	UVoxelHeightmap();

#if WITH_EDITOR
	void MigrateMaterials();
#endif

	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface
};