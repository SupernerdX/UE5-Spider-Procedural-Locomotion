// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class SVoxelInputBindingEditor : public SEditableText
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ATTRIBUTE(FInputChord, InputChord)
		SLATE_EVENT(TDelegate<void(const FInputChord&)>, OnUpdateInputChord)
		SLATE_EVENT(TDelegate<FText(const FInputChord&)>, OnGetChordConflictMessage)

		SLATE_EVENT(FSimpleDelegate, OnEditBoxLostFocus)
		SLATE_EVENT(FSimpleDelegate, OnChordChanged)
		SLATE_EVENT(FSimpleDelegate, OnEditingStopped)
		SLATE_EVENT(FSimpleDelegate, OnEditingStarted)
	};

	void Construct(const FArguments& InArgs);

public:
	//~ Begin SEditableText Interface
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void OnFocusLost(const FFocusEvent& InFocusEvent) override;
	virtual bool SupportsKeyboardFocus() const override { return true; }
	//~ End SEditableText Interface

	void StartEditing();
	void StopEditing();

	void CommitNewChord();
	void RemoveActiveChord() const;

	bool IsEditing() const { return bIsEditing; }
	bool IsTyping() const { return bIsTyping; }
	bool IsEditedChordValid() const { return EditingInputChord.IsValidChord(); }
	bool IsActiveChordValid() const { return InputChordAttribute.Get().IsValidChord(); }
	const FText& GetNotificationText() const { return NotificationMessage; };
	bool HasConflict() const { return !NotificationMessage.IsEmpty(); }

private:
	void OnChordTyped(const FInputChord& NewChord);
	void OnChordCommitted(const FInputChord& NewChord);

private:
	TAttribute<FInputChord> InputChordAttribute;
	int32 InputChordIndex = -1;
	TDelegate<void(const FInputChord&)> OnUpdateInputChord;
	TDelegate<FText(const FInputChord&)> OnGetChordConflictMessage;

	FSimpleDelegate OnEditBoxLostFocus;
	FSimpleDelegate OnChordChanged;
	FSimpleDelegate OnEditingStopped;
	FSimpleDelegate OnEditingStarted;

	FText NotificationMessage;

	FInputChord EditingInputChord;
	bool bIsEditing = false;
	bool bIsTyping = false;

	static TWeakPtr<SVoxelInputBindingEditor> ChordBeingEdited;
};
