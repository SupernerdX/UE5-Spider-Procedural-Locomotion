// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraphSchemaAction.h"

class UVoxelGraphNode_CallMemberFunction;

struct FVoxelGraphSchemaAction_CollapseToFunction : public FVoxelGraphSchemaAction
{
public:
	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;

	//~ Begin FVoxelGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	//~ End FVoxelGraphSchemaAction Interface

private:
	void GroupSelectedNodes(const TSet<UObject*>& SelectedNodes);
	void AddLocalVariableInputs(UVoxelTerminalGraph* Graph);
	void AddDeclarationOutputs(UVoxelTerminalGraph* Graph);
	void AddOuterInputsOutputs(UVoxelTerminalGraph* Graph);
	void FixupMainGraph(const UVoxelGraphNode_CallMemberFunction* FunctionNode, UEdGraph* EdGraph);

private:
	void ExportNodes(FString& ExportText) const;
	void ImportNodes(UVoxelTerminalGraph* Graph, const FString& ExportText);
	UE_506_SWITCH(FVector2D, FVector2f) GetNodePosition(const UEdGraphNode* Node) const;

private:
	struct FCopiedNode
	{
		TVoxelObjectPtr<UEdGraphNode> OriginalNode;
		TVoxelObjectPtr<UEdGraphNode> NewNode;
		TMap<FEdGraphPinReference, TSet<FEdGraphPinReference>> OutsideConnectedPins;
		TMap<FGuid, TSet<FEdGraphPinReference>> MappedInputsOutputs;

		FCopiedNode(UEdGraphNode* Node, const TMap<FEdGraphPinReference, TSet<FEdGraphPinReference>>& OutsideConnectedPins)
			: OriginalNode(Node)
			, OutsideConnectedPins(OutsideConnectedPins)
		{
		}

		template<typename T>
		T* GetOriginalNode()
		{
			return Cast<T>(OriginalNode.Resolve());
		}

		template<typename T>
		T* GetNewNode()
		{
			return Cast<T>(NewNode.Resolve());
		}
	};

	struct FCopiedLocalVariable
	{
		FGuid NewGuid;
		FGuid OldGuid;

		FGuid InputGuid;
		FGuid OutputGuid;

		TSharedPtr<FCopiedNode> DeclarationNode;
		TSet<TSharedPtr<FCopiedNode>> UsageNodes;

		explicit FCopiedLocalVariable(const FGuid OriginalParameterId)
			: OldGuid(OriginalParameterId)
		{}
	};

	TMap<FGuid, TSharedPtr<FCopiedNode>> CopiedNodes;
	TMap<FGuid, TSharedPtr<FCopiedLocalVariable>> CopiedLocalVariables;

	UE_506_SWITCH(FVector2D, FVector2f) AvgNodePosition;
	UE_506_SWITCH(FVector2D, FVector2f) InputDeclarationPosition;
	UE_506_SWITCH(FVector2D, FVector2f) OutputDeclarationPosition;
};