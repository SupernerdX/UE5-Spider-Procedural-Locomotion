// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_UFunction.h"
#include "VoxelBuffer.h"
#include "VoxelSourceParser.h"
#include "VoxelGraphMigration.h"
#include "VoxelFunctionLibrary.h"
#include "UObject/CoreRedirects.h"
#include "Buffer/VoxelBaseBuffers.h"

TSharedRef<FVoxelNode_UFunction> FVoxelNode_UFunction::Make(UFunction* InFunction)
{
	ensure(InFunction);

	const TSharedRef<FVoxelNode_UFunction> Node = MakeShared<FVoxelNode_UFunction>();
	Node->Function = InFunction;

	// Add empty metadata - we can't cache it since the node is created at runtime
	for (const FProperty& Property : GetFunctionProperties(InFunction))
	{
		Node->MetadataMapCache.Add(Property.GetFName());

#if WITH_EDITOR
		if (const TMap<FName, FString>* MetadataMap = Property.GetMetaDataMap())
		{
			TMap<FName, FString> Map = *MetadataMap;
			Map.Remove(STATIC_FNAME("NativeConst"));
			Map.Remove(STATIC_FNAME("DisplayName"));
			Map.Remove(STATIC_FNAME("HidePinLabel"));
			Map.Remove(STATIC_FNAME("AdvancedDisplay"));
			ensure(Map.Num() == 0);
		}
#endif
	}

	Node->FixupPins();

	return Node;
}

