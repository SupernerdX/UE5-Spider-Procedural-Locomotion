// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGenericStructBuffer.h"
#include "VoxelBufferAccessor.h"
#include "VoxelBufferSplitter.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelGenericStructBuffer);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelGenericStructBuffer FVoxelGenericStructBuffer::MakeDefault(UScriptStruct& Struct)
{
	FVoxelGenericStructBuffer Result = MakeEmpty(Struct);
	Result.Allocate(1);
	return Result;
}

FVoxelGenericStructBuffer FVoxelGenericStructBuffer::MakeEmpty(UScriptStruct& Struct)
{
	checkVoxelSlow(Struct.GetStructureSize() > 0);
	checkVoxelSlow(Struct.GetMinAlignment() <= 16);

	FVoxelGenericStructBuffer Result;
	Result.InnerStruct = &Struct;
	Result.CppStructOps = Struct.GetCppStructOps();
	Result.TypeSize = Struct.GetStructureSize();
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UScriptStruct* FVoxelGenericStructBuffer::GetStruct() const
{
	return StaticStructFast<FVoxelGenericStructBuffer>();
}

FVoxelPinType FVoxelGenericStructBuffer::GetInnerType() const
{
	return FVoxelPinType::MakeStruct(&GetInnerStruct());
}

int32 FVoxelGenericStructBuffer::Num_Slow() const
{
	return Num();
}

bool FVoxelGenericStructBuffer::IsValid_Slow() const
{
	return InnerStruct != nullptr;
}

int64 FVoxelGenericStructBuffer::GetAllocatedSize() const
{
	return ArrayNum * TypeSize;
}

void FVoxelGenericStructBuffer::Allocate(const int32 NewNum)
{
	AllocateZeroed(NewNum);
}

void FVoxelGenericStructBuffer::AllocateZeroed(const int32 NewNum)
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum, 128);
	check(NewNum >= 0);
	check(InnerStruct);
	check(TypeSize > 0);

	if (Data)
	{
		Empty();
	}

	if (NewNum == 0)
	{
		return;
	}

	ArrayNum = NewNum;
	Data = FMemory::MallocZeroed(ArrayNum * TypeSize);

	if (!CppStructOps->HasZeroConstructor())
	{
		VOXEL_SCOPE_COUNTER_FORMAT_COND(ArrayNum > 128, "Construct %s Num=%d", *InnerStruct->GetName(), ArrayNum);

		for (int32 Index = 0; Index < ArrayNum; Index++)
		{
			CppStructOps->Construct(GetMutable(Index).GetStructMemory());
		}
	}

	AllocatedSizeTracker = GetAllocatedSize();
}

void FVoxelGenericStructBuffer::ShrinkTo(const int32 NewNum)
{
	const int32 NumToRemove = ArrayNum - NewNum;

	if (NumToRemove > 0 &&
		CppStructOps->HasDestructor())
	{
		VOXEL_SCOPE_COUNTER_FORMAT_COND(NumToRemove > 128, "FVoxelGenericStructBuffer::ShrinkTo Destruct %s Num=%d", *InnerStruct->GetName(), NumToRemove);

		for (int32 Index = NewNum; Index < ArrayNum; Index++)
		{
			CppStructOps->Destruct(GetMutable(Index).GetStructMemory());
		}
	}

	checkVoxelSlow(0 <= NewNum && NewNum <= ArrayNum);
	ArrayNum = NewNum;
}

void FVoxelGenericStructBuffer::SerializeData(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER_NUM(Num());

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;
	ensure(Version == FVersion::LatestVersion);

	FVoxelSerializationGuard Guard(Ar);

	int32 LocalNum = Num();
	Ar << LocalNum;

	if (Ar.IsLoading())
	{
		Allocate(LocalNum);
	}

	FVoxelInstancedStruct DefaultStruct(&GetInnerStruct());

	for (int32 Index = 0; Index < Num(); Index++)
	{
		GetInnerStruct().SerializeItem(
			Ar,
			GetMutable(Index).GetStructMemory(),
			DefaultStruct.GetStructMemory());
	}
}

bool FVoxelGenericStructBuffer::Equal(const FVoxelBuffer& Other) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Num(), 1);

	if (!ensure(GetStruct() == Other.GetStruct()))
	{
		return false;
	}

	const FVoxelGenericStructBuffer& TypedOther = Other.AsChecked<FVoxelGenericStructBuffer>();

	if (Num() != TypedOther.Num())
	{
		return false;
	}

	for (int32 Index = 0; Index < Num(); Index++)
	{
		if (!(*this)[Index].Identical(TypedOther[Index]))
		{
			return false;
		}
	}

	return true;
}

