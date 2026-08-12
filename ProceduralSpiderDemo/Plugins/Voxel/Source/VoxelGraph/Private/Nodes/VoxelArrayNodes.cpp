// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelArrayNodes.h"
#include "VoxelBuffer.h"

void FVoxelNode_ArrayLength::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelBuffer> Values = ValuesPin.Get<FVoxelBuffer>(Query);

	VOXEL_GRAPH_WAIT(Values)
	{
		ResultPin.Set(Query, Values->Num_Slow());
	};
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_ArrayLength::GetPromotionTypes(const FVoxelPin& Pin) const
{
	return FVoxelPinTypeSet::AllBufferArrays();
}

void FVoxelNode_ArrayLength::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Pin.SetType(NewType);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_ArrayGetItem::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelBuffer> Values = ValuesPin.Get<FVoxelBuffer>(Query);

	if (AreTemplatePinsBuffers())
	{
		const TValue<FVoxelInt32Buffer> Indices = IndexPin.GetBuffer(Query);

		VOXEL_GRAPH_WAIT(Values, Indices)
		{
			const int32 NumValues = Values->Num_Slow();
			for (const int32 Index : *Indices)
			{
				if (!(0 <= Index && Index < NumValues))
				{
					VOXEL_MESSAGE(Error, "{0}: Invalid Index {1}. Values.Num={2}", this, Index, NumValues);
					return;
				}
			}

			ResultPin.Set(Query, Values->Gather(Indices->View()));
		};
	}
	else
	{
		const TValue<int32> Index = IndexPin.GetUniform(Query);

		VOXEL_GRAPH_WAIT(Values, Index)
		{
			const int32 NumValues = Values->Num_Slow();
			if (!(0 <= Index && Index < NumValues))
			{
				VOXEL_MESSAGE(Error, "{0}: Invalid Index {1}. Values.Num={2}", this, Index, NumValues);
				return;
			}

			ResultPin.Set(Query, Values->GetGeneric(Index));
		};
	}
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_ArrayGetItem::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == IndexPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add(FVoxelPinType::Make<int32>());
		OutTypes.Add(FVoxelPinType::Make<FVoxelInt32Buffer>());
		return OutTypes;
	}
	else if (Pin == ValuesPin)
	{
		return FVoxelPinTypeSet::AllBufferArrays();
	}
	else
	{
		ensure(Pin == ResultPin);
		return FVoxelPinTypeSet::All();
	}
}

