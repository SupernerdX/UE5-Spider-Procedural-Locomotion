// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelRuntimeStructBuffer.h"
#include "VoxelRuntimeStruct.h"
#include "VoxelBufferAccessor.h"
#include "VoxelBufferSplitter.h"
#include "Buffer/VoxelIntegerBuffers.h"
#include "StructUtils/UserDefinedStruct.h"

FVoxelRuntimeStructBuffer FVoxelRuntimeStructBuffer::MakeDefault(UScriptStruct& Struct)
{
	FVoxelRuntimeStructBuffer Result = MakeEmpty(Struct);
	Result.Allocate(1);
	return Result;
}

FVoxelRuntimeStructBuffer FVoxelRuntimeStructBuffer::MakeEmpty(UScriptStruct& Struct)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(Struct.IsA<UUserDefinedStruct>());

	FVoxelRuntimeStructBuffer Result;
	Result.InnerStruct = &Struct;

	Result.PropertyNameToBuffer.Reserve(16);

	for (const FProperty& Property : GetStructProperties(Struct))
	{
		if (!FVoxelPinType::IsSupported(Property))
		{
			continue;
		}

		Result.PropertyNameToBuffer.Add_EnsureNew(
			Property.GetFName(),
			FVoxelBuffer::MakeEmpty(FVoxelPinType(Property)));
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UScriptStruct* FVoxelRuntimeStructBuffer::GetStruct() const
{
	return StaticStructFast<FVoxelRuntimeStructBuffer>();
}

FVoxelPinType FVoxelRuntimeStructBuffer::GetInnerType() const
{
	return FVoxelPinType::MakeStruct(&GetInnerStruct());
}

int32 FVoxelRuntimeStructBuffer::Num_Slow() const
{
	return Num();
}

bool FVoxelRuntimeStructBuffer::IsValid_Slow() const
{
	if (!InnerStruct)
	{
		return false;
	}

	int32 NewNum = 1;
	for (const auto& It : PropertyNameToBuffer)
	{
		if (!It.Value->IsValid_Slow())
		{
			return false;
		}

		if (!ensure(FVoxelBufferAccessor::MergeNum(NewNum, It.Value->Num_Slow())))
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

int64 FVoxelRuntimeStructBuffer::GetAllocatedSize() const
{
	int64 AllocatedSize = 0;
	for (const auto& It : PropertyNameToBuffer)
	{
		AllocatedSize += It.Value->GetAllocatedSize();
	}
	return AllocatedSize;
}

void FVoxelRuntimeStructBuffer::Allocate(const int32 NewNum)
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum, 128);

	for (const auto& It : PropertyNameToBuffer)
	{
		It.Value->Allocate(NewNum);
	}

	PrivateNum = NewNum;
}

void FVoxelRuntimeStructBuffer::AllocateZeroed(const int32 NewNum)
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum, 128);

	for (const auto& It : PropertyNameToBuffer)
	{
		It.Value->AllocateZeroed(NewNum);
	}

	PrivateNum = NewNum;
}

void FVoxelRuntimeStructBuffer::ShrinkTo(const int32 NewNum)
{
	for (const auto& It : PropertyNameToBuffer)
	{
		It.Value->ShrinkTo(NewNum);
	}

	checkVoxelSlow(0 <= NewNum && NewNum <= PrivateNum);
	PrivateNum = NewNum;
}

void FVoxelRuntimeStructBuffer::SerializeData(FArchive& Ar)
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

	for (const auto& It : PropertyNameToBuffer)
	{
		It.Value->SerializeData(Ar);
	}
}

bool FVoxelRuntimeStructBuffer::Equal(const FVoxelBuffer& Other) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Num(), 1);

	if (!ensure(GetStruct() == Other.GetStruct()))
	{
		return false;
	}

	const FVoxelRuntimeStructBuffer& TypedOther = Other.AsChecked<FVoxelRuntimeStructBuffer>();
	if (!ensure(IsCompatibleWith(TypedOther)))
	{
		return false;
	}

	if (Num() != TypedOther.Num())
	{
		return false;
	}

	for (const auto& It : PropertyNameToBuffer)
	{
		if (!It.Value->Equal(TypedOther.GetBufferChecked(It.Key)))
		{
			return false;
		}
	}

	return true;
}

