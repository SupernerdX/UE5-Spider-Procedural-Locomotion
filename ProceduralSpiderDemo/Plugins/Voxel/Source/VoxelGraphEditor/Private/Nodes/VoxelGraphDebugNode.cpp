// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelGraphDebugNode.h"

void UVoxelGraphDebugNode::AllocateDefaultPins()
{
	if (!Node)
	{
		return;
	}

	PinMap.Reset();
	for (const Voxel::Graph::FPin& Pin : Node->GetPins())
	{
		UEdGraphPin* GraphPin = CreatePin(
			Pin.Direction == Voxel::Graph::EPinDirection::Input ? EGPD_Input : EGPD_Output,
			Pin.Type.GetEdGraphPinType(),
			Pin.Name);

		GraphPin->PinToolTip = "Name=" + Pin.Name.ToString() + "\nType=" + Pin.Type.ToString();

		if (Pin.Direction == Voxel::Graph::EPinDirection::Input)
		{
			FVoxelPinValue DefaultValue = Pin.GetDefaultValue();
			if (DefaultValue.IsValid() &&
				DefaultValue.GetType().HasPinDefaultValue())
			{
				DefaultValue.ApplyToPinDefaultValue(*GraphPin);
			}
		}

		PinMap.Add(&Pin, GraphPin);
	}

	Super::AllocateDefaultPins();
}

FText UVoxelGraphDebugNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (!Node)
	{
		return {};
	}

	return FText::FromName(Node->NodeRef.EdGraphNodeTitle);
}

FLinearColor UVoxelGraphDebugNode::GetNodeTitleColor() const
{
	if (!Node)
	{
		return Super::GetNodeTitleColor();
	}

	const UEdGraphNode* EdNode = Node->NodeRef.GetGraphNode_EditorOnly();
	if (!EdNode)
	{
		return Super::GetNodeTitleColor();
	}

	return EdNode->GetNodeTitleColor();
}

FText UVoxelGraphDebugNode::GetTooltipText() const
{
	if (!Node)
	{
		return Super::GetTooltipText();
	}

	const UEdGraphNode* EdNode = Node->NodeRef.GetGraphNode_EditorOnly();
	if (!EdNode)
	{
		return Super::GetTooltipText();
	}

	return EdNode->GetTooltipText();
}

bool UVoxelGraphDebugNode::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const
{
	return false;
}

bool UVoxelGraphDebugNode::CanJumpToDefinition() const
{
	if (!Node)
	{
		return false;
	}

	const UEdGraphNode* EdNode = Node->NodeRef.GetGraphNode_EditorOnly();
	if (!EdNode)
	{
		return false;
	}

	return true;
}

void UVoxelGraphDebugNode::JumpToDefinition() const
{
	if (!Node)
	{
		return;
	}

	const UEdGraphNode* EdNode = Node->NodeRef.GetGraphNode_EditorOnly();
	if (!EdNode)
	{
		return;
	}

	FVoxelUtilities::FocusObject(EdNode);
}