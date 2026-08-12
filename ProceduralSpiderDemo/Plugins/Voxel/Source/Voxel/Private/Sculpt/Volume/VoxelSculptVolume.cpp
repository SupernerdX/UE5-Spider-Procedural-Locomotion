// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelSculptVolume.h"
#include "Sculpt/Volume/VoxelVolumeModifier.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "Sculpt/Volume/VoxelSculptVolumeCache.h"
#include "Sculpt/Volume/VoxelSculptVolumeLocalData.h"
#include "Sculpt/Volume/VoxelSculptVolumeNetworkClient.h"
#include "Sculpt/Volume/VoxelSculptVolumeNetworkServer.h"
#include "Sculpt/Volume/VoxelVolumeSculptPreviousDistanceProvider.h"
#include "Sculpt/VoxelSculptSave.h"
#include "VoxelLayers.h"
#include "VoxelVersion.h"
#include "VoxelDependency.h"
#include "Bulk/VoxelBulkLoader.h"
#include "Bulk/VoxelDummyBulkLoader.h"
#include "Surface/VoxelSurfaceTypeTable.h"

#include "TransactionCommon.h"

#if WITH_EDITOR
#include "LevelEditor.h"
#include "Editor/TransBuffer.h"
#endif

class FVoxelSculptVolumeSingleton : public FVoxelSingleton
{
public:
	TVoxelMap<FGuid, TVoxelBulkPtr<FVoxelSculptVolumeData>> GuidToData;

	//~ Begin FVoxelSingleton Interface
	virtual void Tick() override
	{
#if WITH_EDITOR
		if (!GIsTransacting &&
			GEditor &&
			GEditor->Trans &&
			CastChecked<UTransBuffer>(GEditor->Trans)->UndoBuffer.Num() == 0)
		{
			GuidToData.Reset();
		}
#endif
	}
	//~ End FVoxelSingleton Interface
};
FVoxelSculptVolumeSingleton* GVoxelSculptVolumeSingleton = new FVoxelSculptVolumeSingleton();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVolumeSculptStampRef UVoxelSculptVolumeComponent::GetStamp() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(PrivateStamp))
	{
		ConstCast(PrivateStamp) = FVoxelVolumeSculptStampRef::New();
	}

	const AVoxelSculptVolume& SculptActor = *GetOuterAVoxelSculptVolume();

	PrivateStamp->WeakSculptActor = SculptActor;

	// DataSource will be null during UVoxelStampComponentBase::PostInitProperties
	if (SculptActor.DataSource)
	{
		if (const TSharedPtr<FVoxelSculptVolumeLocalData> LocalData = SculptActor.DataSource->AsLocalData())
		{
			LocalData->SetAsset(ExternalAsset);
		}
	}

	return PrivateStamp;
}

void UVoxelSculptVolumeComponent::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	INLINE_LAMBDA
	{
		if (HasAnyFlags(RF_ClassDefaultObject) ||
			ExternalAsset)
		{
			return;
		}

		const AVoxelSculptVolume* SculptActor = GetOuterAVoxelSculptVolume();
		if (!SculptActor ||
			SculptActor->HasAnyFlags(RF_ClassDefaultObject))
		{
			return;
		}

		const TSharedPtr<IVoxelSculptVolumeDataSource> DataSource = SculptActor->DataSource;
		if (!DataSource)
		{
			ensureVoxelSlow(SculptActor->HasAllFlags(RF_NeedInitialization));
			return;
		}

		const TSharedPtr<FVoxelSculptVolumeLocalData> LocalData = DataSource->AsLocalData();
		if (!ensureVoxelSlow(LocalData))
		{
			return;
		}

		TVoxelSet<TVoxelObjectPtr<UObject>> WeakObjects;
		LocalData->GetData().GatherObjects(WeakObjects);

		for (const TVoxelObjectPtr<UObject>& WeakObject : WeakObjects)
		{
			UObject* Object = WeakObject.Resolve();
			if (!Object)
			{
				continue;
			}

			ReferencedAssets.Add(Object);
		}
	};

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	FVoxelSerializationGuard Guard(Ar);

	if (Ar.CustomVer(GVoxelCustomVersionGUID) >= FVoxelVersion::AddExternalSculptSaves)
	{
		bool bHasExternalSaveAsset = ExternalAsset != nullptr;
		Ar << bHasExternalSaveAsset;

		if (bHasExternalSaveAsset)
		{
			return;
		}
	}

	const AVoxelSculptVolume* SculptActor = GetOuterAVoxelSculptVolume();
	if (!SculptActor ||
		SculptActor->HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	const TSharedPtr<IVoxelSculptVolumeDataSource> DataSource = SculptActor->DataSource;
	if (!DataSource)
	{
		ensureVoxelSlow(SculptActor->HasAllFlags(RF_NeedInitialization));
		return;
	}

	const TSharedPtr<FVoxelSculptVolumeLocalData> LocalData = DataSource->AsLocalData();
	if (!ensureVoxelSlow(LocalData))
	{
		return;
	}

	if (Ar.IsTransacting() ||
		UE::Transaction::DiffUtil::IsGeneratingDiffableObject(Ar))
	{
		// Don't serialize bulk data, that would be too big/take too long
		// Instead, store a shared snapshot of the data globally and serialize a GUID to it
		// This data is cleared when the undo transaction buffer is cleared

		FGuid Guid;

		if (Ar.IsSaving())
		{
			Guid = FGuid::NewGuid();
			GVoxelSculptVolumeSingleton->GuidToData.Add_EnsureNew(Guid, LocalData->GetData());
		}

		Ar << Guid;

		if (Ar.IsLoading())
		{
			const TVoxelBulkPtr<FVoxelSculptVolumeData> Data = GVoxelSculptVolumeSingleton->GuidToData.FindRef(Guid);
			if (ensureVoxelSlow(Data))
			{
				LocalData->SetSculptData(
					Data.ToBulkRef(),
					MakeShared<FVoxelDummyBulkLoader>());
			}
		}

		return;
	}

	if (Ar.IsLoading())
	{
		const TSharedRef<FVoxelSculptVolumeData> NewData = MakeShared<FVoxelSculptVolumeData>();
		NewData->Serialize(Ar);
		LocalData->SetSculptData(
			MakeVoxelBulkRef(NewData),
			MakeShared<FVoxelDummyBulkLoader>());
		return;
	}

	ConstCast(*LocalData->GetData()).Serialize(Ar);
}