void FVoxelRuntimeStructBuffer::BulkEqual(
	const FVoxelBuffer& Other,
	const TVoxelArrayView<bool> Result,
	const EBulkEqualFlags Flags) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Result.Num(), 1);
	checkVoxelSlow(GetStruct() == Other.GetStruct());
	checkVoxelSlow(FVoxelBufferAccessor(*this, Other, Result).IsValid());

	const FVoxelRuntimeStructBuffer& TypedOther = Other.AsChecked<FVoxelRuntimeStructBuffer>();
	if (!ensure(IsCompatibleWith(TypedOther)))
	{
		return;
	}

	if (Flags == EBulkEqualFlags::Set)
	{
		bool bIsSet = false;
		for (const auto& It : PropertyNameToBuffer)
		{
			It.Value->BulkEqual(
				TypedOther.GetBufferChecked(It.Key),
				Result,
				bIsSet ? EBulkEqualFlags::And : EBulkEqualFlags::Set);

			bIsSet = true;
		}
	}
	else
	{
		checkVoxelSlow(Flags == EBulkEqualFlags::And);

		for (const auto& It : PropertyNameToBuffer)
		{
			It.Value->BulkEqual(
				TypedOther.GetBufferChecked(It.Key),
				Result,
				EBulkEqualFlags::And);
		}
	}
}

void FVoxelRuntimeStructBuffer::CopyFrom(
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

	const FVoxelRuntimeStructBuffer& TypedSource = Source.AsChecked<FVoxelRuntimeStructBuffer>();
	if (!ensure(IsCompatibleWith(TypedSource)))
	{
		return;
	}

	for (const auto& It : PropertyNameToBuffer)
	{
		It.Value->CopyFrom(
			TypedSource.GetBufferChecked(It.Key),
			SrcIndex,
			DestIndex,
			NumToCopy);
	}
}

void FVoxelRuntimeStructBuffer::MoveFrom(FVoxelBuffer&& Source)
{
	checkVoxelSlow(Source.GetInnerType() == GetInnerType());

	FVoxelRuntimeStructBuffer& TypedSource = Source.AsChecked<FVoxelRuntimeStructBuffer>();
	if (!ensure(IsCompatibleWith(TypedSource)))
	{
		return;
	}

	for (const auto& It : PropertyNameToBuffer)
	{
		It.Value->MoveFrom(MoveTemp(TypedSource.GetBufferChecked(It.Key)));
	}
}

void FVoxelRuntimeStructBuffer::IndirectCopyFrom(const FVoxelBuffer& Source, const TConstVoxelArrayView<int32> SourceToThis)
{
	VOXEL_FUNCTION_COUNTER_NUM(SourceToThis.Num(), 128);
	checkVoxelSlow(Source.GetInnerType() == GetInnerType());

	const FVoxelRuntimeStructBuffer& TypedSource = Source.AsChecked<FVoxelRuntimeStructBuffer>();
	if (!ensure(IsCompatibleWith(TypedSource)))
	{
		return;
	}

	for (const auto& It : PropertyNameToBuffer)
	{
		It.Value->IndirectCopyFrom(
			TypedSource.GetBufferChecked(It.Key),
			SourceToThis);
	}
}

TSharedRef<FVoxelBuffer> FVoxelRuntimeStructBuffer::Gather(const TConstVoxelArrayView<int32> Indices) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Indices.Num());

	FVoxelRuntimeStructBuffer Result = MakeEmpty(GetInnerStruct());
	if (!ensure(IsCompatibleWith(Result)))
	{
		return MakeSharedCopy(Result);
	}

	for (auto& It : PropertyNameToBuffer)
	{
		Result.SetBuffer(It.Key, It.Value->Gather(Indices));
	}

	return MakeSharedCopy(Result);
}

TSharedRef<FVoxelBuffer> FVoxelRuntimeStructBuffer::Replicate(
	const TConstVoxelArrayView<int32> Counts,
	const int32 NewNum) const
{
	VOXEL_FUNCTION_COUNTER_NUM(NewNum);

	FVoxelRuntimeStructBuffer Result = MakeEmpty(GetInnerStruct());
	if (!ensure(IsCompatibleWith(Result)))
	{
		return MakeSharedCopy(Result);
	}

	for (auto& It : PropertyNameToBuffer)
	{
		Result.SetBuffer(It.Key, It.Value->Replicate(Counts, NewNum));
	}

	return MakeSharedCopy(Result);
}

