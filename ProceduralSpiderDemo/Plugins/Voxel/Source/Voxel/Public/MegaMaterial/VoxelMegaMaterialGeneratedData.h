// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "MegaMaterial/VoxelMegaMaterialGeneratedMaterial.h"
#include "MegaMaterial/VoxelCubicTextureArrayData.h"
#include "VoxelMegaMaterialGeneratedData.generated.h"

class UVoxelMetadata;
class UVoxelMegaMaterial;
class UVoxelSurfaceTypeAsset;
class FGlobalComponentRecreateRenderStateContext;

USTRUCT()
struct FVoxelMegaMaterialSurfaceInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelSurfaceTypeAsset> SurfaceType;

	UPROPERTY()
	TArray<TObjectPtr<UVoxelMetadata>> UsedMetadatas;
};

UCLASS()
class VOXEL_API UVoxelMegaMaterialGeneratedData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FVoxelMaterialRenderIndex, FVoxelMegaMaterialSurfaceInfo> IndexToSurfaceInfo;

	UPROPERTY()
	TMap<FVoxelMaterialRenderIndex, FVoxelMegaMaterialMaterialGeneratedMaterial> IndexToGeneratedMaterial;

	UPROPERTY()
	TArray<TObjectPtr<UVoxelMetadata>> MetadataIndexToMetadata;

	UPROPERTY()
	TMap<EVoxelMegaMaterialTarget, TObjectPtr<UMaterialInterface>> TargetToMaterial;

	// Generated texture array data for cubic surface types
	UPROPERTY()
	FVoxelCubicTextureArrayData CubicTextureArrayData;

public:
	UVoxelMegaMaterial* GetMegaMaterial();
	void SetMegaMaterial(UVoxelMegaMaterial* NewMegaMaterial);

	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface

public:
#if WITH_EDITOR
	void ForceRebuild();
	void QueueRebuild();
	bool IsRebuildQueued() const;
	void RebuildNow();
	static void RebuildIfNeeded(const UObject* ChangedObject);
#endif

private:
	UPROPERTY()
	TSoftObjectPtr<UVoxelMegaMaterial> SoftMegaMaterial;

	UPROPERTY()
	TMap<EVoxelMegaMaterialTarget, FVoxelMegaMaterialTargetGeneratedMaterial> TargetToGeneratedMaterial;

	TVoxelObjectPtr<UVoxelMegaMaterial> WeakMegaMaterial;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TSet<TObjectPtr<const UObject>> WatchedObjects;
#endif

#if WITH_EDITOR
	void GenerateMaterialForTarget(
		const UVoxelMegaMaterial& MegaMaterial,
		EVoxelMegaMaterialTarget Target,
		const FVoxelMegaMaterialTargetBaseState& BaseState,
		TSharedPtr<FGlobalComponentRecreateRenderStateContext>& Context);

	void GenerateMaterial(
		const UVoxelMegaMaterial& MegaMaterial,
		const FVoxelMegaMaterialMaterialState& State,
		FVoxelMegaMaterialMaterialGeneratedMaterial& GeneratedMaterial,
		TSharedPtr<FGlobalComponentRecreateRenderStateContext>& Context);
#endif
};
