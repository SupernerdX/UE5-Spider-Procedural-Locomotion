// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelCompilationGraph.h"
#include "Nodes/VoxelGraphNode.h"
#include "VoxelGraphDebugNode.generated.h"

UCLASS()
class VOXELGRAPHEDITOR_API UVoxelGraphDebugNode : public UVoxelGraphNode
{
	GENERATED_BODY()

public:
	TSharedPtr<const Voxel::Graph::FGraph> Graph;
	const Voxel::Graph::FNode* Node = nullptr;

	TMap<const Voxel::Graph::FPin*, UEdGraphPin*> PinMap;

public:
	//~ Begin UVoxelGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override;

	virtual bool CanJumpToDefinition() const override;
	virtual void JumpToDefinition() const override;
	//~ End UVoxelGraphNode Interface
};