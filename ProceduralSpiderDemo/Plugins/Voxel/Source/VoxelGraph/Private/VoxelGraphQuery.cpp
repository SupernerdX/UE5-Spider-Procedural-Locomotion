// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphQuery.h"
#include "VoxelGraphContext.h"
#include "VoxelCompiledGraph.h"
#include "VoxelBufferSplitter.h"
#include "VoxelCompiledTerminalGraph.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelGraphQueryImpl);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelGraphQueryImpl::FVoxelGraphQueryImpl(
	FVoxelGraphContext& Context,
	const FVoxelCompiledGraph& CompiledGraph,
	const FVoxelCompiledTerminalGraph& CompiledTerminalGraph)
	: Context(Context)
	, CompiledGraph(CompiledGraph)
	, CompiledTerminalGraph(CompiledTerminalGraph)
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(CompiledGraph.FindTerminalGraph(CompiledTerminalGraph.TerminalGraphGuid) == &CompiledTerminalGraph);

	ComputedNodes.SetNum(CompiledTerminalGraph.NumNodes, false);
	PinValues.SetNumZeroed(CompiledTerminalGraph.NumPins);
}

const FVoxelGraphQueryImpl& FVoxelGraphQueryImpl::GetChild(
	const FVoxelCompiledGraph& NewCompiledGraph,
	const FVoxelCompiledTerminalGraph& NewCompiledTerminalGraph,
	const FFunctionCallData* NewFunctionCallData) const
{
	if (ChildKeyToChild.Num() == 0)
	{
		ChildKeyToChild.Reserve(8);
	}

	const FVoxelGraphQueryImpl*& Child = ChildKeyToChild.FindOrAdd(FChildKey
	{
		&NewCompiledGraph,
		NewFunctionCallData ? &NewFunctionCallData->Node : nullptr,
		NewFunctionCallData ? &NewFunctionCallData->Query : nullptr
	});

	if (!Child)
	{
		FVoxelGraphQueryImpl* NewChild = Context.Allocate<FVoxelGraphQueryImpl>(
			Context,
			NewCompiledGraph,
			NewCompiledTerminalGraph);

		if (NewFunctionCallData)
		{
			NewChild->FunctionCallData = *NewFunctionCallData;
		}

		NewChild->NameIndexToUniformParameter_Ptr = NameIndexToUniformParameter_Ptr;
		NewChild->NameIndexToBufferParameter_Ptr = NameIndexToBufferParameter_Ptr;

		Child = NewChild;
	}

	checkVoxelSlow(&Child->Context == &Context);
	checkVoxelSlow(&Child->CompiledGraph == &NewCompiledGraph);
	checkVoxelSlow(&Child->CompiledTerminalGraph == &NewCompiledTerminalGraph);

	return *Child;
}

FVoxelGraphQueryImpl& FVoxelGraphQueryImpl::CloneParameters() const
{
	FVoxelGraphQueryImpl* Impl = Context.Allocate<FVoxelGraphQueryImpl>(
		Context,
		CompiledGraph,
		CompiledTerminalGraph);

	Impl->FunctionCallData = FunctionCallData;
	Impl->NameIndexToUniformParameter_Storage = *NameIndexToUniformParameter_Ptr;
	Impl->NameIndexToBufferParameter_Storage = *NameIndexToBufferParameter_Ptr;
	return *Impl;
}

TVoxelArray<FVoxelGraphQueryImpl*> FVoxelGraphQueryImpl::Split(const FVoxelBufferSplitter& Splitter) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Splitter.Num());

	TVoxelArray<FVoxelGraphQueryImpl*> Result;
	FVoxelUtilities::SetNumZeroed(Result, Splitter.NumOutputs());

	for (const int32 Index : Splitter.GetValidOutputs())
	{
		Result[Index] = &CloneParameters();
	}

	for (int32 ParameterIndex = 0; ParameterIndex < NameIndexToBufferParameter_Ptr->Num(); ParameterIndex++)
	{
		const FVoxelGraphParameters::FBufferParameter* Parameter = (*NameIndexToBufferParameter_Ptr)[ParameterIndex];
		if (!Parameter)
		{
			continue;
		}

		TVoxelInlineArray<FVoxelGraphParameters::FBufferParameter*, 8> NewParameters;
		FVoxelUtilities::SetNumZeroed(NewParameters, Splitter.NumOutputs());

		const FVoxelGraphParameterManager::FBufferInfo Info = GVoxelGraphParameterManager->GetBufferInfo(ParameterIndex);

		for (const int32 OutputIndex : Splitter.GetValidOutputs())
		{
			void* Data = Context.Allocate(Info.TypeSize);
			Info.Constructor(Data);
			Context.AddDeleter(Data, Info.Destructor);

			FVoxelGraphParameters::FBufferParameter* NewParameter = static_cast<FVoxelGraphParameters::FBufferParameter*>(Data);
			NewParameters[OutputIndex] = NewParameter;
			Result[OutputIndex]->NameIndexToBufferParameter_Storage[ParameterIndex] = NewParameter;
		}

		Info.Split(*Parameter, Splitter, NewParameters);
	}

	return Result;
}