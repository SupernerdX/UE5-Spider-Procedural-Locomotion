// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphNode_Struct.h"

#include "VoxelEdGraph.h"
#include "VoxelGraphVisuals.h"
#include "VoxelGraphToolkit.h"
#include "VoxelGraphMigration.h"
#include "VoxelTemplateNode.h"
#include "VoxelTerminalGraph.h"
#include "Nodes/VoxelNode_UFunction.h"

#include "UnrealEdGlobals.h"
#include "SourceCodeNavigation.h"
#include "Editor/UnrealEdEngine.h"
#include "Preferences/UnrealEdOptions.h"

void UVoxelGraphNode_Struct::AllocateDefaultPins()
{
	ON_SCOPE_EXIT
	{
		Super::AllocateDefaultPins();
	};

	OnNodeRecreateRequested = MakeSharedVoid();

	if (!Struct.IsValid())
	{
		return;
	}

	if (!Struct->NodeGuid.IsValid())
	{
		Struct->NodeGuid = FGuid::NewGuid();
	}

	Struct->OnNodeRecreateRequested.Add(MakeWeakPtrDelegate(OnNodeRecreateRequested, MakeWeakObjectPtrLambda(this, [this]
	{
		ReconstructNode();
	})));

	CachedName = Struct->GetDisplayName();

	for (const FVoxelPin& Pin : Struct->GetPins())
	{
		UEdGraphPin* GraphPin = CreatePin(
			Pin.bIsInput ? EGPD_Input : EGPD_Output,
			Pin.GetType().GetEdGraphPinType(),
			Pin.Name);

		GraphPin->PinFriendlyName = FText::FromString(Pin.Metadata.DisplayName);
		if (GraphPin->PinFriendlyName.IsEmpty())
		{
			GraphPin->PinFriendlyName = INVTEXT(" ");
		}

		GraphPin->PinToolTip = Pin.Metadata.Tooltip.Get();

		if (Pin.Metadata.IsOptional())
		{
			GraphPin->PinToolTip += "\n\nThis pin is optional and will be automatically set if unplugged";
		}

		GraphPin->bHidden = Struct->IsPinHidden(Pin);

		InitializeDefaultValue(*Struct, Pin, *GraphPin);
	}

	// If we only have a single pin hide its name
	if (Pins.Num() == 1 &&
		Pins[0]->Direction == EGPD_Output)
	{
		Pins[0]->PinFriendlyName = INVTEXT(" ");
	}
}

void UVoxelGraphNode_Struct::PrepareForCopying()
{
	Super::PrepareForCopying();

	if (!Struct.IsValid())
	{
		return;
	}

	Struct->PreSerialize();
}

bool UVoxelGraphNode_Struct::CanPasteHere(const UEdGraph* TargetGraph) const
{
	if (!Super::CanPasteHere(TargetGraph))
	{
		return false;
	}

	if (!Struct.IsValid())
	{
		return false;
	}

	const UVoxelEdGraph* VoxelGraph = Cast<UVoxelEdGraph>(TargetGraph);
	if (!VoxelGraph)
	{
		return false;
	}

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = VoxelGraph->GetGraphToolkit();
	if (!Toolkit ||
		!Toolkit->Asset)
	{
		return false;
	}

	return Struct->CanPasteHere(*Toolkit->Asset, VoxelGraph->GetTypedOuter<UVoxelTerminalGraph>());
}

void UVoxelGraphNode_Struct::PostPasteNode()
{
	Super::PostPasteNode();

	if (!Struct.IsValid())
	{
		return;
	}

	Struct->NodeGuid = FGuid::NewGuid();
	Struct->PostSerialize();
}

bool UVoxelGraphNode_Struct::CanDuplicateNode() const
{
	if (!Struct.IsValid())
	{
		return true;
	}

	return Struct->CanBeDuplicated();
}

bool UVoxelGraphNode_Struct::CanUserDeleteNode() const
{
	if (!Struct.IsValid())
	{
		return true;
	}

	return Struct->CanBeDeleted();
}

FText UVoxelGraphNode_Struct::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (!Struct.IsValid())
	{
		return FText::FromString(CachedName);
	}

	if (Struct->GetMetadataContainer().HasMetaData(STATIC_FNAME("Autocast")))
	{
		return INVTEXT("->");
	}

	const FText CompactTitle = Struct->GetMetadataContainer().GetMetaDataText(FBlueprintMetadata::MD_CompactNodeTitle);
	if (!CompactTitle.IsEmpty())
	{
		return CompactTitle;
	}

	return FText::FromString(Struct->GetDisplayName());
}

