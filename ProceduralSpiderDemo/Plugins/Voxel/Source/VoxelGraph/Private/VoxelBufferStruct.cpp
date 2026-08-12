// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelBufferStruct.h"
#include "VoxelBufferAccessor.h"
#include "VoxelBufferSplitter.h"
#include "VoxelTerminalBuffer.h"

int32 FVoxelBufferStruct::Num_Slow() const
{
	return Num();
}

bool FVoxelBufferStruct::IsValid_Slow() const
{
	for (const FVoxelBufferStruct& Buffer : GetBufferStructs())
	{
		if (!Buffer.IsValid_Slow())
		{
			return false;
		}
	}

	int32 NewNum = 1;
	for (const FVoxelTerminalBuffer& Buffer : GetTerminalBuffers())
	{
		if (!ensure(FVoxelBufferAccessor::MergeNum(NewNum, Buffer.Num())))
		{
			return false;
		}
	}

	if (!ensure(Num() == NewNum))
	{
		return false;
	}

	return true;
}

int64 FVoxelBufferStruct::GetAllocatedSize() const
{
	int64 AllocatedSize = 0;
	for (const FVoxelTerminalBuffer& Buffer : GetTerminalBuffers())
	{
		AllocatedSize += Buffer.GetAllocatedSize();
	}
	return AllocatedSize;
}

void FVoxelBufferStruct::Allocate(const int32 NewNum)
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum, 128);

	for (FVoxelTerminalBuffer& TerminalBuffer : GetTerminalBuffers())
	{
		TerminalBuffer.Allocate(NewNum);
	}

	PrivateNum = NewNum;

	for (const FVoxelBufferStruct& Buffer : GetBufferStructs())
	{
		Buffer.PrivateNum = NewNum;
	}
}

void FVoxelBufferStruct::AllocateZeroed(const int32 NewNum)
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum, 128);

	for (FVoxelTerminalBuffer& TerminalBuffer : GetTerminalBuffers())
	{
		TerminalBuffer.AllocateZeroed(NewNum);
	}

	PrivateNum = NewNum;

	for (const FVoxelBufferStruct& Buffer : GetBufferStructs())
	{
		Buffer.PrivateNum = NewNum;
	}
}

void FVoxelBufferStruct::ShrinkTo(const int32 NewNum)
{
	for (FVoxelTerminalBuffer& TerminalBuffer : GetTerminalBuffers())
	{
		TerminalBuffer.ShrinkTo(NewNum);
	}

	checkVoxelSlow(0 <= NewNum && NewNum <= PrivateNum);
	PrivateNum = NewNum;

	for (const FVoxelBufferStruct& Buffer : GetBufferStructs())
	{
		Buffer.PrivateNum = NewNum;
	}
}

void FVoxelBufferStruct::SerializeData(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER_NUM(Num());

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;
	ensure(Version == FVersion::LatestVersion);

	for (FVoxelTerminalBuffer& TerminalBuffer : GetTerminalBuffers())
	{
		TerminalBuffer.SerializeData(Ar);
	}
}

bool FVoxelBufferStruct::Equal(const FVoxelBuffer& Other) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Num(), 128);

	if (!ensure(GetStruct() == Other.GetStruct()))
	{
		return false;
	}

	const FVoxelBufferStruct& TypedOther = Other.AsChecked<FVoxelBufferStruct>();
	if (Num() != TypedOther.Num())
	{
		return false;
	}

	for (int32 Index = 0; Index < NumTerminalBuffers(); Index++)
	{
		if (!GetTerminalBuffer(Index).Equal(TypedOther.GetTerminalBuffer(Index)))
		{
			return false;
		}
	}

	return true;
}

void FVoxelBufferStruct::BulkEqual(
	const FVoxelBuffer& Other,
	const TVoxelArrayView<bool> Result,
	const EBulkEqualFlags Flags) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Result.Num(), 1);
	checkVoxelSlow(GetStruct() == Other.GetStruct());
	checkVoxelSlow(FVoxelBufferAccessor(*this, Other, Result).IsValid());

	const FVoxelBufferStruct& TypedOther = Other.AsChecked<FVoxelBufferStruct>();

	if (Flags == EBulkEqualFlags::Set)
	{
		GetTerminalBuffer(0).BulkEqual(
			TypedOther.GetTerminalBuffer(0),
			Result,
			EBulkEqualFlags::Set);

		for (int32 Index = 1; Index < NumTerminalBuffers(); Index++)
		{
			GetTerminalBuffer(Index).BulkEqual(
				TypedOther.GetTerminalBuffer(Index),
				Result,
				EBulkEqualFlags::And);
		}
	}
	else
	{
		checkVoxelSlow(Flags == EBulkEqualFlags::And);

		for (int32 Index = 0; Index < NumTerminalBuffers(); Index++)
		{
			GetTerminalBuffer(Index).BulkEqual(
				TypedOther.GetTerminalBuffer(Index),
				Result,
				EBulkEqualFlags::And);
		}
	}
}

