// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelStampDelta.h"

class FVoxelStampDeltaListTabManager : public FVoxelSingleton
{
public:
	const FName StampDeltaListTabId = "VoxelStampDeltaList";

	FVoxelStampDeltaListTabManager() = default;

	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override;
	//~ End FVoxelSingleton Interface

	void OpenStampDeltaList(const TVoxelArray<FVoxelStampDelta>& List) const;
	bool IsTabOpened() const;
};
extern FVoxelStampDeltaListTabManager* GVoxelStampDeltaListTabManager;

class SVoxelStampDeltaListEntry : public SMultiColumnTableRow<TSharedPtr<FVoxelStampDelta>>
{
public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(
		const FArguments& Args,
		const TSharedRef<STableViewBase>& OwnerTableView,
		const FVoxelStampDelta& NewStampDelta);

	//~ Begin STableRow Interface
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	//~ End STableRow Interface

private:
	FVoxelStampDelta StampDelta;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXELEDITOR_API SVoxelStampDeltaList : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		FArguments()
			: _IsTab(false)
		{
		}

		SLATE_ARGUMENT(bool, IsTab)
	};

	void Construct(const FArguments& Args);
	void UpdateStamps(const TVoxelArray<FVoxelStampDelta>& List);

private:
	TArray<TSharedPtr<FVoxelStampDelta>> StampDeltaArray;
	TSharedPtr<SListView<TSharedPtr<FVoxelStampDelta>>> ListView;

	TSharedPtr<SHeaderRow> HeaderRow;
};