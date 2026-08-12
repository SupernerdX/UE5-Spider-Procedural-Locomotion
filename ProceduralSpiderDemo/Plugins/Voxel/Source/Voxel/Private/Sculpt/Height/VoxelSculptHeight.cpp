// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelSculptHeight.h"
#include "Sculpt/Height/VoxelHeightModifier.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"
#include "Sculpt/Height/VoxelSculptHeightAsset.h"
#include "Sculpt/Height/VoxelSculptHeightCache.h"
#include "Sculpt/Height/VoxelSculptHeightLocalData.h"
#include "Sculpt/Height/VoxelSculptHeightNetworkClient.h"
#include "Sculpt/Height/VoxelSculptHeightNetworkServer.h"
#include "Sculpt/Height/VoxelHeightSculptPreviousHeightProvider.h"
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

class FVoxelSculptHeightSingleton : public FVoxelSingleton
{
public:
	TVoxelMap<FGuid, TVoxelBulkPtr<FVoxelSculptHeightData>> GuidToData;

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
FVoxelSculptHeightSingleton* GVoxelSculptHeightSingleton = new FVoxelSculptHeightSingleton();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelHeightSculptStampRef UVoxelSculptHeightComponent::GetStamp() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(PrivateStamp))
	{
		ConstCast(PrivateStamp) = FVoxelHeightSculptStampRef::New();
	}

	const AVoxelSculptHeight& SculptActor = *GetOuterAVoxelSculptHeight();

	PrivateStamp->WeakSculptActor = SculptActor;

	// DataSource will be null during UVoxelStampComponentBase::PostInitProperties
	if (SculptActor.DataSource)
	{
		if (const TSharedPtr<FVoxelSculptHeightLocalData> LocalData = SculptActor.DataSource->AsLocalData())
		{
			LocalData->SetAsset(ExternalAsset);
		}
	}

	return PrivateStamp;
}

void UVoxelSculptHeightComponent::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	INLINE_LAMBDA
	{
		if (HasAnyFlags(RF_ClassDefaultObject) ||
			ExternalAsset)
		{
			return;
		}

		const AVoxelSculptHeight* SculptActor = GetOuterAVoxelSculptHeight();
		if (!SculptActor ||
			SculptActor->HasAnyFlags(RF_ClassDefaultObject))
		{
			return;
		}

		const TSharedPtr<IVoxelSculptHeightDataSource> DataSource = SculptActor->DataSource;
		if (!DataSource)
		{
			ensureVoxelSlow(SculptActor->HasAllFlags(RF_NeedInitialization));
			return;
		}

		const TSharedPtr<FVoxelSculptHeightLocalData> LocalData = DataSource->AsLocalData();
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
			if (Ar.CustomVer(GVoxelCustomVersionGUID) < FVoxelVersion::AddBulkDataToHeightSaveAssets)
			{
				if (ensure(PrivateStamp))
				{
					PrivateStamp->bStoreRelativeHeights = ExternalAsset->bRelativeHeight_DEPRECATED;
				}
			}

			return;
		}
	}

	const AVoxelSculptHeight* SculptActor = GetOuterAVoxelSculptHeight();
	if (!SculptActor ||
		SculptActor->HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	const TSharedPtr<IVoxelSculptHeightDataSource> DataSource = SculptActor->DataSource;
	if (!DataSource)
	{
		ensureVoxelSlow(SculptActor->HasAllFlags(RF_NeedInitialization));
		return;
	}

	const TSharedPtr<FVoxelSculptHeightLocalData> LocalData = DataSource->AsLocalData();
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
			GVoxelSculptHeightSingleton->GuidToData.Add_EnsureNew(Guid, LocalData->GetData());
		}

		Ar << Guid;

		if (Ar.IsLoading())
		{
			const TVoxelBulkPtr<FVoxelSculptHeightData> Data = GVoxelSculptHeightSingleton->GuidToData.FindRef(Guid);
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
		const TSharedRef<FVoxelSculptHeightData> NewData = MakeShared<FVoxelSculptHeightData>();
		if (Ar.CustomVer(GVoxelCustomVersionGUID) < FVoxelVersion::AddBulkDataToHeightSaveAssets)
		{
			if (ensure(PrivateStamp))
			{
				PrivateStamp->bStoreRelativeHeights = PrivateStamp->bStoreRelativeHeights;
				NewData->SetLegacyChunksType(PrivateStamp->bStoreRelativeHeights);
			}
		}
		NewData->Serialize(Ar);
		LocalData->SetSculptData(
			MakeVoxelBulkRef(NewData),
			MakeShared<FVoxelDummyBulkLoader>());
		return;
	}

	if (Ar.CustomVer(GVoxelCustomVersionGUID) < FVoxelVersion::AddBulkDataToHeightSaveAssets)
	{
		if (ensure(PrivateStamp))
		{
			PrivateStamp->bStoreRelativeHeights = PrivateStamp->bStoreRelativeHeights;
			ConstCast(LocalData->GetData().GetShared())->SetLegacyChunksType(PrivateStamp->bStoreRelativeHeights);
		}
	}

	ConstCast(*LocalData->GetData()).Serialize(Ar);
}

