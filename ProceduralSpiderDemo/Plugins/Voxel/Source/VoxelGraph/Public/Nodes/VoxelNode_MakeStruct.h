// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelNode_MakeStruct.generated.h"

// Make a blueprint struct
USTRUCT(Category = "Struct", meta = (NativeMakeFunc))
struct VOXELGRAPH_API FVoxelNode_MakeStruct : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_OUTPUT_PIN(FVoxelWildcard, Struct);

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
	void FixupInputPins();

private:
	TVoxelArray<FPinRef_Input> InputPins;
};