// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelGraphEnvironment.h"
#include "VoxelParameterOverridesDetails.h"
#include "Scatter/VoxelScatterActor.h"

VOXEL_CUSTOMIZE_CLASS(AVoxelScatterActor)(IDetailLayoutBuilder& DetailLayout)
{
	FVoxelEditorUtilities::EnableRealtime();

	DetailLayout.HideCategory("Rendering");
	DetailLayout.HideCategory("Replication");
	DetailLayout.HideCategory("Input");
	DetailLayout.HideCategory("Collision");
	DetailLayout.HideCategory("LOD");
	DetailLayout.HideCategory("HLOD");
	DetailLayout.HideCategory("Cooking");
	DetailLayout.HideCategory("DataLayers");
	DetailLayout.HideCategory("Networking");
	DetailLayout.HideCategory("Physics");

	KeepAlive(FVoxelParameterOverridesDetails::Create(
		DetailLayout,
		[&](TVoxelArray<FVoxelParameterOverridesDetails::FWeakOwner>& OutOwners)
		{
			for (AVoxelScatterActor* Actor : GetObjectsBeingCustomized(DetailLayout))
			{
				OutOwners.Add(FVoxelParameterOverridesDetails::FWeakOwner
				{
					Actor,
					MakeWeakObjectPtrLambda(Actor, [Actor]
					{
						return Actor->GetParameterOverrides().GetHash();
					}),
					MakeWeakObjectPtrLambda(Actor, [Actor](FVoxelDependencyCollector& DependencyCollector)
					{
						return FVoxelGraphEnvironment::CreatePreview(
							Actor,
							*Actor,
							FTransform::Identity,
							DependencyCollector);
					}),
					MakeWeakObjectPtrLambda(Actor, [=]
					{
						Actor->PreEditChange(&FindFPropertyChecked(AVoxelScatterActor, ParameterOverrides));
					}),
					MakeWeakObjectPtrLambda(Actor, [=]
					{
						FPropertyChangedEvent Event(&FindFPropertyChecked(AVoxelScatterActor, ParameterOverrides));
						Actor->PostEditChangeProperty(Event);
					})
				});
			}
		},
		FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout)));
}