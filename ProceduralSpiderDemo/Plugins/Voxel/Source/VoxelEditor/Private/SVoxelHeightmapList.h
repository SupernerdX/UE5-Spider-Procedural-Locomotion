// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Heightmap/VoxelHeightmap.h"

struct FVoxelHeightStampLayer
{
	const TVoxelObjectPtr<UObject> WeakObject;
	const TSharedRef<FAssetThumbnail> AssetThumbnail;

	const bool bHeightmap;
	const int32 Index;

	FVoxelHeightStampLayer(UObject* Object, UObject* ThumbnailAsset, int32 Index);
};

class FVoxelHeightStampLayerDragDropOp : public FDecoratedDragDropOp
{
public:
	DRAG_DROP_OPERATOR_TYPE(FVoxelHeightStampLayerDragDropOp, FDecoratedDragDropOp)
	TWeakPtr<FVoxelHeightStampLayer> Layer;

	static TSharedRef<FVoxelHeightStampLayerDragDropOp> New(const TSharedPtr<FVoxelHeightStampLayer>& Layer);

	void SetValidTarget(bool IsValidTarget);
};

class SVoxelHeightmapList : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ARGUMENT(TVoxelObjectPtr<UVoxelHeightmap>, Asset)
		SLATE_ARGUMENT(FNotifyHook*, NotifyHook)
	};

	void Construct(const FArguments& InArgs);
	void OnPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);
	void PostUndo();

	//~ Begin SCompoundWidget Interface
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	//~ End SCompoundWidget Interface

private:
	void BuildList();

	void BindCommands();
	TSharedRef<ITableRow> MakeWidgetFromLayer(TSharedPtr<FVoxelHeightStampLayer> Layer, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedPtr<SWidget> OnContextMenuOpening();

private:
	void AddLayer(const UVoxelHeightmap_Weight* WeightmapToCopy, UTexture2D* WeightmapTexture);
	void DeleteLayer(const TSharedPtr<FVoxelHeightStampLayer>& Layer);

private:
	TSharedPtr<IDetailsView> DetailsView;
	TVoxelObjectPtr<UVoxelHeightmap> WeakAsset;

private:
	TSharedPtr<SListView<TSharedPtr<FVoxelHeightStampLayer>>> ListView;

	TArray<TSharedPtr<FVoxelHeightStampLayer>> Layers;
	TSharedPtr<FUICommandList> CommandList;
};