// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "StaticMesh/VoxelStaticMeshMetadata.h"
#include "StaticMesh/VoxelStaticMeshSettings.h"
#include "VoxelStaticMesh.generated.h"

class UVoxelStaticMesh;
class FVoxelStaticMeshData;
class UVoxelSurfaceTypeInterface;

UCLASS(BlueprintType, meta = (VoxelAssetType, AssetColor=Red))
class VOXEL_API UVoxelStaticMesh : public UVoxelAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", AssetRegistrySearchable)
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = 0.00001))
	int32 VoxelSize = 20;

	// Relative to the size
	// Bigger = higher memory usage but more accurate distance outside the mesh bounds
	UPROPERTY(EditAnywhere, Category = "Config", meta = (UIMin = 0, UIMax = 1))
	float BoundsExtension = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FVoxelStaticMeshMetadata> Metadatas;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ShowOnlyInnerProperties))
	FVoxelStaticMeshSettings VoxelizerSettings;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Config", AssetRegistrySearchable)
	TSet<FName> Tags;

	UPROPERTY(EditAnywhere, Category = "Preview")
	TObjectPtr<UVoxelSurfaceTypeInterface> PreviewMaterial;

	UPROPERTY()
	bool bPreviewAssetVoxelSize = true;

	UPROPERTY()
	int32 PreviewVoxelSize = 100;
#endif

public:
#if WITH_EDITOR
	FSimpleMulticastDelegate OnChanged_EditorOnly;
#endif

#if WITH_EDITOR
	void BakeMetadatas_EditorOnly();
	void CreateMeshData_EditorOnly();
	void OnReimport();
#endif

public:
	TSharedPtr<const FVoxelStaticMeshData> GetData();
	TSharedPtr<const FVoxelStaticMeshData> GetData_NoPrepare();

public:
	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

private:
	TSharedPtr<const FVoxelStaticMeshData> PrivateData;

public:
#if WITH_EDITOR
	static void Migrate(
		TObjectPtr<UStaticMesh>& OldMesh,
		TObjectPtr<UVoxelStaticMesh>& NewMesh);
#endif
};