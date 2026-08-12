// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelRuntimePinValue.h"
#include "VoxelBuffer.h"
#include "VoxelPinValueOps.h"
#include "VoxelRuntimeStruct.h"
#include "VoxelObjectPinType.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "StructUtils/UserDefinedStruct.h"

void FVoxelRuntimePinValue::InitializeImpl()
{
	if (Type.IsBuffer())
	{
		if (Type.IsBufferArray())
		{
			TSharedRef<FVoxelBuffer> Buffer = FVoxelBuffer::MakeEmpty(Type.GetInnerType());
			SharedStructType = Buffer->GetStruct();
			SharedStruct = MakeSharedVoidRef(MoveTemp(Buffer));
		}
		else
		{
			TSharedRef<FVoxelBuffer> Buffer = FVoxelBuffer::MakeDefault(Type.GetInnerType());
			SharedStructType = Buffer->GetStruct();
			SharedStruct = MakeSharedVoidRef(MoveTemp(Buffer));
		}
	}
	else if (Type.IsStruct())
	{
		checkVoxelSlow(!Type.IsUserDefinedStruct());

		SharedStructType = Type.GetStruct();
		SharedStruct = MakeSharedStruct(SharedStructType);
	}
}

FVoxelRuntimePinValue FVoxelRuntimePinValue::MakeStruct(const FConstVoxelStructView Struct)
{
	checkVoxelSlow(!Struct.GetScriptStruct()->IsA<UUserDefinedStruct>());

	FVoxelRuntimePinValue Result;
	Result.Type = FVoxelPinType::MakeStruct(Struct.GetScriptStruct());
	Result.SharedStructType = Struct.GetScriptStruct();
	Result.SharedStruct = Struct.MakeSharedCopy();
	return Result;
}

FVoxelRuntimePinValue FVoxelRuntimePinValue::MakeRuntimeStruct(
	const FConstVoxelStructView UserStruct,
	const FVoxelPinType::FRuntimeValueContext& Context)
{
	return MakeRuntimeStruct(MakeShared<FVoxelRuntimeStruct>(UserStruct, Context));
}

FVoxelRuntimePinValue FVoxelRuntimePinValue::MakeRuntimeStruct(const TSharedRef<FVoxelRuntimeStruct>& Struct)
{
	FVoxelRuntimePinValue Result;
	Result.Type = FVoxelPinType::MakeStruct(Struct->UserStruct);
	Result.SharedStructType = StaticStructFast<FVoxelRuntimeStruct>();
	Result.SharedStruct = MakeSharedVoidRef(Struct);
	return Result;
}