FLinearColor UVoxelGraphNode_Struct::GetNodeTitleColor() const
{
	if (Struct.IsValid() &&
		Struct->GetMetadataContainer().HasMetaDataHierarchical(STATIC_FNAME("NodeColor")))
	{
		return FVoxelGraphVisuals::GetNodeColor(GetStringMetaDataHierarchical(&Struct->GetMetadataContainer(), STATIC_FNAME("NodeColor")));
	}

	if (Struct.IsValid() &&
		Struct->IsPureNode())
	{
		return GetDefault<UGraphEditorSettings>()->PureFunctionCallNodeTitleColor;
	}

	return GetDefault<UGraphEditorSettings>()->FunctionCallNodeTitleColor;
}

FText UVoxelGraphNode_Struct::GetTooltipText() const
{
	if (!Struct.IsValid())
	{
		return {};
	}

	return FText::FromString(Struct->GetTooltip());
}

FSlateIcon UVoxelGraphNode_Struct::GetIconAndTint(FLinearColor& OutColor) const
{
	FSlateIcon Icon("EditorStyle", "Kismet.AllClasses.FunctionIcon");
	OutColor = GetDefault<UGraphEditorSettings>()->FunctionCallNodeTitleColor;

	if (!Struct.IsValid())
	{
		return Icon;
	}

	if (Struct->IsPureNode())
	{
		OutColor = GetDefault<UGraphEditorSettings>()->PureFunctionCallNodeTitleColor;
	}

	if (Struct->GetMetadataContainer().HasMetaDataHierarchical("NativeMakeFunc"))
	{
		Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.MakeStruct_16x");
		OutColor = FLinearColor::White;
	}

	if (Struct->GetMetadataContainer().HasMetaDataHierarchical("NativeBreakFunc"))
	{
		Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.BreakStruct_16x");
		OutColor = FLinearColor::White;
	}

	if (Struct->GetMetadataContainer().HasMetaDataHierarchical("NodeIcon"))
	{
		Icon = FVoxelGraphVisuals::GetNodeIcon(GetStringMetaDataHierarchical(&Struct->GetMetadataContainer(), "NodeIcon"));
	}

	if (Struct->GetMetadataContainer().HasMetaDataHierarchical("NodeIconColor"))
	{
		OutColor = FVoxelGraphVisuals::GetNodeColor(GetStringMetaDataHierarchical(&Struct->GetMetadataContainer(), "NodeIconColor"));
	}

	return Icon;
}

bool UVoxelGraphNode_Struct::IsCompact() const
{
	if (!Struct.IsValid())
	{
		return {};
	}

	return
		Struct->GetMetadataContainer().HasMetaData(STATIC_FNAME("Autocast")) ||
		Struct->GetMetadataContainer().HasMetaData(FBlueprintMetadata::MD_CompactNodeTitle);
}

bool UVoxelGraphNode_Struct::GetOverlayInfo(FString& Type, FString& Tooltip, FString& Color)
{
	if (!Struct.IsValid())
	{
		return false;
	}

	const UStruct& MetadataContainer = Struct->GetMetadataContainer();
	if (!MetadataContainer.HasMetaDataHierarchical("OverlayTooltip") &&
		!MetadataContainer.HasMetaDataHierarchical("OverlayType") &&
		!MetadataContainer.HasMetaDataHierarchical("OverlayColor"))
	{
		return false;
	}

	if (MetadataContainer.HasMetaDataHierarchical("OverlayTooltip"))
	{
		Tooltip = GetStringMetaDataHierarchical(&MetadataContainer, "OverlayTooltip");
	}

	if (MetadataContainer.HasMetaDataHierarchical("OverlayType"))
	{
		Type = GetStringMetaDataHierarchical(&MetadataContainer, "OverlayType");
	}
	else
	{
		Type = "Warning";
	}

	if (MetadataContainer.HasMetaDataHierarchical("OverlayColor"))
	{
		Color = GetStringMetaDataHierarchical(&MetadataContainer, "OverlayColor");
	}

	return true;
}

bool UVoxelGraphNode_Struct::GetGraphWarningInfo(const UVoxelGraph& Graph, FString& OutWarning)
{
	if (!Struct.IsValid())
	{
		return false;
	}

	return !Struct->IsDesignedForGraph(Graph, &OutWarning);
}

