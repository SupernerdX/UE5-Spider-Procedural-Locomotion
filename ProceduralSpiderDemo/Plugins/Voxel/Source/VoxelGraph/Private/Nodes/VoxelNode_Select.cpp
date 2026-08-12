// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_Select.h"
#include "VoxelBufferAccessor.h"
#include "VoxelBufferSplitter.h"

FVoxelNode_Select::FVoxelNode_Select()
{
	GetPin(IndexPin).SetType(FVoxelPinType::Make<FVoxelBoolBuffer>());
	FixupValuePins();
}

void FVoxelNode_Select::Initialize(FInitializer& Initializer)
{
	for (FPinRef_Input& PinRef : ValuePins)
	{
		Initializer.InitializePinRef(PinRef);
	}
}

void FVoxelNode_Select::Compute(const FVoxelGraphQuery Query) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelPinType IndexType = IndexPin.GetType_RuntimeOnly();

	if (IndexType.Is<bool>() ||
		IndexType.Is<uint8>() ||
		IndexType.Is<int32>())
	{
		// Uniform select

		const FValue IndexValue = IndexPin.Get(Query);

		VOXEL_GRAPH_WAIT(IndexType, IndexValue)
		{
			const int32 Index = INLINE_LAMBDA -> int32
			{
				if (IndexType.Is<bool>())
				{
					return IndexValue.Get<bool>() ? 1 : 0;
				}
				else if (IndexType.Is<uint8>())
				{
					return IndexValue.Get<uint8>();
				}
				else
				{
					return IndexValue.Get<int32>();
				}
			};

			if (!ValuePins.IsValidIndex(Index))
			{
				return;
			}

			const FValue Value = ValuePins[Index].Get(Query);

			VOXEL_GRAPH_WAIT(Value)
			{
				ResultPin.Set(Query, Value);
			};
		};

		return;
	}

	if (!ensure(
		IndexType.Is<FVoxelBoolBuffer>() ||
		IndexType.Is<FVoxelByteBuffer>() ||
		IndexType.Is<FVoxelInt32Buffer>()))
	{
		return;
	}

	const TValue<FVoxelBuffer> Indices = IndexPin.Get<FVoxelBuffer>(Query);

	VOXEL_GRAPH_WAIT(IndexType, Indices)
	{
		if (Indices->Num_Slow() == 0)
		{
			ResultPin.Set(Query, FVoxelBuffer::MakeEmpty(ResultPin.GetInnerType_RuntimeOnly()));
			return;
		}

		if (Indices->IsConstant_Slow())
		{
			if (Indices->IsA<FVoxelBoolBuffer>())
			{
				checkVoxelSlow(ValuePins.Num() == 2);

				const FValue Value = ValuePins[Indices->AsChecked<FVoxelBoolBuffer>().GetConstant() ? 1 : 0].Get(Query);

				VOXEL_GRAPH_WAIT(Value)
				{
					ResultPin.Set(Query, Value);
				};
			}
			else if (Indices->IsA<FVoxelByteBuffer>())
			{
				const int32 Index = Indices->AsChecked<FVoxelByteBuffer>().GetConstant();
				if (!ValuePins.IsValidIndex(Index))
				{
					return;
				}

				const FValue Value = ValuePins[Index].Get(Query);

				VOXEL_GRAPH_WAIT(Value)
				{
					ResultPin.Set(Query, Value);
				};
			}
			else
			{
				const int32 Index = Indices->AsChecked<FVoxelInt32Buffer>().GetConstant();
				if (!ValuePins.IsValidIndex(Index))
				{
					return;
				}

				const FValue Value = ValuePins[Index].Get(Query);

				VOXEL_GRAPH_WAIT(Value)
				{
					ResultPin.Set(Query, Value);
				};
			}

			return;
		}

		const TSharedRef<FVoxelBufferSplitter> Splitter = INLINE_LAMBDA
		{
			if (Indices->IsA<FVoxelBoolBuffer>())
			{
				return MakeShared<FVoxelBufferSplitter>(Indices->AsChecked<FVoxelBoolBuffer>());
			}
			else if (Indices->IsA<FVoxelByteBuffer>())
			{
				return MakeShared<FVoxelBufferSplitter>(Indices->AsChecked<FVoxelByteBuffer>(), ValuePins.Num());
			}
			else
			{
				return MakeShared<FVoxelBufferSplitter>(Indices->AsChecked<FVoxelInt32Buffer>(), ValuePins.Num());
			}
		};
		checkVoxelSlow(Splitter->NumOutputs() == ValuePins.Num());

		if (const TVoxelOptional<int32> UniqueOutput = Splitter->GetUniqueOutput())
		{
			const FValue Value = ValuePins[UniqueOutput.GetValue()].Get(Query);

			VOXEL_GRAPH_WAIT(Value)
			{
				ResultPin.Set(Query, Value);
			};

			return;
		}

		const TVoxelArray<FVoxelGraphQueryImpl*> Queries = Query->Split(*Splitter);
		checkVoxelSlow(Queries.Num() == Splitter->NumOutputs());

		TVoxelArray<TVoxelOptional<TValue<FVoxelBuffer>>> Buffers;
		FVoxelUtilities::SetNum(Buffers, Splitter->NumOutputs());

		for (const int32 Index : Splitter->GetValidOutputs())
		{
			Buffers[Index] = ValuePins[Index].Get<FVoxelBuffer>(FVoxelGraphQuery(*Queries[Index], Query.GetCallstack()));
		}

		Query.AddTask([this, Query, Buffers, Splitter]
		{
			VOXEL_FUNCTION_COUNTER();

			TVoxelInlineArray<const FVoxelBuffer*, 8> LocalBuffers;
			FVoxelUtilities::SetNumZeroed(LocalBuffers, Splitter->NumOutputs());

			for (const int32 Index : Splitter->GetValidOutputs())
			{
				const FVoxelBuffer& Buffer = **Buffers[Index];

				if (!FVoxelBufferAccessor(Buffer, Splitter->GetOutputNum(Index)).IsValid())
				{
					RaiseBufferError();
					return;
				}

				LocalBuffers[Index] = &Buffer;
			}

			const TSharedRef<FVoxelBuffer> Buffer = FVoxelBuffer::MakeEmpty(ResultPin.GetInnerType_RuntimeOnly());
			Buffer->MergeFrom(*Splitter, LocalBuffers);

			ResultPin.Set(Query, Buffer);
		});
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_Select::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == IndexPin)
	{
		FVoxelPinTypeSet OutTypes;

		OutTypes.Add<bool>();
		OutTypes.Add<FVoxelBoolBuffer>();

		OutTypes.Add<int32>();
		OutTypes.Add<FVoxelInt32Buffer>();

		OutTypes.Add<uint8>();
		OutTypes.Add<FVoxelByteBuffer>();

		OutTypes.Add(FVoxelPinTypeSet::AllEnums());
		OutTypes.Add(FVoxelPinTypeSet::AllEnums().GetBufferTypes());

		return OutTypes;
	}
	else
	{
		return FVoxelPinTypeSet::All();
	}
}

