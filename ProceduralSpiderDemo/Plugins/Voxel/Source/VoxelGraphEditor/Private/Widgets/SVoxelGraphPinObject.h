// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "KismetPins/SGraphPinObject.h"
#include "SVoxelGraphPin.h"

class SVoxelGraphPinObject : public SGraphPinObject
{
public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

	//~ Begin SGraphPinObject Interface
	virtual TSharedRef<SWidget> GetDefaultValueWidget() override;
	virtual void OnAssetSelectedFromPicker(const FAssetData& AssetData) override;
	virtual TSharedRef<SWidget> GetLabelWidget(const FName& InPinLabelStyle) override
	{
		return SVoxelGraphPin::InitializeEditableLabel(
			SharedThis(this),
			SGraphPinObject::GetLabelWidget(InPinLabelStyle));
	}
	//~ End SGraphPinObject Interface
};