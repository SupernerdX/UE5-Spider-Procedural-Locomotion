// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelNode_HeightSplitter.generated.h"

USTRUCT(Category = "Math|Misc")
struct VOXELGRAPH_API FVoxelNode_HeightSplitter : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Height, nullptr);

public:
	FVoxelNode_HeightSplitter();

	//~ Begin FVoxelNode Interface
	virtual void Initialize(FInitializer& Initializer) override;
	virtual void Compute(FVoxelGraphQuery Query) const override;
	virtual void PostSerialize() override;
	//~ End FVoxelNode Interface

public:
	struct FLayerPin
	{
		TPinRef_Input<FVoxelFloatBuffer> Height;
		TPinRef_Input<FVoxelFloatBuffer> Falloff;
	};
	TVoxelArray<FLayerPin> LayerPins;
	TVoxelArray<TPinRef_Output<FVoxelFloatBuffer>> ResultPins;

	UPROPERTY()
	int32 NumLayerPins = 1;

	void FixupLayerPins();

public:
#if WITH_EDITOR
	class FDefinition : public Super::FDefinition
	{
	public:
		GENERATED_VOXEL_NODE_DEFINITION_BODY(FVoxelNode_HeightSplitter);

		virtual FString GetAddPinLabel() const override;
		virtual FString GetAddPinTooltip() const override;

		virtual bool CanAddInputPin() const override { return true; }
		virtual void AddInputPin() override;

		virtual bool CanRemoveInputPin() const override;
		virtual void RemoveInputPin() override;
	};
#endif
};