FVoxelStampRef UVoxelSculptVolumeComponent::GetStamp_Internal() const
{
	return GetStamp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelSculptVolume::AVoxelSculptVolume()
{
	bReplicates = true;

	Component = CreateDefaultSubobject<UVoxelSculptVolumeComponent>("RootComponent");
	RootComponent = Component;
}

void AVoxelSculptVolume::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	Dependency = FVoxelDependency3D::Create("VoxelSculptVolume");
	DataSource = MakeShared<FVoxelSculptVolumeLocalData>(*this);
}

void AVoxelSculptVolume::BeginPlay()
{
	Super::BeginPlay();

	if (!bReplicateSculptData)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return;
	}

	const ENetMode NetMode = GetNetMode();
	if (NetMode == NM_Standalone)
	{
		return;
	}

	if (!IsNameStableForNetworking())
	{
		VOXEL_MESSAGE(Error,
			"{0}: Cannot replicate sculpt data on dynamically spawned sculpt volumes. "
			"Set bReplicateSculptData to false or add the actor to your static scene instead.",
			this);

		return;
	}

	const FName NetworkId = GetFName();

	if (NetMode == NM_ListenServer ||
		NetMode == NM_DedicatedServer)
	{
		const TSharedRef<FVoxelSculptVolumeNetworkServer> Server = MakeShared<FVoxelSculptVolumeNetworkServer>(*this);
		Server->Register(*World, NetworkId);
		DataSource = Server;

		ensure(!UnregisterDataSource);
		UnregisterDataSource = MakeWeakObjectPtrLambda(this, [this, Server]
		{
			Server->Unregister();
			UnregisterDataSource = {};
		});
	}
	else if (ensure(NetMode == NM_Client))
	{
		const TSharedRef<FVoxelSculptVolumeNetworkClient> Client = MakeShared<FVoxelSculptVolumeNetworkClient>(*this);
		Client->Register(*World, NetworkId);
		DataSource = Client;
	}
}

void AVoxelSculptVolume::Destroyed()
{
	Super::Destroyed();

	if (UnregisterDataSource)
	{
		UnregisterDataSource();
	}
}

void AVoxelSculptVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UnregisterDataSource)
	{
		UnregisterDataSource();
	}
}

