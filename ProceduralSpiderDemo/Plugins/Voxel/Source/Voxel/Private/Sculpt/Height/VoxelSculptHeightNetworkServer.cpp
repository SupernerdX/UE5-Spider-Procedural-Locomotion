// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelSculptHeightNetworkServer.h"
#include "Sculpt/Height/VoxelSculptHeight.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"
#include "Sculpt/Height/VoxelHeightModifier.h"
#include "Sculpt/Height/VoxelHeightChunks.h"
#include "Sculpt/Height/VoxelSculptHeightNetworkMessages.h"
#include "Bulk/VoxelDummyBulkLoader.h"
#include "Sculpt/ENET/VoxelNetworkLog.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, float, GVoxelHeightSculptOldBulkDataDuration, 60.f,
	"voxel.height.sculpt.OldBulkDataDuration",
	"How long to keep old bulk datas around (in seconds)");

FVoxelSculptHeightNetworkServer::FVoxelSculptHeightNetworkServer(AVoxelSculptHeight& SculptActor)
	: IVoxelSculptHeightDataSource(SculptActor)
	, BulkLoader(MakeShared<FVoxelDummyBulkLoader>())
{
}

FVoxelSculptHeightNetworkServer::~FVoxelSculptHeightNetworkServer()
{
	VOXEL_FUNCTION_COUNTER();

	// Ensure all promises are fired
	ClearQueue();
}

FVoxelFuture FVoxelSculptHeightNetworkServer::ApplyModifier(
	const FGuid ModifierGuid,
	const TSharedRef<FVoxelHeightModifier>& Modifier)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedPtr<IVoxelHeightModifierRuntime> ModifierRuntime = Modifier->GetRuntime();
	if (!ensureVoxelSlow(ModifierRuntime))
	{
		return {};
	}

	const TVoxelBulkRef<FVoxelHeightSerializedModifier> SerializedModifier(MakeShared<FVoxelHeightSerializedModifier>(Modifier));

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

TSharedRef<IVoxelBulkLoader> FVoxelSculptHeightNetworkServer::GetBulkLoader() const
{
	return BulkLoader;
}

FVoxelFuture FVoxelSculptHeightNetworkServer::ApplyModifier(const TSharedRef<FVoxelHeightModifier>& Modifier)
{
	return ApplyModifier(FGuid::NewGuid(), Modifier);
}

void FVoxelSculptHeightNetworkServer::SetSculptData(
	const TVoxelBulkRef<FVoxelSculptHeightData>& NewData,
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

	const TSharedRef<FVoxelSculptHeightServerToClientMessage_SetSculptData> Message = MakeShared<FVoxelSculptHeightServerToClientMessage_SetSculptData>();
	Message->Data = NewData;
	BroadcastMessage(Message);
}

