// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValue.h"
#include "VoxelTerminalBuffer.h"
#include "VoxelMetadataRef.generated.h"

class UPCGMetadata;
class UVoxelMetadata;
struct FPCGPoint;
struct FVoxelPinType;
struct FVoxelBuffer;
struct FVoxelInt32Buffer;
struct FVoxelFloatBuffer;
struct FVoxelMetadataImpl;
struct FVoxelRuntimePinValue;
struct FVoxelMetadataRefStatics;
enum class EVoxelMetadataMaterialType : uint8;

USTRUCT()
struct VOXEL_API FVoxelMetadataRef
{
	GENERATED_BODY()

public:
	using BufferType = FVoxelBuffer;

public:
	FVoxelMetadataRef() = default;
	explicit FVoxelMetadataRef(TVoxelObjectPtr<UVoxelMetadata> Metadata);
	explicit FVoxelMetadataRef(UVoxelMetadata* Metadata);
	explicit FVoxelMetadataRef(const TObjectPtr<UVoxelMetadata>& Metadata);

	FORCEINLINE bool IsValid() const
	{
		return PrivateIndex != -1;
	}

	FORCEINLINE operator bool() const
	{
		return IsValid();
	}

	FORCEINLINE bool operator==(const FVoxelMetadataRef& Other) const
	{
		return PrivateIndex == Other.PrivateIndex;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelMetadataRef& Ref)
	{
		return Ref.PrivateIndex;
	}

	FName GetFName() const;
	FString GetName() const;
	FGuid GetGuid() const;
	FVoxelPinType GetInnerType() const;
	FVoxelRuntimePinValue GetDefaultValue() const;
	TVoxelOptional<EVoxelMetadataMaterialType> GetMaterialType() const;
	TVoxelObjectPtr<UVoxelMetadata> GetMetadata() const;
	TSharedRef<FVoxelBuffer> MakeDefaultBuffer(int32 Num) const;

	friend void operator<<(FArchive& Ar, FVoxelMetadataRef& MetadataRef);

public:
	void UpdateFromSourceObject() const;

public:
	void Blend(
		const FVoxelBuffer& Value,
		const FVoxelFloatBuffer& Alpha,
		FVoxelBuffer& InOutResult) const;

	void IndirectBlend(
		TConstVoxelArrayView<int32> IndexToResult,
		const FVoxelBuffer& Value,
		const FVoxelFloatBuffer& Alpha,
		FVoxelBuffer& InOutResult) const;

	void IndirectBlend(
		const TVoxelOptional<FVoxelInt32Buffer>& IndexToResult,
		const FVoxelBuffer& Value,
		const FVoxelFloatBuffer& Alpha,
		FVoxelBuffer& InOutResult) const;

	void AddToPCG(
		UPCGMetadata& PCGMetadata,
		TConstVoxelArrayView<FPCGPoint> Points,
		FName Name,
		const FVoxelBuffer& Values) const;

	void WriteMaterialData(
		const FVoxelBuffer& Values,
		TVoxelArrayView<uint8> OutBytes,
		EVoxelMetadataMaterialType MaterialType) const;

	FVoxelPinValue GetValue(
		const FVoxelBuffer& Buffer,
		int32 Index) const;

	FVoxelFloatBuffer GetChannel(
		const FVoxelBuffer& Buffer,
		EVoxelTextureChannel Channel) const;

public:
	static TVoxelArray<FVoxelMetadataRef> GetUniqueValidRefs(TConstVoxelArrayView<UVoxelMetadata*> Metadatas);
	static TVoxelArray<FVoxelMetadataRef> GetUniqueValidRefs(TConstVoxelArrayView<TObjectPtr<UVoxelMetadata>> Metadatas);
	static TVoxelArray<FVoxelMetadataRef> GetUniqueValidRefs(TConstVoxelArrayView<FVoxelMetadataRef> Refs);
	static bool FindFromGuid(
		const FGuid Guid,
		FVoxelMetadataRef& OutMetadata);

private:
	int32 PrivateIndex = -1;

