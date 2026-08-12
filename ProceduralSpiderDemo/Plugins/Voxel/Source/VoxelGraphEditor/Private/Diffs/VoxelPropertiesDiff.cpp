// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPropertiesDiff.h"

#include "AssetSelection.h"
#include "AsyncDetailViewDiff.h"
#include "SVoxelDetailsSplitter.h"
#include "SVoxelDiffSplitter.h"
#include "Widgets/Layout/LinkableScrollBar.h"
#include "Editor/PropertyEditor/Private/SDetailsViewBase.h"

class SVoxelDetailsViewAccessor : public SDetailsViewBase
{
public:
	void HighlightPropertyNode(const TSharedPtr<FDetailTreeNode>& Node)
	{
		if (const TSharedPtr<FDetailTreeNode> PrevHighlightedNodePtr = CurrentlyHighlightedNode.Pin())
		{
			PrevHighlightedNodePtr->SetIsHighlighted(false);
		}

		if (!Node)
		{
			CurrentlyHighlightedNode = nullptr;
			return;
		}

		Node->SetIsHighlighted(true);

		TSharedPtr<FDetailTreeNode> Ancestor = Node;
		while(Ancestor->GetParentNode().IsValid())
		{
			Ancestor = Ancestor->GetParentNode().Pin();
			DetailTree->SetItemExpansion(Ancestor.ToSharedRef(), true);
		}

		// scroll to the found node
		DetailTree->RequestScrollIntoView(Node.ToSharedRef());

		CurrentlyHighlightedNode = Node;
	}

	TSharedPtr<FDetailsViewStyleKey> StyleKeySP;
	TSharedPtr<FDetailsViewObjectFilter> ObjectFilter;
	FSelectedActorInfo SelectedActorInfo;
	TArray<TVoxelObjectPtr<UObject>> UnfilteredSelectedObjects;
	TArray<TVoxelObjectPtr<UObject>> SelectedObjects;
	TArray< TVoxelObjectPtr<AActor> > SelectedActors;
	TArray<TSharedPtr<FComplexPropertyNode>> RootPropertyNodes;
	FOnObjectArrayChanged OnObjectArrayChanged;
	TSharedPtr<IDetailRootObjectCustomization> RootObjectCustomization;
	bool bViewingClassDefaultObject;
	FDelegateHandle PostUndoRedoDelegateHandle;
	TSharedPtr<class SWrapBox> SectionSelectorBox;
	bool bIsRefreshing;
};


TSharedRef<FVoxelPropertiesDiffMode> FVoxelPropertiesDiffMode::Assets(UObject* OldAsset, UObject* NewAsset, const bool bInNewAssetReadOnly)
{
	bNewAssetReadOnly = bInNewAssetReadOnly;

	OldAssetScrollBar = SNew(SLinkableScrollBar);
	OldAssetDetails = CreateDetailsView(OldAsset, OldAssetScrollBar, true);

	NewAssetScrollBar = SNew(SLinkableScrollBar);
	NewAssetDetails = CreateDetailsView(NewAsset, NewAssetScrollBar, bNewAssetReadOnly);

	OldAssetDiff = MakeShared<FAsyncDetailViewDiff>(OldAssetDetails.ToSharedRef(), NewAssetDetails.ToSharedRef());
	NewAssetDiff = MakeShared<FAsyncDetailViewDiff>(NewAssetDetails.ToSharedRef(), OldAssetDetails.ToSharedRef());

	SLinkableScrollBar::LinkScrollBars(
		OldAssetScrollBar.ToSharedRef(),
		NewAssetScrollBar.ToSharedRef(),
		MakeAttributeSP(NewAssetDiff.ToSharedRef(), &FAsyncDetailViewDiff::GenerateScrollSyncRate));

	return SharedThis(this);
}

