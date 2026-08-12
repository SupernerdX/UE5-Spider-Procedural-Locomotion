// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "VoxelBufferAccessor.h"

void FVoxelSurfaceTypeBlendPinType::Convert(
	const bool bSetObject,
	TVoxelObjectPtr<UVoxelSurfaceTypeInterface>& OutObject,
	UVoxelSurfaceTypeInterface& InObject,
	FVoxelSurfaceTypeBlend& Struct)
{
	if (bSetObject)
	{
		if (Struct.IsNull())
		{
			OutObject = nullptr;
		}
		else
		{
			OutObject = Struct.GetTopLayer().Type.GetSurfaceTypeInterface();
		}
	}
	else
	{
		Struct = FVoxelSurfaceTypeBlend::FromType(FVoxelSurfaceType(&InObject));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSurfaceTypeBlendBuffer::AllocateZeroed(const int32 NewNum)
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum, 128);

	Allocate(NewNum);

	for (int32 Index = 0; Index < NewNum; Index++)
	{
		Set(Index, FVoxelSurfaceTypeBlend());
	}
}

bool FVoxelSurfaceTypeBlendBuffer::Equal(const FVoxelBuffer& Other) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Num(), 128);

	if (!ensure(GetStruct() == Other.GetStruct()))
	{
		return false;
	}

	const FVoxelSurfaceTypeBlendBuffer& TypedOther = Other.AsChecked<FVoxelSurfaceTypeBlendBuffer>();

	if (Num() != TypedOther.Num())
	{
		return false;
	}

	for (int32 Index = 0; Index < Num(); Index++)
	{
		if ((*this)[Index] != TypedOther[Index])
		{
			return false;
		}
	}

	return true;
}

void FVoxelSurfaceTypeBlendBuffer::BulkEqual(
	const FVoxelBuffer& Other,
	const TVoxelArrayView<bool> Result,
	const EBulkEqualFlags Flags) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Result.Num(), 1);
	checkVoxelSlow(GetStruct() == Other.GetStruct());
	checkVoxelSlow(FVoxelBufferAccessor(*this, Other, Result).IsValid());

	const FVoxelSurfaceTypeBlendBuffer& TypedOther = Other.AsChecked<FVoxelSurfaceTypeBlendBuffer>();

	if (Flags == EBulkEqualFlags::Set)
	{
		for (int32 Index = 0; Index < Result.Num(); Index++)
		{
			Result[Index] = (*this)[Index] == TypedOther[Index];
		}
	}
	else
	{
		checkVoxelSlow(Flags == EBulkEqualFlags::And);

		for (int32 Index = 0; Index < Result.Num(); Index++)
		{
			Result[Index] &= (*this)[Index] == TypedOther[Index];
		}
	}
}