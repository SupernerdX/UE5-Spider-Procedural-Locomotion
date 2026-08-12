// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_Equal.h"
#include "VoxelBufferAccessor.h"

void FVoxelNode_Equal::Compute(const FVoxelGraphQuery Query) const
{
	const FValue A = APin.Get(Query);
	const FValue B = BPin.Get(Query);

	VOXEL_GRAPH_WAIT(A, B)
	{
		checkVoxelSlow(APin.GetType_RuntimeOnly() == BPin.GetType_RuntimeOnly());

		const FVoxelPinType Type = APin.GetType_RuntimeOnly();

		if (!Type.IsBuffer())
		{
			const bool bIsEqual = INLINE_LAMBDA
			{
				if (!Type.IsStruct())
				{
					switch (Type.GetInternalType())
					{
					default: ensure(false); return false;
					case EVoxelPinInternalType::Bool: return A.Get<bool>() == B.Get<bool>();
					case EVoxelPinInternalType::Float: return A.Get<float>() == B.Get<float>();
					case EVoxelPinInternalType::Double: return A.Get<double>() == B.Get<double>();
					case EVoxelPinInternalType::UInt16: return A.Get<uint16>() == B.Get<uint16>();
					case EVoxelPinInternalType::Int32: return A.Get<int32>() == B.Get<int32>();
					case EVoxelPinInternalType::Int64: return A.Get<int64>() == B.Get<int64>();
					case EVoxelPinInternalType::Name: return A.Get<FName>() == B.Get<FName>();
					case EVoxelPinInternalType::Byte: return A.Get<uint8>() == B.Get<uint8>();
					case EVoxelPinInternalType::Class: return A.Get<TSubclassOf<UObject>>() == B.Get<TSubclassOf<UObject>>();
					}
				}
				checkVoxelSlow(Type.IsStruct());

				return A.GetStructView().Identical(B.GetStructView());
			};

			bool bResult = bIsEqual;
			if (GetStruct() == StaticStructFast<FVoxelNode_NotEqual>())
			{
				bResult = !bResult;
			}
			ResultPin.Set(Query, bResult);

			return;
		}

		const FVoxelBuffer& BufferA = A.Get<FVoxelBuffer>();
		const FVoxelBuffer& BufferB = B.Get<FVoxelBuffer>();

		const FVoxelBufferAccessor BufferAccessor(BufferA, BufferB);
		if (!BufferAccessor.IsValid())
		{
			RaiseBufferError();
			return;
		}

		const TSharedRef<FVoxelBoolBuffer> Result = MakeShared<FVoxelBoolBuffer>();
		Result->Allocate(BufferAccessor.Num());

		BufferA.BulkEqual(
			BufferB,
			Result->View());

		if (GetStruct() == StaticStructFast<FVoxelNode_NotEqual>())
		{
			for (bool& bValue : Result->View())
			{
				bValue = !bValue;
			}
		}

		ResultPin.Set(Query, Result);
	};
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_Equal::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == ResultPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add<bool>();
		OutTypes.Add<FVoxelBoolBuffer>();
		return OutTypes;
	}

	return FVoxelPinTypeSet::All();
}

void FVoxelNode_Equal::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	if (Pin == ResultPin)
	{
		GetPin(ResultPin).SetType(NewType);
		GetPin(APin).SetType(NewType.IsBuffer() ? GetPin(APin).GetType().GetBufferType() : GetPin(APin).GetType().GetInnerType());
		GetPin(BPin).SetType(NewType.IsBuffer() ? GetPin(BPin).GetType().GetBufferType() : GetPin(BPin).GetType().GetInnerType());
	}
	else
	{
		GetPin(ResultPin).SetType(NewType.IsBuffer() ? GetPin(ResultPin).GetType().GetBufferType() : GetPin(ResultPin).GetType().GetInnerType());
		GetPin(APin).SetType(NewType);
		GetPin(BPin).SetType(NewType);
	}
}
#endif