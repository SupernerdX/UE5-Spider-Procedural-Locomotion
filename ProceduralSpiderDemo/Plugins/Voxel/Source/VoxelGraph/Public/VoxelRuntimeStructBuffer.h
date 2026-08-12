// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBuffer.h"
#include "VoxelRuntimeStructBuffer.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelRuntimeStructBuffer final : public FVoxelBuffer
{
	GENERATED_BODY()

public:
	FVoxelRuntimeStructBuffer() = default;
	FORCEINLINE FVoxelRuntimeStructBuffer(FVoxelRuntimeStructBuffer&& Other)
		: InnerStruct(Other.InnerStruct)
		, PropertyNameToBuffer(MoveTemp(Other.PropertyNameToBuffer))
	{
		Other.PrivateNum = -1;
	}
	FORCEINLINE FVoxelRuntimeStructBuffer(const FVoxelRuntimeStructBuffer& Other)
		: InnerStruct(Other.InnerStruct)
		, PropertyNameToBuffer(Other.PropertyNameToBuffer)
	{
	}

	FORCEINLINE FVoxelRuntimeStructBuffer& operator=(FVoxelRuntimeStructBuffer&& Other)
	{
		this->~FVoxelRuntimeStructBuffer();
		new(this) FVoxelRuntimeStructBuffer(MoveTemp(Other));
		return *this;

	}
	FORCEINLINE FVoxelRuntimeStructBuffer& operator=(const FVoxelRuntimeStructBuffer& Other)
	{
		this->~FVoxelRuntimeStructBuffer();
		new(this) FVoxelRuntimeStructBuffer(Other);
		return *this;
	}

	static FVoxelRuntimeStructBuffer MakeDefault(UScriptStruct& Struct);
	static FVoxelRuntimeStructBuffer MakeEmpty(UScriptStruct& Struct);

public:
	//~ Begin FVoxelBuffer Interface
	virtual UScriptStruct* GetStruct() const override;
	virtual FVoxelPinType GetInnerType() const override;
	virtual int32 Num_Slow() const override;
	virtual bool IsValid_Slow() const override;
	virtual int64 GetAllocatedSize() const override;
	virtual void Allocate(int32 NewNum) override;
	virtual void AllocateZeroed(int32 NewNum) override;
	virtual void ShrinkTo(int32 NewNum) override;
	virtual void SerializeData(FArchive& Ar) override;
	virtual bool Equal(const FVoxelBuffer& Other) const override;
	virtual void BulkEqual(const FVoxelBuffer& Other, TVoxelArrayView<bool> Result, EBulkEqualFlags Flags) const override;
	virtual void CopyFrom(const FVoxelBuffer& Source, int32 SrcIndex, int32 DestIndex, int32 NumToCopy) override;
	virtual void MoveFrom(FVoxelBuffer&& Source) override;
	virtual void IndirectCopyFrom(const FVoxelBuffer& Source, TConstVoxelArrayView<int32> SourceToThis) override;
	virtual TSharedRef<FVoxelBuffer> Gather(TConstVoxelArrayView<int32> Indices) const override;
	virtual TSharedRef<FVoxelBuffer> Replicate(TConstVoxelArrayView<int32> Counts, int32 NewNum) const override;
	virtual void Split(const FVoxelBufferSplitter& Splitter, TVoxelArrayView<FVoxelBuffer*> OutBuffers) const override;
	virtual void MergeFrom(const FVoxelBufferSplitter& Splitter, TConstVoxelArrayView<const FVoxelBuffer*> Buffers) override;
	virtual void HashCombine(TVoxelArrayView<uint64> InOutHashes) const final override;
	virtual void SetGeneric(int32 Index, const FVoxelRuntimePinValue& Value) override;
	virtual FVoxelRuntimePinValue GetGeneric(int32 Index) const override;
	//~ End FVoxelBuffer Interface

public:
	FORCEINLINE int32 Num() const
	{
		if (PrivateNum == -1)
		{
			PrivateNum = ComputeNum();
		}
		checkVoxelSlow(PrivateNum == ComputeNum());
		return PrivateNum;
	}
	FORCEINLINE bool IsConstant() const
	{
		return Num() == 1;
	}
	FORCEINLINE bool IsValidIndex(const int32 Index) const
	{
		return 0 <= Index && (IsConstant() || Index < Num());
	}
	FORCEINLINE UScriptStruct& GetInnerStruct() const
	{
		checkVoxelSlow(InnerStruct);
		return *InnerStruct;
	}

public:
	FORCEINLINE TSharedPtr<FVoxelBuffer> GetBuffer(const FName PropertyName)
	{
		return PropertyNameToBuffer.FindRef(PropertyName);
	}
	FORCEINLINE TSharedPtr<const FVoxelBuffer> GetBuffer(const FName PropertyName) const
	{
		return PropertyNameToBuffer.FindRef(PropertyName);
	}

	FORCEINLINE FVoxelBuffer& GetBufferChecked(const FName PropertyName)
	{
		return *PropertyNameToBuffer[PropertyName];
	}
	FORCEINLINE const FVoxelBuffer& GetBufferChecked(const FName PropertyName) const
	{
		return *PropertyNameToBuffer[PropertyName];
	}

	void SetBuffer(
		FName PropertyName,
		const TSharedRef<FVoxelBuffer>& Buffer);

private:
	mutable int32 PrivateNum = -1;
	UScriptStruct* InnerStruct = nullptr;
	TVoxelMap<FName, TSharedPtr<FVoxelBuffer>> PropertyNameToBuffer;

	int32 ComputeNum() const;
	bool IsCompatibleWith(const FVoxelRuntimeStructBuffer& Other) const;
};