void FVoxelRuntimeStructBuffer::Split(
	const FVoxelBufferSplitter& Splitter,
	const TVoxelArrayView<FVoxelBuffer*> OutBuffers) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Num());
	checkVoxelSlow(Splitter.NumOutputs() == OutBuffers.Num());

	for (FVoxelBuffer* Buffer : OutBuffers)
	{
		if (!ensure(IsCompatibleWith(Buffer->AsChecked<FVoxelRuntimeStructBuffer>())))
		{
			return;
		}
	}

	TVoxelArray<FVoxelBuffer*> LocalBuffers;
	FVoxelUtilities::SetNumZeroed(LocalBuffers, OutBuffers.Num());

	for (const auto& It : PropertyNameToBuffer)
	{
		for (const int32 Index : Splitter.GetValidOutputs())
		{
			LocalBuffers[Index] = &OutBuffers[Index]->AsChecked<FVoxelRuntimeStructBuffer>().GetBufferChecked(It.Key);
		}

		It.Value->Split(Splitter, LocalBuffers);
	}
}

void FVoxelRuntimeStructBuffer::MergeFrom(
	const FVoxelBufferSplitter& Splitter,
	const TConstVoxelArrayView<const FVoxelBuffer*> Buffers)
{
	VOXEL_FUNCTION_COUNTER_NUM(Splitter.Num());
	checkVoxelSlow(Splitter.NumOutputs() == Buffers.Num());

	TVoxelArray<const FVoxelBuffer*> LocalBuffers;
	FVoxelUtilities::SetNumFast(LocalBuffers, Buffers.Num());

	for (const auto& It : PropertyNameToBuffer)
	{
		for (int32 OutputIndex = 0; OutputIndex < Buffers.Num(); OutputIndex++)
		{
			LocalBuffers[OutputIndex] = &Buffers[OutputIndex]->AsChecked<FVoxelRuntimeStructBuffer>().GetBufferChecked(It.Key);
		}

		It.Value->MergeFrom(Splitter, LocalBuffers);
	}

	// Ensure Num is valid
	(void)Num();
}

void FVoxelRuntimeStructBuffer::HashCombine(const TVoxelArrayView<uint64> InOutHashes) const
{
	VOXEL_FUNCTION_COUNTER_NUM(InOutHashes.Num());
	checkVoxelSlow(Num() == 1 || InOutHashes.Num() == Num());

	for (const auto& It : PropertyNameToBuffer)
	{
		It.Value->HashCombine(InOutHashes);
	}
}

void FVoxelRuntimeStructBuffer::SetGeneric(
	const int32 Index,
	const FVoxelRuntimePinValue& Value)
{
	const FVoxelRuntimeStruct& Struct = Value.GetStructView().Get<FVoxelRuntimeStruct>();

	for (const auto& It : Struct.PropertyNameToValue)
	{
		const TSharedPtr<FVoxelBuffer> Buffer = PropertyNameToBuffer.FindRef(It.Key);
		if (!ensure(Buffer))
		{
			continue;
		}

		Buffer->SetGeneric(Index, It.Value);
	}
}

FVoxelRuntimePinValue FVoxelRuntimeStructBuffer::GetGeneric(const int32 Index) const
{
	const TSharedRef<FVoxelRuntimeStruct> Result = MakeShared<FVoxelRuntimeStruct>();
	Result->UserStruct = CastEnsured<UUserDefinedStruct>(&GetInnerStruct());
	Result->PropertyNameToValue.Reserve(PropertyNameToBuffer.Num());

	for (const auto& It : PropertyNameToBuffer)
	{
		Result->PropertyNameToValue.Add_EnsureNew(It.Key, It.Value->GetGeneric(Index));
	}

	return FVoxelRuntimePinValue::MakeRuntimeStruct(Result);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelRuntimeStructBuffer::SetBuffer(
	const FName PropertyName,
	const TSharedRef<FVoxelBuffer>& Buffer)
{
	TSharedPtr<FVoxelBuffer>* Value = PropertyNameToBuffer.Find(PropertyName);
	if (!ensure(Value))
	{
		return;
	}

	*Value = Buffer;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 FVoxelRuntimeStructBuffer::ComputeNum() const
{
	int32 NewNum = 1;
	for (const auto& It : PropertyNameToBuffer)
	{
		ensure(FVoxelBufferAccessor::MergeNum(NewNum, It.Value->Num_Slow()));
	}
	ensure(NewNum >= 0);

	return NewNum;
}

bool FVoxelRuntimeStructBuffer::IsCompatibleWith(const FVoxelRuntimeStructBuffer& Other) const
{
	if (InnerStruct != Other.InnerStruct)
	{
		return false;
	}

	return ensure(PropertyNameToBuffer.HasSameKeys_Ordered(Other.PropertyNameToBuffer));
}