// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelInputBindingEditor.h"
#include "Widgets/Text/SlateEditableTextLayout.h"

TWeakPtr<SVoxelInputBindingEditor> SVoxelInputBindingEditor::ChordBeingEdited;

void SVoxelInputBindingEditor::Construct(const FArguments& InArgs)
{
	InputChordAttribute = InArgs._InputChord;
	OnUpdateInputChord = InArgs._OnUpdateInputChord;
	OnGetChordConflictMessage = InArgs._OnGetChordConflictMessage;

	OnEditBoxLostFocus = InArgs._OnEditBoxLostFocus;
	OnChordChanged = InArgs._OnChordChanged;
	OnEditingStopped = InArgs._OnEditingStopped;
	OnEditingStarted = InArgs._OnEditingStarted;

	SEditableText::Construct( 
		SEditableText::FArguments() 
		.Text_Lambda([this]
		{
			if (bIsEditing)
			{
				return EditingInputChord.GetInputText();
			}

			const FInputChord InputChord = InputChordAttribute.Get();
			if (InputChord.IsValidChord())
			{
				return InputChord.GetInputText();
			}

			return FText::GetEmpty();
		})
		.HintText(INVTEXT("Type a new binding"))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
	);

	EditableTextLayout->LoadText();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FReply SVoxelInputBindingEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (!bIsEditing)
	{
		return FReply::Unhandled();
	}

	TGuardValue<bool> TypingGuard(bIsTyping, true);

	if (!EKeys::IsModifierKey(Key))
	{
		EditingInputChord.Key = Key;
	}

	EditableTextLayout->BeginEditTransation();

	EditingInputChord.bCtrl = InKeyEvent.IsControlDown();
	EditingInputChord.bAlt = InKeyEvent.IsAltDown();
	EditingInputChord.bShift = InKeyEvent.IsShiftDown();
	EditingInputChord.bCmd = InKeyEvent.IsCommandDown();

	EditableTextLayout->LoadText();
	EditableTextLayout->GoTo(ETextLocation::EndOfDocument);

	EditableTextLayout->EndEditTransaction();
	
	OnChordTyped(EditingInputChord);
	
	return FReply::Handled();
}

FReply SVoxelInputBindingEditor::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	return FReply::Unhandled();
}

FReply SVoxelInputBindingEditor::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent)
{
	return FReply::Unhandled();
}

FReply SVoxelInputBindingEditor::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton &&
		!bIsEditing)
	{
		StartEditing();
		return FReply::Handled().SetUserFocus(AsShared(), EFocusCause::Mouse);
	}

	return FReply::Unhandled();
}

FReply SVoxelInputBindingEditor::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

void SVoxelInputBindingEditor::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	SEditableText::OnFocusLost(InFocusEvent);
	OnEditBoxLostFocus.ExecuteIfBound();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelInputBindingEditor::StartEditing()
{
	if (const TSharedPtr<SVoxelInputBindingEditor> ActiveChordEditor = ChordBeingEdited.Pin())
	{
		ActiveChordEditor->StopEditing();
	}

	ChordBeingEdited = SharedThis(this);

	NotificationMessage = FText::GetEmpty();
	EditingInputChord = {};
	bIsEditing = true;

	OnEditingStarted.ExecuteIfBound();
}

void SVoxelInputBindingEditor::StopEditing()
{
	if (const TSharedPtr<SVoxelInputBindingEditor> ActiveChordEditor = ChordBeingEdited.Pin())
	{
		if (ActiveChordEditor.Get() == this)
		{
			ChordBeingEdited.Reset();
		}
	}

	OnEditingStopped.ExecuteIfBound();

	bIsEditing = false;

	EditingInputChord = {};
	NotificationMessage = FText::GetEmpty();
}

void SVoxelInputBindingEditor::CommitNewChord()
{
	if (EditingInputChord.IsValidChord())
	{
		OnChordCommitted(EditingInputChord);
	}
}

void SVoxelInputBindingEditor::RemoveActiveChord() const
{
	OnUpdateInputChord.Execute({});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void SVoxelInputBindingEditor::OnChordTyped( const FInputChord& NewChord )
{
	if (NewChord.IsValidChord())
	{
		NotificationMessage = OnGetChordConflictMessage.Execute(NewChord);
	}

	OnChordChanged.ExecuteIfBound();
}


void SVoxelInputBindingEditor::OnChordCommitted( const FInputChord& NewChord)
{
	check(NewChord.IsValidChord());

	OnUpdateInputChord.Execute(NewChord);
}
