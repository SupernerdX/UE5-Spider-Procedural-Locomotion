// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelGraphParameters.h"
#include "VoxelRuntimePinValue.h"

class UVoxelGraph;
class UVoxelTerminalGraph;
class FVoxelCompiledGraph;
class IVoxelParameterOverridesOwner;
struct FVoxelParameterOverrides;

class VOXELGRAPH_API FVoxelGraphEnvironment : public TSharedFromThis<FVoxelGraphEnvironment>
{
public:
	const TVoxelObjectPtr<UObject> Owner;
	const TSharedRef<const FVoxelCompiledGraph> RootCompiledGraph;
	const TVoxelMap<FGuid, FVoxelRuntimePinValue> ParameterGuidToValue;
	const FTransform LocalToWorld;
	const FTransform2d LocalToWorld2D;
	const bool bIsPreviewScene;

public:
	static TSharedPtr<FVoxelGraphEnvironment> Create(
		TVoxelObjectPtr<UObject> Owner,
		const IVoxelParameterOverridesOwner& OverridesOwner,
		const FTransform& LocalToWorld,
		FVoxelDependencyCollector& CompiledGraphDependencyCollector);

	static TSharedRef<FVoxelGraphEnvironment> Create(
		TVoxelObjectPtr<UObject> Owner,
		const UVoxelGraph& Graph,
		const FVoxelParameterOverrides& Overrides,
		const FTransform& LocalToWorld,
		FVoxelDependencyCollector& CompiledGraphDependencyCollector);

	static TSharedPtr<FVoxelGraphEnvironment> CreatePreview(
		TVoxelObjectPtr<UObject> Owner,
		const IVoxelParameterOverridesOwner& OverridesOwner,
		const FTransform& LocalToWorld,
		FVoxelDependencyCollector& CompiledGraphDependencyCollector);

private:
	FVoxelGraphEnvironment(
		bool bIsPreviewScene,
		TVoxelObjectPtr<UObject> Owner,
		const TSharedRef<const FVoxelCompiledGraph>& RootCompiledGraph,
		TVoxelMap<FGuid, FVoxelRuntimePinValue>&& ParameterGuidToValue,
		const FTransform& LocalToWorld);

	TVoxelArray<TSharedRef<FVoxelGraphParameters::FUniformParameter>> UniformParameters;
	TVoxelArray<const FVoxelGraphParameters::FUniformParameter*> DefaultNameIndexToUniformParameter;

	static TSharedRef<FVoxelGraphEnvironment> CreateImpl(
		bool bIsPreviewScene,
		TVoxelObjectPtr<UObject> Owner,
		const UVoxelGraph& Graph,
		const FVoxelParameterOverrides& Overrides,
		const FTransform& LocalToWorld,
		FVoxelDependencyCollector& CompiledGraphDependencyCollector);

	friend class FVoxelGraphContext;

public:
	template<typename T, typename... ArgTypes>
	requires
	(
		FVoxelGraphParameters::IsUniform<T> &&
		std::is_constructible_v<T, ArgTypes&&...>
	)
	T& AddParameter(ArgTypes&&... Args)
	{
		const TSharedRef<T> Parameter = MakeShared<T>(Forward<ArgTypes>(Args)...);
		UniformParameters.Add(Parameter);

		const int32 ParameterIndex = FVoxelGraphParameterManager::GetUniformIndex<T>();

		if (DefaultNameIndexToUniformParameter.Num() <= ParameterIndex)
		{
			DefaultNameIndexToUniformParameter.SetNumZeroed(FVoxelGraphParameterManager::NumUniform());
		}
		DefaultNameIndexToUniformParameter[ParameterIndex] = &Parameter.Get();

		return *Parameter;
	}
};