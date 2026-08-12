// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialCache.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "MegaMaterial/VoxelMegaMaterialUsageTracker.h"
#include "MegaMaterial/VoxelMegaMaterialGeneratedData.h"
#include "Surface/VoxelSurfaceTypeInterface.h"
#include "Surface/VoxelCubicSurfaceTypeAsset.h"
#include "Surface/VoxelCubicTextureTemplate.h"
#include "Materials/MaterialInterface.h"

DEFINE_VOXEL_FACTORY(UVoxelMegaMaterial);

#if WITH_EDITOR
UVoxelMegaMaterial* UVoxelMegaMaterial::CreateTransient()
{
	UVoxelMegaMaterial* Result = NewObject<UVoxelMegaMaterial>();
	Result->bEditorOnly = true;
	Result->NonNaniteMaterialType = EVoxelMegaMaterialGenerationType::Generated;
	Result->NaniteDisplacementMaterialType = EVoxelMegaMaterialGenerationType::Generated;
	Result->LumenMaterialType = EVoxelMegaMaterialGenerationType::Custom;
	return Result;
}
#endif

UVoxelMegaMaterial::UVoxelMegaMaterial()
{
	DitherNoiseTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/EngineMaterials/Good64x64TilingNoiseHighFreq.Good64x64TilingNoiseHighFreq")));

#if WITH_EDITOR
	UsageTracker = MakeShared<FVoxelMegaMaterialUsageTracker>(*this);
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelMegaMaterialProxy> UVoxelMegaMaterial::GetProxy()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!Proxy)
	{
		Proxy = MakeShareable(new FVoxelMegaMaterialProxy(*this));
		Proxy->Initialize(nullptr);
	}

	return Proxy.ToSharedRef();
}

#if WITH_EDITOR
UVoxelMegaMaterialGeneratedData& UVoxelMegaMaterial::GetGeneratedData_EditorOnly()
{
	if (!GeneratedData)
	{
		GeneratedData = &UVoxelMegaMaterialCache::GetGeneratedData(*this);
	}

	check(GeneratedData);
	return *GeneratedData;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelMegaMaterial::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	UVoxelSurfaceTypeInterface::Migrate(Materials, SurfaceTypes);
#endif

	GeneratedData = CookedGeneratedData;

#if WITH_EDITOR
	FVoxelUtilities::DelayedCall(MakeWeakObjectPtrLambda(this, [this]
	{
		GetGeneratedData_EditorOnly().QueueRebuild();
	}));
#endif
}

void UVoxelMegaMaterial::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
void UVoxelMegaMaterial::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
	{
		return;
	}

	// Update template for all cubic surfaces when template changes
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UVoxelMegaMaterial, CubicTextureTemplate) ||
		PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UVoxelMegaMaterial, CubicSurfaceTypes))
	{
		for (UVoxelCubicSurfaceTypeAsset* CubicSurface : CubicSurfaceTypes)
		{
			if (CubicSurface)
			{
				CubicSurface->Template = CubicTextureTemplate;
				CubicSurface->SyncWithTemplate();
			}
		}
	}

	UsageTracker->Sync();

	GetGeneratedData_EditorOnly().QueueRebuild();
}

void UVoxelMegaMaterial::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	VOXEL_FUNCTION_COUNTER();

	Super::BeginCacheForCookedPlatformData(TargetPlatform);

	LOG_VOXEL(Log, "UVoxelMegaMaterial::BeginCacheForCookedPlatformData");

	ensureVoxelSlow(!CookedGeneratedData);

	CookedGeneratedData = NewObject<UVoxelMegaMaterialGeneratedData>(this, "GeneratedMaterials");
	CookedGeneratedData->SetMegaMaterial(this);
	CookedGeneratedData->RebuildNow();
}
#endif