#if WITH_EDITOR
void FVoxelNode_UFunction::SetFunction_EditorOnly(UFunction* NewFunction)
{
	ensure(NewFunction);
	ensure(!Function);
	Function = NewFunction;

	FixupPins();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
UStruct& FVoxelNode_UFunction::GetMetadataContainer() const
{
	if (!Function)
	{
		return Super::GetMetadataContainer();
	}

	return *Function;
}

FString FVoxelNode_UFunction::GetCategory() const
{
	if (!ensure(Function))
	{
		return {};
	}

	FString Category;
	if (!Function->GetStringMetaDataHierarchical(STATIC_FNAME("Category"), &Category) ||
		Category.IsEmpty())
	{
		ensureMsgf(
			Function->GetOuterUClass()->GetStringMetaDataHierarchical(STATIC_FNAME("Category"), &Category) &&
			!Category.IsEmpty(),
			TEXT("%s is missing a Category"),
			*Function->GetName());
	}
	return Category;
}

FString FVoxelNode_UFunction::GetDisplayName() const
{
	if (!Function)
	{
		return CachedName.ToString();
	}

	return FVoxelNode::GetDisplayName();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_UFunction::Initialize(FInitializer& Initializer)
{
	if (!ensureVoxelSlow(Function))
	{
		return;
	}

	CachedFunction = MakeShared<UVoxelFunctionLibrary::FCachedFunction>(*Function);

	ensure(InputPins.Num() == 0);
	InputPins.Reserve(GetPins().Num());

	ensure(FunctionPins.Num() == 0);
	FunctionPins.Reserve(GetPins().Num());

	for (const FProperty& Property : GetFunctionProperties(Function))
	{
		FunctionPins.Add_EnsureNoGrow(FFunctionPin
		{
			IsFunctionInput(Property),
			FindPin(Property.GetFName())->GetType(),
			UVoxelFunctionLibrary::MakeType(Property),
		});

		if (IsFunctionInput(Property))
		{
			FPinRef_Input& InputPinRef = InputPins.Emplace_GetRef_EnsureNoGrow(Property.GetFName());
			Initializer.InitializePinRef(InputPinRef);
		}
		else
		{
			FPinRef_Output& OutputPinRef = FunctionPins.Last().OutputPinRef.Emplace(Property.GetFName());
			Initializer.InitializePinRef(OutputPinRef);
		}
	}
}

void FVoxelNode_UFunction::Compute(const FVoxelGraphQuery Query) const
{
	if (!ensureVoxelSlow(CachedFunction))
	{
		return;
	}

	TVoxelInlineArray<FValue, 16> InputValues;
	InputValues.Reserve(InputPins.Num());

	for (const FPinRef_Input& Pin : InputPins)
	{
		InputValues.Add(Pin.Get(Query));
	}

	Query.AddTask([this, Query, InputValues = MoveTemp(InputValues)]
	{
		TVoxelInlineArray<FVoxelRuntimePinValue*, 16> Values;
		TVoxelInlineArray<FVoxelRuntimePinValue, 16> Storage;
		Values.Reserve(FunctionPins.Num());
		Storage.Reserve(FunctionPins.Num());

		int32 InputIndex = 0;
		for (const FFunctionPin& FunctionPin : FunctionPins)
		{
			Values.Add_EnsureNoGrow(INLINE_LAMBDA
			{
				if (FunctionPin.bIsInput)
				{
					const FVoxelRuntimePinValue& InputValue = InputValues[InputIndex++].GetValue();
					checkVoxelSlow(InputValue.GetType().CanBeCastedTo(FunctionPin.PinType));

					if (FunctionPin.PropertyType.IsWildcard())
					{
						return ConstCast(&InputValue);
					}
					else if (FunctionPin.PinType.CanBeCastedTo(FunctionPin.PropertyType))
					{
						return ConstCast(&InputValue);
					}
					else
					{
						checkVoxelSlow(FunctionPin.PinType.GetBufferType().CanBeCastedTo(FunctionPin.PropertyType));

						return &Storage.Add_GetRef_EnsureNoGrow(FVoxelRuntimePinValue::Make(FVoxelBuffer::MakeConstant(InputValue)));
					}
				}
				else
				{
					if (FunctionPin.PropertyType.IsWildcard())
					{
						return &Storage.Emplace_GetRef_EnsureNoGrow();
					}
					else
					{
						return &Storage.Add_GetRef_EnsureNoGrow(FVoxelRuntimePinValue(FunctionPin.PropertyType));
					}
				}
			});
		}

		UVoxelFunctionLibrary::Call(
			*this,
			*CachedFunction,
			Query,
			Values);

		for (int32 Index = 0; Index < FunctionPins.Num(); Index++)
		{
			const FFunctionPin& FunctionPin = FunctionPins[Index];
			if (FunctionPin.bIsInput)
			{
				continue;
			}

			FVoxelRuntimePinValue& Value = *Values[Index];

			if (Value.IsBuffer() &&
				Value.GetType().GetInnerType() == FunctionPin.PinType)
			{
				// Convert buffer to scalar
				checkVoxelSlow(!AreTemplatePinsBuffers());

				const FVoxelBuffer& Buffer = Value.Get<FVoxelBuffer>();
				const int32 BufferNum = Buffer.Num_Slow();

				if (BufferNum == 0 ||
					!ensure(BufferNum == 1))
				{
					Value = FVoxelRuntimePinValue(FunctionPin.PinType);
				}
				else
				{
					Value = Buffer.GetGenericConstant();
				}
			}
			checkVoxelSlow(Value.GetType().CanBeCastedTo(FunctionPin.PinType));

#if WITH_EDITOR
			if (!ensureMsgf(Value.IsValidValue_Slow(), TEXT("Invalid value produced by %s"), *Function->GetName()))
			{
				Value = FVoxelRuntimePinValue(FunctionPin.PinType);
			}
#endif

			FunctionPin.OutputPinRef->Set(Query, MoveTemp(Value));
		}
	});
}

void FVoxelNode_UFunction::PreSerialize()
{
	Super::PreSerialize();

#if WITH_EDITOR
	if (!Function)
	{
		return;
	}

	CachedName = FName(Function->GetDisplayNameText().ToString());
	CachedPath = Function->GetPathName();
	MetadataMapCache.Reset();

	for (const FProperty& Property : GetFunctionProperties(Function))
	{
		FVoxelFunctionNodeMetadataMap& Cache = MetadataMapCache.Add(Property.GetFName());
		if (const TMap<FName, FString>* MetadataMap = Property.GetMetaDataMap())
		{
			Cache.MetadataMap = *MetadataMap;
			Cache.MetadataMap.Remove(STATIC_FNAME("NativeConst"));
			Cache.MetadataMap.Remove(STATIC_FNAME("DisplayName"));
			Cache.MetadataMap.Remove(STATIC_FNAME("HidePinLabel"));
			Cache.MetadataMap.Remove(STATIC_FNAME("AdvancedDisplay"));
		}
	}
#endif
}

void FVoxelNode_UFunction::PostSerialize()
{
	if (!Function)
	{
		if (!CachedPath.IsEmpty())
		{
			VOXEL_SCOPE_COUNTER("FindFirstObject");

			const FCoreRedirectObjectName RedirectedName = FCoreRedirects::GetRedirectedName(
				ECoreRedirectFlags::Type_Function,
				FCoreRedirectObjectName(CachedPath));

			Function = FindFirstObject<UFunction>(*RedirectedName.ToString(), EFindFirstObjectOptions::EnsureIfAmbiguous);
		}

		if (!Function)
		{
			Function = GVoxelGraphMigration->FindNewFunction(CachedName);
		}
	}

	FixupPins();

	Super::PostSerialize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_UFunction::FixupPins()
{
	VOXEL_FUNCTION_COUNTER();

	if (!Function)
	{
		return;
	}

	for (const auto& It : MakeCopy(GetNameToPin()))
	{
		RemovePin(It.Key);
	}

#if WITH_EDITOR
	const bool bNotTemplate = INLINE_LAMBDA
	{
		if (Function->HasMetaData(STATIC_FNAME("NotTemplate")))
		{
			return true;
		}

		for (const FProperty& Property : GetFunctionProperties(Function))
		{
			if (Property.HasMetaData(STATIC_FNAME("PositionPin")) ||
				Property.HasMetaData(STATIC_FNAME("SplineKeyPin")))
			{
				return true;
			}
		}

		return false;
	};
#else
	const bool bNotTemplate = false;
#endif


	for (const FProperty& Property : GetFunctionProperties(Function))
	{
		FVoxelPinType Type = UVoxelFunctionLibrary::MakeType(Property);

		FVoxelPinMetadata Metadata;

		const TMap<FName, FString>* MetadataMap = nullptr;
		if (WITH_EDITOR)
		{
#if WITH_EDITOR
			MetadataMap = Property.GetMetaDataMap();
#endif
		}
		else
		{
			const FVoxelFunctionNodeMetadataMap* CachedMetadataMap = MetadataMapCache.Find(Property.GetFName());
			if (ensure(CachedMetadataMap))
			{
				MetadataMap = &CachedMetadataMap->MetadataMap;
			}
		}

		if (MetadataMap)
		{
			for (auto& It : *MetadataMap)
			{
				const FName Name = It.Key;

#if WITH_EDITOR
				if (Name == STATIC_FNAME("NativeConst"))
				{
					continue;
				}
				if (Name == STATIC_FNAME("DisplayName"))
				{
					ensure(Metadata.DisplayName.IsEmpty());
					Metadata.DisplayName = It.Value;
					continue;
				}
				if (Name == STATIC_FNAME("HidePinLabel"))
				{
					ensure(Metadata.DisplayName.IsEmpty());
					Metadata.DisplayName = " ";
					continue;
				}
				if (Name == STATIC_FNAME("AdvancedDisplay"))
				{
					ensure(Metadata.Category.IsEmpty());
					Metadata.Category = "Advanced";
					continue;
				}
#endif

				if (Name == STATIC_FNAME("PositionPin"))
				{
					ensure(!Metadata.bPositionPin);
					Metadata.bPositionPin = true;
				}
				else if (Name == STATIC_FNAME("HideDefault"))
				{
					ensure(!Metadata.bHideDefault);
					Metadata.bHideDefault = true;
				}
				else if (Name == STATIC_FNAME("SplineKeyPin"))
				{
					ensure(!Metadata.bSplineKeyPin);
					Metadata.bSplineKeyPin = true;
				}
				else if (Name == STATIC_FNAME("DisplayLast"))
				{
					ensure(!Metadata.bDisplayLast);
					Metadata.bDisplayLast = true;
				}
				else if (Name == STATIC_FNAME("ShowInDetail"))
				{
					ensure(!Metadata.bShowInDetail);
					Metadata.bShowInDetail = true;
				}
				else if (Name == STATIC_FNAME("HidePin"))
				{
					ensure(!Metadata.bHidePin);
					Metadata.bHidePin = true;
				}
				else if (Name == STATIC_FNAME("ArrayPin"))
				{
					ensure(!Metadata.bArrayPin);
					Metadata.bArrayPin = true;
				}
				else
				{
					ensure(false);
				}
			}
		}

#if WITH_EDITOR
		if (Function->GetMetaData("AdvancedDisplay").Contains(Property.GetName()))
		{
			ensure(Metadata.Category.IsEmpty());
			Metadata.Category = "Advanced";
		}

		Metadata.Tooltip = FVoxelUtilities::GetPropertyTooltip(*Function, Property);

		if (IsFunctionInput(Property) &&
			!Type.IsWildcard() &&
			!Type.Is<FVoxelSeed>())
		{
			Metadata.DefaultValue = GVoxelSourceParser->GetPropertyDefault(Function, Property.GetFName());

			if (!Metadata.DefaultValue.IsEmpty())
			{
				// Sanitize the default value
				// Required for namespaced enums
				FVoxelPinValue Value(Type.GetPinDefaultValueType());

				if (Value.Is<FName>())
				{
					ensure(Metadata.DefaultValue.RemoveFromStart("\""));
					ensure(Metadata.DefaultValue.RemoveFromEnd("\""));
				}

				ensure(Value.ImportFromString(Metadata.DefaultValue));
				Metadata.DefaultValue = Value.ExportToString();
			}
		}
#endif

		EVoxelPinFlags Flags = EVoxelPinFlags::None;
		if (Type.IsBuffer() && !bNotTemplate)
		{
			Flags |= EVoxelPinFlags::TemplatePin;
		}

		CreatePin(
			Type,
			IsFunctionInput(Property),
			Property.GetFName(),
			Metadata,
			Flags);
	}
}