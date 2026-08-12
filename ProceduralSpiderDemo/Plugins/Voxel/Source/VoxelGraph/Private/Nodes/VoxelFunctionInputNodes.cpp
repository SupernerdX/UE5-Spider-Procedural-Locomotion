// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelFunctionInputNodes.h"
#include "Nodes/VoxelCallFunctionNodes.h"

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_FunctionInputBase::GetPromotionTypes(const FVoxelPin& Pin) const
{
	return FVoxelPinTypeSet::All();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_FunctionInput_WithDefaults::Initialize(FInitializer& Initializer)
{
	if (!DefaultValue.IsValid())
	{
		return;
	}

	RuntimeDefaultValue = FVoxelPinType::MakeRuntimeValue(
		GetPin(ValuePin).GetType(),
		DefaultValue,
		{});
}

void FVoxelNode_FunctionInput_WithDefaults::Compute(const FVoxelGraphQuery Query) const
{
	if (!Query->FunctionCallData)
	{
		if (!Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No function call", this);
			return;
		}

		if (bHasPreviewNode)
		{
			const FValue Value = PreviewPin.Get(Query);

			VOXEL_GRAPH_WAIT(Value)
			{
				ValuePin.Set(Query, Value);
			};

			return;
		}

		if (bHasDefaultNode)
		{
			ensure(!RuntimeDefaultValue.IsValid());

			const FValue Value = DefaultPin.Get(Query);

			VOXEL_GRAPH_WAIT(Value)
			{
				ValuePin.Set(Query, Value);
			};

			return;
		}

		if (RuntimeDefaultValue.IsValid())
		{
			ValuePin.Set(Query, RuntimeDefaultValue);
			return;
		}

		VOXEL_MESSAGE(Error, "{0}: Input is NoDefault and there is no Set Preview Value node in this function", this);
		return;
	}

	const FVoxelNode_CallFunction* CallFunctionNode = Query->FunctionCallData->Node.As<FVoxelNode_CallFunction>();
	if (!ensure(CallFunctionNode))
	{
		VOXEL_MESSAGE(Error, "{0}: No matching input found on {1}", this, Query->FunctionCallData->Node);
		return;
	}

	const TSharedPtr<FPinRef_Input> InputPinRef = CallFunctionNode->GuidToInputPinRef.FindRef(Guid);
	if (!ensureVoxelSlow(InputPinRef))
	{
		VOXEL_MESSAGE(Error, "{0}: No matching input found on {1}", this, Query->FunctionCallData->Node);
		return;
	}

	if (InputPinRef->IsDefaultValue() &&
		!InputPinRef->GetType_RuntimeOnly().HasPinDefaultValue() &&
		!bHasDefaultNode)
	{
		if (RuntimeDefaultValue.IsValid())
		{
			ValuePin.Set(Query, RuntimeDefaultValue);
			return;
		}

		VOXEL_MESSAGE(Error, "{0}: No valid default value for input found", this);
		return;
	}

	if (InputPinRef->IsDefaultValue() &&
		bHasDefaultNode)
	{
		const FValue Value = DefaultPin.Get(Query);

		VOXEL_GRAPH_WAIT(Value)
		{
			ValuePin.Set(Query, Value);
		};

		return;
	}

	const FVoxelGraphQueryImpl& NewQuery = INLINE_LAMBDA -> const FVoxelGraphQueryImpl&
	{
		const FVoxelGraphQueryImpl& ParentQuery = Query->FunctionCallData->Query;

		if (ParentQuery.HasSameParameters(Query.GetImpl()))
		{
			// No parameter changes were made, we can reuse the same query
			return ParentQuery;
		}

		return Query->GetChild(
			ParentQuery.CompiledGraph,
			ParentQuery.CompiledTerminalGraph,
			ParentQuery.FunctionCallData.GetPtrOrNull());
	};

	const FValue Value = InputPinRef->Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));

	VOXEL_GRAPH_WAIT(Value)
	{
		ValuePin.Set(Query, Value);
	};
}