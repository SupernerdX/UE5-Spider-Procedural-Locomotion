// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNodeEvaluator.h"
#include "Sculpt/Height/Tools/VoxelHeightTool.h"
#include "VoxelParameterOverridesOwner.h"
#include "VoxelGraphHeightTool.generated.h"

class AStaticMeshActor;
class UVoxelHeightSculptGraph;
struct FVoxelOutputNode_OutputSculptHeight;

UCLASS(DisplayName = "Graph", meta = (Order = 10, Icon = "ClassIcon.Blueprint"))
class VOXEL_API UVoxelGraphHeightTool
	: public UVoxelHeightTool
	, public IVoxelParameterOverridesObjectOwner
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelHeightSculptGraph> Graph;

	UPROPERTY()
	FVoxelParameterOverrides ParameterOverrides;

public:
	//~ Begin UVoxelTool Interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick() override;
	//~ End UVoxelTool Interface

	//~ Begin UVoxelHeightTool Interface
	virtual TSharedPtr<FVoxelHeightModifier> GetModifier(float StrengthMultiplier) const override;
	//~ End UVoxelHeightTool Interface

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

	TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptHeight> GetEvaluator() const;
};