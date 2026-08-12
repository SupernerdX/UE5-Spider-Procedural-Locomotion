// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelNode_Equal.generated.h"

// Returns true if A is exactly equal to B (A == B)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "==", Keywords = "== equal"))
struct VOXELGRAPH_API FVoxelNode_Equal : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, Result);

public:
	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;

#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
	virtual void PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType) override;
#endif
	//~ End FVoxelNode Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Returns true if A does not equal B (A != B)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "!=", Keywords = "!= not equal"))
struct VOXELGRAPH_API FVoxelNode_NotEqual : public FVoxelNode_Equal
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()
};