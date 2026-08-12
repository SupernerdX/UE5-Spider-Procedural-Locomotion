// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelObjectNodes.h"
#include "VoxelObjectPinType.h"
#include "VoxelBufferAccessor.h"

void FVoxelNode_IsValidObject::Compute(const FVoxelGraphQuery Query) const
{
	if (AreTemplatePinsBuffers())
	{
		const TValue<FVoxelBuffer> Objects = ObjectPin.Get<FVoxelBuffer>(Query);

		VOXEL_GRAPH_WAIT(Objects)
		{
			if (!ensure(Objects->GetInnerType().IsStruct()))
			{
				return;
			}

			const TSharedPtr<const FVoxelObjectPinType> ObjectPinType = FVoxelObjectPinType::StructToPinType().FindRef(Objects->GetInnerType().GetStruct());
			if (!ensure(ObjectPinType))
			{
				return;
			}

			FVoxelBoolBuffer Result;
			Result.Allocate(Objects->Num_Slow());

			for (int32 Index = 0; Index < Objects->Num_Slow(); Index++)
			{
				Result.Set(Index, !ObjectPinType->GetWeakObject(Objects->GetGeneric(Index).GetStructView()).IsExplicitlyNull());
			}

			ResultPin.Set(Query, MoveTemp(Result));
		};
	}
	else
	{
		const FValue Object = ObjectPin.Get(Query);

		VOXEL_GRAPH_WAIT(Object)
		{
			if (!ensure(Object.GetType().IsStruct()))
			{
				return;
			}

			const TSharedPtr<const FVoxelObjectPinType> ObjectPinType = FVoxelObjectPinType::StructToPinType().FindRef(Object.GetType().GetStruct());
			if (!ensure(ObjectPinType))
			{
				return;
			}

			const bool bIsValid = !ObjectPinType->GetWeakObject(Object.GetStructView()).IsExplicitlyNull();

			ResultPin.Set(Query, bIsValid);
		};
	}
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_IsValidObject::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == ResultPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add<bool>();
		OutTypes.Add<FVoxelBoolBuffer>();
		return OutTypes;
	}

	return FVoxelPinTypeSet::AllObjects();
}
#endif