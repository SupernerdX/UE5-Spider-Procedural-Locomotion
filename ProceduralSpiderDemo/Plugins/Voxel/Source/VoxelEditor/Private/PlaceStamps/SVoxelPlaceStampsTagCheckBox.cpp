// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelPlaceStampsTagCheckBox.h"
#include "SVoxelPlaceStampsTab.h"

void SVoxelPlaceStampsTagCheckBox::Construct(const FArguments& Args)
{
	OnContentTagClicked = Args._OnContentTagClicked;
	OnContentTagShiftClicked = Args._OnContentTagShiftClicked;

	const FName ContentTag = Args._ContentTag;
	const FLinearColor TagColor = SVoxelPlaceStampsTab::GetStampTagColor(ContentTag);

	SCheckBox::Construct(
		SCheckBox::FArguments()
		.Style(FAppStyle::Get(), "ContentBrowser.FilterButton")
		.IsChecked(Args._IsChecked)
		.OnCheckStateChanged_Lambda([this](const ECheckBoxState NewState)
		{
			OnContentTagClicked.ExecuteIfBound(NewState == ECheckBoxState::Checked);
		}));

	SetToolTipText(FText::FromString("Display content with tag: " + FName::NameToDisplayString(ContentTag.ToString(), false) + """.\nUse Shift+Click to exclusively select this tag."));

	SetContent(
		SNew(SBorder)
		.Padding(1.f)
		.BorderImage(FVoxelEditorStyle::GetBrush("Graph.NewAssetDialog.FilterCheckBox.Border"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("ContentBrowser.FilterImage"))
				.ColorAndOpacity_Lambda([=, this]
				{
					FLinearColor Color = TagColor;
					if (!IsChecked())
					{
						Color.A = 0.1f;
					}

					return Color;
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(MakeAttributeLambda([this]
			{
				return bIsPressed ? FMargin(4.f, 2.f, 3.f, 0.f) : FMargin(4.f, 1.f, 3.f, 1.f);
			}))
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(FName::NameToDisplayString(ContentTag.ToString(), false)))
				.ColorAndOpacity_Lambda([this]
				{
					if (IsHovered())
					{
						return FLinearColor(0.75f, 0.75f, 0.75f, 1.f);
					}

					return IsChecked() ? FLinearColor::White : FLinearColor::Gray;
				})
				.TextStyle(FVoxelEditorStyle::Get(), "Graph.NewAssetDialog.ActionFilterTextBlock")
			]
		]
	);
}

FReply SVoxelPlaceStampsTagCheckBox::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FReply Reply = SCheckBox::OnMouseButtonUp(MyGeometry, MouseEvent);

	if (FSlateApplication::Get().GetModifierKeys().IsShiftDown() &&
		MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnContentTagShiftClicked.ExecuteIfBound();
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Reply;
}