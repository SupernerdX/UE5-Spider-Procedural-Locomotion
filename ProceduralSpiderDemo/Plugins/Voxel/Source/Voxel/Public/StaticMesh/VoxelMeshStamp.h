// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeStamp.h"
#include "Surface/VoxelSurfaceType.h"
#include "Components/StaticMeshComponent.h"
#include "VoxelMeshStamp.generated.h"

class UVoxelStaticMesh;
class UVoxelNormalMetadata;
class UVoxelSurfaceTypeInterface;
class FVoxelStaticMeshData;

USTRUCT(meta = (ShortName = "Mesh", Icon = "ClassIcon.StaticMesh", SortOrder = 0))
struct VOXEL_API FVoxelMeshStamp final : public FVoxelVolumeStamp
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config", DisplayName = "Mesh")
	TObjectPtr<UVoxelStaticMesh> NewMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> Mesh;

	UPROPERTY()
	TObjectPtr<UObject> Material;

	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelSurfaceTypeInterface> SurfaceType;

	// Tricubic interpolation is ~3x slower but better looking
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	bool bUseTricubic = true;

public:
	//~ Begin FVoxelVolumeStamp Interface
	virtual UObject* GetAsset() const override;
	virtual void FixupProperties() override;
#if WITH_EDITOR
	virtual void SetupPreview(IPreview& Preview) const override;
#endif
	//~ End FVoxelVolumeStamp Interface
};

USTRUCT()
struct VOXEL_API FVoxelMeshStampRuntime : public FVoxelVolumeStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_BODY(FVoxelMeshStamp)

public:
	TSharedPtr<const FVoxelStaticMeshData> MeshData;
	FVoxelSurfaceType SurfaceType;
	TSharedPtr<FVoxelRuntimeMetadataOverrides> MetadataOverrides;
	TVoxelMap<FVoxelMetadataRef, TSharedPtr<const FVoxelBuffer>> MetadataRefToBuffer;

public:
	//~ Begin FVoxelVolumeStampRuntime Interface
	virtual bool Initialize(FVoxelDependencyCollector& DependencyCollector) override;
	virtual FVoxelBox GetLocalBounds() const override;

	virtual void Apply(
		const FVoxelVolumeBulkQuery& Query,
		const FVoxelVolumeTransform& StampToQuery) const override;

	virtual void Apply(
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery) const override;
	//~ End FVoxelVolumeStampRuntime Interface
};