void FVoxelNode_Select::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	if (Pin == IndexPin)
	{
		GetPin(IndexPin).SetType(NewType);

		const bool bIndexIsBuffer = GetPin(IndexPin).GetType().IsBuffer();
		const bool bResultIsBuffer = GetPin(ResultPin).GetType().IsBuffer();

		if (bIndexIsBuffer != bResultIsBuffer)
		{
			const FVoxelPinType ResultType = GetPin(ResultPin).GetType();
			if (bIndexIsBuffer)
			{
				GetPin(ResultPin).SetType(ResultType.GetBufferType());
			}
			else
			{
				GetPin(ResultPin).SetType(ResultType.GetInnerType());
			}
		}
	}
	else
	{
		GetPin(ResultPin).SetType(NewType);

		for (FPinRef& ValuePin : ValuePins)
		{
			GetPin(ValuePin).SetType(NewType);
		}

		const bool bIndexIsBuffer = GetPin(IndexPin).GetType().IsBuffer();
		const bool bResultIsBuffer = GetPin(ResultPin).GetType().IsBuffer();

		if (bIndexIsBuffer != bResultIsBuffer)
		{
			const FVoxelPinType IndexType = GetPin(IndexPin).GetType();
			GetPin(IndexPin).SetType(bResultIsBuffer ? IndexType.GetBufferType() : IndexType.GetInnerType());
		}
	}

	FixupValuePins();
}
#endif

void FVoxelNode_Select::PostSerialize()
{
	Super::PostSerialize();

	FixupValuePins();
}

