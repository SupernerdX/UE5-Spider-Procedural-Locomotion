// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "SVoxelSegmentedControl.h"

class VOXELEDITOR_API SVoxelShapeStampTypeSelector : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, Handle)
		SLATE_ARGUMENT(TSharedPtr<IPropertyUtilities>, PropertyUtilities)
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
	TSharedPtr<SBox> Content;
	TSharedPtr<SVoxelSegmentedControl<TSharedPtr<UScriptStruct*>>> SegmentedControl;

	TVoxelArray<UScriptStruct*> Structs;

	UScriptStruct* SelectedStruct = nullptr;
};