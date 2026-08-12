// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelTerminalBuffer.h"
#include "VoxelBufferAccessor.h"
#include "VoxelBufferSplitter.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelBuffer);
DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelTerminalBufferAllocation);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void* FVoxelTerminalBufferAllocation::Allocate(const int64 NumBytes)
{
	VOXEL_FUNCTION_COUNTER();
	check(NumBytes > 0);

	void* Data = FMemory::Malloc(sizeof(FVoxelTerminalBufferAllocation) + NumBytes);

	const FVoxelTerminalBufferAllocation* Allocation = new(Data) FVoxelTerminalBufferAllocation(NumBytes);
	return Allocation->GetData();
}

FVoxelTerminalBufferAllocation::FVoxelTerminalBufferAllocation(const int64 NumBytes)
{
	AddRef();

	PrivateNumBytes = NumBytes;

	if (VOXEL_DEBUG)
	{
		FMemory::Memset(GetData(), 0xDF, NumBytes);
	}

	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelBuffer, NumBytes);
}

void FVoxelTerminalBufferAllocation::Destroy()
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(NumRefs.Get() == 0);

	CheckMagic();

	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelBuffer, PrivateNumBytes);

	this->~FVoxelTerminalBufferAllocation();

	ConstCast(Magic) = 0;

	FMemory::Free(this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 FVoxelTerminalBuffer::Num_Slow() const
{
	return Num();
}

bool FVoxelTerminalBuffer::IsValid_Slow() const
{
	return true;
}

int64 FVoxelTerminalBuffer::GetAllocatedSize() const
{
	const FVoxelTerminalBufferAllocation* Allocation = GetAllocation();
	if (!Allocation)
	{
		return 0;
	}

	return Allocation->GetAllocatedSize();
}

void FVoxelTerminalBuffer::Allocate(const int32 NewNum)
{
	checkVoxelSlow(NewNum >= 0);

	if (NewNum == 0)
	{
		Empty();
		return;
	}

	if (NewNum > ArrayNum)
	{
		// Reallocate

		if (FVoxelTerminalBufferAllocation* Allocation = GetAllocation())
		{
			Allocation->RemoveRef();
		}

		Data = FVoxelTerminalBufferAllocation::Allocate(int64(NewNum) * GetTypeSize_Slow());
	}
	else
	{
		// Reuse allocation
	}

	ArrayNum = NewNum;
	IndexMask = NewNum == 1 ? 0 : 0xFFFFFFFF;
}

void FVoxelTerminalBuffer::AllocateZeroed(const int32 NewNum)
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum, 128);

	Allocate(NewNum);

	FMemory::Memzero(GetRawData(), int64(ArrayNum) * GetTypeSize_Slow());
}

void FVoxelTerminalBuffer::ShrinkTo(const int32 NewNum)
{
	checkVoxelSlow(0 <= NewNum && NewNum <= ArrayNum);
	ArrayNum = NewNum;
	IndexMask = ArrayNum == 1 ? 0 : 0xFFFFFFFF;
}

void FVoxelTerminalBuffer::SerializeData(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER_NUM(Num());

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;
	ensure(Version == FVersion::LatestVersion);

	int32 LocalNum = Num();
	Ar << LocalNum;

	if (Ar.IsLoading())
	{
		Allocate(LocalNum);

		Ar.Serialize(GetRawData(), int64(ArrayNum) * GetTypeSize_Slow());
	}
	else if (ensure(Ar.IsSaving()))
	{
		Ar.Serialize(GetRawData(), int64(ArrayNum) * GetTypeSize_Slow());
	}
}

bool FVoxelTerminalBuffer::Equal(const FVoxelBuffer& Other) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Num(), 128);

	if (!ensure(GetStruct() == Other.GetStruct()))
	{
		return false;
	}

	const FVoxelTerminalBuffer& TypedOther = Other.AsChecked<FVoxelTerminalBuffer>();
	if (Num() != TypedOther.Num())
	{
		return false;
	}

	return FVoxelUtilities::MemoryEqual(
		GetRawData(),
		TypedOther.GetRawData(),
		int64(ArrayNum) * GetTypeSize_Slow());
}

void FVoxelTerminalBuffer::BulkEqual(
	const FVoxelBuffer& Other,
	const TVoxelArrayView<bool> Result,
	const EBulkEqualFlags Flags) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Result.Num(), 1);
	checkVoxelSlow(GetStruct() == Other.GetStruct());
	checkVoxelSlow(FVoxelBufferAccessor(*this, Other, Result).IsValid());

	const FVoxelTerminalBuffer& TypedOther = Other.AsChecked<FVoxelTerminalBuffer>();

	SwitchTypeSize([&]<typename Type>()
	{
		if (Flags == EBulkEqualFlags::Set)
		{
			for (int32 Index = 0; Index < Result.Num(); Index++)
			{
				Result[Index] = GetTyped<Type>(Index) == TypedOther.GetTyped<Type>(Index);
			}
		}
		else
		{
			checkVoxelSlow(Flags == EBulkEqualFlags::And);

			for (int32 Index = 0; Index < Result.Num(); Index++)
			{
				Result[Index] &= GetTyped<Type>(Index) == TypedOther.GetTyped<Type>(Index);
			}
		}
	});
}

