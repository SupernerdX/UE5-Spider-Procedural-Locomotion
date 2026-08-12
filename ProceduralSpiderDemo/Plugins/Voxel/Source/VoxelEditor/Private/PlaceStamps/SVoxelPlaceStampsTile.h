// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelPlaceStampsItem.h"

class SVoxelPlaceStampsTile : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ATTRIBUTE(TSet<FName>, VisibleTags)
		SLATE_EVENT(TDelegate<void(bool)>, OnFavoriteStateChanged)
	};

	void Construct(const FArguments& InArgs, const TSharedPtr<FVoxelPlaceStampsItem>& InItem);

protected:
	//~ Begin SCompoundWidget Interface
	virtual TOptional<EMouseCursor::Type> GetCursor() const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	//~ End SCompoundWidget Interface

private:
	bool bIsPressed = false;
	TWeakPtr<FVoxelPlaceStampsItem> WeakItem;
};