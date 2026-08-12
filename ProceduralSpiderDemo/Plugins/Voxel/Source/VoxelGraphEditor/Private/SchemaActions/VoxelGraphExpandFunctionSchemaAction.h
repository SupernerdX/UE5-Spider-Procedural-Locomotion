// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraphSchemaAction.h"

class UVoxelGraph;
class UVoxelGraphNode_CallMemberFunction;

struct FVoxelGraphSchemaAction_ExpandFunction : public FVoxelGraphSchemaAction
{
public:
	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;

	//~ Begin FVoxelGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	//~ End FVoxelGraphSchemaAction Interface

private:
	void FindNearestSuitableLocation(const TSharedPtr<SGraphEditor>& GraphEditor, const UVoxelGraphNode_CallMemberFunction* FunctionNode, const UVoxelTerminalGraph* FunctionGraph);
	void GroupNodesToCopy(const UEdGraph& FunctionEdGraph);
	void ConnectNewNodes(const UVoxelGraphNode_CallMemberFunction* FunctionNode);

private:
	void ExportNodes(FString& ExportText) const;
	void ImportNodes(const TSharedPtr<SGraphEditor>& GraphEditor, UEdGraph* DestGraph, const FString& ExportText);

private:
	struct FCopiedNode
	{
		TVoxelObjectPtr<UEdGraphNode> OriginalNode;
		TVoxelObjectPtr<UEdGraphNode> NewNode;
		TMap<FEdGraphPinReference, TSet<FGuid>> MappedOriginalPinsToInputsOutputs;
	};

	FIntPoint SuitablePosition;
	FSlateRect PastedNodesBounds = FSlateRect(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
	TMap<FGuid, TSharedPtr<FCopiedNode>> CopiedNodes;
};