void FVoxelNode_ArrayGetItem::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	if (Pin == IndexPin)
	{
		GetPin(IndexPin).SetType(NewType);

		if (NewType.IsBuffer())
		{
			GetPin(ResultPin).SetType(GetPin(ResultPin).GetType().GetBufferType());
		}
		else
		{
			GetPin(ResultPin).SetType(GetPin(ResultPin).GetType().GetInnerType());
		}
	}
	else if (Pin == ValuesPin)
	{
		GetPin(ValuesPin).SetType(NewType.GetBufferType().WithBufferArray(true));

		if (GetPin(IndexPin).GetType().IsBuffer())
		{
			GetPin(ResultPin).SetType(NewType.GetBufferType().WithBufferArray(false));
		}
		else
		{
			GetPin(ResultPin).SetType(NewType.GetInnerType());
		}
	}
	else
	{
		ensure(Pin == ResultPin);

		GetPin(ValuesPin).SetType(NewType.GetBufferType().WithBufferArray(true));
		Pin.SetType(NewType);

		if (NewType.IsBuffer())
		{
			GetPin(IndexPin).SetType(FVoxelPinType::Make<FVoxelInt32Buffer>());
		}
		else
		{
			GetPin(IndexPin).SetType(FVoxelPinType::Make<int32>());
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_MakeArray::Compute(const FVoxelGraphQuery Query) const
{
	const TVoxelArray<FValue> Items = ItemPins.Get(Query);

	VOXEL_GRAPH_WAIT(Items)
	{
		if (Items.Num() == 0)
		{
			return;
		}

		const TSharedRef<FVoxelBuffer> Buffer = FVoxelBuffer::MakeEmpty(ResultPin.GetInnerType_RuntimeOnly());
		Buffer->Allocate(Items.Num());

		for (int32 Index = 0; Index < Items.Num(); Index++)
		{
			Buffer->SetGeneric(Index, Items[Index]);
		}

		ResultPin.Set(Query, FVoxelRuntimePinValue::Make(Buffer));
	};
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_MakeArray::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == ResultPin)
	{
		return FVoxelPinTypeSet::AllBufferArrays();
	}
	else
	{
		return FVoxelPinTypeSet::AllUniforms();
	}
}

void FVoxelNode_MakeArray::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	GetPin(ResultPin).SetType(NewType.GetBufferType().WithBufferArray(true));

	for (const FName ItemPin : GetVariadicPinPinNames(ItemPins))
	{
		FindPin(ItemPin)->SetType(NewType.GetInnerType());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FName FVoxelNode_MakeArray::FDefinition::Variadic_AddPinTo(const FName VariadicPinName)
{
	ensure(!VariadicPinName.IsNone());
	const FName NewPin = Node.Variadic_AddPin(VariadicPinName);

	const FVoxelPinType ResultPinType = Node.GetPin(Node.ResultPin).GetType();
	if (!ResultPinType.IsWildcard())
	{
		Node.GetPin(FPinRef(NewPin)).SetType(ResultPinType.GetInnerType());
	}

	return NewPin;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_AddToArray::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelBuffer> Array = ArrayPin.Get<FVoxelBuffer>(Query);
	const TVoxelArray<FValue> Items = ItemPins.Get(Query);

	VOXEL_GRAPH_WAIT(Array, Items)
	{
		const int32 NumValues = Items.Num();
		if (NumValues == 0)
		{
			ResultPin.Set(Query, FVoxelRuntimePinValue::Make(Array));
			return;
		}

		const int32 ArrayNum = Array->Num_Slow();

		const TSharedRef<FVoxelBuffer> Buffer = FVoxelBuffer::MakeEmpty(ResultPin.GetInnerType_RuntimeOnly());
		Buffer->Allocate(ArrayNum + Items.Num());
		Buffer->CopyFrom(*Array, 0, 0, ArrayNum);

		for (int32 Index = 0; Index < Items.Num(); Index++)
		{
			Buffer->SetGeneric(ArrayNum + Index, Items[Index]);
		}

		ResultPin.Set(Query, FVoxelRuntimePinValue::Make(Buffer));
	};
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_AddToArray::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == ArrayPin ||
		Pin == ResultPin)
	{
		return FVoxelPinTypeSet::AllBufferArrays();
	}
	else
	{
		return FVoxelPinTypeSet::AllUniforms();
	}
}

void FVoxelNode_AddToArray::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	GetPin(ArrayPin).SetType(NewType.GetBufferType().WithBufferArray(true));
	GetPin(ResultPin).SetType(NewType.GetBufferType().WithBufferArray(true));

	for (const FName ItemPin : GetVariadicPinPinNames(ItemPins))
	{
		FindPin(ItemPin)->SetType(NewType.GetInnerType());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FName FVoxelNode_AddToArray::FDefinition::Variadic_AddPinTo(const FName VariadicPinName)
{
	ensure(!VariadicPinName.IsNone());
	const FName NewPin = Node.Variadic_AddPin(VariadicPinName);

	const FVoxelPinType ResultPinType = Node.GetPin(Node.ResultPin).GetType();
	if (!ResultPinType.IsWildcard())
	{
		Node.GetPin(FPinRef(NewPin)).SetType(ResultPinType.GetInnerType());
	}

	return NewPin;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_AppendArray::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelBuffer> A = APin.Get<FVoxelBuffer>(Query);
	const TValue<FVoxelBuffer> B = BPin.Get<FVoxelBuffer>(Query);

	VOXEL_GRAPH_WAIT(A, B)
	{
		const int32 NumA = A->Num_Slow();
		const int32 NumB = B->Num_Slow();

		const TSharedRef<FVoxelBuffer> Buffer = FVoxelBuffer::MakeEmpty(ResultPin.GetInnerType_RuntimeOnly());
		Buffer->Allocate(NumA + NumB);
		Buffer->CopyFrom(*A, 0, 0, NumA);
		Buffer->CopyFrom(*B, 0, NumA, NumB);
		ResultPin.Set(Query, FVoxelRuntimePinValue::Make(Buffer));
	};
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_AppendArray::GetPromotionTypes(const FVoxelPin& Pin) const
{
	return FVoxelPinTypeSet::AllBufferArrays();
}

void FVoxelNode_AppendArray::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	GetPin(APin).SetType(NewType.GetBufferType().WithBufferArray(true));
	GetPin(BPin).SetType(NewType.GetBufferType().WithBufferArray(true));
	GetPin(ResultPin).SetType(NewType.GetBufferType().WithBufferArray(true));
}
#endif