// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBuffer.h"
#include "VoxelTerminalBuffer.generated.h"

DECLARE_VOXEL_MEMORY_STAT(VOXELGRAPH_API, STAT_VoxelBuffer, "Voxel Buffer");

struct VOXELGRAPH_API FVoxelTerminalBufferAllocation
{
public:
	static void* Allocate(int64 NumBytes);

	FORCEINLINE static FVoxelTerminalBufferAllocation* GetAllocation(void* Data)
	{
		if (!Data)
		{
			return nullptr;
		}

		FVoxelTerminalBufferAllocation* Allocation = reinterpret_cast<FVoxelTerminalBufferAllocation*>(static_cast<uint8*>(Data) - sizeof(FVoxelTerminalBufferAllocation));
		Allocation->CheckMagic();
		return Allocation;
	}

	VOXEL_COUNT_INSTANCES();

public:
	FORCEINLINE void* GetData() const
	{
		CheckMagic();
		return reinterpret_cast<uint8*>(ConstCast(this)) + sizeof(FVoxelTerminalBufferAllocation);
	}
	FORCEINLINE int64 GetAllocatedSize() const
	{
		CheckMagic();
		return PrivateNumBytes;
	}

public:
	FORCEINLINE void AddRef()
	{
		CheckMagic();

		NumRefs.Increment();
	}
	FORCEINLINE void RemoveRef()
	{
		CheckMagic();

		if (NumRefs.Decrement_ReturnNew() == 0)
		{
			Destroy();
		}
	}

private:
	const uint32 Magic = 0xDEADBEEF;
	FVoxelCounter32 NumRefs;
	int64 PrivateNumBytes = 0;

	explicit FVoxelTerminalBufferAllocation(int64 NumBytes);

	void Destroy();

	FORCEINLINE void CheckMagic() const
	{
		checkVoxelSlow(Magic == 0xDEADBEEF);
	}
};

USTRUCT()
struct VOXELGRAPH_API FVoxelTerminalBuffer : public FVoxelBuffer
{
	GENERATED_BODY()

public:
	FVoxelTerminalBuffer() = default;
	FORCEINLINE FVoxelTerminalBuffer(FVoxelTerminalBuffer&& Other)
		: ArrayNum(Other.ArrayNum)
		, IndexMask(Other.IndexMask)
		, Data(Other.Data)
	{
		Other.ArrayNum = 0;
		Other.IndexMask = 0;
		Other.Data = nullptr;
	}
	FORCEINLINE FVoxelTerminalBuffer(const FVoxelTerminalBuffer& Other)
		: ArrayNum(Other.ArrayNum)
		, IndexMask(Other.IndexMask)
		, Data(Other.Data)
	{
		if (FVoxelTerminalBufferAllocation* Allocation = GetAllocation())
		{
			Allocation->AddRef();
		}
	}
	FORCEINLINE virtual ~FVoxelTerminalBuffer() override
	{
		Empty();
	}

	FORCEINLINE FVoxelTerminalBuffer& operator=(FVoxelTerminalBuffer&& Other)
	{
		Empty();

		ArrayNum = Other.ArrayNum;
		IndexMask = Other.IndexMask;
		Data = Other.Data;

		Other.ArrayNum = 0;
		Other.IndexMask = 0;
		Other.Data = nullptr;

		return *this;
	}
	FORCEINLINE FVoxelTerminalBuffer& operator=(const FVoxelTerminalBuffer& Other)
	{
		Empty();

		ArrayNum = Other.ArrayNum;
		IndexMask = Other.IndexMask;
		Data = Other.Data;

		if (FVoxelTerminalBufferAllocation* Allocation = GetAllocation())
		{
			Allocation->AddRef();
		}

		return *this;
	}

public:
	FORCEINLINE void Empty()
	{
		if (FVoxelTerminalBufferAllocation* Allocation = GetAllocation())
		{
			Allocation->RemoveRef();
		}

		ArrayNum = 0;
		IndexMask = 0;
		Data = nullptr;
	}

protected:
	template<typename OtherType>
	FORCEINLINE OtherType ReinterpretAs() const
	{
		if (FVoxelTerminalBufferAllocation* Allocation = GetAllocation())
		{
			Allocation->AddRef();
		}

		OtherType Other;
		Other.ArrayNum = ArrayNum;
		Other.IndexMask = IndexMask;
		Other.Data = Data;
		return Other;
	}

public:
	//~ Begin FVoxelBuffer Interface
	virtual int32 Num_Slow() const final override;
	virtual bool IsValid_Slow() const final override;
	virtual int64 GetAllocatedSize() const final override;
	virtual void Allocate(int32 NewNum) final override;
	virtual void AllocateZeroed(int32 NewNum) override;
	virtual void ShrinkTo(int32 NewNum) final override;
	virtual void SerializeData(FArchive& Ar) override;
	virtual bool Equal(const FVoxelBuffer& Other) const override;
	virtual void BulkEqual(const FVoxelBuffer& Other, TVoxelArrayView<bool> Result, EBulkEqualFlags Flags) const override;
	virtual void CopyFrom(const FVoxelBuffer& Source, int32 SrcIndex, int32 DestIndex, int32 NumToCopy) final override;
	virtual void MoveFrom(FVoxelBuffer&& Source) final override;
	virtual void IndirectCopyFrom(const FVoxelBuffer& Source, TConstVoxelArrayView<int32> SourceToThis) final override;
	virtual TSharedRef<FVoxelBuffer> Gather(TConstVoxelArrayView<int32> Indices) const final override;
	virtual TSharedRef<FVoxelBuffer> Replicate(TConstVoxelArrayView<int32> Counts, int32 NewNum) const final override;
	virtual void Split(const FVoxelBufferSplitter& Splitter, TVoxelArrayView<FVoxelBuffer*> OutBuffers) const final override;
	virtual void MergeFrom(const FVoxelBufferSplitter& Splitter, TConstVoxelArrayView<const FVoxelBuffer*> Buffers) final override;
	virtual void HashCombine(TVoxelArrayView<uint64> InOutHashes) const final override;
	//~ End FVoxelBuffer Interface

public:
	void ExpandConstant(int32 NewNum);
	void ExpandConstantIfNeeded(int32 NewNum);