void AVoxelSculptVolume::BeginDestroy()
{
	Super::BeginDestroy();

	if (UnregisterDataSource)
	{
		UnregisterDataSource();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVolumeSculptStampRef AVoxelSculptVolume::GetStamp() const
{
	check(Component);
	return Component->GetStamp();
}

UVoxelSculptVolumeComponent& AVoxelSculptVolume::GetComponent() const
{
	check(Component);
	return *Component;
}

TSharedRef<FVoxelDependency3D> AVoxelSculptVolume::GetDependency() const
{
	return Dependency.ToSharedRef();
}

TSharedRef<IVoxelBulkLoader> AVoxelSculptVolume::GetBulkLoader() const
{
	return DataSource->GetBulkLoader();
}

TVoxelBulkRef<FVoxelSculptVolumeData> AVoxelSculptVolume::GetSculptData() const
{
	return DataSource->GetData();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelOptional<FVoxelSculptVolumeContext> AVoxelSculptVolume::GetSculptContext() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return {};
	}

	const FVoxelVolumeSculptStampRef Stamp = GetStamp();

	const FMatrix SculptToWorld = FScaleMatrix(Stamp->Scale) * ActorToWorld().ToMatrixWithScale();

	TSharedPtr<IVoxelVolumeSculptPreviousDistanceProvider> PreviousDistanceProvider;
	if (Stamp->BlendMode == EVoxelVolumeBlendMode::Intersect)
	{
		PreviousDistanceProvider = MakeShared<FVoxelVolumeSculptPreviousDistanceProvider_NaN>();
	}
	else
	{
		const TVoxelOptional<FVoxelWeakStackLayer> WeakLayer = Stamp->GetWeakStackLayer(*World);
		if (!WeakLayer)
		{
			return {};
		}

		if (!Cache ||
			Cache->SculptToWorld != SculptToWorld ||
			!ensure(Cache->WeakSculptActor == this))
		{
			Cache = MakeShared<FVoxelSculptVolumeCache>(SculptToWorld, this);
		}

		PreviousDistanceProvider = MakeShared<FVoxelVolumeSculptPreviousDistanceProvider_Cache>(
			Cache.ToSharedRef(),
			FVoxelLayers::Get(World),
			FVoxelSurfaceTypeTable::Get(),
			*WeakLayer);
	}

	return FVoxelSculptVolumeContext(
		SculptToWorld,
		Stamp->bStoreMovableDistances,
		Stamp->MaxErrorPercentage,
		PreviousDistanceProvider.ToSharedRef());
}

void AVoxelSculptVolume::SetSculptData(
	const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData,
	const TSharedRef<IVoxelBulkLoader>& NewBulkLoader)
{
	DataSource->SetSculptData(NewData, NewBulkLoader);
}

void AVoxelSculptVolume::SetSculptData(const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData)
{
	SetSculptData(NewData, MakeShared<FVoxelDummyBulkLoader>());
}

FVoxelFuture AVoxelSculptVolume::ApplyModifier(const TSharedRef<FVoxelVolumeModifier>& Modifier)
{
	return DataSource->ApplyModifier(Modifier);
}

void AVoxelSculptVolume::ClearSculptCache()
{
	Cache.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

using FVoxelSculptVolumeSaveVersion = DECLARE_VOXEL_VERSION
(
	FirstVersion,
	RemoveDistanceType
);

TVoxelFuture<FVoxelVolumeSculptSave> AVoxelSculptVolume::GetSave(const bool bCompress) const
{
	const TSharedRef<const FVoxelSculptVolumeData> SculptData = GetSculptData().GetShared();

	return Voxel::AsyncTask([bCompress, SculptData]
	{
		VOXEL_FUNCTION_COUNTER();

		const int32 Version = FVoxelSculptVolumeSaveVersion::LatestVersion;

		FVoxelWriter Writer;
		Writer << Version;

		ConstCast(SculptData)->SerializeAsBytes(Writer);

		TVoxelArray64<uint8> Data = MoveTemp(Writer.Bytes);

		if (bCompress)
		{
			Data = FVoxelUtilities::Compress(Data);
		}

		FVoxelVolumeSculptSave Result;
		Result.Data = MakeShared<FVoxelVolumeSculptSave::FData>();
		Result.Data->bIsCompressed = bCompress;
		Result.Data->Data = MoveTemp(Data);
		return Result;
	});
}

FVoxelFuture AVoxelSculptVolume::LoadFromSave(const FVoxelVolumeSculptSave& Save)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Save.IsValid())
	{
		VOXEL_MESSAGE(Error, "Save is invalid");
		return {};
	}

	return Voxel::AsyncTask([Save, WeakThis = MakeVoxelObjectPtr(this)]() -> FVoxelFuture
	{
		VOXEL_FUNCTION_COUNTER();

		TConstVoxelArrayView64<uint8> Data = Save.Data->Data;

		TVoxelArray64<uint8> DataStorage;
		if (Save.Data->bIsCompressed)
		{
			FVoxelUtilities::Decompress(Data, DataStorage);
			Data = DataStorage;
		}
		(void)DataStorage;

		FVoxelReader Reader(Data);

		int32 Version = 0;
		Reader << Version;

		if (Version < FVoxelSculptVolumeSaveVersion::RemoveDistanceType)
		{
			uint8 DistanceType = {};
			Reader << DistanceType;
		}

		const TSharedRef<FVoxelSculptVolumeData> SculptData = MakeShared<FVoxelSculptVolumeData>();
		SculptData->SerializeAsBytes(Reader);

		if (!ensure(Reader.IsAtEndWithoutError()))
		{
			VOXEL_MESSAGE(Error, "Failed to load save: corrupted");
			return {};
		}

		return Voxel::GameTask([WeakThis, Result = MakeVoxelBulkRef(SculptData)]
		{
			AVoxelSculptVolume* This = WeakThis.Resolve();
			if (!ensureVoxelSlow(This))
			{
				return;
			}

			This->SetSculptData(Result);
		});
	});
}