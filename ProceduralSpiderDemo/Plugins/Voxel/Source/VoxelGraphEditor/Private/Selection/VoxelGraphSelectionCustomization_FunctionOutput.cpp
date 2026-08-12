// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Selection/VoxelGraphSelectionCustomization_FunctionOutput.h"
#include "VoxelTerminalGraph.h"

void FVoxelGraphSelectionCustomization_FunctionOutput::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	DetailLayout.HideCategory("Config");
	DetailLayout.HideCategory("Function");

	if (!Guid.IsValid())
	{
		return;
	}

	const TSharedRef<FVoxelStructDetailsWrapper> Wrapper = FVoxelStructDetailsWrapper::Make<UVoxelTerminalGraph, FVoxelGraphFunctionOutput>(
		DetailLayout,
		[Guid = Guid](const UVoxelTerminalGraph& InTerminalGraph)
		{
			return InTerminalGraph.FindOutput(Guid);
		},
		[Guid = Guid](UVoxelTerminalGraph& InTerminalGraph, const FVoxelGraphFunctionOutput& NewOutput)
		{
			InTerminalGraph.UpdateFunctionOutput(Guid, [&](FVoxelGraphFunctionOutput& InOutput)
			{
				InOutput = NewOutput;
			});
		});
	KeepAlive(Wrapper);

	IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Output", INVTEXT("Output"));
	Wrapper->AddChildrenTo(Category);
}