TSharedRef<FVoxelPropertiesDiffMode> FVoxelPropertiesDiffMode::Details(
	const TSharedPtr<IDetailsView>& InOldAssetDetails,
	const TSharedPtr<SLinkableScrollBar>& InOldAssetScrollBar,
	const TSharedPtr<IDetailsView>& InNewAssetDetails,
	const TSharedPtr<SLinkableScrollBar>& InNewAssetScrollBar,
	const bool bInNewAssetReadOnly)
{
	bNewAssetReadOnly = bInNewAssetReadOnly;
	OldAssetDetails = InOldAssetDetails;
	OldAssetDetails->SetIsPropertyEditingEnabledDelegate(MakeLambdaDelegate([]
	{
		return false;
	}));
	OldAssetScrollBar = InOldAssetScrollBar;
	NewAssetDetails = InNewAssetDetails;
	NewAssetDetails->SetIsPropertyEditingEnabledDelegate(MakeLambdaDelegate([bInNewAssetReadOnly]
	{
		return !bInNewAssetReadOnly;
	}));
	NewAssetScrollBar = InNewAssetScrollBar;

	OldAssetDiff = MakeShared<FAsyncDetailViewDiff>(OldAssetDetails.ToSharedRef(), NewAssetDetails.ToSharedRef());
	NewAssetDiff = MakeShared<FAsyncDetailViewDiff>(NewAssetDetails.ToSharedRef(), OldAssetDetails.ToSharedRef());

	SLinkableScrollBar::LinkScrollBars(
		OldAssetScrollBar.ToSharedRef(),
		NewAssetScrollBar.ToSharedRef(),
		MakeAttributeSP(NewAssetDiff.ToSharedRef(), &FAsyncDetailViewDiff::GenerateScrollSyncRate));

	return SharedThis(this);
}

TSharedRef<FVoxelPropertiesDiffMode> FVoxelPropertiesDiffMode::OnTryApplyGuidProperty(const FVoxelOnTryApplyGuidProperty& InOnTryApplyGuidProperty)
{
	OnTryApplyGuidPropertyDelegate = InOnTryApplyGuidProperty;
	return SharedThis(this);
}

TSharedRef<FVoxelPropertiesDiffMode> FVoxelPropertiesDiffMode::OnPropertyApplied(const FSimpleDelegate& InOnPropertyApplied)
{
	OnPropertyAppliedDelegate = InOnPropertyApplied;
	return SharedThis(this);
}

FString FVoxelPropertiesDiffMode::GetName()
{
	return "Asset Defaults";
}

void FVoxelPropertiesDiffMode::Tick() const
{
	constexpr float MaxTickTimeMs = 0.01f;
	if (OldAssetDiff)
	{
		OldAssetDiff->Tick(MaxTickTimeMs);
	}
	if (NewAssetDiff)
	{
		NewAssetDiff->Tick(MaxTickTimeMs);
	}
}

void FVoxelPropertiesDiffMode::GenerateDifferencesList(TArray<TSharedPtr<FVoxelDiffEntry>>& OutEntries)
{
	OldAssetDiff->FlushQueue();
	OldAssetDiff->ForEach(ETreeTraverseOrder::PreOrder, [&](const TUniquePtr<FAsyncDetailViewDiff::DiffNodeType>& Node) -> ETreeTraverseControl
	{
		TSharedPtr<IPropertyHandle> Handle;
		FPropertyPath PropertyPath;
		if (const TSharedPtr<FDetailTreeNode> LeftTreeNode = Node->ValueA.Pin())
		{
			Handle = LeftTreeNode->CreatePropertyHandle();
			PropertyPath = LeftTreeNode->GetPropertyPath();
		}
		else if (const TSharedPtr<FDetailTreeNode> RightTreeNode = Node->ValueB.Pin())
		{
			Handle = RightTreeNode->CreatePropertyHandle();
			PropertyPath = RightTreeNode->GetPropertyPath();
		}

		if (!PropertyPath.IsValid())
		{
			return ETreeTraverseControl::Continue;
		}

		EPropertyDiffType::Type PropertyDiffType;
		switch(Node->DiffResult)
		{
		case ETreeDiffResult::MissingFromTree1: PropertyDiffType = EPropertyDiffType::PropertyAddedToB; break;
		case ETreeDiffResult::MissingFromTree2: PropertyDiffType = EPropertyDiffType::PropertyAddedToA; break;
		case ETreeDiffResult::DifferentValues: PropertyDiffType = EPropertyDiffType::PropertyValueChanged; break;
		default: return ETreeTraverseControl::Continue;
		}

		FString Name;
		if (Handle &&
			Handle->HasMetaData("PropertyName"))
		{
			Name = Handle->GetMetaData("PropertyName");
		}
		else
		{
			Name = FPropertySoftPath(PropertyPath).ToDisplayName();
		}

		OutEntries.Add(MakeShared<FVoxelPropertyDiffEntry>(AsShared(), Node->ValueA, Node->ValueB, Name, PropertyDiffType));
		return ETreeTraverseControl::SkipChildren;
	});
}