void FVoxelTerminalBuffer::CopyFrom(
	const FVoxelBuffer& Source,
	const int32 SrcIndex,
	const int32 DestIndex,
	const int32 NumToCopy)
{
	VOXEL_FUNCTION_COUNTER_NUM(NumToCopy, 128);
	checkVoxelSlow(Source.GetInnerType() == GetInnerType());

	if (NumToCopy == 0)
	{
		return;
	}

	const FVoxelTerminalBuffer& TypedSource = Source.AsChecked<FVoxelTerminalBuffer>();

	checkVoxelSlow(0 <= DestIndex && DestIndex + NumToCopy <= Num());
	checkVoxelSlow(TypedSource.IsValidIndex(SrcIndex));
	checkVoxelSlow(TypedSource.IsValidIndex(SrcIndex + NumToCopy - 1));

	SwitchTypeSize([&]<typename Type>()
	{
		if (TypedSource.IsConstant())
		{
			FVoxelUtilities::SetAll(
				GetTypedView<Type>().Slice(DestIndex, NumToCopy),
				TypedSource.GetConstant<Type>());
		}
		else
		{
			FVoxelUtilities::Memcpy(
				GetTypedView<Type>().Slice(DestIndex, NumToCopy),
				TypedSource.GetTypedView<Type>().Slice(SrcIndex, NumToCopy));
		}
	});
}

void FVoxelTerminalBuffer::MoveFrom(FVoxelBuffer&& Source)
{
	checkVoxelSlow(Source.GetInnerType() == GetInnerType());

	*this = MoveTemp(Source.AsChecked<FVoxelTerminalBuffer>());
}

void FVoxelTerminalBuffer::IndirectCopyFrom(const FVoxelBuffer& Source, const TConstVoxelArrayView<int32> SourceToThis)
{
	VOXEL_FUNCTION_COUNTER_NUM(SourceToThis.Num(), 128);
	checkVoxelSlow(Source.GetInnerType() == GetInnerType());

	const FVoxelTerminalBuffer& TypedSource = Source.AsChecked<FVoxelTerminalBuffer>();

	SwitchTypeSize([&]<typename Type>()
	{
		for (int32 SourceIndex = 0; SourceIndex < SourceToThis.Num(); SourceIndex++)
		{
			this->SetTyped<Type>(SourceToThis[SourceIndex], TypedSource.GetTyped<Type>(SourceIndex));
		}
	});
}

TSharedRef<FVoxelBuffer> FVoxelTerminalBuffer::Gather(const TConstVoxelArrayView<int32> Indices) const
{
	const TSharedRef<FVoxelTerminalBuffer> Result = MakeSharedStruct<FVoxelTerminalBuffer>(GetStruct());
	Gather(Indices, *Result);
	return Result;
}

TSharedRef<FVoxelBuffer> FVoxelTerminalBuffer::Replicate(
	const TConstVoxelArrayView<int32> Counts,
	const int32 NewNum) const
{
	const TSharedRef<FVoxelTerminalBuffer> Result = MakeSharedStruct<FVoxelTerminalBuffer>(GetStruct());
	Replicate(Counts, NewNum, *Result);
	return Result;
}

void FVoxelTerminalBuffer::Split(
	const FVoxelBufferSplitter& Splitter,
	const TVoxelArrayView<FVoxelBuffer*> OutBuffers) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Num());
	checkVoxelSlow(Splitter.NumOutputs() == OutBuffers.Num());

	if (IsConstant())
	{
		// Fast path
		SwitchTypeSize([&]<typename Type>()
		{
			for (const int32 Index : Splitter.GetValidOutputs())
			{
				OutBuffers[Index]->AsChecked<FVoxelTerminalBuffer>().SetConstant(GetConstant<Type>());
			}
		});
		return;
	}

	for (const int32 Index : Splitter.GetValidOutputs())
	{
		OutBuffers[Index]->AsChecked<FVoxelTerminalBuffer>().Allocate(Splitter.GetOutputNum(Index));
	}

	SwitchTypeSize([&]<typename Type>()
	{
		Splitter.Iterate([&](const int32 ReadIndex, const int32 WriteIndex, const int32 OutputIndex)
		{
			OutBuffers[OutputIndex]->AsChecked<FVoxelTerminalBuffer>().SetTyped(WriteIndex, GetTyped<Type>(ReadIndex));
		});
	});
}

