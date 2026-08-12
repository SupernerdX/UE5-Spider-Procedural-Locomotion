// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValueBase.h"
#include "VoxelPinValue.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelTerminalPinValue final : public FVoxelPinValueBase
{
	GENERATED_BODY()

public:
	FVoxelTerminalPinValue() = default;
	explicit FVoxelTerminalPinValue(const FVoxelPinType& Type);

private:
	explicit FVoxelTerminalPinValue(const FVoxelPinValueBase& Value)
		: FVoxelPinValueBase((Value))
	{
	}
	explicit FVoxelTerminalPinValue(FVoxelPinValueBase&& Value)
		: FVoxelPinValueBase((MoveTemp(Value)))
	{
	}

public:
	FVoxelPinValue ToValue() const;
	// Used to copy data to stamp refs in-place, avoiding refs to older data in graph parameter details
	void CopyFrom(const FVoxelTerminalPinValue& Other);

	FORCEINLINE TVoxelArrayView<uint8> GetRawView()
	{
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
		case EVoxelPinInternalType::Object: return MakeByteVoxelArrayView(Object);
		case EVoxelPinInternalType::Struct: return Struct.GetStructView();
		}
	}
	FORCEINLINE TConstVoxelArrayView<uint8> GetRawView() const
	{
		return ConstCast(this)->GetRawView();
	}

public:
	bool operator==(const FVoxelTerminalPinValue& Other) const
	{
		return Super::operator==(Other);
	}

	friend struct FVoxelPinValue;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType, meta = (DisableSplitPin, HasNativeMake = "/Script/VoxelGraph.VoxelPinValueBlueprintLibrary.K2_MakeVoxelPinValue", HasNativeBreak = "/Script/VoxelGraph.VoxelPinValueBlueprintLibrary.K2_BreakVoxelPinValue"))
struct VOXELGRAPH_API FVoxelPinValue final : public FVoxelPinValueBase
{
	GENERATED_BODY()

public:
	FVoxelPinValue() = default;
	explicit FVoxelPinValue(const FVoxelPinType& Type);

public:
	template<typename T>
	requires
	(
		IsSafeVoxelPinValue<T> &&
		!IsVoxelBuffer<T> &&
		!std::derived_from<T, UObject>
	)
	static FVoxelPinValue Make(const T& Value)
	{
		FVoxelPinValue Result(FVoxelPinType::Make<T>());
		Result.Get<T>() = Value;
		return Result;
	}
	template<typename T>
	requires std::derived_from<T, UObject>
	static FVoxelPinValue Make(T* Value)
	{
		FVoxelPinValue Result(FVoxelPinType::Make<T>());
		Result.Get<T>() = Value;
		return Result;
	}

public:
	static FVoxelPinValue MakeFromPinDefaultValue(const UEdGraphPin& Pin);
	static FVoxelPinValue MakeFromK2PinDefaultValue(const UEdGraphPin& Pin);

	static FVoxelPinValue MakeStruct(FConstVoxelStructView Struct);
	static FVoxelPinValue MakeFromProperty(const FProperty& Property, const void* Memory);

	using FVoxelPinValueBase::Fixup;
	void Fixup(const FVoxelPinType& NewType);
	// Used to copy data to stamp refs in-place, avoiding refs to older data in graph parameter details
	void CopyFrom(const FVoxelPinValue& Other);
	bool ImportFromUnrelated(FVoxelPinValue Other);
	bool Serialize(FArchive& Ar);
	void PostSerialize(const FArchive& Ar);

	FVoxelTerminalPinValue& AsTerminalValue();
	const FVoxelTerminalPinValue& AsTerminalValue() const;

public:
	FORCEINLINE bool IsArray() const
	{
		ensureVoxelSlow(!Type.IsBuffer() || Type.IsBufferArray());
		return Type.IsBuffer();
	}
	FORCEINLINE FVoxelPinValue& WithType(const FVoxelPinType& OtherType)
	{
		checkVoxelSlow(Type.IsBuffer() == OtherType.IsBuffer());

		if (Type.GetInnerType().Is<uint8>())
		{
			checkVoxelSlow(OtherType.GetInnerType().Is<uint8>());
		}
		else if (Type.GetInnerType().IsClass())
		{
			checkVoxelSlow(OtherType.GetInnerType().IsClass());
		}
		else
		{
			checkVoxelSlow(Type == OtherType);
		}

		Type = OtherType;
		return *this;
	}

	FORCEINLINE const TArray<FVoxelTerminalPinValue>& GetArray() const
	{
		checkVoxelSlow(IsArray());
		return Array;
	}
	FORCEINLINE void AddValue(const FVoxelTerminalPinValue& InnerValue)
	{
		checkVoxelSlow(IsArray());
		checkVoxelSlow(Type.GetInnerType() == InnerValue.GetType());
		Array.Add(InnerValue);
	}

private:
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FVoxelTerminalPinValue> Array;

	UPROPERTY()
	FName EnumValueName;

	explicit FVoxelPinValue(const FVoxelPinValueBase& Value)
		: FVoxelPinValueBase((Value))
	{
	}
	explicit FVoxelPinValue(FVoxelPinValueBase&& Value)
		: FVoxelPinValueBase((MoveTemp(Value)))
	{
	}

	void InitializeFromPinDefaultValue(const UEdGraphPin& Pin);

	virtual bool HasArray() const override {return true;}
	virtual FString ExportToString_Array() const override;
	virtual void ExportToProperty_Array(const FProperty& Property, void* Memory) const override;
	virtual bool ImportFromString_Array(const FString& Value) override;
	virtual void Fixup_Array() override;
	virtual bool Equal_Array(const FVoxelPinValueBase& Other) const override;

public:
	bool operator==(const FVoxelPinValue& Other) const
	{
		return Super::operator==(Other);
	}

	friend FVoxelTerminalPinValue;
	friend class FVoxelParameterDetails;
	friend class SVoxelPropertyEditorArray;
	friend class FVoxelPinValueCustomization;
	friend class FVoxelPinValueCustomizationHelper;
};

template<>
struct TStructOpsTypeTraits<FVoxelPinValue> : public TStructOpsTypeTraitsBase2<FVoxelPinValue>
{
	enum
	{
		WithSerializer = true,
		WithPostSerialize = true,
	};
};