void FVoxelBufferStruct::CopyFrom(
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

	const FVoxelBufferStruct& TypedSource = Source.AsChecked<FVoxelBufferStruct>();

	for (int32 Index = 0; Index < NumTerminalBuffers(); Index++)
	{
		GetTerminalBuffer(Index).CopyFrom(
			TypedSource.GetTerminalBuffer(Index),
			SrcIndex,
			DestIndex,
			NumToCopy);
	}
}

void FVoxelBufferStruct::MoveFrom(FVoxelBuffer&& Source)
{
	checkVoxelSlow(Source.GetInnerType() == GetInnerType());

	for (int32 Index = 0; Index < NumTerminalBuffers(); Index++)
	{
		GetTerminalBuffer(Index).MoveFrom(MoveTemp(Source.AsChecked<FVoxelBufferStruct>().GetTerminalBuffer(Index)));
	}
}

void FVoxelBufferStruct::IndirectCopyFrom(
	const FVoxelBuffer& Source,
	const TConstVoxelArrayView<int32> SourceToThis)
{
	VOXEL_FUNCTION_COUNTER_NUM(SourceToThis.Num(), 128);
	checkVoxelSlow(Source.GetInnerType() == GetInnerType());

	const FVoxelBufferStruct& TypedSource = Source.AsChecked<FVoxelBufferStruct>();

	for (int32 Index = 0; Index < NumTerminalBuffers(); Index++)
	{
		GetTerminalBuffer(Index).IndirectCopyFrom(
			TypedSource.GetTerminalBuffer(Index),
			SourceToThis);
	}
}

TSharedRef<FVoxelBuffer> FVoxelBufferStruct::Gather(const TConstVoxelArrayView<int32> Indices) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Indices.Num());

	const TSharedRef<FVoxelBufferStruct> Result = MakeSharedStruct<FVoxelBufferStruct>(GetStruct());
	for (int32 Index = 0; Index < NumTerminalBuffers(); Index++)
	{
		GetTerminalBuffer(Index).Gather(
			Indices,
			Result->GetTerminalBuffer(Index));
	}
	return Result;
}

TSharedRef<FVoxelBuffer> FVoxelBufferStruct::Replicate(
	const TConstVoxelArrayView<int32> Counts,
	const int32 NewNum) const
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum);

	const TSharedRef<FVoxelBufferStruct> Result = MakeSharedStruct<FVoxelBufferStruct>(GetStruct());
	for (int32 Index = 0; Index < NumTerminalBuffers(); Index++)
	{
		GetTerminalBuffer(Index).Replicate(
			Counts,
			NewNum,
			Result->GetTerminalBuffer(Index));
	}
	return Result;
}

void FVoxelBufferStruct::Split(
	const FVoxelBufferSplitter& Splitter,
	const TVoxelArrayView<FVoxelBuffer*> OutBuffers) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Num());
	checkVoxelSlow(Splitter.NumOutputs() == OutBuffers.Num());

	TVoxelArray<FVoxelTerminalBuffer*> TerminalBuffers;
	FVoxelUtilities::SetNumZeroed(TerminalBuffers, OutBuffers.Num());

	for (int32 TerminalBufferIndex = 0; TerminalBufferIndex < NumTerminalBuffers(); TerminalBufferIndex++)
	{
		for (const int32 Index : Splitter.GetValidOutputs())
		{
			TerminalBuffers[Index] = &OutBuffers[Index]->AsChecked<FVoxelBufferStruct>().GetTerminalBuffer(TerminalBufferIndex);
		}

		GetTerminalBuffer(TerminalBufferIndex).Split(Splitter, TerminalBuffers);
	}
}

