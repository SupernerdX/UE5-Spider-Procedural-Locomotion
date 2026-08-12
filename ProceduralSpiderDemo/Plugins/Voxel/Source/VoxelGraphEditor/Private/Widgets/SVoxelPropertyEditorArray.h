// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelPinType.h"

class VOXELGRAPHEDITOR_API SVoxelPropertyEditorArray : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(const FArguments& InArgs, const TSharedRef<IPropertyHandle>& Handle);

private:
	FReply OnDragDropTarget(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent) const;
	bool IsValidAssetDropOp(TSharedPtr<FDragDropOperation> InOperation) const;
	bool WillAddValidElements(TSharedPtr<FDragDropOperation> InOperation) const;

private:
	TSharedPtr<IPropertyHandle> PropertyHandle;
};