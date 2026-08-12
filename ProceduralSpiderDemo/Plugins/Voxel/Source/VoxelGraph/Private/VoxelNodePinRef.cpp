// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNodePinRef.h"
#include "VoxelNode.h"
#include "VoxelBuffer.h"
#include "VoxelGraphPositionParameter.h"

void FVoxelNode::FPinRef_Input::ComputeLinkedNode(const FVoxelGraphQuery Query) const
{
	if (LinkedPinMetadata.bNoCache)
	{
		LinkedNode->ComputeNoCachePin(Query, LinkedPinIndex);
	}
	else
	{
		LinkedNode->ComputeIfNeeded(Query, LinkedPinIndex);
	}
}

FVoxelRuntimePinValue FVoxelNode::FPinRef_Input::GetSynchronous(const FVoxelGraphQueryImpl& Query) const
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_COUNTER_FNAME(Name);

	FVoxelGraphCallstack* Callstack = nullptr;
#if WITH_EDITOR
	Callstack = &Query.Context.Callstacks_EditorOnly.Emplace_GetRef();
	Callstack->Node = OuterNode;
#endif

	const FValue Value = Get(FVoxelGraphQuery(Query, Callstack));
	Query.Context.Execute();

	FVoxelRuntimePinValue Result = Value.GetValue();
	if (!Result.IsBuffer() ||
		Type.IsBufferArray())
	{
		return Result;
	}

	// If we output a buffer, we expect to have a Position query parameter & we expect their nums to match

	const int32 BufferNum = Result.Get<FVoxelBuffer>().Num_Slow();

	const int32 PositionNum = INLINE_LAMBDA
	{
		if (const FVoxelGraphParameters::FPosition2D* Position = Query.FindParameter<FVoxelGraphParameters::FPosition2D>())
		{
			return Position->Num();
		}

		if (const FVoxelGraphParameters::FPosition3D* Position = Query.FindParameter<FVoxelGraphParameters::FPosition3D>())
		{
			return Position->Num();
		}

		ensure(false);
		return BufferNum;
	};

	if (BufferNum != PositionNum && BufferNum != 1)
	{
		OuterNode->RaiseBufferError();
		return FVoxelRuntimePinValue::Make(FVoxelBuffer::MakeDefault(Type.GetInnerType()));
	}

	return Result;
}