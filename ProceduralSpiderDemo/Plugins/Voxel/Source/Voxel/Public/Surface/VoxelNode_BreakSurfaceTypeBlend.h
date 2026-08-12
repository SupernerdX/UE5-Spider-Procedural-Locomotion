// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelSurfaceTypeBuffer.h"
#include "VoxelSurfaceTypeBlendBuffer.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelNode_BreakSurfaceTypeBlend.generated.h"

USTRUCT(Category = "Surface Type")
struct VOXEL_API FVoxelNode_BreakSurfaceTypeBlend : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceTypeBlend, nullptr);

public:
	FVoxelNode_BreakSurfaceTypeBlend();

	//~ Begin FVoxelNode Interface
	virtual void Initialize(FInitializer& Initializer) override;
	virtual void Compute(FVoxelGraphQuery Query) const override;
	virtual void PostSerialize() override;
	//~ End FVoxelNode Interface

public:
	struct FLayerPin
	{
		TPinRef_Output<FVoxelSurfaceTypeBuffer> Type;
		TPinRef_Output<FVoxelFloatBuffer> Weight;
	};
	TVoxelArray<FLayerPin> LayerPins;

	UPROPERTY()
	int32 NumLayerPins = 3;

	void FixupLayerPins();

public:
#if WITH_EDITOR
	class FDefinition : public Super::FDefinition
	{
	public:
		GENERATED_VOXEL_NODE_DEFINITION_BODY(FVoxelNode_BreakSurfaceTypeBlend);

		virtual FString GetAddPinLabel() const override;
		virtual FString GetAddPinTooltip() const override;

		virtual bool CanAddInputPin() const override;
		virtual void AddInputPin() override;

		virtual bool CanRemoveInputPin() const override;
		virtual void RemoveInputPin() override;
	};
#endif
};