void FVoxelTerminalBuffer::MergeFrom(
	const FVoxelBufferSplitter& Splitter,
	const TConstVoxelArrayView<const FVoxelBuffer*> Buffers)
{
	VOXEL_FUNCTION_COUNTER_NUM(Splitter.Num());
	checkVoxelSlow(Splitter.NumOutputs() == Buffers.Num());

	for (const int32 Index : Splitter.GetValidOutputs())
	{
		checkVoxelSlow(Buffers[Index]->GetStruct() == GetStruct());
	}

	AllocateZeroed(Splitter.Num());

	SwitchTypeSize([&]<typename Type>()
	{
		Splitter.Iterate([&](const int32 ReadIndex, const int32 WriteIndex, const int32 OutputIndex)
		{
			this->SetTyped(ReadIndex, Buffers[OutputIndex]->AsChecked<FVoxelTerminalBuffer>().GetTyped<Type>(WriteIndex));
		});
	});
}

void FVoxelTerminalBuffer::HashCombine(const TVoxelArrayView<uint64> InOutHashes) const
{
	VOXEL_FUNCTION_COUNTER_NUM(InOutHashes.Num());
	checkVoxelSlow(Num() == 1 || InOutHashes.Num() == Num());

	SwitchTypeSize([&]<typename Type>()
	{
		if (IsConstant())
		{
			const uint64 Hash = FVoxelUtilities::MurmurHash(GetConstant<Type>());
			for (int32 Index = 0; Index < Num(); Index++)
			{
				InOutHashes[Index] = FVoxelUtilities::MurmurHash(InOutHashes[Index] ^ Hash);
			}
		}
		else
		{
			for (int32 Index = 0; Index < Num(); Index++)
			{
				InOutHashes[Index] = FVoxelUtilities::MurmurHash(InOutHashes[Index] ^ FVoxelUtilities::MurmurHash(GetTyped<Type>(Index)));
			}
		}
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelTerminalBuffer::ExpandConstant(const int32 NewNum)
{
	checkVoxelSlow(IsConstant());

	SwitchTypeSize([&]<typename Type>()
	{
		const Type Constant = GetConstant<Type>();

		Allocate(NewNum);
		FVoxelUtilities::SetAll(GetTypedView<Type>(), Constant);
	});
}

void FVoxelTerminalBuffer::ExpandConstantIfNeeded(const int32 NewNum)
{
	if (Num() == NewNum)
	{
		return;
	}

	if (!ensure(IsConstant()))
	{
		return;
	}

	ExpandConstant(NewNum);
}

void FVoxelTerminalBuffer::Gather(
	const TConstVoxelArrayView<int32> Indices,
	FVoxelTerminalBuffer& OutResult) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Indices.Num());
	checkVoxelSlow(OutResult.GetStruct() == GetStruct());

	if (IsConstant())
	{
		// Fast path
		SwitchTypeSize([&]<typename Type>()
		{
			OutResult.SetConstant(GetConstant<Type>());
		});
		return;
	}

	OutResult.Allocate(Indices.Num());

	SwitchTypeSize([&]<typename Type>()
	{
		for (int32 WriteIndex = 0; WriteIndex < Indices.Num(); WriteIndex++)
		{
			const int32 ReadIndex = Indices[WriteIndex];
			if (ReadIndex == -1)
			{
				Type Value;
				FMemory::Memzero(Value);

				OutResult.SetTyped(WriteIndex, Value);
			}
			else
			{
				OutResult.SetTyped(WriteIndex, GetTyped<Type>(ReadIndex));
			}
		}
	});
}

void FVoxelTerminalBuffer::Replicate(
	const TConstVoxelArrayView<int32> Counts,
	const int32 NewNum,
	FVoxelTerminalBuffer& OutResult) const
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum);
	checkVoxelSlow(OutResult.GetStruct() == GetStruct());

	if (VOXEL_DEBUG)
	{
		int32 ActualNum = 0;
		for (const int32 Count : Counts)
		{
			ActualNum += Count;
		}
		check(NewNum == ActualNum);
	}

	if (NewNum == 0)
	{
		return;
	}

	if (IsConstant())
	{
		// Fast path
		SwitchTypeSize([&]<typename Type>()
		{
			OutResult.SetConstant(GetConstant<Type>());
		});
		return;
	}

	OutResult.Allocate(NewNum);

	SwitchTypeSize([&]<typename Type>()
	{
		int32 WriteIndex = 0;
		for (int32 ReadIndex = 0; ReadIndex < Counts.Num(); ReadIndex++)
		{
			const int32 Count = Counts[ReadIndex];
			if (Count == 0)
			{
				continue;
			}

			const Type Value = GetTyped<Type>(ReadIndex);

			FVoxelUtilities::SetAll(
				OutResult.GetTypedView<Type>().Slice(WriteIndex, Count),
				Value);

			WriteIndex += Count;
		}
		checkVoxelSlow(WriteIndex == NewNum);
	});
}