// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelParameter.h"
#include "Nodes/VoxelGraphNode.h"
#include "VoxelGraphNode_CustomizeParameter.generated.h"

UCLASS()
class UVoxelGraphNode_CustomizeParameter : public UVoxelGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGuid Guid;

	UPROPERTY()
	FVoxelParameter CachedParameter;

	const FVoxelParameter* GetParameter() const;
	FVoxelParameter GetParameterSafe() const;

	//~ Begin UVoxelGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual bool CanPasteHere(const UEdGraph* TargetGraph) const override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual bool ShowPaletteIconOnNode() const override { return true; }
	virtual FString GetSearchTerms() const override;
	//~ End UVoxelGraphNode Interface

private:
	FSharedVoidPtr OnParameterChangedPtr;
};