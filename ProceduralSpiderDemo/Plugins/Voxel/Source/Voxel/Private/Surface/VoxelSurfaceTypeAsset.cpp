// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSurfaceTypeAsset.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialGeneratedData.h"

DEFINE_VOXEL_FACTORY(UVoxelSurfaceTypeAsset);

void UVoxelSurfaceTypeAsset::PostInitProperties()
{
	Super::PostInitProperties();

	FVoxelUtilities::DelayedCall(MakeWeakObjectPtrLambda(this, [this]
	{
		if (Seed.Seed.IsEmpty())
		{
			Seed.Randomize();

			MarkPackageDirty();
		}
	}));
}

void UVoxelSurfaceTypeAsset::PostDuplicate(const EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	Seed.Randomize();
}

#if WITH_EDITOR
void UVoxelSurfaceTypeAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	bool bRefreshMegaMaterials = false;
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_OWN_MEMBER_NAME(Seed) ||
		PropertyChangedEvent.GetMemberPropertyName() == GET_OWN_MEMBER_NAME(BlendSmoothness))
	{
		bRefreshMegaMaterials = true;
	}

	if (Seed.Seed.IsEmpty())
	{
		Seed.Randomize();
		bRefreshMegaMaterials = true;
	}

	if (bRefreshMegaMaterials)
	{
		ForEachObjectOfClass_Copy<UVoxelMegaMaterial>([&](UVoxelMegaMaterial& MegaMaterial)
		{
			if (MegaMaterial.SurfaceTypes.Contains(this))
			{
				MegaMaterial.GetGeneratedData_EditorOnly().QueueRebuild();
			}
		});
	}
}
#endif