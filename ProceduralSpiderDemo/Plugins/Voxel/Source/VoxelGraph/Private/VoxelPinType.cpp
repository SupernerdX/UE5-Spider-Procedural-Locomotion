// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPinType.h"
#include "VoxelPinValueOps.h"
#include "VoxelExposedSeed.h"
#include "VoxelObjectPinType.h"
#include "Buffer/VoxelBaseBuffers.h"

#include "EdGraph/EdGraphPin.h"
#include "UObject/CoreRedirects.h"
#include "Engine/UserDefinedEnum.h"
#include "StructUtils/PropertyBag.h"
#include "StructUtils/UserDefinedStruct.h"

// Keep around to be able to load old packages
constexpr FVoxelGuid GVoxelPinTypeCustomVersionGUID = VOXEL_GUID("FEDF91A24CD4A616E6623F39B74053B3");
FCustomVersionRegistration GRegisterVoxelPinTypeCustomVersionGUID(GVoxelPinTypeCustomVersionGUID, 1, TEXT("VoxelPinTypeVer"));

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPinType::FVoxelPinType(const FEdGraphPinType& PinType)
{
	if (PinType == FEdGraphPinType())
	{
		// Empty type
		return;
	}

	const int64 Value = StaticEnumFast<EVoxelPinInternalType>()->GetValueByNameString(PinType.PinCategory.ToString());
	if (ensureVoxelSlow(Value != -1))
	{
		ensure(
			!PinType.PinSubCategoryObject.IsValid() ||
			PinType.PinSubCategoryObject->IsA<UField>());

		InternalType = EVoxelPinInternalType(Value);
		bIsBuffer = PinType.IsArray();
		bIsBufferArray = PinType.PinSubCategory == STATIC_FNAME("Array") && ensure(bIsBuffer);
		PrivateInternalField = Cast<UField>(PinType.PinSubCategoryObject.Get());
	}

	Fixup();

	// Raise ensures
	(void)IsValid();
}

FVoxelPinType::FVoxelPinType(const FProperty& Property, const bool bAllowBitFields)
{
	if (Property.IsA<FArrayProperty>())
	{
		*this = FVoxelPinType(*CastFieldChecked<FArrayProperty>(Property).Inner);
		bIsBuffer = true;
		// If it's from a property, it's likely from BP and thus an array
		bIsBufferArray = true;
	}
	else if (Property.IsA<FBoolProperty>())
	{
		InternalType = EVoxelPinInternalType::Bool;
		ensure(bAllowBitFields || CastFieldChecked<FBoolProperty>(Property).IsNativeBool());
	}
	else if (Property.IsA<FFloatProperty>())
	{
		InternalType = EVoxelPinInternalType::Float;
	}
	else if (Property.IsA<FDoubleProperty>())
	{
		InternalType = EVoxelPinInternalType::Double;
	}
	else if (Property.IsA<FUInt16Property>())
	{
		InternalType = EVoxelPinInternalType::UInt16;
	}
	else if (Property.IsA<FIntProperty>())
	{
		InternalType = EVoxelPinInternalType::Int32;
	}
	else if (Property.IsA<FInt64Property>())
	{
		InternalType = EVoxelPinInternalType::Int64;
	}
	else if (Property.IsA<FNameProperty>())
	{
		InternalType = EVoxelPinInternalType::Name;
	}
	else if (Property.IsA<FByteProperty>())
	{
		InternalType = EVoxelPinInternalType::Byte;
		PrivateInternalField = CastFieldChecked<FByteProperty>(Property).Enum;
	}
	else if (Property.IsA<FEnumProperty>())
	{
		InternalType = EVoxelPinInternalType::Byte;
		PrivateInternalField = CastFieldChecked<FEnumProperty>(Property).GetEnum();
	}
	else if (Property.IsA<FClassProperty>())
	{
		InternalType = EVoxelPinInternalType::Class;
		PrivateInternalField = CastFieldChecked<FClassProperty>(Property).MetaClass;
	}
	else if (Property.IsA<FSoftClassProperty>())
	{
		InternalType = EVoxelPinInternalType::Class;
		PrivateInternalField = CastFieldChecked<FSoftClassProperty>(Property).MetaClass;
	}
	else if (Property.IsA<FObjectProperty>())
	{
		InternalType = EVoxelPinInternalType::Object;
		PrivateInternalField = CastFieldChecked<FObjectProperty>(Property).PropertyClass;
	}
	else if (Property.IsA<FSoftObjectProperty>())
	{
		InternalType = EVoxelPinInternalType::Object;
		PrivateInternalField = CastFieldChecked<FSoftObjectProperty>(Property).PropertyClass;
	}
	else if (Property.IsA<FStructProperty>())
	{
		*this = MakeStruct(CastFieldChecked<FStructProperty>(Property).Struct);
	}
	else
	{
		ensure(false);
	}

	ensure(IsValid());
}