void FVoxelNode_Select::FixupValuePins()
{
	for (const FPinRef& Pin : ValuePins)
	{
		RemovePin(Pin.GetName());
	}
	ValuePins.Reset();

	const FVoxelPinType IndexType = GetPin(IndexPin).GetType();

	if (IndexType.IsWildcard())
	{
		return;
	}

	const FVoxelPinType IndexInnerType = IndexType.GetInnerType();

	if (IndexInnerType.Is<bool>())
	{
		ValuePins.Add(CreateInputPin(FVoxelPinType::MakeWildcard(), "False", {}));
		ValuePins.Add(CreateInputPin(FVoxelPinType::MakeWildcard(), "True", {}));
	}
	else if (IndexInnerType.Is<int32>())
	{
		for (int32 Index = 0; Index < NumIntegerOptions; Index++)
		{
			ValuePins.Add(CreateInputPin(
				FVoxelPinType::MakeWildcard(),
				FName("Option", Index + 1),
				VOXEL_PIN_METADATA(
					void,
					nullptr,
					DisplayName("Option " + FString::FromInt(Index)))));
		}
	}
	else if (IndexInnerType.Is<uint8>())
	{
		if (const UEnum* Enum = IndexInnerType.GetEnum())
		{
			for (int32 EnumValue = 0; EnumValue < Enum->GetMaxEnumValue(); EnumValue++)
			{
				if (!Enum->IsValidEnumValue(EnumValue))
				{
					FName EnumName = Enum->GetFName();
					EnumName.SetNumber(EnumName.GetNumber() + EnumValue);

					FVoxelPinMetadata MetaData = VOXEL_PIN_METADATA(void, nullptr, HidePin);
					ValuePins.Add(CreateInputPin(FVoxelPinType::MakeWildcard(), EnumName, MetaData));
					continue;
				}

				const int32 EnumIndex = Enum->GetIndexByValue(EnumValue);

				FVoxelPinMetadata MetaData = VOXEL_PIN_METADATA(void, nullptr, DisplayName(Enum->GetDisplayNameTextByIndex(EnumIndex).ToString()));
#if WITH_EDITOR
				MetaData.bHidePin =
					Enum->HasMetaData(TEXT("Hidden"), EnumIndex) ||
					Enum->HasMetaData(TEXT("Spacer"), EnumIndex);
#endif

				ValuePins.Add(CreateInputPin(FVoxelPinType::MakeWildcard(), Enum->GetNameByIndex(EnumIndex), MetaData));
			}
		}
		else
		{
			for (int32 Index = 0; Index < NumIntegerOptions; Index++)
			{
				ValuePins.Add(CreateInputPin(
					FVoxelPinType::MakeWildcard(),
					FName("Option", Index + 1),
					VOXEL_PIN_METADATA(
						void,
						nullptr,
						DisplayName("Option " + FString::FromInt(Index)))));
			}
		}
	}
	else
	{
		ensure(false);
	}

	for (FPinRef& ValuePin : ValuePins)
	{
		GetPin(ValuePin).SetType(GetPin(ResultPin).GetType());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString FVoxelNode_Select::FDefinition::GetAddPinLabel() const
{
	return "Add Option";
}

FString FVoxelNode_Select::FDefinition::GetAddPinTooltip() const
{
	return "Adds a new option to the node";
}

FString FVoxelNode_Select::FDefinition::GetRemovePinTooltip() const
{
	return "Removes last option from the node";
}

bool FVoxelNode_Select::FDefinition::CanAddInputPin() const
{
	const FVoxelPinType IndexInnerType = Node.GetPin(Node.IndexPin).GetType().GetInnerType();
	if (IndexInnerType.Is<int32>())
	{
		return true;
	}

	return
		IndexInnerType.Is<uint8>() &&
		!IndexInnerType.GetEnum();
}

void FVoxelNode_Select::FDefinition::AddInputPin()
{
	Node.NumIntegerOptions++;
	Node.FixupValuePins();
}

bool FVoxelNode_Select::FDefinition::CanRemoveInputPin() const
{
	const FVoxelPinType IndexInnerType = Node.GetPin(Node.IndexPin).GetType().GetInnerType();
	if (Node.NumIntegerOptions <= 2)
	{
		return false;
	}

	if (IndexInnerType.Is<int32>())
	{
		return true;
	}

	return
		IndexInnerType.Is<uint8>() &&
		!IndexInnerType.GetEnum();
}

void FVoxelNode_Select::FDefinition::RemoveInputPin()
{
	Node.NumIntegerOptions--;
	Node.FixupValuePins();
}
#endif