TSharedRef<SWidget> FVoxelPropertiesDiffMode::GetWidget() const
{
	using DetailsSplitter = SVoxelDiffSplitter<TSharedPtr<FDetailTreeNode>>;
	return
		SNew(DetailsSplitter)
		.CanCopyValue_Lambda([this](const TSharedPtr<FDetailTreeNode>& SourceNode, const TSharedPtr<FDetailTreeNode>& DestinationNode, const ETreeDiffResult DiffResult)
		{
			return FVoxelPropertiesSplitterHelper::CanCopyPropertyValue(SourceNode, DestinationNode, DiffResult);
		})
		.CopyValue_Lambda([this](const TSharedPtr<FDetailTreeNode>& SourceNode, const TSharedPtr<FDetailTreeNode>& DestinationNode, const ETreeDiffResult DiffResult)
		{
			ON_SCOPE_EXIT
			{
				NewAssetDetails->ForceRefresh();
				OnPropertyAppliedDelegate.ExecuteIfBound();
			};

			const auto CopyPropertyValue = [&]()
			{
				if (DiffResult != ETreeDiffResult::DifferentValues)
				{
					return false;
				}

				if (!SourceNode ||
					!DestinationNode)
				{
					return false;
				}
				const TSharedPtr<IPropertyHandle> SourceHandle = SourceNode->CreatePropertyHandle();
				const TSharedPtr<IPropertyHandle> DestinationHandle = DestinationNode->CreatePropertyHandle();
				if (!ensure(SourceHandle) ||
					!ensure(DestinationNode))
				{
					return false;
				}

				if (!SourceHandle->HasMetaData(STATIC_FNAME("Guid")) ||
					!DestinationHandle->HasMetaData(STATIC_FNAME("Guid")))
				{
					return false;
				}

				FGuid SourceGuid;
				FGuid DestinationGuid;
				if (!FGuid::Parse(SourceHandle->GetMetaData(STATIC_FNAME("Guid")), SourceGuid) ||
					!FGuid::Parse(DestinationHandle->GetMetaData(STATIC_FNAME("Guid")), DestinationGuid))
				{
					return false;
				}

				if (SourceGuid != DestinationGuid)
				{
					return false;
				}

				if (!OnTryApplyGuidPropertyDelegate.IsBound())
				{
					return false;
				}

				return OnTryApplyGuidPropertyDelegate.Execute(SourceGuid);
			};

			if (CopyPropertyValue())
			{
				return;
			}

			FVoxelPropertiesSplitterHelper::CopyPropertyValue(
				SourceNode,
				DestinationNode,
				DiffResult);
		})
		+ DetailsSplitter::Slot()
		.Value(0.5f)
		.Widget(OldAssetDetails)
		.IsReadonly(true)
		.IterateChildren_Lambda([this](const TFunction<ETreeTraverseControl(const TSharedPtr<FDetailTreeNode>&, const TSharedPtr<FDetailTreeNode>&, ETreeDiffResult DiffResult)>& Iterate)
		{
			OldAssetDiff->ForEachRow([&](const TUniquePtr<FAsyncDetailViewDiff::DiffNodeType>& DiffNode, int32, int32) -> ETreeTraverseControl
			{
				return Iterate(DiffNode->ValueA.Pin(), DiffNode->ValueB.Pin(), DiffNode->DiffResult);
			});
		})
		.GetPaintSpaceBounds_Lambda([this](const TSharedPtr<FDetailTreeNode>& Node, const bool bIncludeChildren)
		{
			return OldAssetDetails->GetPaintSpacePropertyBounds(Node.ToSharedRef(), bIncludeChildren);
		})
		.GetTickSpaceBounds_Lambda([this](const TSharedPtr<FDetailTreeNode>& Node, const bool bIncludeChildren)
		{
			return OldAssetDetails->GetTickSpacePropertyBounds(Node.ToSharedRef(), bIncludeChildren);
		})
		.IsReadonly(true)
		+ DetailsSplitter::Slot()
		.Value(0.5f)
		.Widget(NewAssetDetails)
		.IterateChildren_Lambda([this](const TFunction<ETreeTraverseControl(const TSharedPtr<FDetailTreeNode>&, const TSharedPtr<FDetailTreeNode>&, ETreeDiffResult DiffResult)>& Iterate)
		{
			NewAssetDiff->ForEachRow([&](const TUniquePtr<FAsyncDetailViewDiff::DiffNodeType>& DiffNode, int32, int32) -> ETreeTraverseControl
			{
				return Iterate(DiffNode->ValueA.Pin(), DiffNode->ValueB.Pin(), DiffNode->DiffResult);
			});
		})
		.GetPaintSpaceBounds_Lambda([this](const TSharedPtr<FDetailTreeNode>& Node, const bool bIncludeChildren)
		{
			return NewAssetDetails->GetPaintSpacePropertyBounds(Node.ToSharedRef(), bIncludeChildren);
		})
		.GetTickSpaceBounds_Lambda([this](const TSharedPtr<FDetailTreeNode>& Node, const bool bIncludeChildren)
		{
			return NewAssetDetails->GetTickSpacePropertyBounds(Node.ToSharedRef(), bIncludeChildren);
		})
		.IsReadonly(bNewAssetReadOnly);
}

