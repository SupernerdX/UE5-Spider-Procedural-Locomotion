// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelGraphEnvironment.h"
#include "Sculpt/Volume/Tools/VoxelGraphVolumeTool.h"
#include "VoxelParameterOverridesDetails.h"

VOXEL_CUSTOMIZE_CLASS(UVoxelGraphVolumeTool)(IDetailLayoutBuilder& DetailLayout)
{
	FVoxelEditorUtilities::EnableRealtime();

	KeepAlive(FVoxelParameterOverridesDetails::Create(
		DetailLayout,
		[&](TVoxelArray<FVoxelParameterOverridesDetails::FWeakOwner>& OutOwners)
		{
			for (UVoxelGraphVolumeTool* Tool : GetObjectsBeingCustomized(DetailLayout))
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
						Tool->PreEditChange(&FindFPropertyChecked(UVoxelGraphVolumeTool, ParameterOverrides));
					}),
					MakeWeakObjectPtrLambda(Tool, [=]
					{
						FPropertyChangedEvent Event(&FindFPropertyChecked(UVoxelGraphVolumeTool, ParameterOverrides));
						Tool->PostEditChangeProperty(Event);
					})
				});
			}
		},
		FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout)));
}