bool FVoxelPinType::IsSupported(const FProperty& Property)
{
	return
		Property.IsA<FArrayProperty>() ||
		Property.IsA<FBoolProperty>() ||
		Property.IsA<FFloatProperty>() ||
		Property.IsA<FDoubleProperty>() ||
		Property.IsA<FUInt16Property>() ||
		Property.IsA<FIntProperty>() ||
		Property.IsA<FInt64Property>() ||
		Property.IsA<FNameProperty>() ||
		Property.IsA<FByteProperty>() ||
		Property.IsA<FEnumProperty>() ||
		Property.IsA<FClassProperty>() ||
		Property.IsA<FSoftClassProperty>() ||
		Property.IsA<FObjectProperty>() ||
		Property.IsA<FSoftObjectProperty>() ||
		Property.IsA<FStructProperty>();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPinType FVoxelPinType::MakeStruct(UScriptStruct* Struct)
{
	checkVoxelSlow(Struct);
	checkVoxelSlow(Struct != StaticStructFast<FVoxelPinType>());
	checkVoxelSlow(Struct != StaticStructFast<FVoxelPinValue>());
	checkVoxelSlow(Struct != StaticStructFast<FVoxelNameWrapper>());
	checkVoxelSlow(Struct != StaticStructFast<FVoxelRuntimePinValue>());
	checkVoxelSlow(!Struct->IsChildOf(StaticStructFast<FVoxelBuffer>()));

	return MakeImpl(EVoxelPinInternalType::Struct, Struct);
}

FVoxelPinType FVoxelPinType::MakeFromK2(const FEdGraphPinType& PinType)
{
	// From UEdGraphSchema_K2

	FVoxelPinType Type = INLINE_LAMBDA -> FVoxelPinType
	{
		if (PinType.PinCategory == STATIC_FNAME("wildcard"))
		{
			return MakeWildcard();
		}
		else if (PinType.PinCategory == STATIC_FNAME("bool"))
		{
			return Make<bool>();
		}
		else if (PinType.PinCategory == STATIC_FNAME("real"))
		{
			if (PinType.PinSubCategory == STATIC_FNAME("float"))
			{
				return Make<float>();
			}
			else
			{
				ensure(PinType.PinSubCategory == STATIC_FNAME("double"));
				return Make<double>();
			}
		}
		else if (PinType.PinCategory == STATIC_FNAME("int"))
		{
			return Make<int32>();
		}
		else if (PinType.PinCategory == STATIC_FNAME("int64"))
		{
			return Make<int64>();
		}
		else if (PinType.PinCategory == STATIC_FNAME("name"))
		{
			return Make<FName>();
		}
		else if (PinType.PinCategory == STATIC_FNAME("byte"))
		{
			if (UEnum* EnumType = Cast<UEnum>(PinType.PinSubCategoryObject.Get()))
			{
				return MakeEnum(EnumType);
			}
			else
			{
				return Make<uint8>();
			}
		}
		else if (PinType.PinCategory == STATIC_FNAME("class"))
		{
			if (UClass* ClassType = Cast<UClass>(PinType.PinSubCategoryObject.Get()))
			{
				return MakeClass(ClassType);
			}
			else
			{
				return {};
			}
		}
		else if (
			PinType.PinCategory == STATIC_FNAME("object") ||
			PinType.PinCategory == STATIC_FNAME("interface"))
		{
			if (UClass* ObjectType = Cast<UClass>(PinType.PinSubCategoryObject.Get()))
			{
				return MakeObject(ObjectType);
			}
			else
			{
				return {};
			}
		}
		else if (PinType.PinCategory == STATIC_FNAME("struct"))
		{
			if (UScriptStruct* StructType = Cast<UScriptStruct>(PinType.PinSubCategoryObject.Get()))
			{
				return MakeStruct(StructType);
			}
			else
			{
				return {};
			}
		}
		else
		{
			return {};
		}
	};

	Type.bIsBuffer = PinType.IsArray();
	return Type;
}

bool FVoxelPinType::TryParse(const FString& TypeString, FVoxelPinType& OutType)
{
	VOXEL_SCOPE_COUNTER("FindObject");

	// Serializing structs directly doesn't seem to handle redirects properly
	const FCoreRedirectObjectName RedirectedName = FCoreRedirects::GetRedirectedName(
		ECoreRedirectFlags::Type_Struct,
		FCoreRedirectObjectName(TypeString));

	UScriptStruct* Struct = FindObject<UScriptStruct>(nullptr, *RedirectedName.ToString());
	if (!ensure(Struct))
	{
		return false;
	}

	OutType = MakeStruct(Struct);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelPinType::IsValid() const
{
	switch (InternalType)
	{
	case EVoxelPinInternalType::Invalid:
	{
		ensure(!GetInternalField());
		return false;
	}
	case EVoxelPinInternalType::Wildcard:
	{
		ensure(!GetInternalField());
		return true;
	}
	case EVoxelPinInternalType::Bool:
	case EVoxelPinInternalType::Float:
	case EVoxelPinInternalType::Double:
	case EVoxelPinInternalType::UInt16:
	case EVoxelPinInternalType::Int32:
	case EVoxelPinInternalType::Int64:
	case EVoxelPinInternalType::Name:
	{
		return ensure(!GetInternalField());
	}
	case EVoxelPinInternalType::Byte:
	{
		if (!GetInternalField())
		{
			return true;
		}

		return ensureVoxelSlow(Cast<UEnum>(GetInternalField()));
	}
	case EVoxelPinInternalType::Class:
	{
		return ensureVoxelSlow(Cast<UClass>(GetInternalField()));
	}
	case EVoxelPinInternalType::Object:
	{
		return ensureVoxelSlow(Cast<UClass>(GetInternalField()));
	}
	case EVoxelPinInternalType::Struct:
	{
		const UScriptStruct* Struct = Cast<UScriptStruct>(GetInternalField());
		return
			ensureVoxelSlow(Struct) &&
			ensureVoxelSlow(!Struct->IsChildOf(StaticStructFast<FVoxelBuffer>()));
	}
	default:
	{
		ensure(false);
		return false;
	}
	}
}

bool FVoxelPinType::IsUserDefinedEnum() const
{
	return
		Is<uint8>() &&
		GetEnum() &&
		GetEnum()->IsA<UUserDefinedEnum>();
}

bool FVoxelPinType::IsUserDefinedStruct() const
{
	return
		IsStruct() &&
		GetStruct()->IsA<UUserDefinedStruct>();
}

FString FVoxelPinType::ToString() const
{
	if (!ensureVoxelSlow(IsValid()))
	{
		return "INVALID";
	}

	if (IsBuffer())
	{
		FString Name = GetInnerType().ToString();
		if (IsBufferArray())
		{
			Name += " Array";
		}
		else
		{
			Name += " Buffer";
		}
		return Name;
	}

	FString Name = INLINE_LAMBDA -> FString
	{
		switch (InternalType)
		{
		default: ensure(false);
		case EVoxelPinInternalType::Wildcard: return "Wildcard";
		case EVoxelPinInternalType::Bool: return "Boolean";
		case EVoxelPinInternalType::Float: return "Float";
		case EVoxelPinInternalType::Double: return "Double";
		case EVoxelPinInternalType::UInt16: return "uint16";
		case EVoxelPinInternalType::Int32: return "Integer";
		case EVoxelPinInternalType::Int64: return "Integer 64";
		case EVoxelPinInternalType::Name: return "Name";
		case EVoxelPinInternalType::Byte:
		{
			if (const UEnum* Enum = GetEnum())
			{
	#if WITH_EDITOR
				return Enum->GetDisplayNameText().ToString();
	#else
				return Enum->GetName();
	#endif
			}
			else
			{
				return "Byte";
			}
		}
		case EVoxelPinInternalType::Class:
		{
	#if WITH_EDITOR
			return GetBaseClass()->GetDisplayNameText().ToString() + " Class";
	#else
			return GetBaseClass()->GetName() + " Class";
	#endif
		}
		case EVoxelPinInternalType::Object:
		{
	#if WITH_EDITOR
			return GetObjectClass()->GetDisplayNameText().ToString();
	#else
			return GetObjectClass()->GetName();
	#endif
		}
		case EVoxelPinInternalType::Struct:
		{
			if (GetStruct() == StaticStructFast<FInt64Point>())
			{
				return "Int64 Point";
			}
			if (GetStruct() == StaticStructFast<FInt64Vector>())
			{
				return "Int64 Vector";
			}
			if (GetStruct() == StaticStructFast<FInt64Vector4>())
			{
				return "Int64 Vector 4";
			}

	#if WITH_EDITOR
			return GetStruct()->GetDisplayNameText().ToString();
	#else
			return GetStruct()->GetName();
	#endif
		}
		}
	};

	Name.RemoveFromStart("Voxel ");
	Name.RemoveFromStart("EVoxel ");
	return Name;
}

FEdGraphPinType FVoxelPinType::GetEdGraphPinType() const
{
	FEdGraphPinType PinType;
	PinType.PinCategory = UEnum::GetValueAsName(InternalType);
	PinType.ContainerType = IsBuffer() ? EPinContainerType::Array : EPinContainerType::None;
	PinType.PinSubCategory = IsBufferArray() ? STATIC_FNAME("Array") : FName();
	PinType.PinSubCategoryObject = GetInternalField();
	return PinType;
}

FEdGraphPinType FVoxelPinType::GetEdGraphPinType_K2() const
{
	if (!IsValid())
	{
		return {};
	}

	// From UEdGraphSchema_K2

	FEdGraphPinType PinType;
	PinType.ContainerType = IsBuffer() ? EPinContainerType::Array : EPinContainerType::None;

	switch (InternalType)
	{
	case EVoxelPinInternalType::Wildcard:
	{
		PinType.PinCategory = STATIC_FNAME("wildcard");
		return PinType;
	}
	case EVoxelPinInternalType::Bool:
	{
		PinType.PinCategory = STATIC_FNAME("bool");
		return PinType;
	}
	// Always use double with blueprints
	case EVoxelPinInternalType::Float:
	case EVoxelPinInternalType::Double:
	{
		PinType.PinCategory = STATIC_FNAME("real");
		PinType.PinSubCategory = STATIC_FNAME("double");
		return PinType;
	}
	case EVoxelPinInternalType::UInt16:
	{
		ensureVoxelSlow(false);
		PinType.PinCategory = STATIC_FNAME("uint16");
		return PinType;
	}
	case EVoxelPinInternalType::Int32:
	{
		PinType.PinCategory = STATIC_FNAME("int");
		return PinType;
	}
	case EVoxelPinInternalType::Int64:
	{
		PinType.PinCategory = STATIC_FNAME("int64");
		return PinType;
	}
	case EVoxelPinInternalType::Name:
	{
		PinType.PinCategory = STATIC_FNAME("name");
		return PinType;
	}
	case EVoxelPinInternalType::Byte:
	{
		PinType.PinCategory = STATIC_FNAME("byte");
		PinType.PinSubCategoryObject = GetInnerType().GetEnum();
		return PinType;
	}
	case EVoxelPinInternalType::Class:
	{
		PinType.PinCategory = STATIC_FNAME("class");
		PinType.PinSubCategoryObject = GetInnerType().GetBaseClass();
		return PinType;
	}
	case EVoxelPinInternalType::Object:
	{
		PinType.PinCategory = STATIC_FNAME("object");
		PinType.PinSubCategoryObject = GetInnerType().GetObjectClass();
		return PinType;
	}
	case EVoxelPinInternalType::Struct:
	{
		PinType.PinCategory = STATIC_FNAME("struct");
		PinType.PinSubCategoryObject = GetInnerType().GetStruct();
		return PinType;
	}
	default:
	{
		ensure(false);
		PinType.PinCategory = STATIC_FNAME("wildcard");
		return PinType;
	}
	}
}

FPropertyBagPropertyDesc FVoxelPinType::GetPropertyBagDesc() const
{
	if (!IsValid())
	{
		return {};
	}

	const FVoxelPinType ExposedType = GetInnerExposedType();
	if (ExposedType != *this)
	{
		FPropertyBagPropertyDesc PropertyDesc = ExposedType.GetPropertyBagDesc();
		if (IsBufferArray())
		{
			PropertyDesc.ContainerTypes = FPropertyBagContainerTypes(EPropertyBagContainerType::Array);
		}
		return PropertyDesc;
	}

	FPropertyBagPropertyDesc PropertyDesc;
	switch (InternalType)
	{
	case EVoxelPinInternalType::Wildcard:
	{
		ensureVoxelSlow(false);
		return {};
	}
	case EVoxelPinInternalType::Bool:
	{
		PropertyDesc.ValueType = EPropertyBagPropertyType::Bool;
		break;
	}
	case EVoxelPinInternalType::Float:
	{
		PropertyDesc.ValueType = EPropertyBagPropertyType::Float;
		break;
	}
	case EVoxelPinInternalType::Double:
	{
		PropertyDesc.ValueType = EPropertyBagPropertyType::Double;
		break;
	}
	case EVoxelPinInternalType::UInt16:
	{
		ensureVoxelSlow(false);
		return {};
	}
	case EVoxelPinInternalType::Int32:
	{
		PropertyDesc.ValueType = EPropertyBagPropertyType::Int32;
		break;
	}
	case EVoxelPinInternalType::Int64:
	{
		PropertyDesc.ValueType = EPropertyBagPropertyType::Int64;
		break;
	}
	case EVoxelPinInternalType::Name:
	{
		PropertyDesc.ValueType = EPropertyBagPropertyType::Name;
		break;
	}
	case EVoxelPinInternalType::Byte:
	{
		if (UEnum* Enum = GetInnerType().GetEnum())
		{
			PropertyDesc.ValueType = EPropertyBagPropertyType::Enum;
			PropertyDesc.ValueTypeObject = Enum;
		}
		else
		{
			PropertyDesc.ValueType = EPropertyBagPropertyType::Byte;
		}
		break;
	}
	case EVoxelPinInternalType::Class:
	{
		PropertyDesc.ValueType = EPropertyBagPropertyType::Class;
		PropertyDesc.ValueTypeObject = GetInnerType().GetBaseClass();
		break;
	}
	case EVoxelPinInternalType::Object:
	{
		PropertyDesc.ValueType = EPropertyBagPropertyType::Object;
		PropertyDesc.ValueTypeObject = GetInnerType().GetObjectClass();
		break;
	}
	case EVoxelPinInternalType::Struct:
	{
		PropertyDesc.ValueType = EPropertyBagPropertyType::Struct;
		PropertyDesc.ValueTypeObject = GetInnerType().GetStruct();
		break;
	}
	default:
	{
		ensure(false);
		return {};
	}
	}

	return PropertyDesc;
}

bool FVoxelPinType::HasPinDefaultValue() const
{
	ensure(IsValid());

	// No default for wildcards or arrays
	if (IsWildcard() ||
		IsBufferArray())
	{
		return false;
	}

	if (const TSharedPtr<FVoxelPinValueOps> Ops = FVoxelPinValueOps::Find(*this, EVoxelPinValueOpsUsage::HasPinDefaultValue))
	{
		return Ops->HasPinDefaultValue();
	}

	return true;
}

bool FVoxelPinType::CanBeCastedTo(const FVoxelPinType& Other) const
{
	if (*this == Other)
	{
		return true;
	}

	if (InternalType != Other.InternalType ||
		bIsBuffer != Other.bIsBuffer)
	{
		return false;
	}

	switch (InternalType)
	{
	default:
	{
		return
			ensureVoxelSlow(!GetInternalField()) &&
			ensureVoxelSlow(!Other.GetInternalField());
	}
	case EVoxelPinInternalType::Byte:
	{
		// Enums can be casted to byte and the other way around
		return true;
	}
	case EVoxelPinInternalType::Class:
	{
		// Classes can always be casted, check is done in TSubclassOf::Get
		return true;
	}
	case EVoxelPinInternalType::Object:
	{
		const UClass* Class = Cast<UClass>(GetInternalField());
		const UClass* OtherClass = Cast<UClass>(Other.GetInternalField());

		if (!ensureVoxelSlow(Class) ||
			!ensureVoxelSlow(OtherClass))
		{
			return false;
		}

		return Class->IsChildOf(OtherClass);
	}
	case EVoxelPinInternalType::Struct:
	{
		const UScriptStruct* Struct = Cast<UScriptStruct>(GetInternalField());
		const UScriptStruct* OtherStruct = Cast<UScriptStruct>(Other.GetInternalField());

		if (!ensureVoxelSlow(Struct) ||
			!ensureVoxelSlow(OtherStruct))
		{
			return false;
		}

		return Struct->IsChildOf(OtherStruct);
	}
	}
}

bool FVoxelPinType::CanBeCastedTo_K2(const FVoxelPinType& Other) const
{
	if (IsBuffer() == Other.IsBuffer() &&
		InternalType == EVoxelPinInternalType::Float &&
		Other.InternalType == EVoxelPinInternalType::Double)
	{
		return true;
	}

	return CanBeCastedTo(Other);
}

bool FVoxelPinType::CanBeCastedTo_Schema(const FVoxelPinType& Other) const
{
	if (bIsBufferArray != Other.bIsBufferArray)
	{
		return false;
	}

	if (!CanBeCastedTo(Other) &&
		// Inner to buffer
		!CanBeCastedTo(Other.GetInnerType()))
	{
		return false;
	}

	// Do strict inheritance checks for the schema (ie, the user UI)
	// At runtime we allow implicit conversions between classes as TSubclassOf::Get does the inheritance checks
	if (InternalType == EVoxelPinInternalType::Class)
	{
		check(Other.InternalType == EVoxelPinInternalType::Class);

		const UClass* Class = CastChecked<UClass>(GetInternalField());
		const UClass* OtherClass = CastChecked<UClass>(Other.GetInternalField());
		return Class->IsChildOf(OtherClass);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool FVoxelPinType::HasBufferType_EditorOnly() const
{
	if (!IsStruct())
	{
		return true;
	}

	return !GetStruct()->HasMetaData("NoBufferType");
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPinType::PostSerialize(const FArchive& Ar)
{
	Fixup();

	if (IsValid())
	{
		*this = FVoxelPinType(GetEdGraphPinType());
	}
}

void FVoxelPinType::SerializeObjects(FArchive& Ar)
{
	Ar << PrivateInternalField;
}

void FVoxelPinType::AddStructReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PrivateInternalField);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPinType::Fixup()
{
	// Fixup buffer structs to allow redirecting to them
	if (UScriptStruct* Struct = Cast<UScriptStruct>(PrivateInternalField))
	{
		if (Struct->IsChildOf(StaticStructFast<FVoxelBuffer>()))
		{
			const bool bOldIsBufferArray = bIsBufferArray;
			*this = FVoxelBuffer::FindInnerType(Struct).GetBufferType();
			bIsBufferArray = bOldIsBufferArray;
		}
	}

	// Set ourselves to invalid if we were referencing an old type
	if (InternalType == EVoxelPinInternalType::Class ||
		InternalType == EVoxelPinInternalType::Object ||
		InternalType == EVoxelPinInternalType::Struct)
	{
		if (!PrivateInternalField)
		{
			*this = FVoxelPinType();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPinType FVoxelPinType::GetExposedType() const
{
	if (IsBuffer())
	{
		if (IsBufferArray())
		{
			return GetInnerExposedType().GetBufferType().WithBufferArray(true);
		}
		else
		{
			return GetInnerExposedType();
		}
	}

	if (Is<FVoxelSeed>())
	{
		return Make<FVoxelExposedSeed>();
	}

	if (const TSharedPtr<FVoxelPinValueOps> Ops = FVoxelPinValueOps::Find(*this, EVoxelPinValueOpsUsage::GetExposedType))
	{
		return Ops->GetExposedType();
	}

	if (IsStruct())
	{
		if (const TSharedPtr<const FVoxelObjectPinType> ObjectPinType = FVoxelObjectPinType::StructToPinType().FindRef(GetStruct()))
		{
			return MakeObject(ObjectPinType->GetClass());
		}
	}

	return *this;
}

FVoxelPinType FVoxelPinType::GetInnerExposedType() const
{
	return GetInnerType().GetExposedType();
}

FVoxelPinType FVoxelPinType::GetPinDefaultValueType() const
{
	ensure(HasPinDefaultValue());
	ensure(GetInnerExposedType() == GetExposedType());
	return GetExposedType();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRuntimePinValue FVoxelPinType::MakeRuntimeValue(
	const FVoxelPinType& RuntimeType,
	const FVoxelPinValue& ExposedValue,
	const FRuntimeValueContext& Context)
{
	checkUObjectAccess();

	if (!ensure(!RuntimeType.IsWildcard()) ||
		!ensure(ExposedValue.GetType().CanBeCastedTo(RuntimeType.GetExposedType())))
	{
		return {};
	}

	FVoxelRuntimePinValue Value = INLINE_LAMBDA -> FVoxelRuntimePinValue
	{
		if (RuntimeType.IsBuffer())
		{
			if (ExposedValue.IsArray())
			{
				const TSharedRef<FVoxelBuffer> Buffer = FVoxelBuffer::MakeEmpty(RuntimeType.GetInnerType());
				Buffer->Allocate(ExposedValue.GetArray().Num());

				int32 WriteIndex = 0;
				for (const FVoxelTerminalPinValue& ExposedInnerValue : ExposedValue.GetArray())
				{
					Buffer->SetGeneric(WriteIndex, MakeRuntimeValue(
						RuntimeType.GetInnerType(),
						ExposedInnerValue.ToValue(),
						Context));

					WriteIndex++;
				}
				check(WriteIndex == ExposedValue.GetArray().Num());

				return FVoxelRuntimePinValue::Make(Buffer);
			}
			else
			{
				const FVoxelRuntimePinValue InnerValue = MakeRuntimeValue(
					RuntimeType.GetInnerType(),
					ExposedValue,
					Context);

				return FVoxelRuntimePinValue::Make(FVoxelBuffer::MakeConstant(InnerValue));
			}
		}
		checkVoxelSlow(!RuntimeType.IsBuffer());

		switch (RuntimeType.GetInternalType())
		{
		default: break;
		case EVoxelPinInternalType::Bool: return FVoxelRuntimePinValue::Make(ExposedValue.Get<bool>());
		case EVoxelPinInternalType::Float: return FVoxelRuntimePinValue::Make(ExposedValue.Get<float>());
		case EVoxelPinInternalType::Double: return FVoxelRuntimePinValue::Make(ExposedValue.Get<double>());
		case EVoxelPinInternalType::UInt16: return FVoxelRuntimePinValue::Make(ExposedValue.Get<uint16>());
		case EVoxelPinInternalType::Int32: return FVoxelRuntimePinValue::Make(ExposedValue.Get<int32>());
		case EVoxelPinInternalType::Int64: return FVoxelRuntimePinValue::Make(ExposedValue.Get<int64>());
		case EVoxelPinInternalType::Name: return FVoxelRuntimePinValue::Make(ExposedValue.Get<FName>());
		case EVoxelPinInternalType::Byte: return FVoxelRuntimePinValue::Make(ExposedValue.Get<uint8>());
		case EVoxelPinInternalType::Class: return FVoxelRuntimePinValue::Make(ExposedValue.Get<TSubclassOf<UObject>>());
		}

		if (!ensure(RuntimeType.IsStruct()))
		{
			return {};
		}

		if (ExposedValue.Is<FVoxelExposedSeed>())
		{
			return FVoxelRuntimePinValue::Make(FVoxelSeed(ExposedValue.Get<FVoxelExposedSeed>().GetSeed()));
		}

		if (ExposedValue.IsObject())
		{
			if (const TSharedPtr<const FVoxelObjectPinType> ObjectPinType = FVoxelObjectPinType::StructToPinType().FindRef(RuntimeType.GetStruct()))
			{
				UObject* Object = ExposedValue.GetObject();
				const FVoxelInstancedStruct Struct = ObjectPinType->GetStruct(Object);
				ensureVoxelSlow(ObjectPinType->GetObject(Struct) == Object);

				return FVoxelRuntimePinValue::MakeStruct(Struct);
			}
		}

		if (const TSharedPtr<FVoxelPinValueOps> Ops = FVoxelPinValueOps::Find(RuntimeType, EVoxelPinValueOpsUsage::MakeRuntimeValue))
		{
			const FVoxelRuntimePinValue RuntimeValue = Ops->MakeRuntimeValue(ExposedValue, Context);
			ensure(RuntimeValue.GetType() == RuntimeType);
			return RuntimeValue;
		}

		if (ExposedValue.GetStruct().GetScriptStruct()->IsA<UUserDefinedStruct>())
		{
			return FVoxelRuntimePinValue::MakeRuntimeStruct(
				ExposedValue.GetStruct(),
				Context);
		}

		if (!ensure(ExposedValue.GetType().CanBeCastedTo(RuntimeType)))
		{
			return {};
		}

		return FVoxelRuntimePinValue::MakeStruct(ExposedValue.GetStruct());
	};

	if (!Value.IsValid())
	{
		return {};
	}

	Value.SetType(RuntimeType);

	return Value;
}

FVoxelRuntimePinValue FVoxelPinType::MakeRuntimeValueFromInnerValue(
	const FVoxelPinType& RuntimeType,
	const FVoxelPinValue& ExposedInnerValue,
	const FRuntimeValueContext& Context)
{
	ensure(!ExposedInnerValue.IsArray());

	if (RuntimeType.IsBufferArray())
	{
		FVoxelPinValue ExposedValue(ExposedInnerValue.GetType().GetBufferType().WithBufferArray(true));
		ExposedValue.AddValue(ExposedInnerValue.AsTerminalValue());
		return MakeRuntimeValue(RuntimeType, ExposedValue, Context);
	}

	return MakeRuntimeValue(RuntimeType, ExposedInnerValue, Context);
}