void FVoxelPropertiesDiffMode::OnEntrySelected(const TSharedPtr<FVoxelDiffEntry>& Entry)
{
	if (!ensure(Entry->IsA<FVoxelPropertyDiffEntry>()))
	{
		HighlightProperties(nullptr, nullptr);
		return;
	}

	const FVoxelPropertyDiffEntry& PropertyEntry = Entry->Get<FVoxelPropertyDiffEntry>();
	HighlightProperties(PropertyEntry.WeakLeftNode, PropertyEntry.WeakRightNode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPropertiesDiffMode::HighlightProperties(const TWeakPtr<FDetailTreeNode>& WeakLeftNode, const TWeakPtr<FDetailTreeNode>& WeakRightNode) const
{
	reinterpret_cast<SVoxelDetailsViewAccessor*>(OldAssetDetails.Get())->HighlightPropertyNode(WeakLeftNode.Pin());
	reinterpret_cast<SVoxelDetailsViewAccessor*>(NewAssetDetails.Get())->HighlightPropertyNode(WeakRightNode.Pin());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<IDetailsView> FVoxelPropertiesDiffMode::CreateDetailsView(
	UObject* Object,
	const TSharedPtr<SScrollBar>& ExternalScrollBar,
	const bool bReadOnly)
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bShowDifferingPropertiesOption = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.ExternalScrollbar = ExternalScrollBar;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bShowPropertyMatrixButton = false;

	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedRef<IDetailsView> DetailsView = EditModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetIsPropertyEditingEnabledDelegate(MakeLambdaDelegate([bReadOnly]
	{
		return !bReadOnly;
	}));

	DetailsView->ShowAllAdvancedProperties();
	DetailsView->SetObject(Object);

	return DetailsView;
}