bool UVoxelGraphNode_Struct::ShowAsPromotableWildcard(const UEdGraphPin& Pin) const
{
	if (!Struct.IsValid() ||
		Pin.ParentPin ||
		Pin.bOrphanedPin)
	{
		return false;
	}

	const TSharedPtr<const FVoxelPin> StructPin = Struct->FindPin(Pin.PinName);
	if (!ensure(StructPin))
	{
		return false;
	}

	if (!StructPin->IsPromotable() ||
		!Struct->ShowPromotablePinsAsWildcards())
	{
		return false;
	}

	return true;
}

bool UVoxelGraphNode_Struct::IsPinOptional(const UEdGraphPin& Pin) const
{
	if (!Struct.IsValid() ||
		Pin.ParentPin ||
		Pin.bOrphanedPin)
	{
		return false;
	}

	const TSharedPtr<const FVoxelPin> StructPin = Struct->FindPin(Pin.PinName);
	if (!ensureVoxelSlow(StructPin))
	{
		return {};
	}

	return StructPin->Metadata.IsOptional();
}

bool UVoxelGraphNode_Struct::ShouldHidePinDefaultValue(const UEdGraphPin& Pin) const
{
	if (!Struct.IsValid() ||
		Pin.ParentPin ||
		Pin.bOrphanedPin)
	{
		return false;
	}

	const TSharedPtr<const FVoxelPin> StructPin = Struct->FindPin(Pin.PinName);
	if (!ensureVoxelSlow(StructPin))
	{
		return {};
	}

	return StructPin->Metadata.IsOptional();
}

