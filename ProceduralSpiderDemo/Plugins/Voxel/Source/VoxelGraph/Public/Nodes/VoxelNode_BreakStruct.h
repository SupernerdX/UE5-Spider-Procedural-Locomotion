// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelNode_BreakStruct.generated.h"

// Break a blueprint struct
USTRUCT(Category = "Struct", meta = (NativeBreakFunc))
struct VOXELGRAPH_API FVoxelNode_BreakStruct : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelWildcard, Struct, nullptr);

public:
	//~ Begin FVoxelNode Interface
	virtual void Initialize(FInitializer& Initializer) override;
	virtual void Compute(FVoxelGraphQuery Query) const override;

	#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
	virtual void PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType) override;
#endif

	virtual void PostSerialize() override;
	//~ End FVoxelNode Interface

public:
	void FixupOutputPins();

private:
	TVoxelArray<FPinRef_Output> OutputPins;
};