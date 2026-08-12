// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBuffer.h"
#include "VoxelBufferStruct.generated.h"

struct FVoxelTerminalBuffer;

USTRUCT()
struct VOXELGRAPH_API FVoxelBufferStruct : public FVoxelBuffer
{
	GENERATED_BODY()

public:
	//~ Begin FVoxelBuffer Interface
	virtual int32 Num_Slow() const final override;
	virtual bool IsValid_Slow() const final override;
	virtual int64 GetAllocatedSize() const final override;
	virtual void Allocate(int32 NewNum) override;
	virtual void AllocateZeroed(int32 NewNum) override;
	virtual void ShrinkTo(int32 NewNum) final override;
	virtual void SerializeData(FArchive& Ar) override;
	virtual bool Equal(const FVoxelBuffer& Other) const final override;
	virtual void BulkEqual(const FVoxelBuffer& Other, TVoxelArrayView<bool> Result, EBulkEqualFlags Flags) const final override;
	virtual void CopyFrom(const FVoxelBuffer& Source, int32 SrcIndex, int32 DestIndex, int32 NumToCopy) final override;
	virtual void MoveFrom(FVoxelBuffer&& Source) final override;
	virtual void IndirectCopyFrom(const FVoxelBuffer& Source, TConstVoxelArrayView<int32> SourceToThis) final override;
	virtual TSharedRef<FVoxelBuffer> Gather(TConstVoxelArrayView<int32> Indices) const final override;
	virtual TSharedRef<FVoxelBuffer> Replicate(TConstVoxelArrayView<int32> Counts, int32 NewNum) const final override;
	virtual void Split(const FVoxelBufferSplitter& Splitter, TVoxelArrayView<FVoxelBuffer*> OutBuffers) const final override;
	virtual void MergeFrom(const FVoxelBufferSplitter& Splitter, TConstVoxelArrayView<const FVoxelBuffer*> Buffers) override;
	virtual void HashCombine(TVoxelArrayView<uint64> InOutHashes) const final override;
	//~ End FVoxelBuffer Interface

public:
	// Will ensure all child buffers have the same num
	void ExpandConstants();
	void ExpandConstants(int32 NewNum);

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

public:
	FORCEINLINE int32 NumTerminalBuffers() const
	{
		return PrivateNumTerminalBuffers;
	}
	FORCEINLINE FVoxelTerminalBuffer& GetTerminalBuffer(const int32 Index)
	{
		return *reinterpret_cast<FVoxelTerminalBuffer*>(reinterpret_cast<uint8*>(this) + GetTerminalBufferOffsets()[Index]);
	}
	FORCEINLINE const FVoxelTerminalBuffer& GetTerminalBuffer(const int32 Index) const
	{
		return ConstCast(this)->GetTerminalBuffer(Index);
	}

	FORCEINLINE int32 NumBufferStructs() const
	{
		return PrivateNumBufferStructs;
	}
	FORCEINLINE FVoxelBufferStruct& GetBufferStruct(const int32 Index)
	{
		return *reinterpret_cast<FVoxelBufferStruct*>(reinterpret_cast<uint8*>(this) + GetBufferStructOffsets()[Index]);
	}
	FORCEINLINE const FVoxelBufferStruct& GetBufferStruct(const int32 Index) const
	{
		return ConstCast(this)->GetBufferStruct(Index);
	}

public:
	template<typename Type>
	struct TVoxelBufferIterator
	{
		FVoxelBufferStruct* Buffer = nullptr;
		TConstVoxelArrayView<int64> BufferOffsets;

		struct FIterator
		{
			FVoxelBuffer* Buffer = nullptr;
			const int64* OffsetPtr = nullptr;

			FORCEINLINE Type& operator*() const
			{
				FVoxelBuffer* TerminalBuffer = reinterpret_cast<FVoxelBuffer*>(reinterpret_cast<uint8*>(Buffer) + *OffsetPtr);
				return TerminalBuffer->AsChecked<Type>();
			}
			FORCEINLINE void operator++()
			{
				++OffsetPtr;
			}
			FORCEINLINE bool operator!=(const FIterator& Other) const
			{
				return OffsetPtr != Other.OffsetPtr;
			}
		};
		FIterator begin() const
		{
			return { Buffer, BufferOffsets.GetData() };
		}
		FIterator end() const
		{
			return { Buffer, BufferOffsets.GetData() + BufferOffsets.Num() };
		}
	};

	FORCEINLINE TVoxelBufferIterator<FVoxelTerminalBuffer> GetTerminalBuffers()
	{
		return { this, GetTerminalBufferOffsets() };
	}
	FORCEINLINE TVoxelBufferIterator<const FVoxelTerminalBuffer> GetTerminalBuffers() const
	{
		return { ConstCast(this), GetTerminalBufferOffsets() };
	}

