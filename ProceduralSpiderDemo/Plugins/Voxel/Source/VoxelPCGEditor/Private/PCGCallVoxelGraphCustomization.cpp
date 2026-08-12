// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "PCGCallVoxelGraph.h"
#include "VoxelGraphEnvironment.h"
#include "VoxelParameterOverridesDetails.h"

VOXEL_CUSTOMIZE_CLASS(UPCGCallVoxelGraphSettings)(IDetailLayoutBuilder& DetailLayout)
{
	FVoxelEditorUtilities::EnableRealtime();

	const TSharedRef<IPropertyHandle> GraphHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UPCGCallVoxelGraphSettings, Graph));
	GraphHandle->MarkHiddenByCustomization();

	// Force graph at the bottom
	DetailLayout
	.EditCategory(
		"Config",
		{},
		ECategoryPriority::Uncommon)
	.AddProperty(GraphHandle);

	KeepAlive(FVoxelParameterOverridesDetails::Create(
		DetailLayout,
		[&](TVoxelArray<FVoxelParameterOverridesDetails::FWeakOwner>& OutOwners)
		{
			for (UPCGCallVoxelGraphSettings* Settings : GetObjectsBeingCustomized(DetailLayout))
			{
				OutOwners.Add(FVoxelParameterOverridesDetails::FWeakOwner
				{
					Settings,
					MakeWeakObjectPtrLambda(Settings, [Settings]
					{
						return Settings->ParameterOverrides.GetHash();
					}),
					MakeWeakObjectPtrLambda(Settings, [Settings](FVoxelDependencyCollector& DependencyCollector)
					{
						return FVoxelGraphEnvironment::CreatePreview(
							Settings,
							*Settings,
							FTransform::Identity,
							DependencyCollector);
					}),
					MakeWeakObjectPtrLambda(Settings, [=]
					{
						Settings->PreEditChange(&FindFPropertyChecked(UPCGCallVoxelGraphSettings, ParameterOverrides));
					}),
					MakeWeakObjectPtrLambda(Settings, [=]
					{
						FPropertyChangedEvent Event(&FindFPropertyChecked(UPCGCallVoxelGraphSettings, ParameterOverrides));
						Settings->PostEditChangeProperty(Event);
					})
				});
			}
		},
		FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout)));
}