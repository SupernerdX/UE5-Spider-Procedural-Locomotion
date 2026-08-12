// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightStamp.h"
#include "VoxelParameterOverridesOwner.h"
#include "VoxelHeightGraphStamp.generated.h"

class UVoxelHeightGraph;
struct FVoxelOutputNode_OutputHeight;

USTRUCT(meta = (ShortName = "Graph", Icon = "ClassIcon.Blueprint", SortOrder = 2))
struct VOXEL_API FVoxelHeightGraphStamp final
	: public FVoxelHeightStamp
#if CPP
	, public IVoxelParameterOverridesOwner
#endif
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelHeightGraph> Graph;

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
	//~ Begin FVoxelHeightStamp Interface
	virtual UObject* GetAsset() const override;
	virtual void FixupProperties() override;
	//~ End FVoxelHeightStamp Interface
};

USTRUCT()
struct VOXEL_API FVoxelHeightGraphStampRuntime : public FVoxelHeightStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_BODY(FVoxelHeightGraphStamp)

public:
	TVoxelNodeEvaluator<FVoxelOutputNode_OutputHeight> Evaluator;
	FVoxelBox2D LocalBounds;
	FVoxelFloatRange HeightRange;
	bool bRelativeHeightRange = false;
	TSharedPtr<FVoxelRuntimeMetadataOverrides> MetadataOverrides;

public:
	//~ Begin FVoxelHeightStampRuntime Interface
	virtual bool Initialize(FVoxelDependencyCollector& DependencyCollector) override;
	virtual bool Initialize_Parallel(FVoxelDependencyCollector& DependencyCollector) override;
	virtual FVoxelBox GetLocalBounds() const override;
	virtual bool ShouldUseQueryPrevious() const override;
	virtual bool HasRelativeHeightRange() const override;

	virtual void Apply(
		const FVoxelHeightBulkQuery& Query,
		const FVoxelHeightTransform& StampToQuery) const override;

	virtual void Apply(
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery) const override;
	//~ End FVoxelHeightStampRuntime Interface
};