// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNodeEvaluator.h"
#include "Sculpt/Volume/Tools/VoxelVolumeTool.h"
#include "VoxelParameterOverridesOwner.h"
#include "VoxelGraphVolumeTool.generated.h"

class AStaticMeshActor;
class UVoxelVolumeSculptGraph;
struct FVoxelOutputNode_OutputSculptDistance;

UCLASS(DisplayName = "Graph", meta = (Order = 10, Icon = "ClassIcon.Blueprint"))
class VOXEL_API UVoxelGraphVolumeTool
	: public UVoxelVolumeTool
	, public IVoxelParameterOverridesObjectOwner
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelVolumeSculptGraph> Graph;

	UPROPERTY()
	FVoxelParameterOverrides ParameterOverrides;

public:
	//~ Begin UVoxelTool Interface
	virtual void Enter() override;
	virtual void Tick() override;
	//~ End UVoxelTool Interface

	//~ Begin UVoxelVolumeTool Interface
	virtual TSharedPtr<FVoxelVolumeModifier> GetModifier(float StrengthMultiplier) const override;
	//~ End UVoxelVolumeTool Interface

	//~ Begin IVoxelParameterOverridesObjectOwner Interface
	virtual bool ShouldForceEnableOverride(const FGuid& Guid) const override;
	virtual UVoxelGraph* GetGraph() const override;
	virtual FVoxelParameterOverrides& GetParameterOverrides() override;
	//~ End IVoxelParameterOverridesObjectOwner Interface

private:
	UPROPERTY()
	TObjectPtr<AStaticMeshActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;

	TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptDistance> GetEvaluator() const;
};