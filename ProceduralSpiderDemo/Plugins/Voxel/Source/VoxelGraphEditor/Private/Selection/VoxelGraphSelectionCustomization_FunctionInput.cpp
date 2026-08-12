// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Selection/VoxelGraphSelectionCustomization_FunctionInput.h"
#include "VoxelTerminalGraph.h"
#include "Nodes/VoxelGraphNode_FunctionInput.h"

void FVoxelGraphSelectionCustomization_FunctionInput::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	DetailLayout.HideCategory("Config");
	DetailLayout.HideCategory("Function");

	if (!Guid.IsValid())
	{
		return;
	}

	const TSharedRef<FVoxelStructDetailsWrapper> Wrapper = FVoxelStructDetailsWrapper::Make<UVoxelTerminalGraph, FVoxelGraphFunctionInput>(
		DetailLayout,
		[Guid = Guid](const UVoxelTerminalGraph& InTerminalGraph)
		{
			return InTerminalGraph.FindInput(Guid);
		},
		[Guid = Guid](UVoxelTerminalGraph& InTerminalGraph, const FVoxelGraphFunctionInput& NewInput)
		{
			InTerminalGraph.UpdateFunctionInput(Guid, [&](FVoxelGraphFunctionInput& InInput)
			{
				InInput = NewInput;
			});
		});
	KeepAlive(Wrapper);

	Wrapper->InstanceMetadataMap.Add("ShowDefaultValue", bHasInputDefaultNode ? "false" : "true");

	IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Input", INVTEXT("Input"));
	Wrapper->AddChildrenTo(Category);
}