	FORCEINLINE TVoxelBufferIterator<FVoxelBufferStruct> GetBufferStructs()
	{
		return { this, GetBufferStructOffsets() };
	}
	FORCEINLINE TVoxelBufferIterator<const FVoxelBufferStruct> GetBufferStructs() const
	{
		return { ConstCast(this), GetBufferStructOffsets() };
	}

protected:
	struct VOXELGRAPH_API FTypeInfo
	{
		int16 NumTerminalBuffers = 0;
		int16 NumBufferStructs = 0;
		int64* Offsets = nullptr;

		explicit FTypeInfo(const FVoxelBufferStruct* Template);
	};

	template<typename T>
	FORCEINLINE void Initialize()
	{
		VOXEL_STATIC_HELPER(const FTypeInfo*)
		{
			StaticValue = new FTypeInfo(this);
		}

		PrivateNumTerminalBuffers = StaticValue->NumTerminalBuffers;
		PrivateNumBufferStructs = StaticValue->NumBufferStructs;
		PrivateInternalOffsets = StaticValue->Offsets;
	}

private:
	mutable int32 PrivateNum = -1;
	int16 PrivateNumTerminalBuffers = 0;
	int16 PrivateNumBufferStructs = 0;
	int64* PrivateInternalOffsets = nullptr;

	FORCEINLINE TConstVoxelArrayView<int64> GetTerminalBufferOffsets() const
	{
		return MakeVoxelArrayView(PrivateInternalOffsets, int32(PrivateNumTerminalBuffers));
	}
	FORCEINLINE TConstVoxelArrayView<int64> GetBufferStructOffsets() const
	{
		return MakeVoxelArrayView(PrivateInternalOffsets + PrivateNumTerminalBuffers, int32(PrivateNumBufferStructs));
	}

	int32 ComputeNum() const;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GENERATED_VOXEL_BUFFER_STRUCT_BODY_IMPL(InThisType, InType, InTypeCpp) \
		using UniformType = InType; \
		using UniformTypeCpp = InTypeCpp; \
		using BufferType = TVoxelBufferType<InType>; \
		\
	private: \
		virtual FVoxelRuntimePinValue GetGeneric(const int32 Index) const override \
		{ \
			return FVoxelRuntimePinValue::Make(UniformType(operator[](Index))); \
		} \
		virtual void SetGeneric(const int32 Index, const FVoxelRuntimePinValue& Value) override \
		{ \
			return Set(Index, InTypeCpp(Value.Get<UniformType>())); \
		}  \
		\
		virtual FMakeBuffer Internal_GetMakeEmpty() const override \
		{ \
			return [] { return ReinterpretCastRef<TSharedRef<FVoxelBuffer>>(MakeShared<InThisType>()); }; \
		} \
		virtual FMakeBuffer Internal_GetMakeDefault() const override \
		{ \
			return [] { return ReinterpretCastRef<TSharedRef<FVoxelBuffer>>(MakeSharedDefault()); }; \
		} \
		\
	public: \
		FORCEINLINE InThisType() \
		{ \
			checkStatic(std::is_final_v<VOXEL_THIS_TYPE>); \
			checkStatic(std::is_same_v<Super, FVoxelBufferStruct>); \
			Initialize<InType>(); \
		} \
		FORCEINLINE InThisType(EInitializer_MakeDefault) \
		{ \
			Initialize<InType>(); \
			Allocate(1); \
			Set(0, FVoxelUtilities::MakeSafe<InTypeCpp>()); \
		} \
		FORCEINLINE InThisType(const InTypeCpp& Constant) \
		{ \
			Initialize<InType>(); \
			Allocate(1); \
			Set(0, Constant); \
		} \
		FORCEINLINE InThisType& operator=(const InTypeCpp& Constant) \
		{ \
			Allocate(1); \
			Set(0, Constant); \
			return *this; \
		} \
		\
		FORCEINLINE static InThisType MakeDefault() \
		{ \
			return InThisType(DefaultBuffer); \
		} \
		FORCEINLINE static TSharedRef<InThisType> MakeSharedDefault() \
		{ \
			return MakeShared<InThisType>(DefaultBuffer); \
		} \
		\
		FORCEINLINE InTypeCpp GetConstant() const \
		{ \
			checkVoxelSlow(IsConstant()); \
			checkStatic(std::is_const_v<decltype(operator[](0))>); \
			return operator[](0); \
		} \
		\
		FORCEINLINE virtual UScriptStruct* GetStruct() const override \
		{ \
			return StaticStructFast<InThisType>(); \
		} \
		FORCEINLINE virtual FVoxelPinType GetInnerType() const override \
		{ \
			return FVoxelPinType::Make<InType>(); \
		}

#define GENERATED_VOXEL_BUFFER_STRUCT_BODY(InThisType, InType) \
	GENERATED_VOXEL_BUFFER_STRUCT_BODY_IMPL(InThisType, InType, InType)