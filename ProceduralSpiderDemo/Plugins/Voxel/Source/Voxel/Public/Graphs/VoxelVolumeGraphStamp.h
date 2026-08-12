// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeStamp.h"
#include "VoxelParameterOverridesOwner.h"
#include "VoxelVolumeGraphStamp.generated.h"

class UVoxelVolumeGraph;
struct FVoxelOutputNode_OutputVolume;

USTRUCT(meta = (ShortName = "Graph", Icon = "ClassIcon.Blueprint", SortOrder = 2))
struct VOXEL_API FVoxelVolumeGraphStamp final
	: public FVoxelVolumeStamp
#if CPP
	, public IVoxelParameterOverridesOwner
#endif
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelVolumeGraph> Graph;

	UPROPERTY()
	FVoxelParameterOverrides ParameterOverrides;

public:
	TSharedPtr<FVoxelGraphEnvironment> CreateEnvironment(
		const FVoxelStampRuntime& Runtime,
		FVoxelDependencyCollector& DependencyCollector) const;

public:
	//~ Begin IVoxelParameterOverridesOwner Interface
	virtual TSharedPtr<IVoxelParameterOverridesOwner> GetShared() const override;
	virtual bool ShouldForceEnableOverride(const FGuid& Guid) const override;
	virtual UVoxelGraph* GetGraph() const override;
	virtual FVoxelParameterOverrides& GetParameterOverrides() override;
	virtual FProperty* GetParameterOverridesProperty() const override;
	//~ End IVoxelParameterOverridesOwner Interface

public:
	//~ Begin FVoxelVolumeStamp Interface
	virtual UObject* GetAsset() const override;
	virtual void FixupProperties() override;
	//~ End FVoxelVolumeStamp Interface
};

USTRUCT()
struct VOXEL_API FVoxelVolumeGraphStampRuntime : public FVoxelVolumeStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_BODY(FVoxelVolumeGraphStamp)

public:
	TVoxelNodeEvaluator<FVoxelOutputNode_OutputVolume> Evaluator;
	FVoxelBox LocalBounds;
	TSharedPtr<FVoxelRuntimeMetadataOverrides> MetadataOverrides;

public:
	//~ Begin FVoxelVolumeStampRuntime Interface
	virtual bool Initialize(FVoxelDependencyCollector& DependencyCollector) override;
	virtual bool Initialize_Parallel(FVoxelDependencyCollector& DependencyCollector) override;
	virtual FVoxelBox GetLocalBounds() const override;
	virtual bool ShouldUseQueryPrevious() const override;

	virtual void Apply(
		const FVoxelVolumeBulkQuery& Query,
		const FVoxelVolumeTransform& StampToQuery) const override;

	virtual void Apply(
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery) const override;
	//~ End FVoxelVolumeStampRuntime Interface
};