void FVoxelSculptHeightNetworkServer::SetData(const TVoxelBulkRef<FVoxelSculptHeightData>& Data)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread() || IsInAsyncLoadingThread());

	if (GetData().GetHash() == Data.GetHash())
	{
		return;
	}

	HashToOldBulkData.Add_EnsureNew(GetData().GetHash(), FOldBulkData(GetData(), FPlatformTime::Seconds() + GVoxelHeightSculptOldBulkDataDuration));

	IVoxelSculptHeightDataSource::SetData(Data);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<IVoxelNetworkRemoteClient> FVoxelSculptHeightNetworkServer::CreateClientImpl()
{
	return MakeShared<FVoxelSculptHeightNetworkRemoteClient>(SharedThis(this));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptHeightNetworkServer::Tick()
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

TVoxelBulkPtr<FVoxelSculptHeightData> FVoxelSculptHeightNetworkServer::FindSculptData(const FVoxelBulkHash& RootHash) const
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

void FVoxelSculptHeightNetworkServer::ClearQueue()
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

void FVoxelSculptHeightNetworkServer::ProcessQueue()
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

bool FVoxelSculptHeightNetworkServer::ProcessQueue_ShouldContinue()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (PendingModifier)
	{
		if (!PendingModifier->Future.IsComplete())
		{
			LOG_VOXEL_NET(Verbose, "FVoxelSculptHeightNetworkServer: Waiting for modifier to complete");
			return false;
		}

		const TVoxelBulkRef<FVoxelSculptHeightData> OldData = GetData();
		const TVoxelBulkRef<FVoxelSculptHeightData> NewData(PendingModifier->Future.GetSharedValueChecked());

		SetData(NewData);

		{
			const TSharedRef<FVoxelSculptHeightServerToClientMessage_ApplyModifier> Message = MakeShared<FVoxelSculptHeightServerToClientMessage_ApplyModifier>();
			Message->ModifierGuid = PendingModifier->ModifierGuid;
			Message->ModifierData = MoveTemp(PendingModifier->ModifierData);
			Message->HashBefore = OldData.GetHash();
			Message->HashAfter = NewData.GetHash();
			BroadcastMessage(Message);
		}

		LOG_VOXEL_NET(Verbose, "FVoxelSculptHeightNetworkServer: modifier applied in %s (queued for %s)",
			*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - PendingModifier->StartTime),
			*FVoxelUtilities::SecondsToString(PendingModifier->StartTime - PendingModifier->QueueTime));

		PendingModifier->Promise.Set();
		PendingModifier.Reset();
		return true;
	}

	if (QueuedModifiers.Num() > 0)
	{
		const TVoxelOptional<FVoxelSculptHeightContext> Context = GetContext();
		if (!ensure(Context))
		{
			return false;
		}

		FQueuedModifier QueuedModifier = MoveTemp(QueuedModifiers[0]);
		QueuedModifiers.RemoveAt(0);

		const TVoxelFuture<const FVoxelSculptHeightData> Future = GetData()->ApplyModifier(
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

FVoxelSculptHeightNetworkRemoteClient::FVoxelSculptHeightNetworkRemoteClient(const TSharedRef<FVoxelSculptHeightNetworkServer>& Server)
	: Server(Server)
{
}

void FVoxelSculptHeightNetworkRemoteClient::OnConnect()
{
	const TSharedRef<FVoxelSculptHeightServerToClientMessage_InitialLoad> Message = MakeShared<FVoxelSculptHeightServerToClientMessage_InitialLoad>();
	Message->Data = Server->GetData();
	SendMessage(Message);
}

void FVoxelSculptHeightNetworkRemoteClient::OnMessageReceived(const TSharedRef<FVoxelNetworkClientToServerMessage>& Message)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (const FVoxelSculptHeightClientToServerMessage_RequestBulkData* RequestBulkData = Message->As<FVoxelSculptHeightClientToServerMessage_RequestBulkData>())
	{
		const auto SendReply = [this](const FVoxelBulkHash& Hash, const TSharedPtr<const TVoxelArray64<uint8>>& BulkData)
		{
			const TSharedRef<FVoxelSculptHeightServerToClientMessage_SendBulkData> Reply = MakeShared<FVoxelSculptHeightServerToClientMessage_SendBulkData>();
			Reply->Hash = Hash;
			if (BulkData)
			{
				Reply->BulkData = *BulkData;
			}
			SendMessage(Reply);
		};

		const FVoxelBulkHash& Hash = RequestBulkData->Hash;
		const FVoxelBulkHint& Hint = RequestBulkData->Hint;

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

				const TSharedRef<TVoxelArray64<uint8>> Bytes = MakeShared<TVoxelArray64<uint8>>(ChunkPtr.WriteToBytes());
				SendReply(Hash, Bytes);
				return true;
			};

			const auto ProcessChunk = [this, TrySendChunk](
				const FVoxelBulkHash& RootHash,
				const FIntPoint& FarChunkKey,
				const FIntPoint* MidChunkKey = nullptr,
				const FIntPoint* NearChunkKey = nullptr) -> bool
			{
				const TVoxelBulkPtr<FVoxelSculptHeightData> SculptData = Server->FindSculptData(RootHash);
				if (!SculptData)
				{
					return false;
				}

				const TVoxelBulkPtr<FVoxelHeightFarChunk> FarChunk = SculptData->GetKeyToFarChunk().FindRef(FarChunkKey);
				if (!MidChunkKey)
				{
					return TrySendChunk(FarChunk);
				}

				const TVoxelBulkPtr<FVoxelHeightMidChunk> MidChunk = FarChunk->KeyToMidChunk.FindRef(FVoxelHeightChunkKey(*MidChunkKey));
				if (!NearChunkKey)
				{
					return TrySendChunk(MidChunk);
				}

				const TVoxelBulkPtr<FVoxelHeightNearChunk> NearChunk = MidChunk->KeyToNearChunk.FindRef(FVoxelHeightChunkKey(*NearChunkKey));
				return TrySendChunk(NearChunk);
			};

			if (const FVoxelHeightFarChunkHint* FarChunkHint = Hint.Data->As<FVoxelHeightFarChunkHint>())
			{
				if (ProcessChunk(
					FarChunkHint->RootHash,
					FarChunkHint->FarChunkKey))
				{
					return;
				}
			}
			else if (const FVoxelHeightMidChunkHint* MidChunkHint = Hint.Data->As<FVoxelHeightMidChunkHint>())
			{
				if (ProcessChunk(
					MidChunkHint->RootHash,
					MidChunkHint->FarChunkKey,
					&MidChunkHint->MidChunkKey))
				{
					return;
				}
			}
			else if (const FVoxelHeightNearChunkHint* NearChunkHint = Hint.Data->As<FVoxelHeightNearChunkHint>())
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
		.Then_GameThread(MakeWeakPtrLambda(this, [this, Hash, SendReply](const TSharedPtr<const TVoxelArray64<uint8>>& BulkData)
		{
			SendReply(Hash, BulkData);
		}));

		return;
	}

	if (const FVoxelSculptHeightClientToServerMessage_ApplyModifier* ApplyModifier = Message->As<FVoxelSculptHeightClientToServerMessage_ApplyModifier>())
	{
		const TVoxelBulkPtr<FVoxelHeightSerializedModifier> SerializedModifier = TVoxelBulkPtr<FVoxelHeightSerializedModifier>::LoadFromBytes(ApplyModifier->ModifierData);
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