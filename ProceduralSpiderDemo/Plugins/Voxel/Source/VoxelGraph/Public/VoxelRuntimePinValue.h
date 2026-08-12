// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValue.h"
#include "VoxelRuntimePinValue.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelRuntimePinValue
{
	GENERATED_BODY()

private:
	FVoxelPinType Type;

	union
	{
		bool bBool;
		uint8 Byte;
		float Float;
		double Double;
		uint16 UInt16;
		int32 Int32;
		int64 Int64;
		FVoxelNameWrapper Name;
		UClass* Class;
		UScriptStruct* SharedStructType;
		uint64 Raw;
	};
	FSharedVoidPtr SharedStruct;

	void InitializeImpl();

	friend class UVoxelFunctionLibrary;

public:
	FORCEINLINE FVoxelRuntimePinValue()
	{
		checkStatic(sizeof(FVoxelRuntimePinValue) == 40);
		Raw = 0;
	}
	FORCEINLINE explicit FVoxelRuntimePinValue(const FVoxelPinType& Type)
		: Type(Type)
	{
		Raw = 0;

		checkVoxelSlow(!Type.IsWildcard());
		checkVoxelSlow(!Type.GetInnerType().IsObject());

		if (Type.IsBuffer() ||
			Type.GetInternalType() == EVoxelPinInternalType::Struct)
		{
			InitializeImpl();
		}
	}

	FORCEINLINE void SetType(const FVoxelPinType& OtherType)
	{
		checkVoxelSlow(Type.IsBuffer() == OtherType.IsBuffer());

		const FVoxelPinType InnerTypeA = GetType().GetInnerType();
		const FVoxelPinType InnerTypeB = OtherType.GetInnerType();

		if (InnerTypeA.Is<uint8>())
		{
			checkVoxelSlow(InnerTypeB.Is<uint8>());
		}
		else if (InnerTypeA.IsClass())
		{
			checkVoxelSlow(InnerTypeB.IsClass());
		}
		else
		{
			checkVoxelSlow(InnerTypeA == InnerTypeB);
		}

		Type = OtherType;
	}

public:
	static FVoxelRuntimePinValue MakeStruct(FConstVoxelStructView Struct);

	static FVoxelRuntimePinValue MakeRuntimeStruct(
		FConstVoxelStructView UserStruct,
		const FVoxelPinType::FRuntimeValueContext& Context);

	static FVoxelRuntimePinValue MakeRuntimeStruct(const TSharedRef<FVoxelRuntimeStruct>& Struct);

public:
	template<typename T>
	requires
	(
		IsSafeVoxelPinValue<T> &&
		std::is_copy_constructible_v<T> &&
		std::is_trivially_destructible_v<T> &&
		!std::is_same_v<T, FVoxelBuffer>
	)
	FORCEINLINE static FVoxelRuntimePinValue Make(const T& Value)
	{
		FVoxelRuntimePinValue Result;
		Result.Type = FVoxelPinType::Make<T>();

		if constexpr (std::is_same_v<T, bool>)
		{
			Result.bBool = Value;
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			Result.Float = Value;
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			Result.Double = Value;
		}
		else if constexpr (std::is_same_v<T, uint16>)
		{
			Result.UInt16 = Value;
		}
		else if constexpr (std::is_same_v<T, int32>)
		{
			Result.Int32 = Value;
		}
		else if constexpr (std::is_same_v<T, int64>)
		{
			Result.Int64 = Value;
		}
		else if constexpr (
			std::is_same_v<T, FName> ||
			std::is_same_v<T, FVoxelNameWrapper>)
		{
			Result.Name = Value;
		}
		else if constexpr (std::is_same_v<T, uint8>)
		{
			Result.Byte = Value;
		}
		else if constexpr (std::is_enum_v<T>)
		{
			Result.Byte = uint8(Value);
		}
		else if constexpr (TIsTSubclassOf<T>::Value)
		{
			Result.Class = Value;
		}
		else
		{
			const FConstVoxelStructView StructView = FConstVoxelStructView::Make(Value);
			Result.SharedStructType = StructView.GetScriptStruct();
			Result.SharedStruct = StructView.MakeSharedCopy();
		}

		return Result;
	}

	template<typename T>
	FORCEINLINE static FVoxelRuntimePinValue Make(const TSharedRef<const T>& Value)
	{
		checkStatic(IsSafeVoxelPinValue<T>);

		UScriptStruct* Struct = FConstVoxelStructView::Make(*Value).GetScriptStruct();

		FVoxelRuntimePinValue Result;
		if constexpr (std::derived_from<T, FVoxelBuffer>)
		{
			Result.Type = Value->GetBufferType();
			Struct = Value->GetStruct();
		}
		else
		{
			Result.Type = FVoxelPinType::MakeStruct(Struct);
		}
		Result.SharedStructType = Struct;
		Result.SharedStruct = MakeSharedVoidRef(Value);
		return Result;
	}

	template<typename T>
	FORCEINLINE static FVoxelRuntimePinValue Make(const TSharedRef<T>& Value)
	{
		return FVoxelRuntimePinValue::Make<T>(ReinterpretCastRef<TSharedRef<const T>>(Value));
	}

	static FVoxelRuntimePinValue Make(FVoxelBuffer&& Buffer);

public:
	template<typename T>
	FORCEINLINE const TSharedRef<const T>& GetSharedStruct() const
	{
		checkStatic(IsStructValue<T>);
		checkVoxelSlow(CanBeCastedTo<T>());
		checkVoxelSlow(Type.IsBuffer() || Type.IsStruct());
		checkVoxelSlow(SharedStructType);
		checkVoxelSlow(SharedStruct);

		return ReinterpretCastRef<TSharedRef<const T>>(SharedStruct);
	}
	FORCEINLINE FConstVoxelStructView GetStructView() const
	{
		checkVoxelSlow(Type.IsBuffer() || Type.IsStruct());
		checkVoxelSlow(SharedStructType);
		checkVoxelSlow(SharedStruct);
		checkVoxelSlow(
			!SharedStructType->IsChildOf(StaticStructFast<FVoxelVirtualStruct>()) ||
			reinterpret_cast<const FVoxelVirtualStruct&>(*SharedStruct).GetStruct() == SharedStructType);
		return FConstVoxelStructView(SharedStructType, SharedStruct.Get());
	}

public:
	FORCEINLINE bool IsValid() const
	{
		return Type.IsValid_Fast();
	}
	bool IsValidValue_Slow() const;

public:
	FORCEINLINE const FVoxelPinType& GetType() const
	{
		return Type;
	}
	FORCEINLINE bool IsBuffer() const
	{
		return Type.IsBuffer();
	}
	FORCEINLINE bool IsStruct() const
	{
		return Type.IsStruct();
	}
	template<typename T>
	FORCEINLINE bool Is() const
	{
		return Type.Is<T>();
	}
	template<typename T>
	FORCEINLINE bool CanBeCastedTo() const
	{
		return Type.CanBeCastedTo<T>();
	}
	FORCEINLINE bool CanBeCastedTo(const FVoxelPinType& Other) const
	{
		return Type.CanBeCastedTo(Other);
	}
	FString ToDebugString(bool bFullValue, bool bParseBaseStructs) const;
	void ExportToProperty(const FProperty& Property, void* Memory) const;

public:
	template<typename T>
	static constexpr bool IsStructValue =
		!std::is_same_v<T, FVoxelNameWrapper> &&
		std::is_same_v<FVoxelUtilities::TPropertyType<T>, FStructProperty>;

	template<typename T>
	requires (!std::is_same_v<T, FName>)
	FORCEINLINE const T& Get() const
	{
		checkStatic(IsSafeVoxelPinValue<T>);
		checkVoxelSlow(Type.CanBeCastedTo<T>());

		if constexpr (IsStructValue<T>)
		{
			checkVoxelSlow(SharedStructType);
			checkVoxelSlow(SharedStruct.IsValid());
			return reinterpret_cast<const T&>(*SharedStruct.Get());
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			return bBool;
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			return Float;
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			return Double;
		}
		else if constexpr (std::is_same_v<T, uint16>)
		{
			return UInt16;
		}
		else if constexpr (std::is_same_v<T, int32>)
		{
			return Int32;
		}
		else if constexpr (std::is_same_v<T, int64>)
		{
			return Int64;
		}
		else if constexpr (std::is_same_v<T, FVoxelNameWrapper>)
		{
			return Name;
		}
		else if constexpr (std::is_same_v<T, uint8>)
		{
			return Byte;
		}
		else if constexpr (std::is_enum_v<T>)
		{
			return ReinterpretCastRef<T>(Byte);
		}
		else if constexpr (TIsTSubclassOf<T>::Value)
		{
			return ReinterpretCastRef<T>(Class);
		}
		else
		{
			checkStatic(std::is_void_v<T>);
			check(false);
			static T* Value = new T();
			return *Value;
		}
	}
	template<typename T>
	requires std::is_same_v<T, FName>
	FORCEINLINE const T Get() const
	{
		return Get<FVoxelNameWrapper>();
	}

	FORCEINLINE TConstVoxelArrayView<uint8> GetRawView() const
	{
		checkVoxelSlow(IsValid());
		checkVoxelSlow(!Type.IsBuffer());

		switch (Type.GetInternalType())
		{
		default: VOXEL_ASSUME(false);
		case EVoxelPinInternalType::Bool: return MakeByteVoxelArrayView(bBool);
		case EVoxelPinInternalType::Float: return MakeByteVoxelArrayView(Float);
		case EVoxelPinInternalType::Double: return MakeByteVoxelArrayView(Double);
		case EVoxelPinInternalType::UInt16: return MakeByteVoxelArrayView(UInt16);
		case EVoxelPinInternalType::Int32: return MakeByteVoxelArrayView(Int32);
		case EVoxelPinInternalType::Int64: return MakeByteVoxelArrayView(Int64);
		case EVoxelPinInternalType::Name: return MakeByteVoxelArrayView(Name);
		case EVoxelPinInternalType::Byte: return MakeByteVoxelArrayView(Byte);
		case EVoxelPinInternalType::Class: return MakeByteVoxelArrayView(Class);
		case EVoxelPinInternalType::Struct: return GetStructView().GetRawView();
		}
	}
};

template<>
struct TIsZeroConstructType<FVoxelRuntimePinValue>
{
	static const bool Value = true;
};