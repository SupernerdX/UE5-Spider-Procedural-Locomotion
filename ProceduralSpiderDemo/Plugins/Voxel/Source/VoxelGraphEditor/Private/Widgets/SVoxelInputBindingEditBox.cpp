// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelInputBindingEditBox.h"
#include "SVoxelInputBindingEditor.h"

void SVoxelInputBindingEditBox::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(ConflictPopup, SMenuAnchor)
		.Placement(MenuPlacement_ComboBox)
		.OnGetMenuContent(this, &SVoxelInputBindingEditBox::OnGetContentForConflictPopup)
		.OnMenuOpenChanged_Lambda([this](const bool bIsOpen)
		{
			if (!bIsOpen)
			{
				BindingEditor->StopEditing();
			}
		})
		[
			SNew(SBox)
			.WidthOverride(200.f)
			[
				SNew(SBorder)
				.VAlign(VAlign_Center)
				.Padding(4.0f, 2.0f)
				.BorderImage_Lambda([this]
				{
					if (BindingEditor->HasKeyboardFocus())
					{
						return FAppStyle::GetBrush(STATIC_FNAME("EditableTextBox.Background.Focused"));
					}
					if (BindingEditor->IsHovered())
					{
						return FAppStyle::GetBrush(STATIC_FNAME("EditableTextBox.Background.Hovered"));
					}
	
					return FAppStyle::GetBrush(STATIC_FNAME("EditableTextBox.Background.Normal"));
				})
				.ForegroundColor(FAppStyle::GetSlateColor(STATIC_FNAME("InvertedForeground")))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						SAssignNew(BindingEditor, SVoxelInputBindingEditor)
						.InputChord(InArgs._InputChord)
						.OnUpdateInputChord(InArgs._OnUpdateInputChord)
						.OnGetChordConflictMessage(InArgs._OnGetChordConflictMessage)
						.OnEditBoxLostFocus(this, &SVoxelInputBindingEditBox::OnChordEditorLostFocus)
						.OnChordChanged(this, &SVoxelInputBindingEditBox::OnChordChanged)
						.OnEditingStarted_Lambda([this]
						{
							ConflictPopup->SetIsOpen(false);
						})
						.OnEditingStopped_Lambda([this]
						{
							if (BindingEditor->IsEditedChordValid() &&
								!BindingEditor->HasConflict())
							{
								BindingEditor->CommitNewChord();
							}
						})
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Visibility_Lambda([this]
						{
							return !BindingEditor->IsEditing() && BindingEditor->IsActiveChordValid() ? EVisibility::Visible : EVisibility::Hidden;
						})
						.ButtonStyle(FAppStyle::Get(), "NoBorder")
						.ContentPadding(0.f)
						.OnClicked_Lambda([this]
						{
							if (!BindingEditor->IsEditing())
							{
								BindingEditor->RemoveActiveChord();
							}

							return FReply::Handled();
						})
						.ForegroundColor(FSlateColor::UseForeground())
						.IsFocusable(false)
						.ToolTipText(INVTEXT("Remove this binding"))
						[
							SNew(SImage)
							.Image(FAppStyle::GetBrush( "Symbols.X" ))
							.ColorAndOpacity(FLinearColor(.7f,0,0,.75f))
						]
					]
				]
			]
		]
	];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FReply SVoxelInputBindingEditBox::OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent )
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Handled();
	}

	if (!BindingEditor->IsEditing())
	{
		BindingEditor->StartEditing();
	}

	return FReply::Handled().SetUserFocus(BindingEditor.ToSharedRef(), EFocusCause::Mouse);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelInputBindingEditBox::OnChordEditorLostFocus()
{
	if (ChordAcceptButton.IsValid() &&
		ChordAcceptButton->HasMouseCapture())
	{
		return;
	}

	if (BindingEditor->IsTyping() ||
		BindingEditor->HasConflict())
	{
		return;
	}

	if (BindingEditor->IsEditing() &&
		BindingEditor->IsEditedChordValid())
	{
		BindingEditor->CommitNewChord();
	}

	BindingEditor->StopEditing();
}

void SVoxelInputBindingEditBox::OnChordChanged()
{
	if (BindingEditor->HasConflict())
	{
		ConflictPopup->SetIsOpen(true, true);
		return;
	}

	ConflictPopup->SetIsOpen(false);

	if (BindingEditor->IsEditedChordValid())
	{
		BindingEditor->CommitNewChord();
		BindingEditor->StopEditing();
		FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::SetDirectly);
	}
}

TSharedRef<SWidget> SVoxelInputBindingEditBox::OnGetContentForConflictPopup()
{
	return
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("NotificationList.ItemBackground"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(2.f, 0.f)
			.AutoHeight()
			[
				SNew(STextBlock)
				.WrapTextAt(200.f)
				.ColorAndOpacity(FLinearColor(.75f, 0.f, 0.f, 1.f))
				.Text_Lambda([this]
				{
					return BindingEditor->GetNotificationText();
				})
				.Visibility_Lambda([this]
				{
					return !BindingEditor->GetNotificationText().IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed;
				})
			]

			+ SVerticalBox::Slot()
			.Padding(2.f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			.AutoHeight()
			[
				SAssignNew(ChordAcceptButton, SButton)
				.ContentPadding(1.f)
				.ToolTipText(INVTEXT("Accept this new binding"))
				.OnClicked_Lambda([this]
				{
					if (BindingEditor->IsEditing())
					{
						BindingEditor->CommitNewChord();
						BindingEditor->StopEditing();
					}

					ConflictPopup->SetIsOpen(false);

					return FReply::Handled();
				})
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Symbols.Check"))
						.ColorAndOpacity(FLinearColor(0.f,.7f,0.f,.75f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.f,0.f)
					[
						SNew(STextBlock)
						.Text(INVTEXT("Override"))
					]
				]
			]
		];
}