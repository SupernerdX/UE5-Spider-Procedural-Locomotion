// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraph.h"
#include "Nodes/VoxelGraphNode.h"
#include "VoxelEdGraph.generated.h"

UCLASS()
class UVoxelEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	void SetLatestVersion();
	void SetToolkit(const TSharedRef<FVoxelGraphToolkit>& Toolkit);
	TSharedPtr<FVoxelGraphToolkit> GetGraphToolkit() const;

	void MigrateIfNeeded();
	void MigrateAndReconstructAll();

	//~ Begin UEdGraph interface
	virtual void NotifyGraphChanged(const FEdGraphEditAction& Action) override;
	virtual void PostEditUndo() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void Serialize(FArchive& Ar) override;
	//~ End UEdGraph interface

public:
	UPROPERTY()
	TWeakObjectPtr<UEdGraphNode> WeakDebugNode;

private:
	TWeakPtr<FVoxelGraphToolkit> WeakToolkit;

private:
	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion,
		SplitInputSetterAndRemoveLocalVariablesDefault,
		AddFeatureScaleAmplitudeToBaseNoises,
		RemoveMacros,
		AddClampedLerpNodes,
		AddNormalizeToGradientNodes,
		AddVoxelMaterial,
		RemoveFunctionInputDefaultPin,
		UpdateOutputHeightRanges
	);

	UPROPERTY()
	int32 Version = -1;
};