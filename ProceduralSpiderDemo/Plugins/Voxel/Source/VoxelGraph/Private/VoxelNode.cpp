// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNode.h"
#include "VoxelGraph.h"
#include "VoxelGraphContext.h"
#include "VoxelComputeNode.h"
#include "VoxelTemplateNode.h"
#include "VoxelSourceParser.h"
#include "Nodes/VoxelOutputNode.h"
#include "VoxelCompiledTerminalGraph.h"
#include "Nodes/VoxelPreviewDataNode.h"
#include "UObject/UObjectThreadContext.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelNode);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP(Game, 999)
{
	VOXEL_FUNCTION_COUNTER();

	for (UScriptStruct* Struct : GetDerivedStructs<FVoxelNode>())
	{
		if (Struct->HasMetaData(STATIC_FNAME("Abstract")))
		{
			continue;
		}

		if (Struct->HasMetaDataHierarchical(STATIC_FNAME("Internal")) &&
			!Struct->IsChildOf(StaticStructFast<FVoxelOutputNode>()))
		{
			continue;
		}

		if (Struct->GetPathName().StartsWith(TEXT("/Script/Voxel")))
		{
			const UStruct* Parent = Struct;
			while (Parent->GetSuperStruct() != FVoxelNode::StaticStruct())
			{
				Parent = Parent->GetSuperStruct();
			}

			if (Parent == FVoxelTemplateNode::StaticStruct())
			{
				ensure(Struct->GetStructCPPName().StartsWith("FVoxelTemplateNode_"));
			}
			else if (Parent == FVoxelComputeNode::StaticStruct())
			{
				ensure(Struct->GetStructCPPName().StartsWith("FVoxelComputeNode_"));
			}
			else if (Parent == FVoxelOutputNode::StaticStruct())
			{
				ensure(Struct->GetStructCPPName().StartsWith("FVoxelOutputNode_"));
			}
			else if (Parent == FVoxelPreviewDataNode::StaticStruct())
			{
				ensure(Struct->GetStructCPPName().StartsWith("FVoxelPreviewDataNode_"));
			}
			else
			{
				ensure(Struct->GetStructCPPName().StartsWith("FVoxelNode_"));
			}
		}

		TArray<FString> Array;
		Struct->GetName().ParseIntoArray(Array, TEXT("_"));

		const FString DefaultName = FName::NameToDisplayString(Struct->GetName(), false);
		const FString FixedName = FName::NameToDisplayString(Array.Last(), false);

		if (Struct->GetDisplayNameText().ToString() == DefaultName)
		{
			Struct->SetMetaData("DisplayName", *FixedName);
		}

		FString Tooltip = Struct->GetToolTipText().ToString();
		if (Tooltip.RemoveFromStart("Voxel Node ") ||
			Tooltip.RemoveFromStart("Voxel Output Node "))
		{
			Struct->SetMetaData("Tooltip", *Tooltip);
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelNode& FVoxelNode::operator=(const FVoxelNode& Other)
{
	VOXEL_SCOPE_COUNTER_FORMAT("FVoxelNode::operator= %s", *GetStruct()->GetName());

	check(GetStruct() == Other.GetStruct());
	ensure(HasEditorData());
	ensure(Other.HasEditorData());

	checkVoxelSlow(PrivateInputPinRefOffsets == Other.PrivateInputPinRefOffsets);
	checkVoxelSlow(PrivateOutputPinRefOffsets == Other.PrivateOutputPinRefOffsets);
	checkVoxelSlow(PrivateVariadicPinRefOffsets == Other.PrivateVariadicPinRefOffsets);

	if (!NodeGuid.IsValid())
	{
		NodeGuid = FGuid::NewGuid();
	}

#if WITH_EDITOR
	ExposedPins = Other.ExposedPins;
	ExposedPinValues = Other.ExposedPinValues;
#endif

	FlushDeferredPins();
	Other.FlushDeferredPins();

	PrivateNameToPinBackup.Reset();
	PrivateNameToPin.Reset();
	PrivateNameToVariadicPin.Reset();
	PrivatePinsOrder.Reset();

	SortOrderCounter = Other.SortOrderCounter;

	TVoxelArray<FDeferredPin> Pins = Other.PrivateNameToPinBackup.ValueArray();

	// Register variadic pins first
	Pins.Sort([](const FDeferredPin& A, const FDeferredPin& B)
	{
		return A.IsVariadicRoot() > B.IsVariadicRoot();
	});

	for (const FDeferredPin& Pin : Pins)
	{
		RegisterPin(Pin, false);
	}

	PrivatePinsOrder.Append(Other.PrivatePinsOrder);

	LoadSerializedData(Other.GetSerializedData());

	return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelNode::Serialize(FArchive& Ar)
{
	if (Ar.GetArchiveName() == TEXTVIEW("ReplaceObjectRef"))
	{
		// Needed to safely handle user enum deletion

		for (FDeferredPin& Pin : DeferredPins)
		{
			ConstCast(Pin).BaseType.SerializeObjects(Ar);
			Pin.ChildType.SerializeObjects(Ar);
		}
		for (auto& It : PrivateNameToPinBackup)
		{
			ConstCast(It.Value).BaseType.SerializeObjects(Ar);
			It.Value.ChildType.SerializeObjects(Ar);
		}
		for (const auto& It : PrivateNameToPin)
		{
			ConstCast(It.Value->BaseType).SerializeObjects(Ar);
			It.Value->ChildType.SerializeObjects(Ar);
		}
		for (const auto& It : PrivateNameToVariadicPin)
		{
			ConstCast(It.Value->PinTemplate).BaseType.SerializeObjects(Ar);
			ConstCast(It.Value->PinTemplate).ChildType.SerializeObjects(Ar);
		}
	}

	// Use regular serialization
	return false;
}

void FVoxelNode::AddStructReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_FUNCTION_COUNTER();

	// Needed to safely handle user enum deletion

	for (FDeferredPin& Pin : DeferredPins)
	{
		ConstCast(Pin).BaseType.AddStructReferencedObjects(Collector);
		Pin.ChildType.AddStructReferencedObjects(Collector);
	}
	for (auto& It : PrivateNameToPinBackup)
	{
		ConstCast(It.Value).BaseType.AddStructReferencedObjects(Collector);
		It.Value.ChildType.AddStructReferencedObjects(Collector);
	}
	for (const auto& It : PrivateNameToPin)
	{
		ConstCast(It.Value->BaseType).AddStructReferencedObjects(Collector);
		It.Value->ChildType.AddStructReferencedObjects(Collector);
	}
	for (const auto& It : PrivateNameToVariadicPin)
	{
		ConstCast(It.Value->PinTemplate).BaseType.AddStructReferencedObjects(Collector);
		ConstCast(It.Value->PinTemplate).ChildType.AddStructReferencedObjects(Collector);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
UStruct& FVoxelNode::GetMetadataContainer() const
{
	return *GetStruct();
}

FString FVoxelNode::GetCategory() const
{
	FString Category;
	ensureMsgf(
		GetMetadataContainer().GetStringMetaDataHierarchical(STATIC_FNAME("Category"), &Category) &&
		!Category.IsEmpty(),
		TEXT("%s is missing a Category"),
		*GetStruct()->GetStructCPPName());
	return Category;
}

FString FVoxelNode::GetDisplayName() const
{
	if (GetMetadataContainer().HasMetaData(STATIC_FNAME("Autocast")) &&
		ensure(PrivatePinsOrder.Num() > 0) &&
		ensure(FindPin(PrivatePinsOrder[0])))
	{
		ensure(!GetMetadataContainer().HasMetaData(STATIC_FNAME("DisplayName")));

		const FString FromType = FindPin(PrivatePinsOrder[0])->GetType().GetInnerType().ToString();
		const FString ToType = GetUniqueOutputPin().GetType().GetInnerType().ToString();
		{
			FString ExpectedName = FromType + "To" + ToType;
			ExpectedName.ReplaceInline(TEXT("Integer 64"), TEXT("Int64"));
			ExpectedName.ReplaceInline(TEXT("Integer"), TEXT("Int32"));
			ExpectedName.ReplaceInline(TEXT(" "), TEXT(""));
			ensure(GetMetadataContainer().GetName() == ExpectedName);
		}
		return "To " + ToType + " (" + FromType + ")";
	}

	return GetMetadataContainer().GetDisplayNameText().ToString();
}

FString FVoxelNode::GetTooltip() const
{
	if (GetMetadataContainer().HasMetaData(STATIC_FNAME("Autocast")) &&
		ensure(PrivatePinsOrder.Num() > 0) &&
		ensure(FindPin(PrivatePinsOrder[0])))
	{
		ensure(!GetMetadataContainer().HasMetaData(STATIC_FNAME("Tooltip")));
		ensure(!GetMetadataContainer().HasMetaData(STATIC_FNAME("ShortTooltip")));

		const FString FromType = FindPin(PrivatePinsOrder[0])->GetType().GetInnerType().ToString();
		const FString ToType = GetUniqueOutputPin().GetType().GetInnerType().ToString();

		return "Cast from " + FromType + " to " + ToType;
	}

	return FVoxelUtilities::GetStructTooltip(GetMetadataContainer());
}
#endif

bool FVoxelNode::CanPasteHere(
	const UVoxelGraph& Graph,
	const UVoxelTerminalGraph* TerminalGraph) const
{
#if WITH_EDITOR
	if (Graph.IsFunctionLibrary())
	{
		return true;
	}

	if (GetMetadataContainer().HasMetaDataHierarchical(STATIC_FNAME("EditorGraphOnly")))
	{
		if (TerminalGraph &&
			TerminalGraph != &Graph.GetEditorTerminalGraph())
		{
			return false;
		}
	}

	const FString GraphType = Graph.GetGraphTypeName();
	FString AllowListString;
	if (GetMetadataContainer().GetStringMetaDataHierarchical(STATIC_FNAME("AllowList"), &AllowListString))
	{
		TArray<FString> AllowList;
		AllowListString.ParseIntoArray(AllowList, TEXT(","));

		bool bIsAllowed = false;
		for (const FString& Type : AllowList)
		{
			if (GraphType == Type.TrimStartAndEnd().ToLower())
			{
				bIsAllowed = true;
				break;
			}
		}

		return bIsAllowed;
	}
#endif

	return true;
}

bool FVoxelNode::IsDesignedForGraph(const UVoxelGraph& Graph, FString* OutWarning) const
{
#if WITH_EDITOR
	if (Graph.IsFunctionLibrary())
	{
		return true;
	}

	FString AllowListString;
	if (!GetMetadataContainer().GetStringMetaDataHierarchical(STATIC_FNAME("SensitiveAllowList"), &AllowListString))
	{
		return true;
	}

	FString WarningType = "InvalidGraph";

	int32 Index = -1;
	if (AllowListString.FindChar(':', Index))
	{
		WarningType = AllowListString.RightChop(Index + 1);
		if (WarningType.IsEmpty())
		{
			WarningType = "InvalidGraph";
		}

		AllowListString = AllowListString.Left(Index);
	}

	const FString GraphType = Graph.GetGraphTypeName();

	TArray<FString> AllowList;
	AllowListString.ParseIntoArray(AllowList, TEXT(","));

	bool bIsAllowed = false;
	for (const FString& Type : AllowList)
	{
		if (GraphType == Type.TrimStartAndEnd().ToLower())
		{
			bIsAllowed = true;
			break;
		}
	}

	if (OutWarning)
	{
		*OutWarning = WarningType;
	}

	return bIsAllowed;
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint32 FVoxelNode::GetNodeHash() const
{
	ensure(HasEditorData());

	// Ensure properties are up to date
	ConstCast(this)->PreSerialize();

	TVoxelArray<uint64> Hashes;
	Hashes.Add(FVoxelUtilities::MurmurHash(GetStruct()));

	// Only hash our own properties, handling child structs properties is too messy/not required
	for (const FProperty& Property : GetStructProperties(StaticStructFast<FVoxelNode>()))
	{
		Hashes.Add(FVoxelUtilities::HashProperty(Property, Property.ContainerPtrToValuePtr<void>(this)));
	}

	return FVoxelUtilities::MurmurHashView(Hashes);
}

bool FVoxelNode::IsNodeIdentical(const FVoxelNode& Other) const
{
	if (GetStruct() != Other.GetStruct())
	{
		return false;
	}

	// Ensure properties are up to date
	// (needed for pin arrays)
	ConstCast(this)->PreSerialize();
	ConstCast(Other).PreSerialize();

	const UScriptStruct* Struct = GetStruct();

	if (Struct->StructFlags & STRUCT_IdenticalNative)
	{
		UScriptStruct::ICppStructOps* TheCppStructOps = Struct->GetCppStructOps();
		check(TheCppStructOps);
		bool bResult = false;
		if (TheCppStructOps->Identical(this, &Other, PPF_None, bResult))
		{
			return bResult;
		}
	}

	const FProperty& NodeGuidProperty = FindFPropertyChecked(FVoxelNode, NodeGuid);
	for (const FProperty& Property : GetStructProperties(Struct))
	{
		// Node GUID will always be different
		if (&Property == &NodeGuidProperty)
		{
			continue;
		}

		for (int32 Index = 0; Index < Property.ArrayDim; Index++)
		{
			if (!Property.Identical_InContainer(this, &Other, Index, PPF_None))
			{
				return false;
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode::PreSerialize()
{
	Super::PreSerialize();

	Version = FVersion::LatestVersion;

	SerializedDataProperty = GetSerializedData();
}

void FVoxelNode::PostSerialize()
{
	if (Version < FVersion::AddIsValid)
	{
		ensure(!SerializedDataProperty.bIsValid);
		SerializedDataProperty.bIsValid = true;
	}
	ensure(SerializedDataProperty.bIsValid);

	LoadSerializedData(SerializedDataProperty);
	SerializedDataProperty = {};
}

void FVoxelNode::PostSerialize(const FArchive& Ar)
{
	PostSerialize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
TSharedRef<FVoxelDefaultNodeDefinition> FVoxelNode::GetNodeDefinition()
{
	return MakeShared<FVoxelDefaultNodeDefinition>(*this);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (!ensure(Pin.IsPromotable()))
	{
		return {};
	}

	if (EnumHasAllFlags(Pin.Flags, EVoxelPinFlags::TemplatePin))
	{
		// GetPromotionTypes should be overridden by child nodes
		ensure(!Pin.GetType().IsWildcard());

		FVoxelPinTypeSet Types;
		Types.Add(Pin.GetType().GetInnerType());
		Types.Add(Pin.GetType().GetBufferType().WithBufferArray(Pin.Metadata.bArrayPin));
		return Types;
	}

	ensure(Pin.BaseType.IsWildcard());
	return FVoxelPinTypeSet::All();
}

void FVoxelNode::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	if (!ensure(Pin.IsPromotable()) ||
		!ensure(GetPromotionTypes(Pin).Contains(NewType)))
	{
		return;
	}

	if (EnumHasAllFlags(Pin.Flags, EVoxelPinFlags::TemplatePin))
	{
		ensure(NewType.GetInnerType() == Pin.GetType().GetInnerType());

		for (FVoxelPin& OtherPin : GetPins())
		{
			if (!EnumHasAllFlags(OtherPin.Flags, EVoxelPinFlags::TemplatePin))
			{
				continue;
			}

			if (NewType.IsBuffer())
			{
				OtherPin.SetType(OtherPin.GetType().GetBufferType());
			}
			else
			{
				OtherPin.SetType(OtherPin.GetType().GetInnerType());
			}
		}
	}

	ensure(Pin.BaseType.IsWildcard());
	Pin.SetType(NewType);
}

void FVoxelNode::PostReconstructNode()
{
	if (!PinToRename.IsNone())
	{
		OnRequestRenameDelayPtr = MakeSharedVoid();
		FVoxelUtilities::DelayedCall(MakeWeakPtrLambda(OnRequestRenameDelayPtr, [this]
		{
			OnRequestRenameDelayPtr = nullptr;
			PinToRename = {};
		}));
	}
}
#endif

void FVoxelNode::PromotePin_Runtime(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	// For convenience
	if (Pin.GetType() == NewType)
	{
		return;
	}

#if WITH_EDITOR
	ensure(GetPromotionTypes(Pin).Contains(NewType));
#endif

	if (!ensure(Pin.IsPromotable()))
	{
		return;
	}

	// If this fails, PromotePin_Runtime needs to be overriden
	ensure(Pin.GetType().IsWildcard() || EnumHasAllFlags(Pin.Flags, EVoxelPinFlags::TemplatePin));

	if (Pin.GetType().IsWildcard())
	{
		Pin.SetType(NewType);
	}

	if (!EnumHasAllFlags(Pin.Flags, EVoxelPinFlags::TemplatePin))
	{
		return;
	}

	ensure(NewType.GetInnerType() == Pin.GetType().GetInnerType());

	for (FVoxelPin& OtherPin : GetPins())
	{
		if (!EnumHasAllFlags(OtherPin.Flags, EVoxelPinFlags::TemplatePin))
		{
			continue;
		}

		if (NewType.IsBuffer())
		{
			OtherPin.SetType(OtherPin.GetType().GetBufferType());
		}
		else
		{
			OtherPin.SetType(OtherPin.GetType().GetInnerType());
		}
	}
}

bool FVoxelNode::AreTemplatePinsBuffers() const
{
	if (CachedAreTemplatePinsBuffers)
	{
		ensure(CachedAreTemplatePinsBuffers.IsSet());
		return CachedAreTemplatePinsBuffers.Get(false);
	}

	const TOptional<bool> Value = AreTemplatePinsBuffersImpl();
	ensure(Value.IsSet());
	return Value.Get(false);
}

TOptional<bool> FVoxelNode::AreTemplatePinsBuffersImpl() const
{
	VOXEL_FUNCTION_COUNTER();

	TOptional<bool> Result;
	for (const FVoxelPin& Pin : GetPins())
	{
		if (!EnumHasAllFlags(Pin.Flags, EVoxelPinFlags::TemplatePin))
		{
			continue;
		}

		if (!Result)
		{
			Result = Pin.GetType().IsBuffer();
		}
		else
		{
			ensure(*Result == Pin.GetType().IsBuffer());
		}
	}
	return Result;
}

#if WITH_EDITOR
void FVoxelNode::RequestRenamePin(const FName PinName)
{
	ensure(PinToRename.IsNone());
	PinToRename = PinName;
}

bool FVoxelNode::IsPinHidden(const FVoxelPin& Pin) const
{
	if (Pin.Metadata.bHidePin)
	{
		return true;
	}

	if (!Pin.Metadata.bShowInDetail)
	{
		return false;
	}

	return !ExposedPins.Contains(Pin.Name);
}

FString FVoxelNode::GetPinDefaultValue(const FVoxelPin& Pin) const
{
	if (!Pin.Metadata.bShowInDetail)
	{
		return Pin.Metadata.DefaultValue;
	}

	const FVoxelNodeExposedPinValue* ExposedPinValue = ExposedPinValues.FindByKey(Pin.Name);
	if (!ensure(ExposedPinValue))
	{
		return Pin.Metadata.DefaultValue;
	}
	return ExposedPinValue->Value.ExportToString();
}

void FVoxelNode::UpdatePropertyBoundDefaultValue(const FVoxelPin& Pin, const FVoxelPinValue& NewValue)
{
	if (!Pin.Metadata.bShowInDetail)
	{
		return;
	}

	if (FVoxelNodeExposedPinValue* PinValue = ExposedPinValues.FindByKey(Pin.Name))
	{
		if (PinValue->Value != NewValue)
		{
			PinValue->Value = NewValue;
			OnExposedPinsUpdated.Broadcast();
		}
	}
	else
	{
		ExposedPinValues.Add({ Pin.Name, NewValue });
		OnExposedPinsUpdated.Broadcast();
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const TVoxelMap<FName, TSharedPtr<FVoxelPin>>& FVoxelNode::GetNameToPin()
{
	FlushDeferredPins();
	return PrivateNameToPin;
}

const TVoxelMap<FName, TSharedPtr<const FVoxelPin>>& FVoxelNode::GetNameToPin() const
{
	return ConstCast(this)->GetNameToPin();
}

TSharedPtr<FVoxelPin> FVoxelNode::FindPin(const FName Name)
{
	return GetNameToPin().FindRef(Name);
}

TSharedPtr<const FVoxelPin> FVoxelNode::FindPin(const FName Name) const
{
	return GetNameToPin().FindRef(Name);
}

FVoxelPin& FVoxelNode::FindPinChecked(const FName Name)
{
	return *GetNameToPin().FindChecked(Name);
}

const FVoxelPin& FVoxelNode::FindPinChecked(const FName Name) const
{
	return *GetNameToPin().FindChecked(Name);
}

FVoxelPin& FVoxelNode::GetPin(const FPinRef& Pin)
{
	return *GetNameToPin().FindChecked(Pin.GetName());
}

const FVoxelPin& FVoxelNode::GetPin(const FPinRef& Pin) const
{
	return *GetNameToPin().FindChecked(Pin.GetName());
}

FVoxelPin& FVoxelNode::GetUniqueInputPin()
{
	FVoxelPin* InputPin = nullptr;
	for (FVoxelPin& Pin : GetPins())
	{
		if (Pin.bIsInput)
		{
			check(!InputPin);
			InputPin = &Pin;
		}
	}
	check(InputPin);

	return *InputPin;
}

FVoxelPin& FVoxelNode::GetUniqueOutputPin()
{
	FVoxelPin* OutputPin = nullptr;
	for (FVoxelPin& Pin : GetPins())
	{
		if (!Pin.bIsInput)
		{
			check(!OutputPin);
			OutputPin = &Pin;
		}
	}
	check(OutputPin);

	return *OutputPin;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FName FVoxelNode::Variadic_AddPin(const FName VariadicPinName, FName PinName)
{
	FlushDeferredPins();

	const TSharedPtr<FVariadicPin> VariadicPin = PrivateNameToVariadicPin.FindRef(VariadicPinName);
	if (!ensure(VariadicPin))
	{
		return {};
	}

	if (PinName.IsNone())
	{
		PinName = VariadicPinName + TEXTVIEW("_0");

		while (
			PrivateNameToPin.Contains(PinName) ||
			PrivateNameToVariadicPin.Contains(PinName))
		{
			PinName.SetNumber(PinName.GetNumber() + 1);
		}
	}

	if (!ensure(!PrivateNameToPin.Contains(PinName)) ||
		!ensure(!PrivateNameToVariadicPin.Contains(PinName)))
	{
		return {};
	}

	FDeferredPin Pin = VariadicPin->PinTemplate;
	Pin.Name = PinName;
	RegisterPin(Pin);

	FixupVariadicPinNames(VariadicPinName);

	return PinName;
}

FName FVoxelNode::Variadic_InsertPin(const FName VariadicPinName, const int32 Position)
{
	FlushDeferredPins();

	const TSharedPtr<FVariadicPin> VariadicPin = PrivateNameToVariadicPin.FindRef(VariadicPinName);
	if (!ensure(VariadicPin))
	{
		return {};
	}

	FName PinName = VariadicPinName + TEXTVIEW("_0");
	while (
		PrivateNameToPin.Contains(PinName) ||
		PrivateNameToVariadicPin.Contains(PinName))
	{
		PinName.SetNumber(PinName.GetNumber() + 1);
	}

	FDeferredPin Pin = VariadicPin->PinTemplate;
	Pin.Name = PinName;
	RegisterPin(Pin);
	SortPins();

	for (int32 Index = Position; Index < VariadicPin->Pins.Num() - 1; Index++)
	{
		VariadicPin->Pins.Swap(Index, VariadicPin->Pins.Num() - 1);
	}

	FixupVariadicPinNames(VariadicPinName);

	return PinName;
}

TVoxelArray<FName> FVoxelNode::GetVariadicPinPinNames(const FVariadicPinRef_Input& VariadicPin) const
{
	FlushDeferredPins();
	return PrivateNameToVariadicPin[VariadicPin.GetName()]->Pins;
}

void FVoxelNode::FixupVariadicPinNames(const FName VariadicPinName)
{
	FlushDeferredPins();

	const TSharedPtr<FVariadicPin> VariadicPin = PrivateNameToVariadicPin.FindRef(VariadicPinName);
	if (!ensure(VariadicPin))
	{
		return;
	}

	for (int32 Index = 0; Index < VariadicPin->Pins.Num(); Index++)
	{
		const FName ArrayPinName = VariadicPin->Pins[Index];
		const TSharedPtr<FVoxelPin> ArrayPin = PrivateNameToPin.FindRef(ArrayPinName);
		if (!ensure(ArrayPin))
		{
			continue;
		}

		ConstCast(ArrayPin->SortOrder) = VariadicPin->PinTemplate.SortOrder + Index / 100.f;

#if WITH_EDITOR
		ConstCast(ArrayPin->Metadata.DisplayName) = FName::NameToDisplayString(VariadicPinName.ToString(), false) + " " + FString::FromInt(Index);
#endif
	}

	SortPins();
}

FName FVoxelNode::CreatePin(
	const FVoxelPinType& Type,
	const bool bIsInput,
	const FName Name,
	const FVoxelPinMetadata& Metadata,
	const EVoxelPinFlags Flags,
	const int32 MinArrayNum)
{
	if (bIsInput)
	{
#if WITH_EDITOR
		// Loading objects during serialization is not allowed
		if (!FUObjectThreadContext::Get().GetSerializeContext())
		{
			ensure(
				Metadata.DefaultValue.IsEmpty() ||
				Type.IsWildcard() ||
				FVoxelPinValue(Type.GetPinDefaultValueType()).ImportFromString(Metadata.DefaultValue));
		}
#endif

		ensure(!Metadata.bNoCache);
	}
	else
	{
#if WITH_EDITOR
		ensure(Metadata.DefaultValue.IsEmpty());
#endif

		ensure(!Metadata.bPositionPin);
		ensure(!Metadata.bSplineKeyPin);
		ensure(!Metadata.bDisplayLast);
		ensure(!Metadata.bNoDefault);
		ensure(!Metadata.bHideDefault);
		ensure(!Metadata.bShowInDetail);
	}

	FVoxelPinType BaseType = Type;
	if (Metadata.bArrayPin)
	{
		BaseType = BaseType.WithBufferArray(true);
	}

	const FVoxelPinType ChildType = BaseType;

	if (EnumHasAnyFlags(Flags, EVoxelPinFlags::TemplatePin))
	{
		BaseType = FVoxelPinType::MakeWildcard();
	}

	RegisterPin(FDeferredPin
	{
		{},
		MinArrayNum,
		Name,
		bIsInput,
		0,
		Flags,
		BaseType,
		ChildType,
		Metadata
	});

	if (Metadata.bDisplayLast)
	{
		PrivatePinsOrder.Add(Name);
		DisplayLastPins++;
	}
	else
	{
		PrivatePinsOrder.Add(Name);
		if (DisplayLastPins > 0)
		{
			for (int32 Index = PrivatePinsOrder.Num() - DisplayLastPins - 1; Index < PrivatePinsOrder.Num() - 1; Index++)
			{
				PrivatePinsOrder.Swap(Index, PrivatePinsOrder.Num() - 1);
			}
		}
	}

	return Name;
}

void FVoxelNode::RemovePin(const FName Name)
{
	FlushDeferredPins();

	const TSharedPtr<FVoxelPin> Pin = FindPin(Name);
	if (ensure(Pin) &&
		!Pin->VariadicPinName.IsNone() &&
		ensure(PrivateNameToVariadicPin.Contains(Pin->VariadicPinName)))
	{
		ensure(PrivateNameToVariadicPin[Pin->VariadicPinName]->Pins.Remove(Name));
	}

	ensure(PrivateNameToPinBackup.Remove(Name));
	ensure(PrivateNameToPin.Remove(Name));
	PrivatePinsOrder.Remove(Name);

#if WITH_EDITOR
	if (Pin->Metadata.bShowInDetail)
	{
		OnExposedPinsUpdated.Broadcast();
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelNode::FPinRef_Input FVoxelNode::CreateInputPin(
	const FVoxelPinType& Type,
	const FName Name,
	const FVoxelPinMetadata& Metadata,
	const EVoxelPinFlags Flags)
{
	return FPinRef_Input(CreatePin(
		Type,
		true,
		Name,
		Metadata,
		Flags));
}

FVoxelNode::FPinRef_Output FVoxelNode::CreateOutputPin(
	const FVoxelPinType& Type,
	const FName Name,
	const FVoxelPinMetadata& Metadata,
	const EVoxelPinFlags Flags)
{
	return FPinRef_Output(CreatePin(
		Type,
		false,
		Name,
		Metadata,
		Flags));
}

FVoxelNode::FVariadicPinRef_Input FVoxelNode::CreateVariadicInputPin(
	const FVoxelPinType& Type,
	const FName Name,
	const FVoxelPinMetadata& Metadata,
	const int32 MinNum,
	const EVoxelPinFlags Flags)
{
	return FVariadicPinRef_Input(CreatePin(
		Type,
		true,
		Name,
		Metadata,
		Flags | EVoxelPinFlags::VariadicPin,
		MinNum));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode::FlushDeferredPinsImpl()
{
	ensure(bIsDeferringPins);
	bIsDeferringPins = false;

	for (const FDeferredPin& Pin : DeferredPins)
	{
		RegisterPin(Pin);
	}
	DeferredPins.Empty();
}

void FVoxelNode::RegisterPin(FDeferredPin Pin, const bool bApplyMinNum)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(!Pin.Name.IsNone());
	ensure(Pin.BaseType.IsValid());

	if (Pin.VariadicPinName.IsNone() &&
		Pin.SortOrder == 0)
	{
		Pin.SortOrder = SortOrderCounter++;
	}

	if (Pin.Metadata.bDisplayLast)
	{
		Pin.SortOrder += 10000.f;
	}

	if (bIsDeferringPins)
	{
		DeferredPins.Add(Pin);
		return;
	}

	PrivateNameToPinBackup.Add_EnsureNew(Pin.Name, Pin);

	const FString PinName = Pin.Name.ToString();

#if WITH_EDITOR
	if (!Pin.Metadata.Tooltip.IsSet())
	{
		Pin.Metadata.Tooltip = MakeAttributeLambda([Struct = GetStruct(), Name = Pin.VariadicPinName.IsNone() ? Pin.Name : Pin.VariadicPinName]
		{
			return GVoxelSourceParser->GetPinTooltip(Struct, Name);
		});
	}

	if (Pin.Metadata.DisplayName.IsEmpty())
	{
		Pin.Metadata.DisplayName = FName::NameToDisplayString(PinName, PinName.StartsWith("b", ESearchCase::CaseSensitive));
		Pin.Metadata.DisplayName.RemoveFromStart("Out ");
	}

	if (Pin.Metadata.bShowInDetail)
	{
		OnExposedPinsUpdated.Broadcast();

		if (!ExposedPinValues.FindByKey(Pin.Name))
		{
			FVoxelNodeExposedPinValue ExposedPinValue;
			ExposedPinValue.Name = Pin.Name;
			ExposedPinValue.Value = FVoxelPinValue(Pin.ChildType.GetPinDefaultValueType());

			if (!Pin.Metadata.DefaultValue.IsEmpty())
			{
				ensure(ExposedPinValue.Value.ImportFromString(Pin.Metadata.DefaultValue));
			}

			ExposedPinValues.Add(ExposedPinValue);
		}
	}
#endif

	if (EnumHasAnyFlags(Pin.Flags, EVoxelPinFlags::VariadicPin))
	{
		ensure(Pin.VariadicPinName.IsNone());

		FDeferredPin PinTemplate = Pin;
		PinTemplate.VariadicPinName = Pin.Name;
		EnumRemoveFlags(PinTemplate.Flags, EVoxelPinFlags::VariadicPin);

		PrivateNameToVariadicPin.Add_EnsureNew(Pin.Name, MakeShared<FVariadicPin>(PinTemplate));

		if (bApplyMinNum)
		{
			for (int32 Index = 0; Index < Pin.MinVariadicNum; Index++)
			{
				Variadic_AddPin(Pin.Name);
			}
		}
	}
	else
	{
		if (!Pin.VariadicPinName.IsNone() &&
			ensure(PrivateNameToVariadicPin.Contains(Pin.VariadicPinName)))
		{
			FVariadicPin& VariadicPin = *PrivateNameToVariadicPin[Pin.VariadicPinName];
#if WITH_EDITOR
			Pin.Metadata.Category = Pin.VariadicPinName.ToString();
#endif
			VariadicPin.Pins.Add(Pin.Name);
		}

		PrivateNameToPin.Add_EnsureNew(Pin.Name, MakeSharedCopy(FVoxelPin(
			Pin.Name,
			Pin.bIsInput,
			Pin.SortOrder,
			Pin.VariadicPinName,
			Pin.Flags,
			Pin.BaseType,
			Pin.ChildType,
			Pin.Metadata)));
	}

	SortPins();
}

void FVoxelNode::SortPins()
{
	PrivateNameToPin.ValueSort([](const TSharedPtr<FVoxelPin>& A, const TSharedPtr<FVoxelPin>& B)
	{
		if (A->bIsInput != B->bIsInput)
		{
			return A->bIsInput > B->bIsInput;
		}

		return A->SortOrder < B->SortOrder;
	});
}

void FVoxelNode::SortVariadicPinNames(const FName VariadicPinName)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedPtr<FVariadicPin> VariadicPin = PrivateNameToVariadicPin.FindRef(VariadicPinName);
	if (!VariadicPin)
	{
		return;
	}

	TArray<int32> SortOrders;
	TArray<TSharedPtr<FVoxelPin>> AffectedPins;
	for (const FName PinName : VariadicPin->Pins)
	{
		if (const TSharedPtr<FVoxelPin> Pin = PrivateNameToPin.FindRef(PinName))
		{
			SortOrders.Add(Pin->SortOrder);
		}
	}

	SortOrders.Sort();
	for (int32 Index = 0; Index < AffectedPins.Num(); Index++)
	{
		ConstCast(AffectedPins[Index]->SortOrder) = SortOrders[Index];
	}

	SortPins();
}

FVoxelNodeSerializedData FVoxelNode::GetSerializedData() const
{
	FlushDeferredPins();

	FVoxelNodeSerializedData SerializedData;
	SerializedData.bIsValid = true;

	for (const auto& It : PrivateNameToPin)
	{
		const FVoxelPin& Pin = *It.Value;
		if (!Pin.IsPromotable())
		{
			continue;
		}

		SerializedData.NameToPinType.Add(Pin.Name, Pin.GetType());
	}

	for (const auto& It : PrivateNameToVariadicPin)
	{
		FVoxelNodeVariadicPinSerializedData VariadicPinSerializedData;
		VariadicPinSerializedData.PinNames = It.Value->Pins;
		SerializedData.VariadicPinNameToSerializedData.Add(It.Key, VariadicPinSerializedData);
	}

#if WITH_EDITOR
	SerializedData.ExposedPins = ExposedPins;
	SerializedData.ExposedPinsValues = ExposedPinValues;
#endif

	return SerializedData;
}

void FVoxelNode::LoadSerializedData(const FVoxelNodeSerializedData& SerializedData)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(SerializedData.bIsValid);

#if WITH_EDITOR
	ExposedPins = SerializedData.ExposedPins;
	ExposedPinValues = SerializedData.ExposedPinsValues;
#endif

	FlushDeferredPins();

	for (const auto& It : PrivateNameToVariadicPin)
	{
		FVariadicPin& VariadicPin = *It.Value;

		const TVoxelArray<FName> Pins = VariadicPin.Pins;
		for (const FName Name : Pins)
		{
			RemovePin(Name);
		}
		ensure(VariadicPin.Pins.Num() == 0);
	}

	for (const auto& It : SerializedData.VariadicPinNameToSerializedData)
	{
		const TSharedPtr<FVariadicPin> VariadicPin = PrivateNameToVariadicPin.FindRef(It.Key);
		if (!ensureVoxelSlow(VariadicPin))
		{
			continue;
		}

		for (const FName Name : It.Value.PinNames)
		{
			Variadic_AddPin(It.Key, Name);
		}
	}

	// Ensure MinNum is applied
	for (const auto& It : PrivateNameToVariadicPin)
	{
		FVariadicPin& VariadicPin = *It.Value;

		for (int32 Index = VariadicPin.Pins.Num(); Index < VariadicPin.PinTemplate.MinVariadicNum; Index++)
		{
			Variadic_AddPin(It.Key);
		}
	}

	TVoxelInlineArray<TSharedPtr<FVoxelPin>, 16> PinsToFix;

	// First set the type of all our pins
	for (const auto& It : SerializedData.NameToPinType)
	{
		const TSharedPtr<FVoxelPin> Pin = PrivateNameToPin.FindRef(It.Key);
		if (!Pin ||
			!Pin->IsPromotable())
		{
			continue;
		}

		if (!It.Value.IsValid())
		{
			// Old type that was removed
			continue;
		}

		Pin->SetType(It.Value);
		PinsToFix.Add(Pin);
	}

#if WITH_EDITOR
	// Then clear the types of any that don't match GetPromotionTypes
	// We need two loops because GetPromotionTypes might query the types of other pins
	for (const TSharedPtr<FVoxelPin>& Pin : PinsToFix)
	{
		if (Pin->GetType().IsWildcard())
		{
			// Always valid
			continue;
		}

		if (ensureVoxelSlow(GetPromotionTypes(*Pin).Contains(Pin->GetType())))
		{
			continue;
		}

		// Clear type
		Pin->SetType(Pin->BaseType);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode::InitializeNodeRuntime(
	const int32 NodeIndex,
	const FVoxelGraphMergedNodeRef& NodeRef,
	TVoxelMap<FName, FPinRef_Input*>& OutNameToInputPinRefs,
	TVoxelMap<FName, FPinRef_Output*>& OutNameToOutputPinRefs)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(!NodeRef.Node.NodeId.IsNone());
	ensure(!NodeRef.Node.IsDeleted());

	FlushDeferredPins();

	ensure(PrivateNodeIndex == -1);
	PrivateNodeIndex = NodeIndex;

	ensure(PrivateNodeRef.Node.IsExplicitlyNull());
	PrivateNodeRef = NodeRef;

	ensure(!CachedAreTemplatePinsBuffers);
	CachedAreTemplatePinsBuffers = AreTemplatePinsBuffersImpl();

	{
		struct FInitializerImpl : FInitializer
		{
			TVoxelUniqueFunction<void()> ClearPinRefsImpl;
			TVoxelUniqueFunction<void(FPinRef_Input& PinRef)> InitializeInputPinRef;
			TVoxelUniqueFunction<void(FPinRef_Output& PinRef)> InitializeOutputPinRef;

			virtual void ClearPinRefs() override
			{
				ClearPinRefsImpl();
			}
			virtual void InitializePinRef(FPinRef_Input& PinRef) override
			{
				InitializeInputPinRef(PinRef);
			}
			virtual void InitializePinRef(FPinRef_Output& PinRef) override
			{
				InitializeOutputPinRef(PinRef);
			}
		};

		FInitializerImpl Initializer;
		Initializer.ClearPinRefsImpl = [&]
		{
			OutNameToInputPinRefs.Empty();
			OutNameToOutputPinRefs.Empty();
		};
		Initializer.InitializeInputPinRef = [&](FPinRef_Input& PinRef)
		{
			PinRef.Metadata = FVoxelPinRuntimeMetadata(GetPin(PinRef).Metadata);
			OutNameToInputPinRefs.Add_EnsureNew(PinRef.GetName(), &PinRef);
		};
		Initializer.InitializeOutputPinRef = [&](FPinRef_Output& PinRef)
		{
			PinRef.Metadata = FVoxelPinRuntimeMetadata(GetPin(PinRef).Metadata);
			OutNameToOutputPinRefs.Add_EnsureNew(PinRef.GetName(), &PinRef);
		};

		for (const int64 Offset : PrivateInputPinRefOffsets)
		{
			FPinRef_Input* PinRef = reinterpret_cast<FPinRef_Input*>(reinterpret_cast<uint8*>(this) + Offset);
			checkVoxelSlow(PinRef->Magic == 0xDEADBEEF);
			Initializer.InitializePinRef(*PinRef);
		}
		for (const int64 Offset : PrivateOutputPinRefOffsets)
		{
			FPinRef_Output* PinRef = reinterpret_cast<FPinRef_Output*>(reinterpret_cast<uint8*>(this) + Offset);
			checkVoxelSlow(PinRef->Magic == 0xDEADBEEF);
			Initializer.InitializePinRef(*PinRef);
		}

		for (const int64 Offset : PrivateVariadicPinRefOffsets)
		{
			FVariadicPinRef_Input& VariadicPinRef = *reinterpret_cast<FVariadicPinRef_Input*>(reinterpret_cast<uint8*>(this) + Offset);
			checkVoxelSlow(VariadicPinRef.Magic == 0xDEADBEEF);

			const TVoxelArray<FName> PinNames = GetVariadicPinPinNames(VariadicPinRef);

			VariadicPinRef.PinRefs.Reserve(PinNames.Num());

			for (const FName PinName : PinNames)
			{
				FPinRef_Input& PinRef = VariadicPinRef.PinRefs.Emplace_GetRef_EnsureNoGrow(FPinRef_Input(PinName));
				Initializer.InitializePinRef(PinRef);
			}
		}

		Initialize(Initializer);
	}

	PostInitialize(OutNameToInputPinRefs, OutNameToOutputPinRefs);

	PrivateOutputPinRefs = OutNameToOutputPinRefs.ValueArray();

	ensure(!bEditorDataRemoved);
	bEditorDataRemoved = true;

	DeferredPins.Empty();
	PrivateInputPinRefOffsets.Empty();
	PrivateOutputPinRefOffsets.Empty();
	PrivateVariadicPinRefOffsets.Empty();
	PrivateNameToPinBackup.Empty();
	PrivatePinsOrder.Empty();
	PrivateNameToPin.Empty();
	PrivateNameToVariadicPin.Empty();

#if WITH_EDITOR
	ExposedPinValues.Empty();
	ExposedPins.Empty();
#endif

	SerializedDataProperty = {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode::ComputeIfNeeded(
	const FVoxelGraphQuery Query,
	const int32 PinIndex) const
{
	checkVoxelSlow(Query->CompiledTerminalGraph.OwnsNode(this));

	if (Query->IsNodeComputed(PrivateNodeIndex))
	{
		return;
	}
	Query->SetNodeComputed(PrivateNodeIndex);

	VOXEL_SCOPE_COUNTER_FNAME(GetStruct()->GetFName());

#if WITH_EDITOR
	FVoxelGraphCallstack& Callstack = Query->Context.Callstacks_EditorOnly.Emplace_GetRef();
	Callstack.Node = this;
	Callstack.Parent = Query.GetCallstack();

	const FVoxelGraphQuery NewQuery(Query.GetImpl(), &Callstack);
	Query->Context.CurrentCallstack_EditorOnly = NewQuery.GetCallstack();
#else
	const FVoxelGraphQuery NewQuery = Query;
#endif

	Compute(NewQuery);

	Query.AddTask([this, Query]
	{
		EnsureOutputValuesAreSet(Query);
	});
}

void FVoxelNode::EnsureOutputValuesAreSet(const FVoxelGraphQuery Query) const
{
	for (const FPinRef_Output* PinRef : PrivateOutputPinRefs)
	{
		if (PinRef->PinIndex == -1)
		{
			continue;
		}

		FVoxelRuntimePinValue& Value = Query->GetPinValue(PinRef->PinIndex);
		if (Value.IsValid())
		{
			continue;
		}

		Value = FVoxelRuntimePinValue(PinRef->Type);
	}
}