// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelNode;
struct FVoxelOutputNode;
class FVoxelGraphContext;
class FVoxelGraphEnvironment;
class FVoxelCompiledTerminalGraph;
class UVoxelTerminalGraph;

template<typename>
struct TVoxelNodeEvaluator;

struct VOXELGRAPH_API FVoxelNodeEvaluator
{
public:
	template<typename T>
	requires std::derived_from<T, FVoxelNode>
	static TVoxelNodeEvaluator<T> Create(
		const TSharedRef<const FVoxelGraphEnvironment>& Environment,
		const UVoxelTerminalGraph& TerminalGraph,
		const FVoxelNode& Node)
	{
		return ReinterpretCastRef<TVoxelNodeEvaluator<T>>(FVoxelNodeEvaluator::Create(
			StaticStructFast<T>(),
			Environment,
			&TerminalGraph,
			&Node));
	}

	template<typename T>
	requires std::derived_from<T, FVoxelOutputNode>
	static TVoxelNodeEvaluator<T> Create(const TSharedRef<const FVoxelGraphEnvironment>& Environment)
	{
		return ReinterpretCastRef<TVoxelNodeEvaluator<T>>(FVoxelNodeEvaluator::Create(
			StaticStructFast<T>(),
			Environment,
			nullptr,
			nullptr));
	}

public:
	FVoxelNodeEvaluator() = default;

	operator bool() const
	{
		return IsValid();
	}
	bool operator==(const FVoxelNodeEvaluator& Other) const = delete;

	bool IsValid() const
	{
		return Node != nullptr;
	}
	bool Equals_EnvironmentPtr(const FVoxelNodeEvaluator& Other) const
	{
		return
			Node == Other.Node &&
			Environment == Other.Environment &&
			TerminalGraph == Other.TerminalGraph;
	}
	bool Equals_SkipEnvironment(const FVoxelNodeEvaluator& Other) const
	{
		return
			Node == Other.Node &&
			TerminalGraph == Other.TerminalGraph;
	}

	FVoxelGraphContext MakeContext(FVoxelDependencyCollector& DependencyCollector) const;

protected:
	const FVoxelNode* Node = nullptr;
	TSharedPtr<const FVoxelGraphEnvironment> Environment;
	const FVoxelCompiledTerminalGraph* TerminalGraph = nullptr;

	static FVoxelNodeEvaluator Create(
		const UScriptStruct* Struct,
		const TSharedRef<const FVoxelGraphEnvironment>& Environment,
		const UVoxelTerminalGraph* TerminalGraph,
		const FVoxelNode* Node);
};

template<typename T>
struct TVoxelNodeEvaluator : FVoxelNodeEvaluator
{
	FORCEINLINE const T* operator->() const
	{
		checkVoxelSlow(IsValid());
		checkVoxelSlow(static_cast<const T*>(Node)->template IsA<T>());
		return static_cast<const T*>(Node);
	}

	template<typename OtherType>
	requires std::derived_from<T, OtherType>
	FORCEINLINE operator const TVoxelNodeEvaluator<OtherType>&() const
	{
		return ReinterpretCastRef<TVoxelNodeEvaluator<OtherType>>(*this);
	}
};