	FVoxelMetadataImpl& GetImpl() const;
	const FVoxelMetadataRefStatics& GetStatics() const;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelMetadataRefBuffer, FVoxelMetadataRef);

USTRUCT()
struct VOXEL_API FVoxelMetadataRefBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelMetadataRefBuffer, FVoxelMetadataRef);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GENERATED_VOXEL_METADATA_REF_BODY(RefType, InObjectType, InBufferType) \
		void __Dummy_ ## RefType (struct InBufferType*, class InObjectType*) {} \
		\
		using BufferType = InBufferType; \
		using ObjectType = InObjectType; \
		\
		RefType() = default; \
		explicit RefType(TVoxelObjectPtr<InObjectType> Metadata); \
		explicit RefType(InObjectType* Metadata); \
		explicit RefType(const TObjectPtr<InObjectType>& Metadata); \
		\
		TVoxelObjectPtr<InObjectType> GetMetadata() const; \
		\
	public: \
		static TVoxelArray<RefType> GetUniqueValidRefs(const TConstVoxelArrayView<InObjectType*> Metadatas); \
		static TVoxelArray<RefType> GetUniqueValidRefs(const TConstVoxelArrayView<TObjectPtr<InObjectType>> Metadatas); \
		static TVoxelArray<RefType> GetUniqueValidRefs(const TConstVoxelArrayView<RefType> Refs); \
		\
	public: \
		static void Blend( \
			const InBufferType& Value, \
			const FVoxelFloatBuffer& Alpha, \
			InBufferType& InOutResult); \
		\
		static void IndirectBlend( \
			TConstVoxelArrayView<int32> IndexToResult, \
			const InBufferType& Value, \
			const FVoxelFloatBuffer& Alpha, \
			InBufferType& InOutResult); \
		\
		static void AddToPCG( \
			UPCGMetadata& PCGMetadata, \
			TConstVoxelArrayView<FPCGPoint> Points, \
			FName Name, \
			const InBufferType& Values); \
		\
		static void WriteMaterialData( \
			const InBufferType& Values, \
			TVoxelArrayView<uint8> OutBytes, \
			EVoxelMetadataMaterialType MaterialType); \
		\
		static FVoxelFloatBuffer GetChannel( \
			const InBufferType& Buffer, \
			EVoxelTextureChannel Channel); \
		\
	public: \
		static void Impl_Blend( \
			const FVoxelBuffer& Value, \
			const FVoxelFloatBuffer& Alpha, \
			FVoxelBuffer& InOutResult); \
		\
		static void Impl_IndirectBlend( \
			TConstVoxelArrayView<int32> IndexToResult, \
			const FVoxelBuffer& Value, \
			const FVoxelFloatBuffer& Alpha, \
			FVoxelBuffer& InOutResult); \
		\
		static void Impl_AddToPCG( \
			UPCGMetadata& PCGMetadata, \
			TConstVoxelArrayView<FPCGPoint> Points, \
			FName Name, \
			const FVoxelBuffer& Values); \
		\
		static void Impl_WriteMaterialData( \
			const FVoxelBuffer& Values, \
			TVoxelArrayView<uint8> OutBytes, \
			EVoxelMetadataMaterialType MaterialType); \
		\
		static FVoxelPinValue Impl_GetValue( \
			const FVoxelBuffer& Buffer, \
			int32 Index); \
		\
		static FVoxelFloatBuffer Impl_GetChannel( \
			const FVoxelBuffer& Buffer, \
			EVoxelTextureChannel Channel); \
		\
	private: \
		using Super::Blend; \
		using Super::AddToPCG; \
		\
	public:

#define DEFINE_VOXEL_METADATA_REF(RefType) \
	RefType::RefType(const TVoxelObjectPtr<RefType::ObjectType> Metadata) \
		: FVoxelMetadataRef(TVoxelObjectPtr<UVoxelMetadata>(Metadata)) \
	{ \
	} \
	RefType::RefType(RefType::ObjectType* Metadata) \
		: FVoxelMetadataRef(static_cast<UVoxelMetadata*>(Metadata)) \
	{ \
	} \
	RefType::RefType(const TObjectPtr<RefType::ObjectType>& Metadata) \
		: FVoxelMetadataRef(TObjectPtr<UVoxelMetadata>(Metadata)) \
	{ \
	} \
	TVoxelObjectPtr<RefType::ObjectType> RefType::GetMetadata() const \
	{ \
		return Super::GetMetadata().CastToChecked<RefType::ObjectType>(); \
	} \
	\
	TVoxelArray<RefType> RefType::GetUniqueValidRefs(const TConstVoxelArrayView<ObjectType*> Metadatas) \
	{ \
		return ReinterpretCastVoxelArray<RefType>(FVoxelMetadataRef::GetUniqueValidRefs(Metadatas.ReinterpretAs<TObjectPtr<UVoxelMetadata>>())); \
	} \
	TVoxelArray<RefType> RefType::GetUniqueValidRefs(const TConstVoxelArrayView<TObjectPtr<ObjectType>> Metadatas) \
	{ \
		return ReinterpretCastVoxelArray<RefType>(FVoxelMetadataRef::GetUniqueValidRefs(Metadatas.ReinterpretAs<TObjectPtr<UVoxelMetadata>>())); \
	} \
	TVoxelArray<RefType> RefType::GetUniqueValidRefs(const TConstVoxelArrayView<RefType> Refs) \
	{ \
		return ReinterpretCastVoxelArray<RefType>(FVoxelMetadataRef::GetUniqueValidRefs(Refs.ReinterpretAs<FVoxelMetadataRef>())); \
	} \
	\
	void RefType::Impl_Blend( \
		const FVoxelBuffer& Value, \
		const FVoxelFloatBuffer& Alpha, \
		FVoxelBuffer& InOutResult) \
	{ \
		Blend(Value.AsChecked<BufferType>(), Alpha, InOutResult.AsChecked<BufferType>()); \
	} \
	void RefType::Impl_IndirectBlend( \
		TConstVoxelArrayView<int32> IndexToResult, \
		const FVoxelBuffer& Value, \
		const FVoxelFloatBuffer& Alpha, \
		FVoxelBuffer& InOutResult) \
	{ \
		IndirectBlend(IndexToResult, Value.AsChecked<BufferType>(), Alpha, InOutResult.AsChecked<BufferType>()); \
	} \
	void RefType::Impl_AddToPCG( \
		UPCGMetadata& PCGMetadata, \
		TConstVoxelArrayView<FPCGPoint> Points, \
		FName Name, \
		const FVoxelBuffer& Values) \
	{ \
		AddToPCG(PCGMetadata, Points, Name, Values.AsChecked<BufferType>()); \
	} \
	void RefType::Impl_WriteMaterialData( \
		const FVoxelBuffer& Values, \
		TVoxelArrayView<uint8> OutBytes, \
		EVoxelMetadataMaterialType MaterialType) \
	{ \
		WriteMaterialData(Values.AsChecked<BufferType>(), OutBytes, MaterialType); \
	} \
	FVoxelPinValue RefType::Impl_GetValue( \
		const FVoxelBuffer& Buffer, \
		const int32 Index) \
	{ \
		return FVoxelPinValue::Make(Buffer.AsChecked<BufferType>()[Index]); \
	} \
	FVoxelFloatBuffer RefType::Impl_GetChannel( \
		const FVoxelBuffer& Buffer, \
		const EVoxelTextureChannel Channel) \
	{ \
		return GetChannel(Buffer.AsChecked<BufferType>(), Channel); \
	}