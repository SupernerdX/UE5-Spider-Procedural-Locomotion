// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class VOXELBLUEPRINTEDITOR_API SVoxelGraphPinStampRef : public SGraphPin
{
public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
	{
		SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
	}

protected:
	//~ Begin SGraphPin Interface
	virtual FSlateColor GetPinColor() const override
	{
		const UEdGraphPin* GraphPin = GetPinObj();
		if (!GraphPin ||
			GraphPin->IsPendingKill())
		{
			return FLinearColor::White;
		}

		if (bIsDiffHighlighted)
		{
			return FSlateColor(FLinearColor(0.9f, 0.2f, 0.15f));
		}

		if (GraphPin->bOrphanedPin)
		{
			return FSlateColor(FLinearColor::Red);
		}

		const FLinearColor PinColor = GetDefault<UGraphEditorSettings>()->ObjectPinTypeColor;
		if (!GetPinObj()->GetOwningNode()->IsNodeEnabled() ||
			GetPinObj()->GetOwningNode()->IsDisplayAsDisabledForced() ||
			!IsEditingEnabled() ||
			GetPinObj()->GetOwningNode()->IsNodeUnrelated())
		{
			return PinColor * FLinearColor(1.0f, 1.0f, 1.0f, 0.5f);
		}

		return PinColor * PinColorModifier;
	}
	//~ End SGraphPin Interface
};