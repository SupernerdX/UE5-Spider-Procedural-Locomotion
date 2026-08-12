// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelISPCNodeHelpers.h"
#include "VoxelComputeNode.generated.h"

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelComputeNode : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	struct FCode
	{
		TArray<FString> Includes;

		void AddInclude(const FString& Include)
		{
			Includes.Add(Include);
		}
	};
	virtual FString GenerateCode(FCode& Code) const VOXEL_PURE_VIRTUAL({});

public:
	//~ Begin FVoxelNode Interface
	virtual bool IsPureNode() const override
	{
		return true;
	}

	virtual void Initialize(FInitializer& Initializer) override;
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface

private:
	FVoxelNodeISPCPtr CachedPtr = nullptr;
	TVoxelArray<FPinRef_Input> InputPins;
	TVoxelArray<FPinRef_Output> OutputPins;
};