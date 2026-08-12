// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "DiffUtils.h"
#include "VoxelDiffMode.h"
#include "VoxelDiffEntry.h"

class SLinkableScrollBar;
class FAsyncDetailViewDiff;

DECLARE_DELEGATE_RetVal_OneParam(bool, FVoxelOnTryApplyGuidProperty, FGuid)

struct FVoxelPropertyDiffEntry : public FVoxelDiffEntry
{
	GENERATED_VOXEL_DIFF_ENTRY_BODY(FVoxelPropertyDiffEntry)

	const FString PropertyName;
	const EPropertyDiffType::Type DiffType;
	const TWeakPtr<FDetailTreeNode> WeakLeftNode;
	const TWeakPtr<FDetailTreeNode> WeakRightNode;
	FVoxelPropertyDiffEntry(
		const TWeakPtr<FVoxelDiffMode>& Mode,
		const TWeakPtr<FDetailTreeNode>& WeakLeftNode,
		const TWeakPtr<FDetailTreeNode>& WeakRightNode,
		const FString& PropertyName,
		const EPropertyDiffType::Type DiffType)
		: FVoxelDiffEntry(Mode)
		, PropertyName(PropertyName)
		, DiffType(DiffType)
		, WeakLeftNode(WeakLeftNode)
		, WeakRightNode(WeakRightNode)
	{}

	virtual TSharedRef<SWidget> GenerateWidget() const override
	{
		FString ChangeType;
		FSlateColor Color = FSlateColor::UseForeground();
		switch (DiffType)
		{
		case EPropertyDiffType::PropertyAddedToA: ChangeType = "removed from"; Color = FLinearColor(0.3f, 0.3f, 1.f); break;
		case EPropertyDiffType::PropertyAddedToB: ChangeType = "added to"; Color = FColorList::MediumForestGreen; break;
		case EPropertyDiffType::PropertyValueChanged: ChangeType = "changed value in"; Color = FLinearColor(0.85f, 0.71f, 0.25f); break;
		default: ensure(false); return SNullWidget::NullWidget;
		}

		return SNew(STextBlock)
			.Text(FText::FromString(PropertyName + " " + ChangeType + " " + "Right Revision"))
			.ToolTipText(FText::FromString(PropertyName + " " + ChangeType + " " + "Right Revision"))
			.ColorAndOpacity(Color);
	}
};

struct FVoxelPropertiesDiffMode : public FVoxelDiffMode
{
public:
	FVoxelPropertiesDiffMode() = default;

	TSharedRef<FVoxelPropertiesDiffMode> Assets(
		UObject* OldAsset,
		UObject* NewAsset,
		bool bInNewAssetReadOnly);
	TSharedRef<FVoxelPropertiesDiffMode> Details(
		const TSharedPtr<IDetailsView>& InOldAssetDetails,
		const TSharedPtr<SLinkableScrollBar>& InOldAssetScrollBar,
		const TSharedPtr<IDetailsView>& InNewAssetDetails,
		const TSharedPtr<SLinkableScrollBar>& InNewAssetScrollBar,
		bool bInNewAssetReadOnly);
	TSharedRef<FVoxelPropertiesDiffMode> OnTryApplyGuidProperty(const FVoxelOnTryApplyGuidProperty& InOnTryApplyGuidProperty);
	TSharedRef<FVoxelPropertiesDiffMode> OnPropertyApplied(const FSimpleDelegate& InOnPropertyApplied);

	//~ Begin FVoxelDiffMode Interface
	virtual FString GetName() override;
	virtual void Tick() const override;
	virtual void GenerateDifferencesList(TArray<TSharedPtr<FVoxelDiffEntry>>& OutEntries) override;
	virtual TSharedRef<SWidget> GetWidget() const override;
	virtual void OnEntrySelected(const TSharedPtr<FVoxelDiffEntry>& Entry) override;
	//~ End FVoxelDiffMode Interface

	void HighlightProperties(const TWeakPtr<FDetailTreeNode>& WeakLeftNode, const TWeakPtr<FDetailTreeNode>& WeakRightNode) const;

private:
	static TSharedRef<IDetailsView> CreateDetailsView(
		UObject* Object,
		const TSharedPtr<SScrollBar>& ExternalScrollBar,
		bool bReadOnly);

protected:
	bool bNewAssetReadOnly = false;

private:
	FVoxelOnTryApplyGuidProperty OnTryApplyGuidPropertyDelegate;
	FSimpleDelegate OnPropertyAppliedDelegate;

	TSharedPtr<SLinkableScrollBar> OldAssetScrollBar;
	TSharedPtr<IDetailsView> OldAssetDetails;
	TSharedPtr<SLinkableScrollBar> NewAssetScrollBar;
	TSharedPtr<IDetailsView> NewAssetDetails;

	TSharedPtr<FAsyncDetailViewDiff> OldAssetDiff;
	TSharedPtr<FAsyncDetailViewDiff> NewAssetDiff;
};