// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelSculptVolumeNetworkServer.h"
#include "Sculpt/Volume/VoxelVolumeModifier.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "Sculpt/Volume/VoxelVolumeChunks.h"
#include "Sculpt/Volume/VoxelSculptVolumeNetworkMessages.h"
#include "Bulk/VoxelDummyBulkLoader.h"
#include "Sculpt/ENET/VoxelNetworkLog.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, float, GVoxelVolumeSculptOldBulkDataDuration, 60.f,
	"voxel.volume.sculpt.OldBulkDataDuration",
	"How long to keep old bulk datas around (in seconds)");

FVoxelSculptVolumeNetworkServer::FVoxelSculptVolumeNetworkServer(AVoxelSculptVolume& SculptActor)
	: IVoxelSculptVolumeDataSource(SculptActor)
	, BulkLoader(MakeShared<FVoxelDummyBulkLoader>())
{
}

FVoxelSculptVolumeNetworkServer::~FVoxelSculptVolumeNetworkServer()
{
	VOXEL_FUNCTION_COUNTER();

	// Ensure all promises are fired
	ClearQueue();
}

FVoxelFuture FVoxelSculptVolumeNetworkServer::ApplyModifier(
	const FGuid ModifierGuid,
	const TSharedRef<FVoxelVolumeModifier>& Modifier)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedPtr<IVoxelVolumeModifierRuntime> ModifierRuntime = Modifier->GetRuntime();
	if (!ensureVoxelSlow(ModifierRuntime))
	{
		return {};
	}

	const TVoxelBulkRef<FVoxelVolumeSerializedModifier> SerializedModifier(MakeShared<FVoxelVolumeSerializedModifier>(Modifier));

	FVoxelPromise Promise;

	QueuedModifiers.Add(FQueuedModifier
	{
		FPlatformTime::Seconds(),
		ModifierGuid,
		SerializedModifier.WriteToBytes(),
		Promise,
		ModifierRuntime
	});

	ProcessQueue();

	return Promise;
}

TSharedRef<IVoxelBulkLoader> FVoxelSculptVolumeNetworkServer::GetBulkLoader() const
{
	return BulkLoader;
}

FVoxelFuture FVoxelSculptVolumeNetworkServer::ApplyModifier(const TSharedRef<FVoxelVolumeModifier>& Modifier)
{
	return ApplyModifier(FGuid::NewGuid(), Modifier);
}

void FVoxelSculptVolumeNetworkServer::SetSculptData(
	const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData,
	const TSharedRef<IVoxelBulkLoader>& NewBulkLoader)
{
	if (GetData().GetHash() == NewData.GetHash() &&
		BulkLoader == NewBulkLoader)
	{
		return;
	}

	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	ClearQueue();

	BulkLoader = NewBulkLoader;
	SetData(NewData);

	const TSharedRef<FVoxelSculptVolumeServerToClientMessage_SetSculptData> Message = MakeShared<FVoxelSculptVolumeServerToClientMessage_SetSculptData>();
	Message->Data = NewData;
	BroadcastMessage(Message);
}

