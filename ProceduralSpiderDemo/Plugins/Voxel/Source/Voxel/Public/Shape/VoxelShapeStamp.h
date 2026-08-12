// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelShape.h"
#include "VoxelVolumeStamp.h"
#include "Surface/VoxelSurfaceType.h"
#include "Components/StaticMeshComponent.h"
#include "VoxelShapeStamp.generated.h"

class FVoxelStaticMeshData;
class UVoxelSurfaceTypeInterface;

USTRUCT(meta = (ShortName = "Shape", Icon = "ClassIcon.Cube", SortOrder = 4, NoK2Make))
struct VOXEL_API FVoxelShapeStamp final : public FVoxelVolumeStamp
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ShowOnlyInnerProperties, NoK2, BaseStruct = "/Script/Voxel.VoxelShape"))
#if CPP
	TVoxelInstancedStruct<FVoxelShape> Shape;
#else
	FVoxelInstancedStruct Shape;
#endif

	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelSurfaceTypeInterface> SurfaceType;

public:
	//~ Begin FVoxelVolumeStamp Interface
#if WITH_EDITOR
	virtual FString GetIdentifier() const override;
	virtual void SetupPreview(IPreview& Preview) const override;
#endif
	//~ End FVoxelVolumeStamp Interface
};

USTRUCT()
struct VOXEL_API FVoxelShapeStampRuntime : public FVoxelVolumeStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_BODY(FVoxelShapeStamp)

public:
	TSharedPtr<const FVoxelShape> Shape;
	FVoxelSurfaceType SurfaceType;
	TSharedPtr<FVoxelRuntimeMetadataOverrides> MetadataOverrides;

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