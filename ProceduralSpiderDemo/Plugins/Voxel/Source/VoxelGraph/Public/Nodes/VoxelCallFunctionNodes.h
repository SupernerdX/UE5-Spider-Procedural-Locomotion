// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelCallFunctionNodes.generated.h"

struct FVoxelNode_FunctionOutput;
class UVoxelFunctionLibraryAsset;

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_CallFunction : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	TVoxelMap<FGuid, TSharedPtr<FPinRef_Input>> GuidToInputPinRef;

	//~ Begin FVoxelNode Interface
	virtual void Initialize(FInitializer& Initializer) override;
	//~ End FVoxelNode Interface

public:
	virtual void Link(FVoxelDependencyCollector& DependencyCollector);

	virtual void FixupPins(
		const UVoxelGraph& Context,
		const FOnVoxelGraphChanged& OnChanged,
		const FOnVoxelGraphChanged& OnForceRecompile) VOXEL_PURE_VIRTUAL();

protected:
	void FixupPinsImpl(
		const UVoxelTerminalGraph* BaseTerminalGraph,
		const FOnVoxelGraphChanged& OnChanged);

	bool ComputeImpl(
		FVoxelGraphQuery Query,
		int32 PinIndex,
		const FVoxelCompiledGraph& CompiledGraph,
		const FGuid& TerminalGraphGuid) const;

	FORCEINLINE bool IsNodeComputed(const FVoxelGraphQuery& Query) const
	{
		return Query->IsNodeComputed(PrivateNodeIndex);
	}

protected:
	struct FOutputPin
	{
		FGuid Guid;
		FPinRef_Output PinRef;
	};
	TVoxelArray<FOutputPin> OutputPins;

	TVoxelMap<int32, int32> PinIndexToOutputPinIndex;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_CallMemberFunction : public FVoxelNode_CallFunction
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	UPROPERTY()
	FGuid Guid;

	UPROPERTY()
	TObjectPtr<UVoxelGraph> ContextOverride;

public:
	//~ Begin FVoxelNode_CallFunction Interface
	virtual void Link(FVoxelDependencyCollector& DependencyCollector) override;

	virtual void FixupPins(
		const UVoxelGraph& Context,
		const FOnVoxelGraphChanged& OnChanged,
		const FOnVoxelGraphChanged& OnForceRecompile) override;
	//~ End FVoxelNode_CallFunction Interface

	//~ Begin FVoxelNode Interface
	virtual void ComputeIfNeeded(
		FVoxelGraphQuery Query,
		int32 PinIndex) const override;
	//~ End FVoxelNode Interface

private:
	TSharedPtr<const FVoxelCompiledGraph> ContextOverrideCompiledGraph;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_CallExternalFunction : public FVoxelNode_CallFunction
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	UPROPERTY()
	TObjectPtr<UVoxelFunctionLibraryAsset> FunctionLibrary;

	UPROPERTY()
	FGuid Guid;

	//~ Begin FVoxelNode_CallFunction Interface
	virtual void Link(FVoxelDependencyCollector& DependencyCollector) override;

	virtual void FixupPins(
		const UVoxelGraph& Context,
		const FOnVoxelGraphChanged& OnChanged,
		const FOnVoxelGraphChanged& OnForceRecompile) override;
	//~ End FVoxelNode_CallFunction Interface

	//~ Begin FVoxelNode Interface
	virtual void ComputeIfNeeded(
		FVoxelGraphQuery Query,
		int32 PinIndex) const override;
	//~ End FVoxelNode Interface

private:
	TSharedPtr<const FVoxelCompiledGraph> CompiledGraph;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_CallParentMainGraph : public FVoxelNode_CallFunction
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	UPROPERTY()
	TObjectPtr<UVoxelGraph> ContextOverride;

	//~ Begin FVoxelNode Interface
	virtual void Initialize(FInitializer& Initializer) override;
	virtual void ComputeIfNeeded(
		FVoxelGraphQuery Query,
		int32 PinIndex) const override;
	virtual void Link(FVoxelDependencyCollector& DependencyCollector) override;

	virtual void FixupPins(
		const UVoxelGraph& Context,
		const FOnVoxelGraphChanged& OnChanged,
		const FOnVoxelGraphChanged& OnForceRecompile) override;
	//~ End FVoxelNode Interface

private:
	bool ComputeImpl(
		FVoxelGraphQuery Query,
		int32 PinIndex,
		const FVoxelCompiledGraph& CompiledGraph) const;

private:
	TSharedPtr<const FVoxelCompiledGraph> ContextOverrideCompiledGraph;
};