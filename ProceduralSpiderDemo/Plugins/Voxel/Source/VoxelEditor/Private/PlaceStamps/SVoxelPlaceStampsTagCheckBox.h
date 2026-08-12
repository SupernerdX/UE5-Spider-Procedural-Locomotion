// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class SVoxelPlaceStampsTagCheckBox : public SCheckBox
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ARGUMENT(FName, ContentTag)
		SLATE_EVENT(TDelegate<void(bool)>, OnContentTagClicked)
		SLATE_EVENT(FSimpleDelegate, OnContentTagShiftClicked)
		SLATE_ATTRIBUTE(ECheckBoxState, IsChecked)
	};

	void Construct(const FArguments& Args);

	//~ Begin SCheckBox Interface
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	//~ End SCheckBox Interface

private:
	TDelegate<void(bool)> OnContentTagClicked;
	FSimpleDelegate OnContentTagShiftClicked;
};