// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelRuntimePinValue.h"
#include "VoxelBuffer.generated.h"

struct FVoxelInt32Buffer;
class FVoxelBulkHasher;
class FVoxelBufferSplitter;

struct FVoxelBufferInitializers
{
	enum EInitializer_MakeDefault
	{
		DefaultBuffer
	};
};

USTRUCT()
struct VOXELGRAPH_API FVoxelBuffer
#if CPP
	: public FVoxelBufferInitializers
#endif
{
	GENERATED_BODY()

public:
	FVoxelBuffer() = default;
	virtual ~FVoxelBuffer() = default;

public:
	template<typename T>
	requires std::derived_from<T, FVoxelBuffer>
	FORCEINLINE bool IsA() const
	{
		if constexpr (std::is_final_v<T>)
		{
			return GetStruct() == StaticStructFast<T>();
		}

		return GetStruct()->IsChildOf(StaticStructFast<T>());
	}

	template<typename T>
	requires std::derived_from<T, FVoxelBuffer>
	FORCEINLINE T* As()
	{
		if (!IsA<T>())
		{
			return nullptr;
		}

		return static_cast<T*>(this);
	}
	template<typename T>
	requires std::derived_from<T, FVoxelBuffer>
	FORCEINLINE const T* As() const
	{
		if (!IsA<T>())
		{
			return nullptr;
		}

		return static_cast<const T*>(this);
	}

	template<typename T>
	requires std::derived_from<T, FVoxelBuffer>
	FORCEINLINE T& AsChecked()
	{
		checkVoxelSlow(IsA<T>());
		return static_cast<T&>(*this);
	}
	template<typename T>
	requires std::derived_from<T, FVoxelBuffer>
	FORCEINLINE const T& AsChecked() const
	{
		checkVoxelSlow(IsA<T>());
		return static_cast<const T&>(*this);
	}

public:
	static TSharedRef<FVoxelBuffer> MakeEmpty(const FVoxelPinType& InnerType);
	static TSharedRef<FVoxelBuffer> MakeDefault(const FVoxelPinType& InnerType);
	static TSharedRef<FVoxelBuffer> MakeConstant(const FVoxelRuntimePinValue& Value);

	static TSharedRef<FVoxelBuffer> MakeZeroed(
		const FVoxelPinType& InnerType,
		int32 Num);

	static FVoxelPinType FindInnerType(UScriptStruct* BufferStruct);

public:
	FORCEINLINE FVoxelPinType GetBufferType() const
	{
		return GetInnerType().GetBufferType();
	}

	TSharedRef<FVoxelBuffer> MakeDeepCopy() const;
	FVoxelRuntimePinValue GetGenericConstant() const;
	void SetAllGeneric(const FVoxelRuntimePinValue& Value);

	void CopyFrom(const FVoxelBuffer& Other);

	void IndirectCopyFrom(
		const FVoxelBuffer& Source,
		const TVoxelOptional<FVoxelInt32Buffer>& SourceToThis);

	bool IsConstant_Slow() const;
	bool IsValidIndex_Slow(int32 Index) const;

	static void Serialize(
		FArchive& Ar,
		TSharedPtr<FVoxelBuffer>& Buffer);

public:
	virtual UScriptStruct* GetStruct() const VOXEL_PURE_VIRTUAL({});
	virtual FVoxelPinType GetInnerType() const VOXEL_PURE_VIRTUAL({});

	virtual int32 Num_Slow() const VOXEL_PURE_VIRTUAL({});
	virtual bool IsValid_Slow() const VOXEL_PURE_VIRTUAL({});
	virtual int64 GetAllocatedSize() const VOXEL_PURE_VIRTUAL({});

	virtual void Allocate(int32 NewNum) VOXEL_PURE_VIRTUAL();
	virtual void AllocateZeroed(int32 NewNum) VOXEL_PURE_VIRTUAL();
	virtual void ShrinkTo(int32 NewNum) VOXEL_PURE_VIRTUAL();
	virtual void SerializeData(FArchive& Ar) VOXEL_PURE_VIRTUAL();
	virtual bool Equal(const FVoxelBuffer& Other) const VOXEL_PURE_VIRTUAL({});

public:
	enum class EBulkEqualFlags : uint8
	{
		Set,
		And
	};
	virtual void BulkEqual(
		const FVoxelBuffer& Other,
		TVoxelArrayView<bool> Result,
		EBulkEqualFlags Flags = EBulkEqualFlags::Set) const VOXEL_PURE_VIRTUAL();

	virtual void CopyFrom(
		const FVoxelBuffer& Source,
		int32 SrcIndex,
		int32 DestIndex,
		int32 NumToCopy) VOXEL_PURE_VIRTUAL();

	virtual void MoveFrom(FVoxelBuffer&& Source) VOXEL_PURE_VIRTUAL();

	virtual void IndirectCopyFrom(
		const FVoxelBuffer& Source,
		TConstVoxelArrayView<int32> SourceToThis) VOXEL_PURE_VIRTUAL();

public:
	virtual TSharedRef<FVoxelBuffer> Gather(TConstVoxelArrayView<int32> Indices) const VOXEL_PURE_VIRTUAL({});

	virtual TSharedRef<FVoxelBuffer> Replicate(
		TConstVoxelArrayView<int32> Counts,
		int32 NewNum) const VOXEL_PURE_VIRTUAL({});

	virtual void Split(
		const FVoxelBufferSplitter& Splitter,
		TVoxelArrayView<FVoxelBuffer*> OutBuffers) const VOXEL_PURE_VIRTUAL();

	virtual void MergeFrom(
		const FVoxelBufferSplitter& Splitter,
		TConstVoxelArrayView<const FVoxelBuffer*> Buffers) VOXEL_PURE_VIRTUAL();

	virtual void HashCombine(TVoxelArrayView<uint64> InOutHashes) const VOXEL_PURE_VIRTUAL();

public:
	virtual void SetGeneric(
		int32 Index,
		const FVoxelRuntimePinValue& Value) VOXEL_PURE_VIRTUAL();

	virtual FVoxelRuntimePinValue GetGeneric(int32 Index) const VOXEL_PURE_VIRTUAL({});

public:
	using FMakeBuffer = TSharedRef<FVoxelBuffer>(*)();

	virtual FMakeBuffer Internal_GetMakeEmpty() const VOXEL_PURE_VIRTUAL({});
	virtual FMakeBuffer Internal_GetMakeDefault() const VOXEL_PURE_VIRTUAL({});
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define DECLARE_VOXEL_BUFFER(InBufferType, InUniformType) \
	checkStatic(std::is_trivially_destructible_v<InUniformType>); \
	struct InBufferType; \
	\
	template<> \
	struct TVoxelBufferTypeImpl<InUniformType> \
	{ \
		using Type = InBufferType; \
	}; \
	template<> \
	struct TVoxelBufferInnerTypeImpl<InBufferType> \
	{ \
		using Type = InUniformType; \
	}; \
	template<> \
	inline constexpr bool IsVoxelBuffer<InBufferType> = true;

#define DECLARE_VOXEL_DOUBLE_BUFFER(InBufferType, InUniformType, InUniformTypeCpp) \
	DECLARE_VOXEL_BUFFER(InBufferType, InUniformType) \
	template<> \
	struct TVoxelDoubleBufferTypeImpl<InUniformTypeCpp> \
	{ \
		using Type = InBufferType; \
	};