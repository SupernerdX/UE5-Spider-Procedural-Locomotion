// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class SVoxelInputBindingEditor;

class SVoxelInputBindingEditBox : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ATTRIBUTE(FInputChord, InputChord)
		SLATE_EVENT(TDelegate<void(const FInputChord&)>, OnUpdateInputChord)
		SLATE_EVENT(TDelegate<FText(const FInputChord&)>, OnGetChordConflictMessage)
	};

	void Construct(const FArguments& InArgs);

private:
	//~ Begin SCompoundWidget Interface
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;
	//~ End SCompoundWidget Interface

	void OnChordEditorLostFocus();
	void OnChordChanged();
	TSharedRef<SWidget> OnGetContentForConflictPopup();

private:
	TSharedPtr<SVoxelInputBindingEditor> BindingEditor;
	TSharedPtr<SMenuAnchor> ConflictPopup;
	TSharedPtr<SButton> ChordAcceptButton;
};