	void Gather(
		TConstVoxelArrayView<int32> Indices,
		FVoxelTerminalBuffer& OutResult) const;

	void Replicate(
		TConstVoxelArrayView<int32> Counts,
		int32 NewNum,
		FVoxelTerminalBuffer& OutResult) const;

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

public:
	FORCEINLINE const void* GetRawData() const
	{
		return Data;
	}
	FORCEINLINE void* GetRawData()
	{
		return Data;
	}

public:
	// Will not handle IsConstant, prefer using [] directly or SetTyped
	template<typename T>
	FORCEINLINE TConstVoxelArrayView<T> GetTypedView() const
	{
		checkVoxelSlow(sizeof(T) == GetTypeSize_Slow());
		return TConstVoxelArrayView<T>(static_cast<const T*>(GetRawData()), ArrayNum);
	}
	template<typename T>
	FORCEINLINE TVoxelArrayView<T> GetTypedView()
	{
		checkVoxelSlow(sizeof(T) == GetTypeSize_Slow());
		return TVoxelArrayView<T>(static_cast<T*>(GetRawData()), ArrayNum);
	}

	template<typename T>
	FORCEINLINE const T& GetConstant() const
	{
		checkVoxelSlow(IsConstant());
		checkVoxelSlow(sizeof(T) == GetTypeSize_Slow());
		return GetTypedView<T>()[0];
	}
	template<typename T>
	FORCEINLINE void SetConstant(const T& Constant)
	{
		checkVoxelSlow(sizeof(T) == GetTypeSize_Slow());

		if (ArrayNum != 1)
		{
			if (FVoxelTerminalBufferAllocation* Allocation = GetAllocation())
			{
				Allocation->RemoveRef();
			}

			ArrayNum = 1;
			IndexMask = 0;
			Data = FVoxelTerminalBufferAllocation::Allocate(sizeof(T));
		}

		GetTypedView<T>()[0] = Constant;
	}

	template<typename T>
	FORCEINLINE const T& GetTyped(const int32 Index) const
	{
		checkVoxelSlow(sizeof(T) == GetTypeSize_Slow());
		checkVoxelSlow(IndexMask == (IsConstant() ? 0 : 0xFFFFFFFF));
		return GetTypedView<T>()[Index & IndexMask];
	}
	template<typename T>
	FORCEINLINE void SetTyped(const int32 Index, const T& Value)
	{
		checkVoxelSlow(sizeof(T) == GetTypeSize_Slow());
		GetTypedView<T>()[Index] = Value;
	}
	template<typename T>
	FORCEINLINE void SetAllTyped(const T& Value)
	{
		checkVoxelSlow(sizeof(T) == GetTypeSize_Slow());
		FVoxelUtilities::SetAll(GetTypedView<T>(), Value);
	}

public:
	template<int32 Size>
	struct alignas(uint64) TBytes
	{
		checkStatic(Size % sizeof(uint64) == 0);
		static constexpr int32 NumWords = Size / sizeof(uint64);

		TVoxelStaticArray<uint64, NumWords> Words{ NoInit };

		TBytes() = default;
		FORCEINLINE explicit TBytes(const int32 Value)
		{
			checkVoxelSlow(Value == 0);
			Words.Memzero();
		}

