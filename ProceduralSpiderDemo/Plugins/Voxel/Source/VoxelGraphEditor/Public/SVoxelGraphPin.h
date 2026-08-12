// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class VOXELGRAPHEDITOR_API SVoxelGraphPin : public SGraphPin
{
public:
	VOXEL_SLATE_ARGS()
	{
		
	};

	void Construct(const FArguments& Args, UEdGraphPin* InGraphPinObj)
	{
		SGraphPin::Construct({}, InGraphPinObj);
		ConstructDebugHighlight(this);
	}

	virtual TSharedRef<SWidget> GetLabelWidget(const FName& InPinLabelStyle) override
	{
		return InitializeEditableLabel(
			SharedThis(this),
			SGraphPin::GetLabelWidget(InPinLabelStyle));
	}

public:
	static TSharedRef<SWidget> InitializeEditableLabel(const TSharedPtr<SGraphPin>& PinWidget, const TSharedRef<SWidget>& DefaultWidget);
	static void ConstructDebugHighlight(SGraphPin* PinWidget);
};