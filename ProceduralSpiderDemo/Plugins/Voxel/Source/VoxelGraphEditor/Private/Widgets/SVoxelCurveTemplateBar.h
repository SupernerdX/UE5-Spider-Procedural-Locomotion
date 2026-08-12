// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Curves/CurveFloat.h"

class FCurveEditor;

class SVoxelCurveTemplateBar : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(const FArguments& InArgs, const TSharedRef<FCurveEditor>& InCurveEditor);

private:
	FReply CurveTemplateClicked(TVoxelObjectPtr<UCurveFloat> FloatCurveAssetWeak) const;

private:
	TSharedPtr<FCurveEditor> CurveEditor;
};