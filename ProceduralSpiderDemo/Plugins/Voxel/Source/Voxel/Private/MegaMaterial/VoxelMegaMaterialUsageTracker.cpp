// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelMegaMaterialUsageTracker.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialGeneratedData.h"
#include "Surface/VoxelSurfaceTypeInterface.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "Surface/VoxelCubicSurfaceTypeAsset.h"

#if WITH_EDITOR
void FVoxelMegaMaterialUsageTracker::Sync()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	UVoxelMegaMaterial* MegaMaterial = WeakMegaMaterial.Resolve();
	if (!ensureVoxelSlow(MegaMaterial))
	{
		return;
	}

	if (!MegaMaterial->bDetectNewSurfaces)
	{
		VOXEL_SCOPE_LOCK(CriticalSection);
		SurfaceTypesToAdd_RequiresLock.Empty();
		return;
	}

	TVoxelArray<FVoxelSurfaceType> NewSurfaceTypes;
	{
		VOXEL_SCOPE_LOCK(CriticalSection);

		NewSurfaceTypes = SurfaceTypesToAdd_RequiresLock.Array();
		SurfaceTypesToAdd_RequiresLock.Reset();
	}

	for (const FVoxelSurfaceType& SurfaceType : NewSurfaceTypes)
	{
		UVoxelSurfaceTypeInterface* SurfaceTypeInterface = SurfaceType.GetSurfaceTypeInterface().Resolve();
		if (!ensureVoxelSlow(SurfaceTypeInterface))
		{
			continue;
		}

		UVoxelCubicSurfaceTypeAsset* CubicSurfaceTypeAsset =
			SurfaceType.GetClass() == FVoxelSurfaceType::EClass::CubicSurfaceType
			? SurfaceType.GetCubicSurfaceTypeAsset().Resolve() 
			: nullptr;
		UVoxelSurfaceTypeAsset* SurfaceTypeAsset = 
			SurfaceType.GetClass() == FVoxelSurfaceType::EClass::SurfaceTypeAsset
			? SurfaceType.GetSurfaceTypeAsset().Resolve()
			: nullptr;

		if (CubicSurfaceTypeAsset
			? MegaMaterial->CubicSurfaceTypes.Contains(CubicSurfaceTypeAsset)
			: SurfaceTypeAsset && MegaMaterial->SurfaceTypes.Contains(SurfaceTypeAsset))
		{
			continue;
		}

		if (MegaMaterial->bEditorOnly)
		{
			MegaMaterial->Modify();
			if (CubicSurfaceTypeAsset)
			{
				MegaMaterial->CubicSurfaceTypes.Remove(nullptr);
				MegaMaterial->CubicSurfaceTypes.AddUnique(CubicSurfaceTypeAsset);
			}
			else if (SurfaceTypeAsset)
			{
				MegaMaterial->SurfaceTypes.Remove(nullptr);
				MegaMaterial->SurfaceTypes.AddUnique(SurfaceTypeAsset);
			}
			MegaMaterial->GetGeneratedData_EditorOnly().QueueRebuild();
			continue;
		}

		if (const TSharedPtr<FVoxelNotification> Notification = SurfaceTypeToWeakNotification.FindRef(SurfaceTypeInterface).Pin())
		{
			Notification->ResetExpiration();
			continue;
		}

		const TSharedRef<FVoxelNotification> Notification = FVoxelNotification::Create("Missing MegaMaterial entry");

		Notification->SetSubText(
			"Add " + SurfaceTypeInterface->GetName() + " to " + MegaMaterial->GetName() + "?\n\n" +
			"This is required to render this surface on a Voxel World");

		Notification->AddButton_ExpireOnClick(
			"Add",
			"Add this surface to the MegaMaterial array. This will rebuild the MegaMaterial.",
			MakeWeakObjectPtrLambda(MegaMaterial, MakeWeakPtrLambda(this, [this, MegaMaterial, bIsCubic = CubicSurfaceTypeAsset != nullptr, WeakSurfaceTypeInterface = MakeVoxelObjectPtr(SurfaceTypeInterface)]
			{
				UVoxelSurfaceTypeInterface* LocalSurfaceTypeInterface = WeakSurfaceTypeInterface.Resolve();
				if (!ensure(LocalSurfaceTypeInterface))
				{
					return;
				}

				MegaMaterial->Modify();
				if (bIsCubic)
				{
					UVoxelCubicSurfaceTypeAsset* LocalCubicSurfaceTypeAsset = CastChecked<UVoxelCubicSurfaceTypeAsset>(LocalSurfaceTypeInterface);
					MegaMaterial->CubicSurfaceTypes.Remove(nullptr);
					MegaMaterial->CubicSurfaceTypes.AddUnique(LocalCubicSurfaceTypeAsset);
				}
				else
				{
					UVoxelSurfaceTypeAsset* LocalSurfaceTypeAsset = CastChecked<UVoxelSurfaceTypeAsset>(LocalSurfaceTypeInterface);
					MegaMaterial->SurfaceTypes.Remove(nullptr);
					MegaMaterial->SurfaceTypes.AddUnique(LocalSurfaceTypeAsset);
				}
				MegaMaterial->GetGeneratedData_EditorOnly().QueueRebuild();
			})));

		Notification->ExpireAndFadeoutIn(10.f);

		SurfaceTypeToWeakNotification.FindOrAdd(SurfaceTypeInterface) = Notification;
	}
}

void FVoxelMegaMaterialUsageTracker::NotifyMissingSurfaceType(const FVoxelSurfaceType SurfaceType)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(!SurfaceType.IsNull()))
	{
		return;
	}

	VOXEL_SCOPE_LOCK(CriticalSection);

	if (SurfaceTypesToAdd_RequiresLock.Contains(SurfaceType))
	{
		return;
	}

	SurfaceTypesToAdd_RequiresLock.Add_EnsureNew(SurfaceType);

	Voxel::GameTask_Async(MakeWeakPtrLambda(this, [this]
	{
		Sync();
	}));
}
#endif