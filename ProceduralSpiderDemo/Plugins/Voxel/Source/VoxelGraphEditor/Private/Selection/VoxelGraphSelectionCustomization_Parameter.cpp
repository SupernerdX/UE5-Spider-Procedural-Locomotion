// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Selection/VoxelGraphSelectionCustomization_Parameter.h"
#include "VoxelGraph.h"
#include "VoxelParameterOverridesDetails.h"

void FVoxelGraphSelectionCustomization_Parameter::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	if (!Guid.IsValid())
	{
		return;
	}

	const TSharedRef<FVoxelStructDetailsWrapper> Wrapper = FVoxelStructDetailsWrapper::Make<UVoxelGraph, FVoxelParameter>(
		DetailLayout,
		[Guid = Guid](const UVoxelGraph& InGraph)
		{
			return InGraph.FindParameter(Guid);
		},
		[Guid = Guid](UVoxelGraph& InGraph, const FVoxelParameter& NewParameter)
		{
			InGraph.UpdateParameter(Guid, [&](FVoxelParameter& InParameter)
			{
				InParameter = NewParameter;
			});
		});
	KeepAlive(Wrapper);

	Wrapper->InstanceMetadataMap.Add("FilterTypes", "AllParameters");

	IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Parameter", INVTEXT("Parameter"));
	Wrapper->AddChildrenTo(Category);

	KeepAlive(FVoxelParameterOverridesDetails::CreateForParameter(
		DetailLayout,
		[&](TVoxelArray<FVoxelParameterOverridesDetails::FWeakOwner>& OutOwners)
		{
			for (UVoxelGraph* Graph : FVoxelEditorUtilities::GetObjectsBeingCustomized<UVoxelGraph>(DetailLayout))
			{
				OutOwners.Add(FVoxelParameterOverridesDetails::FWeakOwner
				{
					Graph,
					nullptr,
					nullptr,
					MakeWeakObjectPtrLambda(Graph, [=]
					{
						Graph->PreEditChange(Graph->GetParameterOverridesProperty());
					}),
					MakeWeakObjectPtrLambda(Graph, [=]
					{
						FPropertyChangedEvent Event(Graph->GetParameterOverridesProperty());
						Graph->PostEditChangeProperty(Event);
					})
				});
			}
		},
		FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout),
		Guid));
}