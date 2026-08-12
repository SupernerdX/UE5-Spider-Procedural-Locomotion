// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphEnvironment.h"
#include "VoxelGraph.h"
#include "VoxelExternalParameter.h"
#include "VoxelParameterView.h"
#include "VoxelParameterOverridesOwner.h"

TSharedPtr<FVoxelGraphEnvironment> FVoxelGraphEnvironment::Create(
	const TVoxelObjectPtr<UObject> Owner,
	const IVoxelParameterOverridesOwner& OverridesOwner,
	const FTransform& LocalToWorld,
	FVoxelDependencyCollector& CompiledGraphDependencyCollector)
{
	const UVoxelGraph* Graph = OverridesOwner.GetGraph();
	if (!Graph)
	{
		return {};
	}

	return CreateImpl(
		false,
		Owner,
		*Graph,
		OverridesOwner.GetParameterOverrides(),
		LocalToWorld,
		CompiledGraphDependencyCollector);
}

TSharedRef<FVoxelGraphEnvironment> FVoxelGraphEnvironment::Create(
	const TVoxelObjectPtr<UObject> Owner,
	const UVoxelGraph& Graph,
	const FVoxelParameterOverrides& Overrides,
	const FTransform& LocalToWorld,
	FVoxelDependencyCollector& CompiledGraphDependencyCollector)
{
	return CreateImpl(
		false,
		Owner,
		Graph,
		Overrides,
		LocalToWorld,
		CompiledGraphDependencyCollector);
}

TSharedPtr<FVoxelGraphEnvironment> FVoxelGraphEnvironment::CreatePreview(
	const TVoxelObjectPtr<UObject> Owner,
	const IVoxelParameterOverridesOwner& OverridesOwner,
	const FTransform& LocalToWorld,
	FVoxelDependencyCollector& CompiledGraphDependencyCollector)
{
	const UVoxelGraph* Graph = OverridesOwner.GetGraph();
	if (!Graph)
	{
		return {};
	}

	return CreateImpl(
		true,
		Owner,
		*Graph,
		OverridesOwner.GetParameterOverrides(),
		LocalToWorld,
		CompiledGraphDependencyCollector);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelGraphEnvironment::FVoxelGraphEnvironment(
	const bool bIsPreviewScene,
	const TVoxelObjectPtr<UObject> Owner,
	const TSharedRef<const FVoxelCompiledGraph>& RootCompiledGraph,
	TVoxelMap<FGuid, FVoxelRuntimePinValue>&& ParameterGuidToValue,
	const FTransform& LocalToWorld)
	: Owner(Owner)
	, RootCompiledGraph(RootCompiledGraph)
	, ParameterGuidToValue(MoveTemp(ParameterGuidToValue))
	, LocalToWorld(LocalToWorld)
	, LocalToWorld2D(FVoxelUtilities::MakeTransform2(LocalToWorld))
	, bIsPreviewScene(bIsPreviewScene)
{
}

TSharedRef<FVoxelGraphEnvironment> FVoxelGraphEnvironment::CreateImpl(
	const bool bIsPreviewScene,
	const TVoxelObjectPtr<UObject> Owner,
	const UVoxelGraph& Graph,
	const FVoxelParameterOverrides& Overrides,
	const FTransform& LocalToWorld,
	FVoxelDependencyCollector& CompiledGraphDependencyCollector)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedRef<const FVoxelCompiledGraph> CompiledGraph = Graph.GetCompiledGraph(CompiledGraphDependencyCollector);

	TVoxelArray<const FVoxelParameterOverrides*> AllParameterOverrides;
	{
		AllParameterOverrides.Reserve(16);
		AllParameterOverrides.Add(&Overrides);

		for (const UVoxelGraph* BaseGraph : Graph.GetBaseGraphs())
		{
			// If OverridesOwner is a graph it'll be added twice but that's fine
			AllParameterOverrides.Add(&ConstCast(BaseGraph)->GetParameterOverrides());
		}
	}

	TVoxelMap<FGuid, FVoxelRuntimePinValue> NewParameterGuidToValue;
	{
		VOXEL_SCOPE_COUNTER_NUM("Build parameters", Graph.NumParameters());

		NewParameterGuidToValue.Reserve(Graph.NumParameters());

		const FVoxelPinType::FRuntimeValueContext RuntimeValueContext
		{
			Owner
		};

		Graph.ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
		{
			const FVoxelPinValue Value = INLINE_LAMBDA
			{
				for (const FVoxelParameterOverrides* ParameterOverrides : AllParameterOverrides)
				{
					const FVoxelParameterValueOverride* Override = ParameterOverrides->GuidToValueOverride.Find(Guid);
					if (!Override ||
						!Override->bEnable)
					{
						continue;
					}

					return Override->Value;
				}

				return FVoxelPinValue(Parameter.Type.GetExposedType());
			};

			FVoxelRuntimePinValue RuntimeValue = FVoxelPinType::MakeRuntimeValue(
				Parameter.Type,
				Value,
				RuntimeValueContext);

			ensure(RuntimeValue.IsValid());

			if (RuntimeValue.GetType().CanBeCastedTo<FVoxelExternalParameter>())
			{
				ConstCast(RuntimeValue.Get<FVoxelExternalParameter>()).ParameterGuid = Guid;
			}

			NewParameterGuidToValue.Add_EnsureNew(Guid, MoveTemp(RuntimeValue));
		});
	}

	return MakeShareable(new FVoxelGraphEnvironment(
		bIsPreviewScene,
		Owner,
		CompiledGraph,
		MoveTemp(NewParameterGuidToValue),
		LocalToWorld));
}