// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNodeEvaluator.h"
#include "Scatter/VoxelScatterNodeRef.h"

class FVoxelRuntime;
struct FVoxelConfig;
struct FVoxelSubsystem;
struct FVoxelGraphQueryImpl;
struct FVoxelNode_ScatterBase;

class VOXEL_API FVoxelScatterNodeRuntime : public TSharedFromThis<FVoxelScatterNodeRuntime>
{
public:
	FVoxelScatterNodeRuntime() = default;
	virtual ~FVoxelScatterNodeRuntime() = default;

public:
	FName GetName() const
	{
		return PrivateName;
	}
	float GetChunkSize() const
	{
		return PrivateChunkSize;
	}
	bool IsInvalidated() const
	{
		return DependencyTracker->IsInvalidated();
	}
	const TVoxelNodeEvaluator<FVoxelNode_ScatterBase>& GetEvaluator() const
	{
		return PrivateEvaluator;
	}
	TVoxelObjectPtr<AVoxelScatterActor> GetActorRef() const
	{
		return PrivateNodeRef.Actor;
	}

public:
	void Initialize(
		const FVoxelSubsystem& Subsystem,
		const FVoxelScatterNodeWeakRef& NodeRef,
		const TVoxelNodeEvaluator<FVoxelNode_ScatterBase>& Evaluator);

public:
	virtual	void Compute(
		const FVoxelSubsystem& Subsystem,
		const TVoxelArray<FVector>& Invokers) {}
	virtual void Render(FVoxelRuntime& Runtime) {}
	virtual void Destroy(FVoxelRuntime& Runtime) {}

protected:
	virtual void Initialize(FVoxelGraphQueryImpl& Query) {}

private:
	FVoxelScatterNodeWeakRef PrivateNodeRef;
	TVoxelNodeEvaluator<FVoxelNode_ScatterBase> PrivateEvaluator;
	FName PrivateName;
	float PrivateChunkSize = 0.f;
	TSharedPtr<FVoxelDependencyTracker> DependencyTracker;
};

template<typename NodeType>
requires std::derived_from<NodeType, FVoxelNode_ScatterBase>
class TVoxelScatterNodeRuntime : public FVoxelScatterNodeRuntime
{
public:
	const TVoxelNodeEvaluator<NodeType>& GetEvaluator() const
	{
		return static_cast<const TVoxelNodeEvaluator<NodeType>&>(static_cast<const FVoxelNodeEvaluator&>(FVoxelScatterNodeRuntime::GetEvaluator()));
	}
};