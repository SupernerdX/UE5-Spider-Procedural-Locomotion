// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampActorCustomizationBase.h"
#include "VoxelStampActor.h"
#include "Sculpt/Height/VoxelSculptHeight.h"
#include "Sculpt/Volume/VoxelSculptVolume.h"

DEFINE_VOXEL_CLASS_LAYOUT(AVoxelStampActor, FVoxelStampActorCustomizationBase);
DEFINE_VOXEL_CLASS_LAYOUT(AVoxelSculptHeight, FVoxelStampActorCustomizationBase);
DEFINE_VOXEL_CLASS_LAYOUT(AVoxelSculptVolume, FVoxelStampActorCustomizationBase);

void FVoxelStampActorCustomizationBase::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
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

	FVoxelEditorUtilities::HideAndMoveToCategory(DetailLayout, "Actor", "Misc", { GET_MEMBER_NAME_STATIC(AActor, Tags) }, false);
	FVoxelEditorUtilities::HideAndMoveToCategory(DetailLayout, "WorldPartition", "Misc");
	FVoxelEditorUtilities::HideAndMoveToCategory(DetailLayout, "LevelInstance", "Misc");

	TArray<FName> Categories;
	DetailLayout.GetCategoryNames(Categories);
	for (const FName Category : Categories)
	{
		FString CategoryName = Category.ToString();
		if (!CategoryName.RemoveFromStart("Voxel ") ||
			CategoryName.Contains("|"))
		{
			continue;
		}

		DetailLayout.EditCategory(Category, FText::FromString(CategoryName));
	}
}