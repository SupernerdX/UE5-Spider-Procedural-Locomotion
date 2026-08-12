// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelSculptVolumeAsset.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "Bulk/VoxelUObjectBulkLoader.h"
#include "VoxelVersion.h"

DEFINE_VOXEL_FACTORY(UVoxelSculptVolumeAsset);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<IVoxelBulkLoader> UVoxelSculptVolumeAsset::GetBulkLoader() const
{
	return BulkLoader.ToSharedRef();
}

TVoxelBulkRef<FVoxelSculptVolumeData> UVoxelSculptVolumeAsset::GetData() const
{
	return PrivateData.ToBulkRef();
}

void UVoxelSculptVolumeAsset::SetData(const TVoxelBulkRef<FVoxelSculptVolumeData>& Data)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	(void)MarkPackageDirty();

	PrivateData = Data;
	OnDataChanged.Broadcast(Data);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSculptVolumeAsset::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	BulkLoader = MakeShared<FVoxelUObjectBulkLoader>();
	PrivateData = MakeVoxelBulkRef(MakeShared<FVoxelSculptVolumeData>());
}

void UVoxelSculptVolumeAsset::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	if (Ar.IsSaving() &&
		PrivateData.IsSet())
	{
		TVoxelSet<TVoxelObjectPtr<UObject>> WeakObjects;
		PrivateData.GatherObjects(WeakObjects);

		for (const TVoxelObjectPtr<UObject>& WeakObject : WeakObjects)
		{
			UObject* Object = WeakObject.Resolve();
			if (!Object)
			{
				continue;
			}

			ReferencedAssets.Add(Object);
		}
	}

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);

	FVoxelSerializationGuard Guard(Ar);

	bool bUseBulkLoader = bEnableStreaming;
	if (Ar.CustomVer(GVoxelCustomVersionGUID) >= FVoxelVersion::AddBulkDataToSaveAssets)
	{
		Ar << bUseBulkLoader;
	}
	else
	{
		bUseBulkLoader = false;
	}

	if (!bUseBulkLoader)
	{
		if (Ar.IsLoading())
		{
			const TSharedRef<FVoxelSculptVolumeData> NewData = MakeShared<FVoxelSculptVolumeData>();
			NewData->Serialize(Ar);
			PrivateData = MakeVoxelBulkRef(NewData);
			return;
		}

		if (!PrivateData.IsSet())
		{
			return;
		}

		ConstCast(*PrivateData).Serialize(Ar);
		return;
	}

	FVoxelBulkHash RootHash;

	if (Ar.IsSaving())
	{
		RootHash = PrivateData.GetHash();

		const bool bSuccess = BulkLoader->Save(
			{ PrivateData },
			MaxWastedMB * 1024 * 1024);

		if (!ensure(bSuccess))
		{
			VOXEL_MESSAGE(Error, "{0} failed to save, see log for more info", this);
		}
	}

	Ar << RootHash;

	BulkLoader->Serialize(Ar, this);

	if (Ar.IsLoading())
	{
		const TVoxelBulkPtr<FVoxelSculptVolumeData> BulkPtr(RootHash);

		PrivateData = MakeVoxelBulkRef(BulkPtr.LoadSync(*BulkLoader));
	}
}