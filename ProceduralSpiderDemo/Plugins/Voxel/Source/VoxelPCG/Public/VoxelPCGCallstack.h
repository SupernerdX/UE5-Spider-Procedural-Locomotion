// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMessageTokens.h"
#include "Graph/PCGStackContext.h"
#include "VoxelPCGCallstack.generated.h"

struct VOXELPCG_API FVoxelPCGCallstack
{
	TVoxelObjectPtr<const UPCGComponent> Component;
	TVoxelObjectPtr<const UPCGNode> Node;
	FPCGStack Stack;

	FVoxelPCGCallstack() = default;

	explicit FVoxelPCGCallstack(const FPCGContext& Context);
};

USTRUCT()
struct VOXELPCG_API FVoxelMessageToken_PCGCallstack : public FVoxelMessageToken_Object
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	TSharedPtr<const FVoxelPCGCallstack> Callstack;

	//~ Begin FVoxelMessageToken Interface
	virtual uint32 GetHash() const override;
	//~ End FVoxelMessageToken Interface
};