void FVoxelGenericStructBuffer::BulkEqual(
	const FVoxelBuffer& Other,
	const TVoxelArrayView<bool> Result,
	const EBulkEqualFlags Flags) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Result.Num(), 1);
	checkVoxelSlow(FVoxelBufferAccessor(*this, Other, Result).IsValid());

	const FVoxelGenericStructBuffer& TypedOther = Other.AsChecked<FVoxelGenericStructBuffer>();

	if (Flags == EBulkEqualFlags::Set)
	{
		for (int32 Index = 0; Index < Result.Num(); Index++)
		{
			Result[Index] = (*this)[Index].Identical(TypedOther[Index]);
		}
	}
	else
	{
		checkVoxelSlow(Flags == EBulkEqualFlags::And);

		for (int32 Index = 0; Index < Result.Num(); Index++)
		{
			Result[Index] &= (*this)[Index].Identical(TypedOther[Index]);
		}
	}
}

void FVoxelGenericStructBuffer::CopyFrom(
	const FVoxelBuffer& Source,
	const int32 SrcIndex,
	const int32 DestIndex,
	const int32 NumToCopy)
{
	VOXEL_FUNCTION_COUNTER_NUM(NumToCopy, 1);

	if (NumToCopy == 0)
	{
		return;
	}

	const FVoxelGenericStructBuffer& TypedSource = Source.AsChecked<FVoxelGenericStructBuffer>();

	for (int32 Index = 0; Index < NumToCopy; Index++)
	{
		Set(DestIndex + Index, TypedSource[SrcIndex + Index]);
	}
}

void FVoxelGenericStructBuffer::MoveFrom(FVoxelBuffer&& Source)
{
	checkVoxelSlow(Source.GetInnerType() == GetInnerType());

	*this = MoveTemp(Source.AsChecked<FVoxelGenericStructBuffer>());
}

void FVoxelGenericStructBuffer::IndirectCopyFrom(const FVoxelBuffer& Source, const TConstVoxelArrayView<int32> SourceToThis)
{
	VOXEL_FUNCTION_COUNTER_NUM(SourceToThis.Num());

	const FVoxelGenericStructBuffer& TypedSource = Source.AsChecked<FVoxelGenericStructBuffer>();

	for (int32 SourceIndex = 0; SourceIndex < SourceToThis.Num(); SourceIndex++)
	{
		Set(SourceToThis[SourceIndex], TypedSource[SourceIndex]);
	}
}

TSharedRef<FVoxelBuffer> FVoxelGenericStructBuffer::Gather(const TConstVoxelArrayView<int32> Indices) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Indices.Num());

	if (IsConstant())
	{
		// Fast path
		FVoxelGenericStructBuffer Result = MakeEmpty(GetInnerStruct());
		Result.SetConstant(GetConstant());
		return MakeSharedCopy(MoveTemp(Result));
	}

	FVoxelGenericStructBuffer Result = MakeEmpty(GetInnerStruct());
	Result.Allocate(Indices.Num());

	for (int32 WriteIndex = 0; WriteIndex < Indices.Num(); WriteIndex++)
	{
		const int32 ReadIndex = Indices[WriteIndex];
		if (ReadIndex == -1)
		{
			continue;
		}

		Result.Set(WriteIndex, (*this)[ReadIndex]);
	}

	return MakeSharedCopy(MoveTemp(Result));
}

TSharedRef<FVoxelBuffer> FVoxelGenericStructBuffer::Replicate(
	const TConstVoxelArrayView<int32> Counts,
	const int32 NewNum) const
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum);

	if (NewNum == 0)
	{
		return MakeSharedCopy(MakeEmpty(GetInnerStruct()));
	}

	if (IsConstant())
	{
		// Fast path
		FVoxelGenericStructBuffer Result = MakeEmpty(GetInnerStruct());
		Result.SetConstant(GetConstant());
		return MakeSharedCopy(MoveTemp(Result));
	}

	FVoxelGenericStructBuffer Result = MakeEmpty(GetInnerStruct());
	Result.AllocateZeroed(NewNum);

	int32 WriteIndex = 0;
	for (int32 ReadIndex = 0; ReadIndex < Counts.Num(); ReadIndex++)
	{
		const int32 Count = Counts[ReadIndex];
		const FConstVoxelStructView Value = (*this)[ReadIndex];

		for (int32 Index = 0; Index < Count; Index++)
		{
			Result.Set(WriteIndex++, Value);
		}
	}
	checkVoxelSlow(WriteIndex == NewNum);

	return MakeSharedCopy(MoveTemp(Result));
}