FVoxelRuntimePinValue FVoxelRuntimePinValue::Make(FVoxelBuffer&& Buffer)
{
	const TSharedRef<FVoxelBuffer> NewBuffer = FVoxelBuffer::MakeEmpty(Buffer.GetInnerType());
	NewBuffer->MoveFrom(MoveTemp(Buffer));
	return	Make(NewBuffer);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelRuntimePinValue::IsValidValue_Slow() const
{
	if (!Type.IsValid_Fast())
	{
		return false;
	}

	if (!Type.CanBeCastedTo<FVoxelBuffer>())
	{
		return true;
	}

	return Get<FVoxelBuffer>().IsValid_Slow();
}

FString FVoxelRuntimePinValue::ToDebugString(const bool bFullValue, bool bParseBaseStructs) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(IsValid()))
	{
		return {};
	}

	if (Type.IsBuffer())
	{
		return {};
	}

	switch (Type.GetInternalType())
	{
	default:
	{
		ensure(false);
		return {};
	}
	case EVoxelPinInternalType::Bool:
	{
		return LexToString(Get<bool>());
	}
	case EVoxelPinInternalType::Float:
	{
		return bFullValue ? LexToString(Get<float>()) : FVoxelUtilities::NumberToString(Get<float>());
	}
	case EVoxelPinInternalType::Double:
	{
		return bFullValue ? LexToString(Get<double>()) : FVoxelUtilities::NumberToString(Get<double>());
	}
	case EVoxelPinInternalType::UInt16:
	{
		return bFullValue ? LexToString(Get<uint16>()) : FVoxelUtilities::NumberToString(Get<uint16>());
	}
	case EVoxelPinInternalType::Int32:
	{
		return bFullValue ? LexToString(Get<int32>()) : FVoxelUtilities::NumberToString(Get<int32>());
	}
	case EVoxelPinInternalType::Int64:
	{
		return bFullValue ? LexToString(Get<int64>()) : FVoxelUtilities::NumberToString(Get<int64>());
	}
	case EVoxelPinInternalType::Name:
	{
		return Get<FName>().ToString();
	}
	case EVoxelPinInternalType::Byte:
	{
		if (const UEnum* Enum = Type.GetEnum())
		{
			return Enum->GetNameStringByValue(Byte);
		}
		return LexToString(Get<uint8>());
	}
	case EVoxelPinInternalType::Class:
	{
		return FSoftObjectPath(Class).ToString();
	}
	case EVoxelPinInternalType::Struct:
	{
		if (bParseBaseStructs)
		{
			if (Type.Is<FVector2D>())
			{
				if (bFullValue)
				{
					return Get<FVector2D>().ToString();
				}

				const FVector2D& Value = Get<FVector2D>();
				return
					"X=" + FVoxelUtilities::NumberToString(Value.X) + " " +
					"Y=" + FVoxelUtilities::NumberToString(Value.Y);
			}
			else if (Type.Is<FVector>())
			{
				if (bFullValue)
				{
					return Get<FVector>().ToString();
				}

				const FVector& Value = Get<FVector>();
				return
					"X=" + FVoxelUtilities::NumberToString(Value.X) + " " +
					"Y=" + FVoxelUtilities::NumberToString(Value.Y) + " " +
					"Z=" + FVoxelUtilities::NumberToString(Value.Z);
			}
			else if (Type.Is<FLinearColor>())
			{
				const FLinearColor& Value = Get<FLinearColor>();
				if (bFullValue)
				{
					return
						"R=" + LexToSanitizedString(Value.R) + " " +
						"G=" + LexToSanitizedString(Value.G) + " " +
						"B=" + LexToSanitizedString(Value.B) + " " +
						"A=" + LexToSanitizedString(Value.A);
				}

				return
					"R=" + FVoxelUtilities::NumberToString(Value.R) + " " +
					"G=" + FVoxelUtilities::NumberToString(Value.G) + " " +
					"B=" + FVoxelUtilities::NumberToString(Value.B) + " " +
					"A=" + FVoxelUtilities::NumberToString(Value.A);
			}
			else if (Type.Is<FVoxelDoubleVector2D>())
			{
				if (bFullValue)
				{
					return Get<FVoxelDoubleVector2D>().ToString();
				}

				const FVoxelDoubleVector2D& Value = Get<FVoxelDoubleVector2D>();
				return
					"X=" + FVoxelUtilities::NumberToString(Value.X) + " " +
					"Y=" + FVoxelUtilities::NumberToString(Value.Y);
			}
			else if (Type.Is<FVoxelDoubleVector>())
			{
				if (bFullValue)
				{
					return Get<FVoxelDoubleVector>().ToString();
				}

				const FVoxelDoubleVector& Value = Get<FVoxelDoubleVector>();
				return
					"X=" + FVoxelUtilities::NumberToString(Value.X) + " " +
					"Y=" + FVoxelUtilities::NumberToString(Value.Y) + " " +
					"Z=" + FVoxelUtilities::NumberToString(Value.Z);
			}
			else if (Type.Is<FVoxelDoubleLinearColor>())
			{
				const FVoxelDoubleLinearColor& Value = Get<FVoxelDoubleLinearColor>();
				if (bFullValue)
				{
					return
						"R=" + LexToSanitizedString(Value.R) + " " +
						"G=" + LexToSanitizedString(Value.G) + " " +
						"B=" + LexToSanitizedString(Value.B) + " " +
						"A=" + LexToSanitizedString(Value.A);
				}

				return
					"R=" + FVoxelUtilities::NumberToString(Value.R) + " " +
					"G=" + FVoxelUtilities::NumberToString(Value.G) + " " +
					"B=" + FVoxelUtilities::NumberToString(Value.B) + " " +
					"A=" + FVoxelUtilities::NumberToString(Value.A);
			}
			else if (Type.Is<FIntPoint>())
			{
				if (bFullValue)
				{
					return Get<FIntPoint>().ToString();
				}

				const FIntPoint& Value = Get<FIntPoint>();
				return
					"X=" + FVoxelUtilities::NumberToString(Value.X) + " " +
					"Y=" + FVoxelUtilities::NumberToString(Value.Y);
			}
			else if (Type.Is<FIntVector>())
			{
				if (bFullValue)
				{
					return Get<FIntVector>().ToString();
				}

				const FIntVector& Value = Get<FIntVector>();
				return
					"X=" + FVoxelUtilities::NumberToString(Value.X) + " " +
					"Y=" + FVoxelUtilities::NumberToString(Value.Y) + " " +
					"Z=" + FVoxelUtilities::NumberToString(Value.Z);
			}
			else if (Type.Is<FIntVector4>())
			{
				const FIntVector4& Value = Get<FIntVector4>();
				if (bFullValue)
				{
					return Value.ToString();
				}

				return
					"X=" + FVoxelUtilities::NumberToString(Value.X) + " " +
					"Y=" + FVoxelUtilities::NumberToString(Value.Y) + " " +
					"Z=" + FVoxelUtilities::NumberToString(Value.Z) + " " +
					"W=" + FVoxelUtilities::NumberToString(Value.W);
			}
			else if (Type.Is<FInt64Point>())
			{
				if (bFullValue)
				{
					return Get<FInt64Point>().ToString();
				}

				const FInt64Point& Value = Get<FInt64Point>();
				return
					"X=" + FVoxelUtilities::NumberToString(Value.X) + " " +
					"Y=" + FVoxelUtilities::NumberToString(Value.Y);
			}
			else if (Type.Is<FInt64Vector>())
			{
				if (bFullValue)
				{
					return Get<FInt64Vector>().ToString();
				}

				const FInt64Vector& Value = Get<FInt64Vector>();
				return
					"X=" + FVoxelUtilities::NumberToString(Value.X) + " " +
					"Y=" + FVoxelUtilities::NumberToString(Value.Y) + " " +
					"Z=" + FVoxelUtilities::NumberToString(Value.Z);
			}
			else if (Type.Is<FInt64Vector4>())
			{
				const FInt64Vector4& Value = Get<FInt64Vector4>();
				if (bFullValue)
				{
					return Value.ToString();
				}

				return
					"X=" + FVoxelUtilities::NumberToString(Value.X) + " " +
					"Y=" + FVoxelUtilities::NumberToString(Value.Y) + " " +
					"Z=" + FVoxelUtilities::NumberToString(Value.Z) + " " +
					"W=" + FVoxelUtilities::NumberToString(Value.W);
			}
			else if (Type.Is<FRotator>())
			{
				return Get<FRotator>().ToString();
			}
			else if (Type.Is<FQuat>())
			{
				return Get<FQuat>().Rotator().ToString();
			}
			else if (Type.Is<FColor>())
			{
				return Get<FColor>().ToString();
			}
		}

		if (const TSharedPtr<FVoxelPinValueOps> Ops = FVoxelPinValueOps::Find(Type, EVoxelPinValueOpsUsage::ToDebugString))
		{
			return Ops->ToDebugString(*this);
		}
		else if (Type.GetExposedType().IsObject())
		{
			if (const TSharedPtr<const FVoxelObjectPinType> ObjectPinType = FVoxelObjectPinType::StructToPinType().FindRef(Type.GetStruct()))
			{
				const UObject* Object = ObjectPinType->GetObject(GetStructView());
				if (!Object)
				{
					return "None";
				}

				return Object->GetName();
			}

			return "Invalid";
		}
		else if (Type.IsUserDefinedStruct())
		{
			const FVoxelRuntimeStruct& Value = reinterpret_cast<const FVoxelRuntimeStruct&>(*SharedStruct.Get());
			FString Result;
			for (const auto& It : Value.PropertyNameToValue)
			{
				if (!Result.IsEmpty())
				{
					Result += ",";
				}

				FString PropertyName = It.Key.ToString();
#if WITH_EDITOR
				if (const FProperty* Property = FindFProperty<FProperty>(Value.UserStruct, It.Key))
				{
					PropertyName = Property->GetDisplayNameText().ToString();
				}
#endif

				Result += PropertyName + "=" + It.Value.ToDebugString(bFullValue, false);
			}

			return Result;
		}
		else
		{
			return FVoxelUtilities::PropertyToText_Direct(
				*FVoxelUtilities::MakeStructProperty(Type.GetStruct()),
				GetStructView().GetStructMemory(),
				nullptr);
		}
	}
	}
}