FVoxelStampRef UVoxelSculptHeightComponent::GetStamp_Internal() const
{
	return GetStamp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelSculptHeight::AVoxelSculptHeight()
{
	bReplicates = true;

	Component = CreateDefaultSubobject<UVoxelSculptHeightComponent>("RootComponent");
	RootComponent = Component;
}

void AVoxelSculptHeight::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	Dependency = FVoxelDependency2D::Create("VoxelSculptHeight");
	DataSource = MakeShared<FVoxelSculptHeightLocalData>(*this);
}

void AVoxelSculptHeight::BeginPlay()
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
			"{0}: Cannot replicate sculpt data on dynamically spawned sculpt actors. "
			"Set bReplicateSculptData to false or add the actor to your static scene instead.",
			this);

		return;
	}

	const FName NetworkId = GetFName();

	if (NetMode == NM_ListenServer ||
		NetMode == NM_DedicatedServer)
	{
		const TSharedRef<FVoxelSculptHeightNetworkServer> Server = MakeShared<FVoxelSculptHeightNetworkServer>(*this);
		Server->SetSculptData(GetSculptData(), GetBulkLoader());
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
		const TSharedRef<FVoxelSculptHeightNetworkClient> Client = MakeShared<FVoxelSculptHeightNetworkClient>(*this);
		Client->Register(*World, NetworkId);
		DataSource = Client;
	}
}

void AVoxelSculptHeight::Destroyed()
{
	Super::Destroyed();

	if (UnregisterDataSource)
	{
		UnregisterDataSource();
	}
}

void AVoxelSculptHeight::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UnregisterDataSource)
	{
		UnregisterDataSource();
	}
}

void AVoxelSculptHeight::BeginDestroy()
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

FVoxelHeightSculptStampRef AVoxelSculptHeight::GetStamp() const
{
	check(Component);
	return Component->GetStamp();
}

UVoxelSculptHeightComponent& AVoxelSculptHeight::GetComponent() const
{
	check(Component);
	return *Component;
}

TSharedRef<FVoxelDependency2D> AVoxelSculptHeight::GetDependency() const
{
	return Dependency.ToSharedRef();
}

TSharedRef<IVoxelBulkLoader> AVoxelSculptHeight::GetBulkLoader() const
{
	return DataSource->GetBulkLoader();
}

