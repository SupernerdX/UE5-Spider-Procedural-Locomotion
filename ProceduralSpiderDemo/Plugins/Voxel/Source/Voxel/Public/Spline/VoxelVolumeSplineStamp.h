// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSplineStamp.h"
#include "VoxelVolumeStamp.h"
#include "VoxelParameterOverridesOwner.h"
#include "Components/SplineComponent.h"
#include "VoxelVolumeSplineStamp.generated.h"

class UVoxelVolumeSplineGraph;
struct FVoxelSplineMetadataRuntime;
struct FVoxelOutputNode_OutputVolumeSpline;

USTRUCT(meta = (ShortName = "Spline", Icon = "ClassIcon.SplineComponent", SortOrder = 1))
struct VOXEL_API FVoxelVolumeSplineStamp final
	: public FVoxelVolumeStamp
#if CPP
	, public IVoxelParameterOverridesOwner
#endif
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelVolumeSplineGraph> Graph;

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
	virtual void FixupComponents(const IVoxelStampComponentInterface& Interface) override;
	virtual TVoxelArray<TSubclassOf<USceneComponent>> GetRequiredComponents() const override;
	//~ End FVoxelVolumeStamp Interface
};

USTRUCT()
struct VOXEL_API FVoxelVolumeSplineStampRuntime : public FVoxelVolumeStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_BODY(FVoxelVolumeSplineStamp)

public:
	TVoxelNodeEvaluator<FVoxelOutputNode_OutputVolumeSpline> Evaluator;
	float MaxWidth = 0.f;
	TSharedPtr<FVoxelRuntimeMetadataOverrides> MetadataOverrides;
	TSharedPtr<const FVoxelSplineMetadataRuntime> MetadataRuntime;

	bool bClosedLoop = false;
	int32 ReparamStepsPerSegment = 0;
	float SplineLength = 0.f;
	FSplineCurves SplineCurves;

	TVoxelArray<FVoxelSplineSegment> Segments;

public:
	//~ Begin FVoxelVolumeStampRuntime Interface
	virtual bool Initialize(FVoxelDependencyCollector& DependencyCollector) override;
	virtual bool Initialize_Parallel(FVoxelDependencyCollector& DependencyCollector) override;
	virtual FVoxelBox GetLocalBounds() const override;
	virtual bool CanPartiallyInvalidate() const override;
	virtual bool ShouldUseQueryPrevious() const override;
	virtual TVoxelInlineArray<FVoxelBox, 1> GetChildren() const override;

	virtual bool TryToPartiallyInvalidate(
		const FVoxelStampRuntime& PreviousRuntime,
		TVoxelArray<FVoxelBox>& OutLocalBoundsToInvalidate) const override;

	virtual void Apply(
		const FVoxelVolumeBulkQuery& Query,
		const FVoxelVolumeTransform& StampToQuery) const override;

	virtual void Apply(
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery) const override;
	//~ End FVoxelVolumeStampRuntime Interface
};