void FVoxelRuntimePinValue::ExportToProperty(const FProperty& Property, void* Memory) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(IsValid()) ||
		// CanBeCastedTo_K2: ExportToProperty does implicit float -> double conversion
		!ensure(Type.GetExposedType().CanBeCastedTo_K2(FVoxelPinType(Property, true))))
	{
		return;
	}

	if (Type.IsBuffer())
	{
		return;
	}

	switch (Type.GetInternalType())
	{
	default:
	{
		ensure(false);
	}
	break;
	case EVoxelPinInternalType::Bool:
	{
		if (!ensure(Property.IsA<FBoolProperty>()))
		{
			return;
		}

		CastFieldChecked<FBoolProperty>(Property).SetPropertyValue(Memory, Get<bool>());
	}
	break;
	case EVoxelPinInternalType::Float:
	{
		if (Property.IsA<FFloatProperty>())
		{
			CastFieldChecked<FFloatProperty>(Property).SetPropertyValue(Memory, Get<float>());
			return;
		}

		if (Property.IsA<FDoubleProperty>())
		{
			CastFieldChecked<FDoubleProperty>(Property).SetPropertyValue(Memory, Get<float>());
			return;
		}

		ensure(false);
		return;
	}
	break;
	case EVoxelPinInternalType::Double:
	{
		if (!ensure(Property.IsA<FDoubleProperty>()))
		{
			return;
		}

		CastFieldChecked<FDoubleProperty>(Property).SetPropertyValue(Memory, Get<double>());
	}
	break;
	case EVoxelPinInternalType::UInt16:
	{
		if (!ensure(Property.IsA<FUInt16Property>()))
		{
			return;
		}

		CastFieldChecked<FUInt16Property>(Property).SetPropertyValue(Memory, Get<uint16>());
	}
	break;
	case EVoxelPinInternalType::Int32:
	{
		if (!ensure(Property.IsA<FIntProperty>()))
		{
			return;
		}

		CastFieldChecked<FIntProperty>(Property).SetPropertyValue(Memory, Get<int32>());
	}
	break;
	case EVoxelPinInternalType::Int64:
	{
		if (!ensure(Property.IsA<FInt64Property>()))
		{
			return;
		}

		CastFieldChecked<FInt64Property>(Property).SetPropertyValue(Memory, Get<int64>());
	}
	break;
	case EVoxelPinInternalType::Name:
	{
		if (!ensure(Property.IsA<FNameProperty>()))
		{
			return;
		}

		CastFieldChecked<FNameProperty>(Property).SetPropertyValue(Memory, Get<FName>());
	}
	break;
	case EVoxelPinInternalType::Byte:
	{
		if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			if (!ensure(Type.GetEnum() == EnumProperty->GetEnum()) ||
				!ensure(EnumProperty->GetUnderlyingProperty()->IsA<FByteProperty>()))
			{
				return;
			}

			CastFieldChecked<FByteProperty>(EnumProperty->GetUnderlyingProperty())->SetPropertyValue(Memory, Get<uint8>());
		}
		else if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
		{
			if (!ensure(Type.GetEnum() == ByteProperty->Enum))
			{
				return;
			}

			ByteProperty->SetPropertyValue(Memory, Get<uint8>());
		}
		else
		{
			ensure(false);
		}
	}
	break;
	case EVoxelPinInternalType::Class:
	{
		if (Property.IsA<FClassProperty>())
		{
			const FClassProperty& ClassProperty = CastFieldChecked<FClassProperty>(Property);
			if (!ensure(Type.GetBaseClass()->IsChildOf(ClassProperty.MetaClass)))
			{
				return;
			}

			checkUObjectAccess();
			ClassProperty.SetObjectPropertyValue(Memory, Class);
		}
		else if (Property.IsA<FSoftClassProperty>())
		{
			const FSoftClassProperty& ClassProperty = CastFieldChecked<FSoftClassProperty>(Property);
			if (!ensure(Type.GetBaseClass()->IsChildOf(ClassProperty.MetaClass)))
			{
				return;
			}

			checkUObjectAccess();
			ClassProperty.SetObjectPropertyValue(Memory, Class);
		}
		else
		{
			ensure(false);
		}
	}
	break;
	case EVoxelPinInternalType::Struct:
	{
		if (Property.IsA<FObjectProperty>())
		{
			const FObjectProperty& ObjectProperty = CastFieldChecked<FObjectProperty>(Property);
			if (!ensure(ObjectProperty.PropertyClass == Type.GetObjectClass()))
			{
				return;
			}

			checkUObjectAccess();

			const TSharedPtr<const FVoxelObjectPinType> ObjectPinType = FVoxelObjectPinType::StructToPinType().FindRef(Type.GetStruct());
			if (!ensure(ObjectPinType))
			{
				return;
			}

			UObject* Object = ObjectPinType->GetObject(GetStructView());
			ObjectProperty.SetObjectPropertyValue(Memory, Object);
		}
		else if (Property.IsA<FSoftObjectProperty>())
		{
			const FSoftObjectProperty& ObjectProperty = CastFieldChecked<FSoftObjectProperty>(Property);
			if (!ensure(ObjectProperty.PropertyClass == Type.GetObjectClass()))
			{
				return;
			}

			checkUObjectAccess();

			const TSharedPtr<const FVoxelObjectPinType> ObjectPinType = FVoxelObjectPinType::StructToPinType().FindRef(Type.GetStruct());
			if (!ensure(ObjectPinType))
			{
				return;
			}

			UObject* Object = ObjectPinType->GetObject(GetStructView());
			ObjectProperty.SetObjectPropertyValue(Memory, Object);
		}
		else
		{
			if (!ensure(Property.IsA<FStructProperty>()))
			{
				return;
			}

			FConstVoxelStructView StructView = GetStructView();
			const FStructProperty& StructProperty = CastFieldChecked<FStructProperty>(Property);
			if (!ensure(Type.GetStruct() == StructProperty.Struct) ||
				!ensure(Type.GetStruct() == StructView.GetScriptStruct()))
			{
				return;
			}

			StructView.GetScriptStruct()->CopyScriptStruct(Memory, StructView.GetStructMemory());
		}
	}
	break;
	}
}