TVoxelBulkRef<FVoxelSculptHeightData> AVoxelSculptHeight::GetSculptData() const
{
	return DataSource->GetData();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelOptional<FVoxelSculptHeightContext> AVoxelSculptHeight::GetSculptContext() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return {};
	}

	const FVoxelHeightSculptStampRef Stamp = GetStamp();

	const FTransform2d SculptToWorld = FVoxelUtilities::MakeTransform2(
		FTransform(FScaleMatrix(FVector(Stamp->ScaleXY, Stamp->ScaleXY, 1.f)) * ActorToWorld().ToMatrixWithScale()));

	const TVoxelOptional<FVoxelWeakStackLayer> WeakLayer = Stamp->GetWeakStackLayer(*World);
	if (!WeakLayer)
	{
		return {};
	}

	const float ScaleZ = ActorToWorld().GetScale3D().Z;
	const float OffsetZ = ActorToWorld().GetLocation().Z;

	if (!Cache ||
		Cache->SculptToWorld != SculptToWorld ||
		Cache->ScaleZ != ScaleZ ||
		Cache->OffsetZ != OffsetZ ||
		!ensure(Cache->WeakSculptActor == this))
	{
		Cache = MakeShared<FVoxelSculptHeightCache>(
			SculptToWorld,
			ScaleZ,
			OffsetZ,
			this);
	}

	const TSharedPtr<IVoxelHeightSculptPreviousHeightProvider> PreviousHeightProvider =
		MakeShared<FVoxelHeightSculptPreviousHeightProvider_Cache>(
			Cache.ToSharedRef(),
			FVoxelLayers::Get(World),
			FVoxelSurfaceTypeTable::Get(),
			*WeakLayer);

	return FVoxelSculptHeightContext(
		SculptToWorld,
		ScaleZ,
		OffsetZ,
		Stamp->bStoreRelativeHeights,
		PreviousHeightProvider.ToSharedRef(),
		Stamp->TargetPrecision);
}

void AVoxelSculptHeight::SetSculptData(
	const TVoxelBulkRef<FVoxelSculptHeightData>& NewData,
	const TSharedRef<IVoxelBulkLoader>& NewBulkLoader)
{
	DataSource->SetSculptData(NewData, NewBulkLoader);
}

void AVoxelSculptHeight::SetSculptData(const TVoxelBulkRef<FVoxelSculptHeightData>& NewData)
{
	SetSculptData(NewData, MakeShared<FVoxelDummyBulkLoader>());
}

FVoxelFuture AVoxelSculptHeight::ApplyModifier(const TSharedRef<FVoxelHeightModifier>& Modifier)
{
	return DataSource->ApplyModifier(Modifier);
}

void AVoxelSculptHeight::ClearSculptCache()
{
	Cache.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelFuture<FVoxelHeightSculptSave> AVoxelSculptHeight::GetSave(const bool bCompress) const
{
	const TSharedRef<const FVoxelSculptHeightData> SculptData = GetSculptData().GetShared();

	return Voxel::AsyncTask([bCompress, SculptData]
	{
		VOXEL_FUNCTION_COUNTER();

		FVoxelWriter Writer;

		ConstCast(SculptData)->SerializeAsBytes(Writer);

		TVoxelArray64<uint8> Data = MoveTemp(Writer.Bytes);

		if (bCompress)
		{
			Data = FVoxelUtilities::Compress(Data);
		}

		FVoxelHeightSculptSave Result;
		Result.Data = MakeShared<FVoxelHeightSculptSave::FData>();
		Result.Data->bIsCompressed = bCompress;
		Result.Data->Data = MoveTemp(Data);
		return Result;
	});
}

FVoxelFuture AVoxelSculptHeight::LoadFromSave(const FVoxelHeightSculptSave& Save)
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

		const TSharedRef<FVoxelSculptHeightData> SculptData = MakeShared<FVoxelSculptHeightData>();
		SculptData->SerializeAsBytes(Reader);

		if (!ensure(Reader.IsAtEndWithoutError()))
		{
			VOXEL_MESSAGE(Error, "Failed to load save: corrupted");
			return {};
		}

		return Voxel::GameTask([WeakThis, Result = MakeVoxelBulkRef(SculptData)]
		{
			AVoxelSculptHeight* This = WeakThis.Resolve();
			if (!ensureVoxelSlow(This))
			{
				return;
			}

			This->SetSculptData(Result);
		});
	});
}