FName UVoxelGraphNode_Struct::GetPinCategory(const UEdGraphPin& Pin) const
{
	if (!Struct.IsValid())
	{
		return {};
	}

	const TSharedPtr<const FVoxelPin> StructPin = Struct->FindPin(Pin.PinName);
	if (!ensure(StructPin))
	{
		return {};
	}

	return *StructPin->Metadata.Category;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<IVoxelNodeDefinition> UVoxelGraphNode_Struct::GetNodeDefinition()
{
	if (!Struct)
	{
		return MakeShared<IVoxelNodeDefinition>();
	}

	const TSharedRef<FVoxelDefaultNodeDefinition> NodeDefinition = Struct->GetNodeDefinition();
	NodeDefinition->Initialize(*this);
	return MakeShared<FVoxelNodeDefinition_Struct>(*this, NodeDefinition);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelGraphNode_Struct::CanRemovePin_ContextMenu(const UEdGraphPin& Pin) const
{
	if (!Struct.IsValid() ||
		Pin.ParentPin ||
		Pin.bOrphanedPin)
	{
		return false;
	}

	return ConstCast(this)->GetNodeDefinition()->CanRemoveSelectedPin(Pin.PinName);
}

void UVoxelGraphNode_Struct::RemovePin_ContextMenu(UEdGraphPin& Pin)
{
	GetNodeDefinition()->RemoveSelectedPin(Pin.PinName);
}

bool UVoxelGraphNode_Struct::CanRenamePin(const UEdGraphPin& Pin) const
{
	if (!Struct.IsValid() ||
		Pin.ParentPin ||
		Pin.bOrphanedPin)
	{
		return false;
	}

	return ConstCast(this)->GetNodeDefinition()->CanRenameSelectedPin(Pin.PinName);
}

bool UVoxelGraphNode_Struct::IsNewPinNameValid(UEdGraphPin& Pin, const FName NewName) const
{
	if (!Struct.IsValid() ||
		Pin.ParentPin ||
		Pin.bOrphanedPin)
	{
		return false;
	}

	return ConstCast(this)->GetNodeDefinition()->IsNewPinNameValid(Pin.PinName, NewName);
}

void UVoxelGraphNode_Struct::RenamePin(UEdGraphPin& Pin, const FName NewName)
{
	GetNodeDefinition()->RenameSelectedPin(Pin.PinName, NewName);
}

bool UVoxelGraphNode_Struct::ShouldPromptRenameOnSpawn(const UEdGraphPin& Pin) const
{
	if (!Struct.IsValid() ||
		Pin.ParentPin ||
		Pin.bOrphanedPin)
	{
		return false;
	}

	return ConstCast(this)->GetNodeDefinition()->ShouldPromptRenameOnSpawn(Pin.PinName);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelGraphNode_Struct::CanPromotePin(const UEdGraphPin& Pin, FVoxelPinTypeSet& OutTypes) const
{
	if (Pin.ParentPin ||
		Pin.bOrphanedPin)
	{
		return false;
	}

	const TSharedPtr<const FVoxelPin> VoxelPin = Struct->FindPin(Pin.PinName);
	if (!ensure(VoxelPin) ||
		!VoxelPin->IsPromotable())
	{
		return false;
	}

	OutTypes = Struct->GetPromotionTypes(*VoxelPin);
	return true;
}

FString UVoxelGraphNode_Struct::GetPinPromotionWarning(const UEdGraphPin& Pin, const FVoxelPinType& NewType) const
{
	const TSharedPtr<const FVoxelPin> VoxelPin = Struct->FindPin(Pin.PinName);
	if (!ensure(VoxelPin) ||
		!VoxelPin->IsPromotable())
	{
		return {};
	}

	return Struct->GetPinPromotionWarning(*VoxelPin, NewType);
}

void UVoxelGraphNode_Struct::PromotePin(UEdGraphPin& Pin, const FVoxelPinType& NewType)
{
	Modify();

	const TSharedPtr<FVoxelPin> VoxelPin = Struct->FindPin(Pin.PinName);
	if (!ensure(VoxelPin) ||
		!ensure(VoxelPin->IsPromotable()))
	{
		return;
	}

	Struct->PromotePin(*VoxelPin, NewType);

	ReconstructFromVoxelNode();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelGraphNode_Struct::TryMigratePin(UEdGraphPin* OldPin, UEdGraphPin* NewPin) const
{
	if (!Super::TryMigratePin(OldPin, NewPin))
	{
		return false;
	}

	if (Struct)
	{
		if (const TSharedPtr<const FVoxelPin> VoxelPin = Struct->FindPin(NewPin->PinName))
		{
			if (VoxelPin->Metadata.bShowInDetail)
			{
				const FString DefaultValueString = Struct->GetPinDefaultValue(*VoxelPin.Get());

				FVoxelPinValue DefaultValue(VoxelPin->GetType().GetPinDefaultValueType());
				ensure(DefaultValue.ImportFromString(DefaultValueString));
				DefaultValue.ApplyToPinDefaultValue(*NewPin);
			}
		}
	}

	return true;
}

void UVoxelGraphNode_Struct::TryMigrateDefaultValue(const UEdGraphPin* OldPin, UEdGraphPin* NewPin) const
{
	if (const TSharedPtr<const FVoxelPin> VoxelPin = Struct->FindPin(NewPin->PinName))
	{
		if (VoxelPin->Metadata.bShowInDetail)
		{
			return;
		}
	}

	Super::TryMigrateDefaultValue(OldPin, NewPin);
}

FName UVoxelGraphNode_Struct::GetMigratedPinName(const FName PinName) const
{
	if (!Struct)
	{
		return PinName;
	}

	UObject* Outer;
	if (Struct->IsA<FVoxelNode_UFunction>())
	{
		Outer = Struct.Get<FVoxelNode_UFunction>().GetFunction();
	}
	else
	{
		Outer = Struct.GetScriptStruct();
	}

	if (!Outer)
	{
		return PinName;
	}

	const FName MigratedName = GVoxelGraphMigration->FindNewPinName(Outer, PinName);
	if (MigratedName.IsNone())
	{
		return PinName;
	}

	return MigratedName;
}

void UVoxelGraphNode_Struct::PreReconstructNode()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PreReconstructNode();

	if (!Struct)
	{
		return;
	}

	for (const UEdGraphPin* Pin : Pins)
	{
		if (Pin->bOrphanedPin ||
			Pin->ParentPin ||
			Pin->LinkedTo.Num() > 0 ||
			!FVoxelPinType(Pin->PinType).IsValid() ||
			!FVoxelPinType(Pin->PinType).HasPinDefaultValue())
		{
			return;
		}

		const TSharedPtr<const FVoxelPin> VoxelPin = Struct->FindPin(Pin->PinName);
		if (!VoxelPin)
		{
			// Orphaned
			continue;
		}

		const FVoxelPinValue DefaultValue = FVoxelPinValue::MakeFromPinDefaultValue(*Pin);
		GetNodeDefinition()->OnPinDefaultValueChanged(Pin->PinName, DefaultValue);
	}
}

void UVoxelGraphNode_Struct::PostReconstructNode()
{
	Super::PostReconstructNode();
	if (Struct)
	{
		Struct->PostReconstructNode();
	}
}

bool UVoxelGraphNode_Struct::IsPinVisible(const UEdGraphPin& Pin)
{
	if (!Struct)
	{
		return false;
	}

	const TSharedPtr<const FVoxelPin> VoxelPin = Struct->FindPin(Pin.PinName);
	if (!VoxelPin)
	{
		return false;
	}

	return GetNodeDefinition()->IsPinVisible(&Pin, this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGraphNode_Struct::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	if (const TSharedPtr<const FVoxelPin> VoxelPin = Struct->FindPin(Pin->PinName))
	{
		const FVoxelPinValue DefaultValue = FVoxelPinValue::MakeFromPinDefaultValue(*Pin);

		if (VoxelPin->Metadata.bShowInDetail)
		{
			Struct->UpdatePropertyBoundDefaultValue(*VoxelPin.Get(), DefaultValue);
		}

		if (ensure(Pin->LinkedTo.Num() == 0) &&
			GetNodeDefinition()->OnPinDefaultValueChanged(Pin->PinName, DefaultValue))
		{
			const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
			if (ensure(Toolkit))
			{
				Toolkit->AddNodeToReconstruct(this);
			}
		}
	}

	Super::PinDefaultValueChanged(Pin);
}

void UVoxelGraphNode_Struct::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (Struct &&
		Struct->PostEditChangeProperty(PropertyChangedEvent) == FVoxelNode::EPostEditChange::Reconstruct)
	{
		ReconstructFromVoxelNode();
 	}

	if (const FProperty* Property = PropertyChangedEvent.MemberProperty)
	{
		if (Property->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelGraphNode_Struct, Struct))
		{
			ReconstructFromVoxelNode();
		}
	}
}

bool UVoxelGraphNode_Struct::CanJumpToDefinition() const
{
	return true;
}

void UVoxelGraphNode_Struct::JumpToDefinition() const
{
	if (!ensure(GUnrealEd) ||
		!GUnrealEd->GetUnrealEdOptions()->IsCPPAllowed())
	{
		return;
	}

	if (!Struct)
	{
		return;
	}

	if (const FVoxelNode_UFunction* FunctionStruct = Struct->As<FVoxelNode_UFunction>())
	{
		if (FSourceCodeNavigation::CanNavigateToFunction(FunctionStruct->GetFunction()))
		{
			FSourceCodeNavigation::NavigateToFunction(FunctionStruct->GetFunction());
		}
		return;
	}

	FString RelativeFilePath;
	if (!FSourceCodeNavigation::FindClassSourcePath(Struct->GetStruct(), RelativeFilePath) ||
		IFileManager::Get().FileSize(*RelativeFilePath) == -1)
	{
		return;
	}

	FString ModuleName;
	const bool bModuleNameFound = INLINE_LAMBDA
	{
		if (!Struct->GetStruct())
		{
			return false;
		}

		const UPackage* Package = Struct->GetStruct()->GetPackage();
		if (!Package)
		{
			return false;
		}

		const FName ShotPackageName = FPackageName::GetShortFName(Package->GetFName());
		if (!FModuleManager::Get().IsModuleLoaded(ShotPackageName))
		{
			return false;
		}

		FModuleStatus ModuleStatus;
		if(!ensure(FModuleManager::Get().QueryModule(ShotPackageName, ModuleStatus)))
		{
			return false;
		}

		ModuleName = FPaths::GetBaseFilename(ModuleStatus.FilePath);
		return true;
	};

	if (!bModuleNameFound)
	{
		return;
	}

	FString Function = GET_FUNCTION_NAME_STRING_CHECKED(FVoxelNode, Compute);
	if (Struct->IsA<FVoxelTemplateNode>())
	{
		Function = GET_FUNCTION_NAME_STRING_CHECKED(FVoxelTemplateNode, GetNodeDefinition);
	}

	const FString FunctionName = Struct->GetStruct()->GetStructCPPName() + "::" + Function;
	FSourceCodeNavigation::NavigateToFunctionSourceAsync(FunctionName, ModuleName, false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGraphNode_Struct::ReconstructFromVoxelNode()
{
	ReconstructNode(false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<const IVoxelNodeDefinition::FNode> FVoxelNodeDefinition_Struct::GetInputs() const
{
	return NodeDefinition->GetInputs();
}

TSharedPtr<const IVoxelNodeDefinition::FNode> FVoxelNodeDefinition_Struct::GetOutputs() const
{
	return NodeDefinition->GetOutputs();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FString FVoxelNodeDefinition_Struct::GetAddPinLabel() const
{
	return NodeDefinition->GetAddPinLabel();
}

FString FVoxelNodeDefinition_Struct::GetAddPinTooltip() const
{
	return NodeDefinition->GetAddPinTooltip();
}

FString FVoxelNodeDefinition_Struct::GetRemovePinTooltip() const
{
	return NodeDefinition->GetRemovePinTooltip();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelNodeDefinition_Struct::CanAddInputPin() const
{
	return NodeDefinition->CanAddInputPin();
}

void FVoxelNodeDefinition_Struct::AddInputPin()
{
	FScope Scope(this, "Add Input Pin");
	NodeDefinition->AddInputPin();
}

bool FVoxelNodeDefinition_Struct::CanRemoveInputPin() const
{
	return NodeDefinition->CanRemoveInputPin();
}

void FVoxelNodeDefinition_Struct::RemoveInputPin()
{
	FScope Scope(this, "Remove Input Pin");
	NodeDefinition->RemoveInputPin();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelNodeDefinition_Struct::Variadic_CanAddPinTo(const FName VariadicPinName) const
{
	return NodeDefinition->Variadic_CanAddPinTo(VariadicPinName);
}

FName FVoxelNodeDefinition_Struct::Variadic_AddPinTo(const FName VariadicPinName)
{
	FScope Scope(this, "Add To Category");
	return NodeDefinition->Variadic_AddPinTo(VariadicPinName);
}

bool FVoxelNodeDefinition_Struct::Variadic_CanRemovePinFrom(const FName VariadicPinName) const
{
	return NodeDefinition->Variadic_CanRemovePinFrom(VariadicPinName);
}

void FVoxelNodeDefinition_Struct::Variadic_RemovePinFrom(const FName VariadicPinName)
{
	FScope Scope(this, "Remove From Category");
	NodeDefinition->Variadic_RemovePinFrom(VariadicPinName);
}

bool FVoxelNodeDefinition_Struct::CanRemoveSelectedPin(const FName PinName) const
{
	return NodeDefinition->CanRemoveSelectedPin(PinName);
}

void FVoxelNodeDefinition_Struct::RemoveSelectedPin(const FName PinName)
{
	FScope Scope(this, "Remove " + PinName.ToString() + " Pin");
	NodeDefinition->RemoveSelectedPin(PinName);
}

bool FVoxelNodeDefinition_Struct::CanRenameSelectedPin(const FName PinName) const
{
	return NodeDefinition->CanRenameSelectedPin(PinName);
}

bool FVoxelNodeDefinition_Struct::IsNewPinNameValid(const FName PinName, const FName NewName) const
{
	return NodeDefinition->IsNewPinNameValid(PinName, NewName);
}

void FVoxelNodeDefinition_Struct::RenameSelectedPin(const FName PinName, const FName NewName)
{
	FScope Scope(this, "Rename " + PinName.ToString() + " Pin to " + NewName.ToString());
	NodeDefinition->RenameSelectedPin(PinName, NewName);
}

bool FVoxelNodeDefinition_Struct::ShouldPromptRenameOnSpawn(const FName PinName) const
{
	return NodeDefinition->ShouldPromptRenameOnSpawn(PinName);
}

void FVoxelNodeDefinition_Struct::InsertPinBefore(const FName PinName)
{
	FScope Scope(this, "Insert Pin Before " + PinName.ToString());
	NodeDefinition->InsertPinBefore(PinName);
}

void FVoxelNodeDefinition_Struct::DuplicatePin(const FName PinName)
{
	FScope Scope(this, "Duplicate Pin " + PinName.ToString());
	NodeDefinition->DuplicatePin(PinName);
}

bool FVoxelNodeDefinition_Struct::IsPinVisible(const UEdGraphPin* Pin, const UEdGraphNode* GraphNode)
{
	return NodeDefinition->IsPinVisible(Pin, GraphNode);
}

bool FVoxelNodeDefinition_Struct::OnPinDefaultValueChanged(const FName PinName, const FVoxelPinValue& NewDefaultValue)
{
	return NodeDefinition->OnPinDefaultValueChanged(PinName, NewDefaultValue);
}

void FVoxelNodeDefinition_Struct::ExposePin(const FName PinName)
{
	FScope Scope(this, "Expose Pin " + PinName.ToString());
	NodeDefinition->ExposePin(PinName);
}