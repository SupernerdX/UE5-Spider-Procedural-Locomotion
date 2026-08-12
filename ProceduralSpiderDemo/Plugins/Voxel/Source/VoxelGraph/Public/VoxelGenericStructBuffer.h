// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBuffer.h"
#include "VoxelGenericStructBuffer.generated.h"

DECLARE_VOXEL_MEMORY_STAT(VOXELGRAPH_API, STAT_VoxelGenericStructBuffer, "Voxel Generic Struct Buffer");

USTRUCT()
struct VOXELGRAPH_API FVoxelGenericStructBuffer final : public FVoxelBuffer
{
	GENERATED_BODY()

public:
	FVoxelGenericStructBuffer() = default;
	FORCEINLINE FVoxelGenericStructBuffer(FVoxelGenericStructBuffer&& Other)
		: InnerStruct(Other.InnerStruct)
		, CppStructOps(Other.CppStructOps)
		, TypeSize(Other.TypeSize)
		, ArrayNum(Other.ArrayNum)
		, Data(Other.Data)
#if VOXEL_STATS
		, AllocatedSizeTracker(MoveTemp(Other.AllocatedSizeTracker))
#endif
	{
		Other.ArrayNum = 0;
		Other.Data = nullptr;
	}
	FORCEINLINE FVoxelGenericStructBuffer(const FVoxelGenericStructBuffer& Other)
		: InnerStruct(Other.InnerStruct)
		, CppStructOps(Other.CppStructOps)
		, TypeSize(Other.TypeSize)
#if VOXEL_STATS
		, AllocatedSizeTracker(MoveTemp(Other.AllocatedSizeTracker))
#endif
	{
		if (Other.Num() > 0)
		{
			Allocate(Other.Num());
			CopyFrom(Other, 0, 0, Num());
		}
	}
	FORCEINLINE virtual ~FVoxelGenericStructBuffer() override
	{
		if (Data)
		{
			Empty();
		}
	}

	FORCEINLINE FVoxelGenericStructBuffer& operator=(FVoxelGenericStructBuffer&& Other)
	{
		this->~FVoxelGenericStructBuffer();
		new(this) FVoxelGenericStructBuffer(MoveTemp(Other));
		return *this;

	}
	FORCEINLINE FVoxelGenericStructBuffer& operator=(const FVoxelGenericStructBuffer& Other)
	{
		this->~FVoxelGenericStructBuffer();
		new(this) FVoxelGenericStructBuffer(Other);
		return *this;
	}

	static FVoxelGenericStructBuffer MakeDefault(UScriptStruct& Struct);
	static FVoxelGenericStructBuffer MakeEmpty(UScriptStruct& Struct);

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

	void Empty();
	void SetConstant(FConstVoxelStructView Constant);

public:
	FORCEINLINE int32 Num() const
	{
		return ArrayNum;
	}
	FORCEINLINE bool IsConstant() const
	{
		return ArrayNum == 1;
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
	FORCEINLINE FConstVoxelStructView operator[](int32 Index) const
	{
		if (IsConstant())
		{
			Index = 0;
		}

		return ConstCast(this)->GetMutable(Index);
	}

	FORCEINLINE FVoxelStructView GetMutable(const int32 Index)
	{
		checkVoxelSlow(InnerStruct);
		checkVoxelSlow(TypeSize > 0);
		checkVoxelSlow(0 <= Index && Index < ArrayNum);

		return FVoxelStructView(InnerStruct, static_cast<uint8*>(Data) + TypeSize * Index);
	}
	FORCEINLINE void Set(const int32 Index, const FConstVoxelStructView Value)
	{
		Value.CopyTo(GetMutable(Index));
	}

	FORCEINLINE FConstVoxelStructView GetConstant() const
	{
		checkVoxelSlow(IsConstant());
		return (*this)[0];
	}

private:
	// Store CppStructOps to always be able to destroy structs on shutdown

	UScriptStruct* InnerStruct = nullptr;
	UScriptStruct::ICppStructOps* CppStructOps = nullptr;
	int32 TypeSize = 0;

	int32 ArrayNum = 0;
	void* Data = nullptr;

	VOXEL_ALLOCATED_SIZE_TRACKER_CUSTOM(STAT_VoxelGenericStructBuffer, AllocatedSizeTracker);
};