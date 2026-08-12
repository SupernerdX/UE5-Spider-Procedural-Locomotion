// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNameWrapper.h"
#include "VoxelPinType.generated.h"

struct FEdGraphPinType;
struct FPropertyBagPropertyDesc;
struct FVoxelBuffer;
struct FVoxelPinType;
struct FVoxelPinValue;
struct FVoxelRuntimeStruct;
struct FVoxelRuntimePinValue;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename>
struct TVoxelBufferTypeImpl;
template<typename>
struct TVoxelDoubleBufferTypeImpl;
template<typename>
struct TVoxelBufferInnerTypeImpl;

template<typename Type>
using TVoxelBufferType = typename TVoxelBufferTypeImpl<Type>::Type;

template<typename Type>
using TVoxelDoubleBufferType = typename TVoxelDoubleBufferTypeImpl<Type>::Type;

template<typename Type>
using TVoxelBufferInnerType = typename TVoxelBufferInnerTypeImpl<Type>::Type;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename>
constexpr bool IsVoxelBuffer = false;

template<typename>
constexpr bool IsVoxelObjectStruct = false;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename Type>
concept HasNativeVoxelBuffer = requires
{
	sizeof(TVoxelBufferType<Type>) != 0;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename>
struct TVoxelBufferTypeSafeImpl
{
	using Type = void;
};

template<typename InType>
requires (sizeof(TVoxelBufferType<InType>) != 0)
struct TVoxelBufferTypeSafeImpl<InType>
{
	using Type = TVoxelBufferType<InType>;
};

template<typename>
struct TVoxelDoubleBufferTypeSafeImpl
{
	using Type = void;
};

template<typename InType>
requires (sizeof(TVoxelDoubleBufferType<InType>) != 0)
struct TVoxelDoubleBufferTypeSafeImpl<InType>
{
	using Type = TVoxelDoubleBufferType<InType>;
};

template<typename Type>
using TVoxelBufferTypeSafe = typename TVoxelBufferTypeSafeImpl<Type>::Type;

template<typename Type>
using TVoxelDoubleBufferTypeSafe = typename TVoxelDoubleBufferTypeSafeImpl<Type>::Type;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelWildcard;
struct FVoxelWildcardBuffer;

template<>
struct TVoxelBufferTypeImpl<FVoxelWildcard>
{
	using Type = FVoxelWildcardBuffer;
};

template<>
struct TVoxelBufferInnerTypeImpl<FVoxelWildcardBuffer>
{
	using Type = FVoxelWildcard;
};

template<>
inline constexpr bool IsVoxelBuffer<FVoxelWildcardBuffer> = true;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
constexpr bool IsSafeVoxelPinType = true;

template<>
inline constexpr bool IsSafeVoxelPinType<FVoxelPinType> = false;
template<>
inline constexpr bool IsSafeVoxelPinType<FVoxelPinValue> = false;
template<>
inline constexpr bool IsSafeVoxelPinType<FVoxelRuntimeStruct> = false;
template<>
inline constexpr bool IsSafeVoxelPinType<FVoxelRuntimePinValue> = false;
template<>
inline constexpr bool IsSafeVoxelPinType<FVoxelInstancedStruct> = false;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
constexpr bool IsSafeVoxelPinValue = IsSafeVoxelPinType<T>;

template<>
inline constexpr bool IsSafeVoxelPinValue<FVoxelWildcard> = false;
template<>
inline constexpr bool IsSafeVoxelPinValue<FVoxelWildcardBuffer> = false;
template<>
inline constexpr bool IsSafeVoxelPinValue<FVoxelRuntimePinValue> = false;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UENUM()
enum class EVoxelPinInternalType : uint8
{
	Invalid,
	Wildcard,
	Bool,
	Float,
	Double,
	UInt16,
	Int32,
	Int64,
	Name,
	Byte,
	Class,
	Object,
	Struct
};

USTRUCT(BlueprintType)
struct VOXELGRAPH_API FVoxelPinType
{
	GENERATED_BODY()

private:
	UPROPERTY()
	EVoxelPinInternalType InternalType = EVoxelPinInternalType::Invalid;

	UPROPERTY()
	bool bIsBuffer = false;

	UPROPERTY()
	bool bIsBufferArray = false;

	UPROPERTY()
	TObjectPtr<UField> PrivateInternalField;

	FORCEINLINE UField* GetInternalField() const
	{
		return ResolveObjectPtrFast(PrivateInternalField);
	}

public:
	FVoxelPinType() = default;
	FVoxelPinType(const FEdGraphPinType& PinType);
	explicit FVoxelPinType(const FProperty& Property, bool bAllowBitFields = false);

	static bool IsSupported(const FProperty& Property);

public:
	template<typename T>
	FORCEINLINE static FVoxelPinType Make()
	{
		checkStatic(IsSafeVoxelPinType<T>);
		checkStatic(!std::is_same_v<T, FVoxelBuffer>);

		if constexpr (IsVoxelBuffer<T>)
		{
			return Make<TVoxelBufferInnerType<T>>().GetBufferType();
		}
		else if constexpr (std::is_same_v<T, FVoxelWildcard>)
		{
			return MakeImpl(EVoxelPinInternalType::Wildcard);
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			return MakeImpl(EVoxelPinInternalType::Bool);
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			return MakeImpl(EVoxelPinInternalType::Float);
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			return MakeImpl(EVoxelPinInternalType::Double);
		}
		else if constexpr (std::is_same_v<T, uint16>)
		{
			return MakeImpl(EVoxelPinInternalType::UInt16);
		}
		else if constexpr (std::is_same_v<T, int32>)
		{
			return MakeImpl(EVoxelPinInternalType::Int32);
		}
		else if constexpr (std::is_same_v<T, int64>)
		{
			return MakeImpl(EVoxelPinInternalType::Int64);
		}
		else if constexpr (
			std::is_same_v<T, FName> ||
			std::is_same_v<T, FVoxelNameWrapper>)
		{
			return MakeImpl(EVoxelPinInternalType::Name);
		}
		else if constexpr (std::is_same_v<T, uint8>)
		{
			return MakeImpl(EVoxelPinInternalType::Byte);
		}
		else if constexpr (std::is_enum_v<T>)
		{
			return FVoxelPinType::MakeImpl(EVoxelPinInternalType::Byte, StaticEnumFast<T>());
		}
		else if constexpr (TIsTSubclassOf<T>::Value)
		{
			return FVoxelPinType::MakeImpl(EVoxelPinInternalType::Class, StaticClassFast<typename TSubclassOfType<T>::Type>());
		}
		else if constexpr (std::derived_from<T, UObject>)
		{
			return FVoxelPinType::MakeImpl(EVoxelPinInternalType::Object, StaticClassFast<T>());
		}
		else
		{
			return FVoxelPinType::MakeImpl(EVoxelPinInternalType::Struct, StaticStructFast<T>());
		}
	}

	FORCEINLINE static FVoxelPinType MakeWildcard()
	{
		return Make<FVoxelWildcard>();
	}
	FORCEINLINE static FVoxelPinType MakeWildcardBuffer()
	{
		return MakeWildcard().GetBufferType();
	}
	FORCEINLINE static FVoxelPinType MakeEnum(UEnum* Enum)
	{
		checkVoxelSlow(Enum);
		return MakeImpl(EVoxelPinInternalType::Byte, Enum);
	}
	FORCEINLINE static FVoxelPinType MakeClass(UClass* BaseClass)
	{
		checkVoxelSlow(BaseClass);
		return MakeImpl(EVoxelPinInternalType::Class, BaseClass);
	}
	FORCEINLINE static FVoxelPinType MakeObject(UClass* Class)
	{
		checkVoxelSlow(Class);
		return MakeImpl(EVoxelPinInternalType::Object, Class);
	}
	static FVoxelPinType MakeStruct(UScriptStruct* Struct);
	static FVoxelPinType MakeFromK2(const FEdGraphPinType& PinType);

	static bool TryParse(const FString& TypeString, FVoxelPinType& OutType);

private:
	FORCEINLINE static FVoxelPinType MakeImpl(const EVoxelPinInternalType Type, UField* Field = nullptr)
	{
		FVoxelPinType PinType;
		PinType.InternalType = Type;
		PinType.PrivateInternalField = Field;
		checkVoxelSlow(PinType.IsValid());
		return PinType;
	}

public:
	FORCEINLINE bool IsValid_Fast() const
	{
		checkVoxelSlow((InternalType != EVoxelPinInternalType::Invalid) == IsValid());
		return InternalType != EVoxelPinInternalType::Invalid;
	}

	FORCEINLINE bool IsBuffer() const
	{
		checkVoxelSlow(!bIsBufferArray || bIsBuffer);
		return bIsBuffer;
	}
	FORCEINLINE bool IsBufferArray() const
	{
		checkVoxelSlow(!bIsBufferArray || bIsBuffer);
		return bIsBufferArray;
	}

	FORCEINLINE FVoxelPinType GetBufferType() const
	{
		FVoxelPinType Result = *this;
		Result.bIsBuffer = true;
		return Result;
	}
	FORCEINLINE FVoxelPinType GetInnerType() const
	{
		FVoxelPinType Result = *this;
		Result.bIsBuffer = false;
		Result.bIsBufferArray = false;
		return Result;
	}

	FORCEINLINE FVoxelPinType WithBufferArray(const bool bNewIsBufferArray) const
	{
		checkVoxelSlow(IsBuffer());
		FVoxelPinType Result = *this;
		Result.bIsBufferArray = bNewIsBufferArray;
		return Result;
	}

public:
	bool IsValid() const;
	bool IsUserDefinedEnum() const;
	bool IsUserDefinedStruct() const;
	FString ToString() const;
	FEdGraphPinType GetEdGraphPinType() const;
	FEdGraphPinType GetEdGraphPinType_K2() const;
	FPropertyBagPropertyDesc GetPropertyBagDesc() const;
	bool HasPinDefaultValue() const;
	bool CanBeCastedTo(const FVoxelPinType& Other) const;
	bool CanBeCastedTo_K2(const FVoxelPinType& Other) const;
	bool CanBeCastedTo_Schema(const FVoxelPinType& Other) const;

#if WITH_EDITOR
	bool HasBufferType_EditorOnly() const;
#endif

	void PostSerialize(const FArchive& Ar);
	void SerializeObjects(FArchive& Ar);
	void AddStructReferencedObjects(FReferenceCollector& Collector);

private:
	void Fixup();

public:
	FVoxelPinType GetExposedType() const;
	FVoxelPinType GetInnerExposedType() const;
	FVoxelPinType GetPinDefaultValueType() const;

public:
	struct FRuntimeValueContext
	{
		TVoxelObjectPtr<UObject> Owner;
	};

	static FVoxelRuntimePinValue MakeRuntimeValue(
		const FVoxelPinType& RuntimeType,
		const FVoxelPinValue& ExposedValue,
		const FRuntimeValueContext& Context);

	static FVoxelRuntimePinValue MakeRuntimeValueFromInnerValue(
		const FVoxelPinType& RuntimeType,
		const FVoxelPinValue& ExposedInnerValue,
		const FRuntimeValueContext& Context);

public:
	template<typename T>
	FORCEINLINE bool Is() const
	{
		checkStatic(IsSafeVoxelPinType<T>);
		checkStatic(!std::is_same_v<T, FVoxelBuffer>);

		if (bIsBuffer != IsVoxelBuffer<T>)
		{
			return false;
		}

		if constexpr (IsVoxelBuffer<T>)
		{
			return GetInnerType().Is<TVoxelBufferInnerType<T>>();
		}

		if constexpr (std::is_same_v<T, bool>)
		{
			return InternalType == EVoxelPinInternalType::Bool;
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			return InternalType == EVoxelPinInternalType::Float;
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			return InternalType == EVoxelPinInternalType::Double;
		}
		else if constexpr (std::is_same_v<T, uint16>)
		{
			return InternalType == EVoxelPinInternalType::UInt16;
		}
		else if constexpr (std::is_same_v<T, int32>)
		{
			return InternalType == EVoxelPinInternalType::Int32;
		}
		else if constexpr (std::is_same_v<T, int64>)
		{
			return InternalType == EVoxelPinInternalType::Int64;
		}
		else if constexpr (
			std::is_same_v<T, FName> ||
			std::is_same_v<T, FVoxelNameWrapper>)
		{
			return InternalType == EVoxelPinInternalType::Name;
		}
		else if constexpr (std::is_same_v<T, uint8>)
		{
			return InternalType == EVoxelPinInternalType::Byte;
		}
		else if constexpr (std::is_enum_v<T>)
		{
			checkVoxelSlow(StaticEnumFast<T>()->GetMaxEnumValue() <= MAX_uint8);
			return
				InternalType == EVoxelPinInternalType::Byte &&
				GetInternalField() == StaticEnumFast<T>();
		}
		else if constexpr (TIsTSubclassOf<T>::Value)
		{
			return
				InternalType == EVoxelPinInternalType::Class &&
				GetInternalField() == StaticClassFast<typename TSubclassOfType<T>::Type>();
		}
		else if constexpr (std::derived_from<T, UObject>)
		{
			return
				InternalType == EVoxelPinInternalType::Object &&
				GetInternalField() == StaticClassFast<T>();
		}
		else
		{
			return
				InternalType == EVoxelPinInternalType::Struct &&
				GetInternalField() == StaticStructFast<T>();
		}
	}

	template<typename T>
	FORCEINLINE bool CanBeCastedTo() const
	{
		if constexpr (std::is_same_v<T, FVoxelBuffer>)
		{
			return IsBuffer();
		}
		else
		{
			return this->CanBeCastedTo(Make<T>());
		}
	}

public:
	FORCEINLINE EVoxelPinInternalType GetInternalType() const
	{
		return InternalType;
	}

	FORCEINLINE bool IsWildcard() const
	{
		return InternalType == EVoxelPinInternalType::Wildcard;
	}
	FORCEINLINE bool IsClass() const
	{
		return
			!IsBuffer() &&
			InternalType == EVoxelPinInternalType::Class;
	}
	FORCEINLINE bool IsObject() const
	{
		return
			!IsBuffer() &&
			InternalType == EVoxelPinInternalType::Object;
	}
	FORCEINLINE bool IsStruct() const
	{
		return
			!IsBuffer() &&
			InternalType == EVoxelPinInternalType::Struct;
	}

	FORCEINLINE UEnum* GetEnum() const
	{
		checkVoxelSlow(!IsBuffer() && InternalType == EVoxelPinInternalType::Byte);
		return CastChecked<UEnum>(GetInternalField(), ECastCheckedType::NullAllowed);
	}
	FORCEINLINE UClass* GetBaseClass() const
	{
		checkVoxelSlow(IsClass());
		return CastChecked<UClass>(GetInternalField(), ECastCheckedType::NullAllowed);
	}
	FORCEINLINE UClass* GetObjectClass() const
	{
		checkVoxelSlow(IsObject());
		return CastChecked<UClass>(GetInternalField(), ECastCheckedType::NullAllowed);
	}
	FORCEINLINE UScriptStruct* GetStruct() const
	{
		checkVoxelSlow(IsStruct());
		return CastChecked<UScriptStruct>(GetInternalField(), ECastCheckedType::NullAllowed);
	}

public:
	FORCEINLINE bool operator==(const FVoxelPinType& Other) const
	{
		return
			InternalType == Other.InternalType &&
			bIsBuffer == Other.bIsBuffer &&
			bIsBufferArray == Other.bIsBufferArray &&
			GetInternalField() == Other.GetInternalField();
	}

	FORCEINLINE friend bool operator==(const FVoxelPinType& PinTypeA, const FEdGraphPinType& PinTypeB)
	{
		return PinTypeA == FVoxelPinType(PinTypeB);
	}
	FORCEINLINE friend bool operator==(const FEdGraphPinType& PinTypeA, const FVoxelPinType& PinTypeB)
	{
		return FVoxelPinType(PinTypeA) == PinTypeB;
	}

	FORCEINLINE friend uint32 GetTypeHash(const FVoxelPinType& Type)
	{
		return
			FVoxelUtilities::MurmurHash((int32(Type.InternalType) * 256 + Type.bIsBuffer) * 2 + Type.bIsBufferArray) ^
			GetTypeHash(Type.GetInternalField());
	}
};
checkStatic(sizeof(FVoxelPinType) == 16);

template<>
struct TStructOpsTypeTraits<FVoxelPinType> : public TStructOpsTypeTraitsBase2<FVoxelPinType>
{
	enum
	{
		WithPostSerialize = true,
	};
};