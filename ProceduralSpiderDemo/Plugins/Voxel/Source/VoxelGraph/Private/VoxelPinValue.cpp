// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPinValue.h"
#include "VoxelPinValueOps.h"
#include "EdGraph/EdGraphPin.h"

FVoxelTerminalPinValue::FVoxelTerminalPinValue(const FVoxelPinType& Type)
	: FVoxelPinValueBase(Type)
{
	ensure(!Type.IsBuffer());
	Fixup();
}

FVoxelPinValue FVoxelTerminalPinValue::ToValue() const
{
	return FVoxelPinValue(FVoxelPinValueBase(*this));
}

void FVoxelTerminalPinValue::CopyFrom(const FVoxelTerminalPinValue& Other)
{
	if (Type != Other.Type)
	{
		*this = Other;
		return;
	}

	if (const TSharedPtr<FVoxelPinValueOps> Ops = FVoxelPinValueOps::Find(GetType(), EVoxelPinValueOpsUsage::CopyFrom))
	{
		Ops->CopyFrom(*this, Other);
		return;
	}

	*this = Other;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPinValue::FVoxelPinValue(const FVoxelPinType& Type)
	: FVoxelPinValueBase(Type)
{
	// Exposed buffers should be arrays
	ensure(!Type.IsBuffer() || Type.IsBufferArray());
	Fixup();
}

FVoxelPinValue FVoxelPinValue::MakeFromPinDefaultValue(const UEdGraphPin& Pin)
{
	const FVoxelPinType Type = FVoxelPinType(Pin.PinType).GetPinDefaultValueType();
	if (!ensureVoxelSlow(Type.IsValid()) ||
		!ensureVoxelSlow(!Type.IsBuffer()))
	{
		return {};
	}

	FVoxelPinValue Result(Type);
	Result.InitializeFromPinDefaultValue(Pin);
	return Result;
}

FVoxelPinValue FVoxelPinValue::MakeFromK2PinDefaultValue(const UEdGraphPin & Pin)
{
	const FVoxelPinType Type = FVoxelPinType::MakeFromK2(Pin.PinType);
	if (!ensure(Type.IsValid()) ||
		!ensure(!Type.IsBuffer()))
	{
		return {};
	}

	FVoxelPinValue Result(Type);
	Result.InitializeFromPinDefaultValue(Pin);
	return Result;
}

FVoxelPinValue FVoxelPinValue::MakeStruct(const FConstVoxelStructView Struct)
{
	return FVoxelPinValue(Super::MakeStruct(Struct));
}

FVoxelPinValue FVoxelPinValue::MakeFromProperty(const FProperty& Property, const void* Memory)
{
	const FVoxelPinType Type(Property);
	if (Type.IsBuffer())
	{
		const FArrayProperty& ArrayProperty = CastFieldChecked<FArrayProperty>(Property);
		FScriptArrayHelper ArrayHelper(&ArrayProperty, Memory);

		FVoxelPinValue Result(Type.WithBufferArray(true));
		for (int32 Index = 0; Index < ArrayHelper.Num(); Index++)
		{
			Result.Array.Add(FVoxelTerminalPinValue(FVoxelPinValueBase::MakeFromProperty(
				*ArrayProperty.Inner,
				ArrayHelper.GetRawPtr(Index))));
		}
		return Result;
	}

	return FVoxelPinValue(Super::MakeFromProperty(Property, Memory));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPinValue::Fixup(const FVoxelPinType& NewType)
{
	if (IsValid())
	{
		if (!CanBeCastedTo(NewType))
		{
			if (!NewType.IsBuffer() &&
				!Type.IsBuffer())
			{
				const FVoxelPinValue OldValue = *this;
				*this = FVoxelPinValue(NewType);
				ImportFromUnrelated(OldValue);
			}
			else
			{
				*this = FVoxelPinValue(NewType);
			}
		}
	}
	else
	{
		*this = FVoxelPinValue(NewType);
	}

	checkVoxelSlow(Type == NewType || Type.CanBeCastedTo(NewType));
	Type = NewType;

	Fixup();
}

void FVoxelPinValue::CopyFrom(const FVoxelPinValue& Other)
{
	if (Type != Other.Type)
	{
		*this = Other;
		return;
	}

	if (!IsArray())
	{
		AsTerminalValue().CopyFrom(Other.AsTerminalValue());
		return;
	}

	Array.SetNum(Other.Array.Num());

	for (int32 Index = 0; Index < Array.Num(); Index++)
	{
		Array[Index].CopyFrom(Other.Array[Index]);
	}
}

bool FVoxelPinValue::ImportFromUnrelated(FVoxelPinValue Other)
{
	VOXEL_FUNCTION_COUNTER();

	if (GetType() == Other.GetType())
	{
		*this = Other;
		return true;
	}

	if (Other.Is<FColor>())
	{
		Other = Make(FLinearColor(
			Other.Get<FColor>().R,
			Other.Get<FColor>().G,
			Other.Get<FColor>().B,
			Other.Get<FColor>().A));
	}
	if (Other.Is<FQuat>())
	{
		Other = Make(Other.Get<FQuat>().Rotator());
	}
	if (Other.Is<FRotator>())
	{
		Other = Make(FVector(
			Other.Get<FRotator>().Pitch,
			Other.Get<FRotator>().Yaw,
			Other.Get<FRotator>().Roll));
	}
	if (Other.Is<int32>())
	{
		Other = Make<float>(Other.Get<int32>());
	}
	if (Other.Is<FIntPoint>())
	{
		Other = Make(FVector2D(
			Other.Get<FIntPoint>().X,
			Other.Get<FIntPoint>().Y));
	}
	if (Other.Is<FIntVector>())
	{
		Other = Make(FVector(
			Other.Get<FIntVector>().X,
			Other.Get<FIntVector>().Y,
			Other.Get<FIntVector>().Z));
	}

#define CHECK(NewType, OldType, ...) \
	if (Is<NewType>() && Other.Is<OldType>()) \
	{ \
		const OldType Value = Other.Get<OldType>(); \
		Get<NewType>() = __VA_ARGS__; \
		return true; \
	}

	CHECK(float, FVector2D, Value.X);
	CHECK(float, FVector, Value.X);
	CHECK(float, FLinearColor, Value.R);

	CHECK(FVector2D, float, FVector2D(Value));
	CHECK(FVector2D, FVector, FVector2D(Value));
	CHECK(FVector2D, FLinearColor, FVector2D(Value));

	CHECK(FVector, float, FVector(Value));
	CHECK(FVector, FVector2D, FVector(Value, 0.f));
	CHECK(FVector, FLinearColor, FVector(Value));

	CHECK(FLinearColor, float, FLinearColor(Value, Value, Value, Value));
	CHECK(FLinearColor, FVector2D, FLinearColor(Value.X, Value.Y, 0.f));
	CHECK(FLinearColor, FVector, FLinearColor(Value));

	CHECK(int32, float, Value);
	CHECK(int32, FVector2D, Value.X);
	CHECK(int32, FVector, Value.X);
	CHECK(int32, FLinearColor, Value.R);

	CHECK(FIntPoint, float, FIntPoint(Value));
	CHECK(FIntPoint, FVector2D, FIntPoint(Value.X, Value.Y));
	CHECK(FIntPoint, FVector, FIntPoint(Value.X, Value.Y));
	CHECK(FIntPoint, FLinearColor, FIntPoint(Value.R, Value.G));

	CHECK(FIntVector, float, FIntVector(Value));
	CHECK(FIntVector, FVector2D, FIntVector(Value.X, Value.Y, 0));
	CHECK(FIntVector, FVector, FIntVector(Value.X, Value.Y, Value.Z));
	CHECK(FIntVector, FLinearColor, FIntVector(Value.R, Value.G, Value.B));

#undef CHECK

	if (const TSharedPtr<FVoxelPinValueOps> Ops = FVoxelPinValueOps::Find(GetType(), EVoxelPinValueOpsUsage::ImportFromUnrelated))
	{
		if (Ops->ImportFromUnrelated(*this, Other))
		{
			return true;
		}
	}

	return ImportFromString(Other.ExportToString());
}

bool FVoxelPinValue::Serialize(FArchive& Ar)
{
	SerializeVoxelVersion(Ar);

	INLINE_LAMBDA
	{
		if (!Ar.IsSaving())
		{
			return;
		}

		EnumValueName = {};

		if (!Type.Is<uint8>())
		{
			return;
		}

		const UEnum* Enum = Type.GetEnum();
		if (!Enum ||
			!ensure(Enum->IsValidEnumValue(Byte)))
		{
			return;
		}

		EnumValueName = Enum->GetNameByValue(Byte);
	};

	// Returning false to fallback to default serialization
	return false;
}

void FVoxelPinValue::PostSerialize(const FArchive& Ar)
{
	if (!Ar.IsLoading() ||
		EnumValueName.IsNone() ||
		!ensure(Type.Is<uint8>()))
	{
		return;
	}

	const UEnum* Enum = Type.GetEnum();
	if (!Enum)
	{
		return;
	}

	const int64 EnumValue = Enum->GetValueByName(EnumValueName);
	if (!ensure(FVoxelUtilities::IsValidUINT8(EnumValue)))
	{
		return;
	}

	Byte = EnumValue;
}

FVoxelTerminalPinValue& FVoxelPinValue::AsTerminalValue()
{
	ensure(!IsArray());
	return ReinterpretCastRef<FVoxelTerminalPinValue>(static_cast<FVoxelPinValueBase&>(*this));
}

const FVoxelTerminalPinValue& FVoxelPinValue::AsTerminalValue() const
{
	ensure(!IsArray());
	return ReinterpretCastRef<FVoxelTerminalPinValue>(static_cast<const FVoxelPinValueBase&>(*this));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPinValue::InitializeFromPinDefaultValue(const UEdGraphPin& Pin)
{
	if (UObject* DefaultObject = Pin.DefaultObject)
	{
		ensure(Pin.DefaultValue.IsEmpty());

		if (IsClass())
		{
			UClass* NewClass = Cast<UClass>(DefaultObject);
			if (ensureVoxelSlow(NewClass) &&
				ensureVoxelSlow(NewClass->IsChildOf(Type.GetBaseClass())))
			{
				GetClass() = NewClass;
			}
		}
		else if (IsObject())
		{
			if (ensureVoxelSlow(DefaultObject->IsA(Type.GetObjectClass())))
			{
				GetObject() = DefaultObject;
			}
		}
		else
		{
			ensure(false);
		}
	}
	else if (!Pin.DefaultValue.IsEmpty())
	{
		ensure(!IsObject());
		ensure(ImportFromString(Pin.DefaultValue));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FString FVoxelPinValue::ExportToString_Array() const
{
	check(IsArray());
	ensure(false);
	return {};
}

void FVoxelPinValue::ExportToProperty_Array(const FProperty& Property, void* Memory) const
{
	check(IsArray());

	if (!ensure(Property.IsA<FArrayProperty>()))
	{
		return;
	}

	const FArrayProperty& ArrayProperty = CastFieldChecked<FArrayProperty>(Property);

	FScriptArrayHelper ArrayHelper(&ArrayProperty, Memory);
	ArrayHelper.Resize(Array.Num());
	for (int32 Index = 0; Index < Array.Num(); Index++)
	{
		Array[Index].ExportToProperty(*ArrayProperty.Inner, ArrayHelper.GetRawPtr(Index));
	}
}

bool FVoxelPinValue::ImportFromString_Array(const FString& Value)
{
	check(IsArray());
	ensure(false);
	return false;
}

void FVoxelPinValue::Fixup_Array()
{
	for (FVoxelTerminalPinValue& Value : Array)
	{
		if (!Value.IsValid() ||
			!Value.CanBeCastedTo(Type.GetInnerType()))
		{
			Value = FVoxelTerminalPinValue(Type.GetInnerType());
		}
		Value.Fixup();
	}
}

bool FVoxelPinValue::Equal_Array(const FVoxelPinValueBase& Other) const
{
	check(IsArray());
	const FVoxelPinValue& OtherValue = static_cast<const FVoxelPinValue&>(Other);

	check(OtherValue.IsArray());
	return Array == OtherValue.Array;
}