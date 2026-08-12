// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPointSet.h"
#include "Nodes/VoxelOutputNode.h"
#include "VoxelOutputNode_OutputPoints.generated.h"

USTRUCT()
struct VOXELPCG_API FVoxelOutputNode_OutputPoints : public FVoxelOutputNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, Points, nullptr);

public:
	//~ Begin FVoxelNode Interface
	virtual void Initialize(FInitializer& Initializer) override;
	virtual void PostSerialize() override;
	//~ End FVoxelNode Interface

public:
	TVoxelArray<TPinRef_Input<FVoxelPointSet>> InputPins;

	UPROPERTY()
	TArray<FName> PinNames;

	void FixupInputPins();

public:
#if WITH_EDITOR
	class FDefinition : public Super::FDefinition
	{
	public:
		GENERATED_VOXEL_NODE_DEFINITION_BODY(FVoxelOutputNode_OutputPoints);

		virtual FString GetAddPinLabel() const override;
		virtual FString GetAddPinTooltip() const override;

		virtual bool CanAddInputPin() const override { return true; }
		virtual void AddInputPin() override;

		virtual bool CanRemoveInputPin() const override;
		virtual void RemoveInputPin() override;

		virtual bool CanRemoveSelectedPin(FName PinName) const override;
		virtual void RemoveSelectedPin(FName PinName) override;

		virtual bool CanRenameSelectedPin(FName PinName) const override;
		virtual bool IsNewPinNameValid(FName PinName, FName NewName) const override;
		virtual void RenameSelectedPin(FName PinName, FName NewName) override;
	};
#endif
};