		FORCEINLINE bool operator==(const TBytes& Other) const
		{
			return Words == Other.Words;
		}
	};

	template<typename LambdaType>
	void SwitchTypeSize(LambdaType&& Lambda) const
	{
		switch (GetTypeSize_Slow())
		{
		default: VOXEL_ASSUME(false);
		case 1: return Lambda.template operator()<uint8>();
		case 2: return Lambda.template operator()<uint16>();
		case 4: return Lambda.template operator()<uint32>();
		case 8: return Lambda.template operator()<uint64>();
		// FVoxelObjectPtr
		case 16: return Lambda.template operator()<TBytes<16>>();
		// FVoxelSurfaceTypeBlend
		case 64: return Lambda.template operator()<TBytes<64>>();
		}
	}

protected:
	virtual int32 GetTypeSize_Slow() const VOXEL_PURE_VIRTUAL({});

private:
	int32 ArrayNum = 0;
	uint32 IndexMask = 0;
	void* Data = nullptr;

	FORCEINLINE FVoxelTerminalBufferAllocation* GetAllocation() const
	{
		return FVoxelTerminalBufferAllocation::GetAllocation(Data);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define DECLARE_VOXEL_TERMINAL_BUFFER(InBufferType, InUniformType) \
	DECLARE_VOXEL_BUFFER(InBufferType, InUniformType)

#define GENERATED_VOXEL_TERMINAL_BUFFER_BODY(InThisType, InType) \
		using UniformType = InType; \
		using BufferType = TVoxelBufferType<InType>; \
		\
	private: \
		using Super::GetTypedView; \
		using Super::GetConstant; \
		using Super::SetConstant; \
		using Super::GetTyped; \
		using Super::SetTyped; \
		\
		virtual void SetGeneric(const int32 Index, const FVoxelRuntimePinValue& Value) override \
		{ \
			Set(Index, Value.Get<InType>()); \
		} \
		virtual FVoxelRuntimePinValue GetGeneric(const int32 Index) const override \
		{ \
			return FVoxelRuntimePinValue::Make(operator[](Index)); \
		} \
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
		virtual int32 GetTypeSize_Slow() const \
		{ \
			return sizeof(InType); \
		} \
		\
	public: \
		FORCEINLINE InThisType() \
		{ \
			checkStatic(std::is_final_v<VOXEL_THIS_TYPE>); \
			checkStatic(std::is_same_v<Super, FVoxelTerminalBuffer>); \
		} \
		FORCEINLINE InThisType(EInitializer_MakeDefault) \
		{ \
			SetConstant(FVoxelUtilities::MakeSafe<InType>()); \
		} \
		FORCEINLINE InThisType(const InType& Constant) \
		{ \
			SetConstant(Constant); \
		} \
		FORCEINLINE InThisType& operator=(const InType& Constant) \
		{ \
			SetConstant(Constant); \
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
		InThisType MakeDeepCopy() const \
		{ \
			return MoveTemp(Super::MakeDeepCopy()->AsChecked<InThisType>()); \
		} \
		\
		template<typename OtherType> \
		requires \
		( \
			std::derived_from<OtherType, FVoxelTerminalBuffer> && \
			sizeof(UniformType) == sizeof(typename OtherType::UniformType) \
		) \
		FORCEINLINE OtherType ReinterpretAs() const \
		{ \
			return Super::ReinterpretAs<OtherType>(); \
		} \
		\
		FORCEINLINE const InType& operator[](const int32 Index) const \
		{ \
			return GetTyped<InType>(Index); \
		} \
		FORCEINLINE void Set(const int32 Index, const InType& Value) \
		{ \
			SetTyped(Index, Value); \
		} \
		FORCEINLINE void SetAll(const InType& Value) \
		{ \
			SetAllTyped(Value); \
		} \
		\
		FORCEINLINE const InType& GetConstant() const \
		{ \
			return Super::GetConstant<InType>(); \
		} \
		FORCEINLINE void SetConstant(const InType& Constant) \
		{ \
			Super::SetConstant(Constant); \
		} \
		\
		FORCEINLINE InType* GetData() \
		{ \
			return View().GetData(); \
		} \
		FORCEINLINE const InType* GetData() const \
		{ \
			return View().GetData(); \
		} \
		FORCEINLINE TVoxelArrayView<InType> View() \
		{ \
			return GetTypedView<InType>(); \
		} \
		FORCEINLINE TConstVoxelArrayView<InType> View() const \
		{ \
			return GetTypedView<InType>(); \
		} \
		\
		FORCEINLINE const InType* begin() const \
		{ \
			return View().begin(); \
		} \
		FORCEINLINE const InType* end() const \
		{ \
			return View().end(); \
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