// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphPreviewSettingsCustomization.h"
#include "Nodes/VoxelGraphNode.h"
#include "Preview/VoxelPreviewHandler.h"

void FVoxelGraphPreviewSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	if (DetailLayout.GetSelectedObjects().Num() != 1)
	{
		return;
	}

	const UVoxelGraphNode* Node = Cast<UVoxelGraphNode>(DetailLayout.GetSelectedObjects()[0].Get());
	if (!Node)
	{
		// Comment
		return;
	}
	const TVoxelObjectPtr<const UVoxelGraphNode> WeakNode = Node;

	{
		TArray<FName> CategoryNames;
		DetailLayout.GetCategoryNames(CategoryNames);
		for (const FName Name : CategoryNames)
		{
			if (Name != STATIC_FNAME("Preview"))
			{
				DetailLayout.HideCategory(Name);
			}
		}
	}

	const TSharedRef<IPropertyHandle> PreviewedPinHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelGraphNode, PreviewedPin), UVoxelGraphNode::StaticClass());
	PreviewedPinHandle->MarkHiddenByCustomization();
	PreviewedPinHandle->SetOnPropertyValueChanged(FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout));

	const TSharedRef<IPropertyHandle> PreviewSettingsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelGraphNode, PreviewSettings), UVoxelGraphNode::StaticClass());
	PreviewSettingsHandle->MarkHiddenByCustomization();

	uint32 NumChildren = 0;
	if (!ensure(PreviewSettingsHandle->GetNumChildren(NumChildren) == FPropertyAccess::Success))
	{
		return;
	}

	if (NumChildren == 0)
	{
		return;
	}

	IDetailCategoryBuilder& Category = DetailLayout.EditCategory(STATIC_FNAME("Preview"));

	Category.AddCustomRow(INVTEXT("Previewed Pin"))
	.NameContent()
	[
		PreviewedPinHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SBox)
		.MinDesiredWidth(125.f)
		[
			SNew(SVoxelDetailComboBox<FName>)
			.RefreshDelegate(this, DetailLayout)
			.Options_Lambda([=]() -> TArray<FName>
			{
				const UVoxelGraphNode* LocalNode = WeakNode.Resolve();
				if (!ensure(LocalNode))
				{
					return {};
				}

				TArray<FName> Names;
				for (const UEdGraphPin* Pin : LocalNode->GetOutputPins())
				{
					if (Pin->ParentPin)
					{
						continue;
					}
					Names.Add(Pin->PinName);
				}
				return Names;
			})
			.CurrentOption_Lambda([WeakNode]() -> FName
			{
				const UVoxelGraphNode* LocalNode = WeakNode.Resolve();
				if (!ensure(LocalNode))
				{
					return {};
				}

				return LocalNode->PreviewedPin;
			})
			.OptionText(MakeLambdaDelegate([=](const FName Option) -> FString
			{
				const UVoxelGraphNode* LocalNode = WeakNode.Resolve();
				if (!ensure(LocalNode))
				{
					return "INVALID";
				}

				const UEdGraphPin* Pin = LocalNode->FindPin(Option, EGPD_Output);
				if (!Pin)
				{
					return "INVALID";
				}

				FString DisplayName = Pin->GetDisplayName().ToString().TrimStartAndEnd();
				if (DisplayName.IsEmpty())
				{
					DisplayName = Pin->PinName.ToString();
				}

				return DisplayName;
			}))
			.OnSelection_Lambda([PreviewedPinHandle](const FName NewValue)
			{
				PreviewedPinHandle->SetValue(NewValue);
			})
		]
	];

	FVoxelPinType PinType;
	TSharedPtr<IPropertyHandle> PreviewHandlerHandle;
	for (uint32 Index = 0; Index < NumChildren; Index++)
	{
		const TSharedPtr<IPropertyHandle> ChildHandle = PreviewSettingsHandle->GetChildHandle(Index);
		if (!ensure(ChildHandle))
		{
			continue;
		}

		const TSharedPtr<IPropertyHandle> PinNameHandle = ChildHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelPinPreviewSettings, PinName));
		const TSharedPtr<IPropertyHandle> PinTypeHandle = ChildHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelPinPreviewSettings, PinType));
		if (!ensure(PinNameHandle) ||
			!ensure(PinTypeHandle))
		{
			continue;
		}

		FName PinName;
		if (!ensure(PinNameHandle->GetValue(PinName) == FPropertyAccess::Success) ||
			PinName != Node->PreviewedPin)
		{
			continue;
		}

		PinType = FVoxelEditorUtilities::GetStructPropertyValue<FEdGraphPinType>(PinTypeHandle);

		PreviewHandlerHandle = ChildHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelPinPreviewSettings, PreviewHandler));
		ensure(PreviewHandlerHandle);
		break;
	}

	if (!ensureVoxelSlow(PreviewHandlerHandle))
	{
		return;
	}

	Category.AddCustomRow(INVTEXT("Preview Type"))
	.NameContent()
	[
		PreviewedPinHandle->CreatePropertyNameWidget(INVTEXT("Preview Type"))
	]
	.ValueContent()
	[
		SNew(SBox)
		.MinDesiredWidth(125.f)
		[
			SNew(SVoxelDetailComboBox<UScriptStruct*>)
			.RefreshDelegate(this, DetailLayout)
			.Options_Lambda([=]() -> TArray<UScriptStruct*>
			{
				const UVoxelGraphNode* LocalNode = WeakNode.Resolve();
				if (!ensure(LocalNode))
				{
					return {};
				}

				TArray<UScriptStruct*> Structs;
				for (const TSharedRef<const FVoxelPreviewHandler>& Handler : FVoxelPreviewHandler::GetHandlers())
				{
					if (!Handler->SupportsType(PinType))
					{
						continue;
					}
					Structs.Add(Handler->GetStruct());
				}
				return Structs;
			})
			.CurrentOption_Lambda([PreviewHandlerHandle]
			{
				return FVoxelEditorUtilities::GetStructPropertyValue<FVoxelInstancedStruct>(PreviewHandlerHandle).GetScriptStruct();
			})
			.OptionText(MakeLambdaDelegate([](UScriptStruct* Option) -> FString
			{
				if (!Option)
				{
					return {};
				}
				return Option->GetDisplayNameText().ToString();
			}))
			.OnSelection_Lambda([PreviewHandlerHandle, RefreshDelegate = FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout)](UScriptStruct* NewValue)
			{
				FVoxelInstancedStruct* PreviewHandler = nullptr;
				if (!ensure(FVoxelEditorUtilities::GetPropertyValue(PreviewHandlerHandle, PreviewHandler)))
				{
					return;
				}

				PreviewHandlerHandle->NotifyPreChange();
				*PreviewHandler = FVoxelInstancedStruct(NewValue);
				PreviewHandlerHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
				PreviewHandlerHandle->NotifyFinishedChangingProperties();

				(void)RefreshDelegate.ExecuteIfBound();
			})
		]
	];

	Wrapper = FVoxelInstancedStructDetailsWrapper::Make(PreviewHandlerHandle.ToSharedRef());
	if (!Wrapper)
	{
		return;
	}

	for (const TSharedPtr<IPropertyHandle>& Handle : Wrapper->AddChildStructure())
	{
		Category.AddProperty(Handle);
	}
}
