// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "SVoxelGraphNode.h"

class SVoxelGraphNodeVariable : public SVoxelGraphNode
{
public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(const FArguments& InArgs, UVoxelGraphNode* InNode);

	//~ Begin SVoxelGraphNode Interface
	virtual void UpdateGraphNode() override;
	virtual const FSlateBrush* GetShadowBrush(bool bSelected) const override;
	virtual void GetOverlayBrushes(bool bSelected, UE_506_SWITCH(FVector2D, const FVector2f&) WidgetSize, TArray<FOverlayBrushInfo>& Brushes) const override;
	//~ End SVoxelGraphNode Interface

private:
	FSlateColor GetVariableColor() const;
	static TSharedRef<SWidget> UpdateTitleWidget(const FText& InTitleText);
};