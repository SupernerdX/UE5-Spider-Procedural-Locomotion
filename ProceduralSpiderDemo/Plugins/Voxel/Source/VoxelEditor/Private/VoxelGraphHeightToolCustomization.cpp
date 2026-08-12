// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelGraphEnvironment.h"
#include "VoxelParameterOverridesDetails.h"
#include "Sculpt/Height/Tools/VoxelGraphHeightTool.h"

VOXEL_CUSTOMIZE_CLASS(UVoxelGraphHeightTool)(IDetailLayoutBuilder& DetailLayout)
{
	FVoxelEditorUtilities::EnableRealtime();

	KeepAlive(FVoxelParameterOverridesDetails::Create(
		DetailLayout,
		[&](TVoxelArray<FVoxelParameterOverridesDetails::FWeakOwner>& OutOwners)
		{
			for (UVoxelGraphHeightTool* Tool : GetObjectsBeingCustomized(DetailLayout))
			{
				OutOwners.Add(FVoxelParameterOverridesDetails::FWeakOwner
				{
					Tool,
					MakeWeakObjectPtrLambda(Tool, [Tool]
					{
						return Tool->GetParameterOverrides().GetHash();
					}),
					MakeWeakObjectPtrLambda(Tool, [Tool](FVoxelDependencyCollector& DependencyCollector)
					{
						return FVoxelGraphEnvironment::CreatePreview(
							Tool,
							*Tool,
							FTransform::Identity,
							DependencyCollector);
					}),
					MakeWeakObjectPtrLambda(Tool, [=]
					{
						Tool->PreEditChange(&FindFPropertyChecked(UVoxelGraphHeightTool, ParameterOverrides));
					}),
					MakeWeakObjectPtrLambda(Tool, [=]
					{
						FPropertyChangedEvent Event(&FindFPropertyChecked(UVoxelGraphHeightTool, ParameterOverrides));
						Tool->PostEditChangeProperty(Event);
					})
				});
			}
		},
		FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout)));
}