void FVoxelSculptVolumeNetworkServer::SetData(const TVoxelBulkRef<FVoxelSculptVolumeData>& Data)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread() || IsInAsyncLoadingThread());

	if (GetData().GetHash() == Data.GetHash())
	{
		return;
	}

	HashToOldBulkData.Add_EnsureNew(GetData().GetHash(), FOldBulkData(GetData(), FPlatformTime::Seconds() + GVoxelVolumeSculptOldBulkDataDuration));

	IVoxelSculptVolumeDataSource::SetData(Data);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<IVoxelNetworkRemoteClient> FVoxelSculptVolumeNetworkServer::CreateClientImpl()
{
	return MakeShared<FVoxelSculptVolumeNetworkRemoteClient>(SharedThis(this));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptVolumeNetworkServer::Tick()
{
	const double CurrentTime = FPlatformTime::Seconds();
	for (auto It = HashToOldBulkData.CreateIterator(); It; ++It)
	{
		if (It.Value().RemoveAt < CurrentTime)
		{
			It.RemoveCurrent();
		}
	}
}

TVoxelBulkPtr<FVoxelSculptVolumeData> FVoxelSculptVolumeNetworkServer::FindSculptData(const FVoxelBulkHash& RootHash) const
{
	if (GetData().GetHash() == RootHash)
	{
		return GetData();
	}

	if (const FOldBulkData* OldBulkData = HashToOldBulkData.Find(RootHash))
	{
		return OldBulkData->Data;
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptVolumeNetworkServer::ClearQueue()
{
	VOXEL_FUNCTION_COUNTER();

	if (PendingModifier)
	{
		PendingModifier->Promise.Set();
	}
	PendingModifier.Reset();

	for (const FQueuedModifier& QueuedModifier : QueuedModifiers)
	{
		QueuedModifier.Promise.Set();
	}
	QueuedModifiers.Empty();
}

void FVoxelSculptVolumeNetworkServer::ProcessQueue()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	while (true)
	{
		if (!ProcessQueue_ShouldContinue())
		{
			break;
		}
	}
}

bool FVoxelSculptVolumeNetworkServer::ProcessQueue_ShouldContinue()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (PendingModifier)
	{
		if (!PendingModifier->Future.IsComplete())
		{
			LOG_VOXEL_NET(Verbose, "FVoxelSculptVolumeNetworkServer: Waiting for modifier to complete");
			return false;
		}

		const TVoxelBulkRef<FVoxelSculptVolumeData> OldData = GetData();
		const TVoxelBulkRef<FVoxelSculptVolumeData> NewData(PendingModifier->Future.GetSharedValueChecked());

		SetData(NewData);

		{
			const TSharedRef<FVoxelSculptVolumeServerToClientMessage_ApplyModifier> Message = MakeShared<FVoxelSculptVolumeServerToClientMessage_ApplyModifier>();
			Message->ModifierGuid = PendingModifier->ModifierGuid;
			Message->ModifierData = MoveTemp(PendingModifier->ModifierData);
			Message->HashBefore = OldData.GetHash();
			Message->HashAfter = NewData.GetHash();
			BroadcastMessage(Message);
		}

		LOG_VOXEL_NET(Verbose, "FVoxelSculptVolumeNetworkServer: modifier applied in %s (queued for %s)",
			*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - PendingModifier->StartTime),
			*FVoxelUtilities::SecondsToString(PendingModifier->StartTime - PendingModifier->QueueTime));

		PendingModifier->Promise.Set();
		PendingModifier.Reset();
		return true;
	}

	if (QueuedModifiers.Num() > 0)
	{
		const TVoxelOptional<FVoxelSculptVolumeContext> Context = GetContext();
		if (!ensure(Context))
		{
			return false;
		}

		FQueuedModifier QueuedModifier = MoveTemp(QueuedModifiers[0]);
		QueuedModifiers.RemoveAt(0);

		const TVoxelFuture<const FVoxelSculptVolumeData> Future = GetData()->ApplyModifier(
			BulkLoader,
			*Context,
			GetData().GetHash(),
			QueuedModifier.ModifierRuntime.ToSharedRef());

		ensure(!PendingModifier);
		PendingModifier.Emplace(FPendingModifier
		{
			QueuedModifier.QueueTime,
			FPlatformTime::Seconds(),
			QueuedModifier.ModifierGuid,
			MoveTemp(QueuedModifier.ModifierData),
			QueuedModifier.Promise,
			Future
		});

		FVoxelFuture(Future).Then_GameThread(MakeWeakPtrLambda(this, [this]
		{
			ProcessQueue();
		}));

		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSculptVolumeNetworkRemoteClient::FVoxelSculptVolumeNetworkRemoteClient(const TSharedRef<FVoxelSculptVolumeNetworkServer>& Server)
	: Server(Server)
{
}

void FVoxelSculptVolumeNetworkRemoteClient::OnConnect()
{
	const TSharedRef<FVoxelSculptVolumeServerToClientMessage_InitialLoad> Message = MakeShared<FVoxelSculptVolumeServerToClientMessage_InitialLoad>();
	Message->Data = Server->GetData();
	SendMessage(Message);
}

void FVoxelSculptVolumeNetworkRemoteClient::OnMessageReceived(const TSharedRef<FVoxelNetworkClientToServerMessage>& Message)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (const FVoxelSculptVolumeClientToServerMessage_RequestBulkData* RequestBulkData = Message->As<FVoxelSculptVolumeClientToServerMessage_RequestBulkData>())
	{
		const FVoxelBulkHash& Hash = RequestBulkData->Hash;
		const FVoxelBulkHint& Hint = RequestBulkData->Hint;

		const auto SendReply = [this, Hash](TVoxelArray<uint8>&& BulkData)
		{
			const TSharedRef<FVoxelSculptVolumeServerToClientMessage_SendBulkData> Reply = MakeShared<FVoxelSculptVolumeServerToClientMessage_SendBulkData>();
			Reply->Hash = Hash;
			Reply->BulkData = MoveTemp(BulkData);
			SendMessage(Reply);
		};

		// Try to find the chunk based on hint
		if (Hint.Data)
		{
			const auto TrySendChunk = [&](const FVoxelBulkPtr& ChunkPtr) -> bool
			{
				if (!ChunkPtr ||
					!ChunkPtr.IsLoaded())
				{
					return false;
				}
				if (!ensureMsgf(ChunkPtr.GetHash() == Hash,
					TEXT("Bulk data hash mismatch: chunk hint resolved to a loaded chunk whose hash %s does not match the requested hash %s"),
					*ChunkPtr.GetHash().ToString(),
					*Hash.ToString()))
				{
					return false;
				}

				SendReply(ChunkPtr.WriteToBytes());
				return true;
			};

			const auto ProcessChunk = [this, TrySendChunk](
				const FVoxelBulkHash& RootHash,
				const FIntVector& FarChunkKey,
				const FIntVector* MidChunkKey = nullptr,
				const FIntVector* NearChunkKey = nullptr) -> bool
			{
				const TVoxelBulkPtr<FVoxelSculptVolumeData> SculptData = Server->FindSculptData(RootHash);
				if (!SculptData)
				{
					return false;
				}

				const TVoxelBulkPtr<FVoxelVolumeFarChunk> FarChunk = SculptData->GetKeyToFarChunk().FindRef(FarChunkKey);
				if (!MidChunkKey)
				{
					return TrySendChunk(FarChunk);
				}

				const TVoxelBulkPtr<FVoxelVolumeMidChunk> MidChunk = FarChunk->KeyToMidChunk.FindRef(FVoxelVolumeChunkKey(*MidChunkKey));
				if (!NearChunkKey)
				{
					return TrySendChunk(MidChunk);
				}

				const TVoxelBulkPtr<FVoxelVolumeNearChunk> NearChunk = MidChunk->KeyToNearChunk.FindRef(FVoxelVolumeChunkKey(*NearChunkKey));
				return TrySendChunk(NearChunk);
			};

			if (const FVoxelVolumeFarChunkHint* FarChunkHint = Hint.Data->As<FVoxelVolumeFarChunkHint>())
			{
				if (ProcessChunk(
					FarChunkHint->RootHash,
					FarChunkHint->FarChunkKey))
				{
					return;
				}
			}
			else if (const FVoxelVolumeMidChunkHint* MidChunkHint = Hint.Data->As<FVoxelVolumeMidChunkHint>())
			{
				if (ProcessChunk(
					MidChunkHint->RootHash,
					MidChunkHint->FarChunkKey,
					&MidChunkHint->MidChunkKey))
				{
					return;
				}
			}
			else if (const FVoxelVolumeNearChunkHint* NearChunkHint = Hint.Data->As<FVoxelVolumeNearChunkHint>())
			{
				if (ProcessChunk(
					NearChunkHint->RootHash,
					NearChunkHint->FarChunkKey,
					&NearChunkHint->MidChunkKey,
					&NearChunkHint->NearChunkKey))
				{
					return;
				}
			}
		}

		// Fallback to bulk loader
		Server->GetBulkLoader()->LoadBulkData(Hash, Hint)
		.Then_GameThread(MakeWeakPtrLambda(this, [this, Hash](const TSharedPtr<const TVoxelArray64<uint8>>& BulkData)
		{
			const TSharedRef<FVoxelSculptVolumeServerToClientMessage_SendBulkData> Reply = MakeShared<FVoxelSculptVolumeServerToClientMessage_SendBulkData>();
			Reply->Hash = Hash;
			if (BulkData)
			{
				Reply->BulkData = *BulkData;
			}
			SendMessage(Reply);
		}));

		return;
	}

	if (const FVoxelSculptVolumeClientToServerMessage_ApplyModifier* ApplyModifier = Message->As<FVoxelSculptVolumeClientToServerMessage_ApplyModifier>())
	{
		const TVoxelBulkPtr<FVoxelVolumeSerializedModifier> SerializedModifier = TVoxelBulkPtr<FVoxelVolumeSerializedModifier>::LoadFromBytes(ApplyModifier->ModifierData);
		if (!ensure(SerializedModifier) ||
			!ensure(SerializedModifier->Modifier))
		{
			return;
		}

		Server->ApplyModifier(
			ApplyModifier->ModifierGuid,
			SerializedModifier->Modifier.ToSharedRef());

		return;
	}

	ensure(false);
	LOG_VOXEL(Error, "Unknown message received: %s", *Message->GetStruct()->GetName());
}