void FVoxelBufferStruct::MergeFrom(
	const FVoxelBufferSplitter& Splitter,
	const TConstVoxelArrayView<const FVoxelBuffer*> Buffers)
{
	VOXEL_FUNCTION_COUNTER_NUM(Splitter.Num());
	checkVoxelSlow(Splitter.NumOutputs() == Buffers.Num());

	TVoxelArray<const FVoxelTerminalBuffer*> TerminalBuffers;
	FVoxelUtilities::SetNumFast(TerminalBuffers, Buffers.Num());

	for (int32 TerminalBufferIndex = 0; TerminalBufferIndex < NumTerminalBuffers(); TerminalBufferIndex++)
	{
		for (int32 OutputIndex = 0; OutputIndex < Buffers.Num(); OutputIndex++)
		{
			TerminalBuffers[OutputIndex] = &Buffers[OutputIndex]->AsChecked<FVoxelBufferStruct>().GetTerminalBuffer(TerminalBufferIndex);
		}

		GetTerminalBuffer(TerminalBufferIndex).MergeFrom(Splitter, TerminalBuffers);
	}

	// Ensure Num is valid
	(void)Num();
}

void FVoxelBufferStruct::HashCombine(const TVoxelArrayView<uint64> InOutHashes) const
{
	VOXEL_FUNCTION_COUNTER_NUM(InOutHashes.Num());
	checkVoxelSlow(Num() == 1 || InOutHashes.Num() == Num());

	for (int32 TerminalBufferIndex = 0; TerminalBufferIndex < NumTerminalBuffers(); TerminalBufferIndex++)
	{
		GetTerminalBuffer(TerminalBufferIndex).HashCombine(InOutHashes);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelBufferStruct::ExpandConstants()
{
	for (FVoxelTerminalBuffer& Buffer : GetTerminalBuffers())
	{
		Buffer.ExpandConstantIfNeeded(Num());
	}
}

void FVoxelBufferStruct::ExpandConstants(const int32 NewNum)
{
	if (Num() != NewNum &&
		!ensure(IsConstant()))
	{
		return;
	}

	for (FVoxelTerminalBuffer& Buffer : GetTerminalBuffers())
	{
		Buffer.ExpandConstantIfNeeded(NewNum);
	}

	PrivateNum = NewNum;

	for (const FVoxelBufferStruct& Buffer : GetBufferStructs())
	{
		Buffer.PrivateNum = NewNum;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBufferStruct::FTypeInfo::FTypeInfo(const FVoxelBufferStruct* Template)
{
	TVoxelArray<int64> TerminalBufferOffsets;
	TVoxelArray<int64> BufferStructOffsets;

	for (const FProperty& Property : GetStructProperties(Template->GetStruct()))
	{
		const UScriptStruct* Struct = CastFieldChecked<FStructProperty>(Property).Struct;
		check(Struct->IsChildOf(FVoxelBuffer::StaticStruct()));

		if (Struct->IsChildOf(FVoxelTerminalBuffer::StaticStruct()))
		{
			const uint8* Pointer = Property.ContainerPtrToValuePtr<uint8>(Template);
			TerminalBufferOffsets.Add(Pointer - reinterpret_cast<const uint8*>(Template));
			continue;
		}
		check(Struct->GetSuperStruct() == FVoxelBufferStruct::StaticStruct());

		const FVoxelBufferStruct& Buffer = *Property.ContainerPtrToValuePtr<FVoxelBufferStruct>(Template);
		BufferStructOffsets.Add(reinterpret_cast<const uint8*>(&Buffer) - reinterpret_cast<const uint8*>(Template));

		for (const int64 Offset : Buffer.GetTerminalBufferOffsets())
		{
			TerminalBufferOffsets.Add(reinterpret_cast<const uint8*>(&Buffer) - reinterpret_cast<const uint8*>(Template) + Offset);
		}
		for (const int64 Offset : Buffer.GetBufferStructOffsets())
		{
			BufferStructOffsets.Add(reinterpret_cast<const uint8*>(&Buffer) - reinterpret_cast<const uint8*>(Template) + Offset);
		}
	}
	check(TerminalBufferOffsets.Num() > 0);

	NumTerminalBuffers = TerminalBufferOffsets.Num();
	NumBufferStructs = BufferStructOffsets.Num();

	Offsets = new int64[NumTerminalBuffers + NumBufferStructs];

	const TVoxelArrayView<int64> OffsetsView = MakeVoxelArrayView(Offsets, int32(NumTerminalBuffers + NumBufferStructs));

	FVoxelUtilities::Memcpy(
		OffsetsView.Slice(0, NumTerminalBuffers),
		TerminalBufferOffsets);

	FVoxelUtilities::Memcpy(
		OffsetsView.Slice(NumTerminalBuffers, NumBufferStructs),
		BufferStructOffsets);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 FVoxelBufferStruct::ComputeNum() const
{
	int32 NewNum = 1;
	for (const FVoxelTerminalBuffer& Buffer : GetTerminalBuffers())
	{
		ensure(FVoxelBufferAccessor::MergeNum(NewNum, Buffer.Num()));
	}
	ensure(NewNum >= 0);

	return NewNum;
}