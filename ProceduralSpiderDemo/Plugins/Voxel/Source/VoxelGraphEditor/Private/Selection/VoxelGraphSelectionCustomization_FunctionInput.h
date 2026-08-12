// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class UVoxelGraphNode_FunctionInput;

class FVoxelGraphSelectionCustomization_FunctionInput : public FVoxelDetailCustomization
{
public:
	const FGuid Guid;
	const bool bHasInputDefaultNode;

	FVoxelGraphSelectionCustomization_FunctionInput(const FGuid& Guid, const bool bHasInputDefaultNode)
		: Guid(Guid)
		, bHasInputDefaultNode(bHasInputDefaultNode)
	{
	}

	//~ Begin IDetailCustomization Interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	//~ End IDetailCustomization Interface
};