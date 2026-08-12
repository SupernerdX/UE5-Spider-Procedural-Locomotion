// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class VOXELEDITOR_API SVoxelStampTypeSelector : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, Handle)
		SLATE_ARGUMENT(TSharedPtr<IPropertyUtilities>, PropertyUtilities)
		SLATE_ARGUMENT(TVoxelSet<UScriptStruct*>, HiddenStructs)
	};

	void Construct(const FArguments& Args);

	//~ Begin SCompoundWidget Interface
	virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
	//~ End SCompoundWidget Interface

private:
	void Refresh();
	void RecreateWidgets();
	void SetStruct(UScriptStruct& Struct);

private:
	TSharedPtr<IPropertyHandle> Handle;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;
	TSharedPtr<SHorizontalBox> HorizontalBox;

	TVoxelArray<UScriptStruct*> HeightStructs;
	TVoxelArray<UScriptStruct*> VolumeStructs;

	UScriptStruct* SelectedStruct = nullptr;

	struct FTab
	{
	};
	TSharedRef<FTab> HeightTab = MakeShared<FTab>();
	TSharedRef<FTab> VolumeTab = MakeShared<FTab>();
};