void FVoxelGenericStructBuffer::Split(
	const FVoxelBufferSplitter& Splitter,
	const TVoxelArrayView<FVoxelBuffer*> OutBuffers) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Num());
	checkVoxelSlow(Splitter.NumOutputs() == OutBuffers.Num());

	if (IsConstant())
	{
		// Fast path
		for (const int32 Index : Splitter.GetValidOutputs())
		{
			OutBuffers[Index]->AsChecked<FVoxelGenericStructBuffer>().SetConstant(GetConstant());
		}
		return;
	}

	for (const int32 Index : Splitter.GetValidOutputs())
	{
		OutBuffers[Index]->AsChecked<FVoxelGenericStructBuffer>().Allocate(Splitter.GetOutputNum(Index));
	}

	Splitter.Iterate([&](const int32 ReadIndex, const int32 WriteIndex, const int32 OutputIndex)
	{
		OutBuffers[OutputIndex]->AsChecked<FVoxelGenericStructBuffer>().Set(WriteIndex, (*this)[ReadIndex]);
	});
}

void FVoxelGenericStructBuffer::MergeFrom(
	const FVoxelBufferSplitter& Splitter,
	const TConstVoxelArrayView<const FVoxelBuffer*> Buffers)
{
	VOXEL_FUNCTION_COUNTER_NUM(Splitter.Num());
	checkVoxelSlow(Splitter.NumOutputs() == Buffers.Num());

	for (const FVoxelBuffer* Buffer : Buffers)
	{
		checkVoxelSlow(Buffer->GetStruct() == GetStruct());
	}

	AllocateZeroed(Splitter.Num());

	Splitter.Iterate([&](const int32 ReadIndex, const int32 WriteIndex, const int32 OutputIndex)
	{
		Set(ReadIndex, Buffers[OutputIndex]->AsChecked<FVoxelGenericStructBuffer>()[WriteIndex]);
	});
}

void FVoxelGenericStructBuffer::HashCombine(const TVoxelArrayView<uint64> InOutHashes) const
{
	VOXEL_FUNCTION_COUNTER_NUM(InOutHashes.Num());
	checkVoxelSlow(Num() == 1 || InOutHashes.Num() == Num());

	if (IsConstant())
	{
		const FConstVoxelStructView Constant = GetConstant();
		const uint64 Hash = FVoxelUtilities::HashStruct(Constant.GetScriptStruct(), Constant.GetStructMemory());
		for (int32 Index = 0; Index < Num(); Index++)
		{
			InOutHashes[Index] = FVoxelUtilities::MurmurHash(InOutHashes[Index] ^ Hash);
		}
	}
	else
	{
		for (int32 Index = 0; Index < Num(); Index++)
		{
			const FConstVoxelStructView Value = (*this)[Index];
			const uint64 Hash = FVoxelUtilities::HashStruct(Value.GetScriptStruct(), Value.GetStructMemory());
			InOutHashes[Index] = FVoxelUtilities::MurmurHash(InOutHashes[Index] ^ Hash);
		}
	}
}

void FVoxelGenericStructBuffer::SetGeneric(
	const int32 Index,
	const FVoxelRuntimePinValue& Value)
{
	Set(Index, Value.GetStructView());
}

FVoxelRuntimePinValue FVoxelGenericStructBuffer::GetGeneric(const int32 Index) const
{
	return FVoxelRuntimePinValue::MakeStruct((*this)[Index]);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGenericStructBuffer::Empty()
{
	VOXEL_FUNCTION_COUNTER();

	if (!Data)
	{
		return;
	}

	check(InnerStruct);
	check(CppStructOps);

	if (CppStructOps->HasDestructor())
	{
		VOXEL_SCOPE_COUNTER_FORMAT_COND(Num() > 128, "Destruct %s Num=%d", *InnerStruct->GetName(), Num());

		for (int32 Index = 0; Index < Num(); Index++)
		{
			CppStructOps->Destruct(GetMutable(Index).GetStructMemory());
		}
	}

	ArrayNum = 0;

	FMemory::Free(Data);
	Data = nullptr;

	AllocatedSizeTracker = GetAllocatedSize();
}

void FVoxelGenericStructBuffer::SetConstant(const FConstVoxelStructView Constant)
{
	VOXEL_FUNCTION_COUNTER();

	Allocate(1);
	Set(0, Constant);
}