// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Nodes/VoxelGraphNode.h"
#include "VoxelGraphNode_CallParentMainGraph.generated.h"

class UVoxelGraphNode_Struct;
class UVoxelGraph;
class UVoxelTerminalGraph;
class UVoxelFunctionLibraryAsset;

UCLASS()
class UVoxelGraphNode_CallParentMainGraph : public UVoxelGraphNode
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual bool ShowPaletteIconOnNode() const override { return true; }

	virtual bool CanJumpToDefinition() const override { return true; }
	virtual void JumpToDefinition() const override;

	virtual FName GetPinCategory(const UEdGraphPin& Pin) const override;
	virtual TSharedRef<IVoxelNodeDefinition> GetNodeDefinition() override;

	virtual bool CanPasteHere(const UEdGraph* TargetGraph) const override;
	//~ End UVoxelGraphNode Interface

	const UVoxelTerminalGraph* GetBaseTerminalGraph() const;

private:
	FSharedVoidPtr OnChangedPtr;
	TWeakObjectPtr<UVoxelGraphNode_Struct> WeakOutputNode;

	friend class FVoxelNodeDefinition_CallParentMainGraph;
};

class FVoxelNodeDefinition_CallParentMainGraph : public IVoxelNodeDefinition
{
public:
	UVoxelGraphNode_CallParentMainGraph& Node;

	explicit FVoxelNodeDefinition_CallParentMainGraph(UVoxelGraphNode_CallParentMainGraph& Node)
		: Node(Node)
	{
	}

	//~ Begin IVoxelNodeDefinition Interface
	virtual TSharedPtr<const FNode> GetOutputs() const override;

	virtual bool OverridePinsOrder() const override
	{
		return true;
	}
	//~ End IVoxelNodeDefinition Interface
};