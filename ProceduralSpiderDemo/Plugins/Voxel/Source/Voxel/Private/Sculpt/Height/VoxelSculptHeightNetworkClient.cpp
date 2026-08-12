// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelSculptHeightNetworkClient.h"
#include "Sculpt/ENET/VoxelNetworkLog.h"
#include "Sculpt/Height/VoxelHeightModifier.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"
#include "Sculpt/Height/VoxelSculptHeightDiff.h"
#include "Sculpt/Height/VoxelSculptHeightNetworkMessages.h"

FVoxelSculptHeightNetworkClientBulkLoader::~FVoxelSculptHeightNetworkClientBulkLoader()
{
	VOXEL_FUNCTION_COUNTER();

	// Ensure all promises are fired

	for (auto& It : HashToPromise_RequiresLock)
	{
		It.Value.Set(nullptr);
	}
}

TVoxelFuture<TSharedPtr<const TVoxelArray64<uint8>>> FVoxelSculptHeightNetworkClientBulkLoader::LoadBulkDataImpl(const FVoxelBulkHash& Hash, const FVoxelBulkHint& Hint)
{
	VOXEL_FUNCTION_COUNTER();

	const TVoxelPromise<TSharedPtr<const TVoxelArray64<uint8>>> Promise = INLINE_LAMBDA
	{
		VOXEL_SCOPE_LOCK(CriticalSection);

		ChunksToRequest_RequiresLock.Add({ Hash, Hint });
		return HashToPromise_RequiresLock.Add_EnsureNew(Hash);
	};

	Voxel::GameTask([WeakClient = WeakClient]
	{
		if (const TSharedPtr<FVoxelSculptHeightNetworkClient> Client = WeakClient.Pin())
		{
			Client->ProcessBulkRequests();
		}
	});

	return Promise;
}

TSharedPtr<const TVoxelArray64<uint8>> FVoxelSculptHeightNetworkClientBulkLoader::LoadBulkDataSyncImpl(const FVoxelBulkHash& Hash)
{
	ensure(false);
	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSculptHeightNetworkClient::FVoxelSculptHeightNetworkClient(AVoxelSculptHeight& SculptActor)
	: IVoxelSculptHeightDataSource(SculptActor)
{
}

FVoxelSculptHeightNetworkClient::~FVoxelSculptHeightNetworkClient()
{
	VOXEL_FUNCTION_COUNTER();

	// Ensure all promises are fired
	ClearQueue();
}

void FVoxelSculptHeightNetworkClient::OnRegistered()
{
	ensure(IsExplicitlyNull(BulkLoader->WeakClient));
	BulkLoader->WeakClient = SharedThis(this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptHeightNetworkClient::OnMessageReceived(const TSharedRef<FVoxelNetworkServerToClientMessage>& Message)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (const FVoxelSculptHeightServerToClientMessage_InitialLoad* InitialLoad = Message->As<FVoxelSculptHeightServerToClientMessage_InitialLoad>())
	{
		if (!ensure(InitialLoad->Data))
		{
			return;
		}

		ensure(!ServerData);
		ServerData = InitialLoad->Data;

		UpdateData();
		return;
	}

	if (FVoxelSculptHeightServerToClientMessage_SendBulkData* SendBulkData = Message->As<FVoxelSculptHeightServerToClientMessage_SendBulkData>())
	{
		TVoxelPromise<TSharedPtr<const TVoxelArray64<uint8>>> Promise;
		{
			VOXEL_SCOPE_LOCK(BulkLoader->CriticalSection);

			if (!ensure(BulkLoader->HashToPromise_RequiresLock.RemoveAndCopyValue(SendBulkData->Hash, Promise)))
			{
				return;
			}
		}

		if (!ensure(SendBulkData->BulkData.Num() > 0))
		{
			Promise.Set(nullptr);
			return;
		}

		Promise.Set(MakeShared<TVoxelArray64<uint8>>(MoveTemp(SendBulkData->BulkData)));
		return;
	}

	if (const TSharedPtr<const FVoxelSculptHeightServerToClientMessage_ApplyModifier> ApplyModifier = CastStruct<FVoxelSculptHeightServerToClientMessage_ApplyModifier>(Message))
	{
		QueuedServerModifiers.Add(FQueuedServerModifier
		{
			FPlatformTime::Seconds(),
			ApplyModifier
		});

		ProcessQueue();
		return;
	}

	if (const FVoxelSculptHeightServerToClientMessage_SetSculptData* SetSculptData = Message->As<FVoxelSculptHeightServerToClientMessage_SetSculptData>())
	{
		if (!ensure(SetSculptData->Data))
		{
			return;
		}

		ClearQueue();

		ServerData = SetSculptData->Data;

		UpdateData();
		return;
	}

	ensure(false);
	LOG_VOXEL(Error, "Unknown message received: %s", *Message->GetStruct()->GetName());
}

TSharedRef<IVoxelBulkLoader> FVoxelSculptHeightNetworkClient::GetBulkLoader() const
{
	return BulkLoader;
}

FVoxelFuture FVoxelSculptHeightNetworkClient::ApplyModifier(const TSharedRef<FVoxelHeightModifier>& Modifier)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedPtr<IVoxelHeightModifierRuntime> ModifierRuntime = Modifier->GetRuntime();
	if (!ensureVoxelSlow(ModifierRuntime))
	{
		return {};
	}

	const FGuid ModifierGuid = FGuid::NewGuid();
	FVoxelPromise Promise;

	// Send message first to reduce latency as much as possible
	{
		const TVoxelBulkRef<FVoxelHeightSerializedModifier> SerializedModifier(MakeShared<FVoxelHeightSerializedModifier>(Modifier));

		const TSharedRef<FVoxelSculptHeightClientToServerMessage_ApplyModifier> Message = MakeShared<FVoxelSculptHeightClientToServerMessage_ApplyModifier>();
		Message->ModifierGuid = ModifierGuid;
		Message->ModifierData = SerializedModifier.WriteToBytes();
		SendMessage(Message);
	}

	QueuedPredictedModifiers.Add(MakeShared<FPredictedModifier>(FPredictedModifier
	{
		FPlatformTime::Seconds(),
		ModifierGuid,
		Promise,
		ModifierRuntime
	}));

	ProcessQueue();

	return Promise;
}

void FVoxelSculptHeightNetworkClient::SetSculptData(
	const TVoxelBulkRef<FVoxelSculptHeightData>& NewData,
	const TSharedRef<IVoxelBulkLoader>& NewBulkLoader)
{
	VOXEL_MESSAGE(Error, "Cannot use SetSculptData on a client");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptHeightNetworkClient::ProcessBulkRequests()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	VOXEL_SCOPE_LOCK(BulkLoader->CriticalSection);

	for (const FVoxelSculptHeightNetworkClientBulkLoader::FChunkData& Chunk : BulkLoader->ChunksToRequest_RequiresLock)
	{
		const TSharedRef<FVoxelSculptHeightClientToServerMessage_RequestBulkData> Message = MakeShared<FVoxelSculptHeightClientToServerMessage_RequestBulkData>();
		Message->Hash = Chunk.Hash;
		Message->Hint = Chunk.Hint;
		SendMessage(Message);
	}
	BulkLoader->ChunksToRequest_RequiresLock.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptHeightNetworkClient::UpdateData()
{
	if (PredictedDatas.Num() > 0)
	{
		SetData(PredictedDatas.Last().NewData);
	}
	else if (ServerData)
	{
		SetData(ServerData.ToBulkRef());
	}
}

void FVoxelSculptHeightNetworkClient::ClearQueue()
{
	VOXEL_FUNCTION_COUNTER();

	PendingServerModifier.Reset();

	if (PendingPredictedModifier)
	{
		PendingPredictedModifier->Promise.Set();
	}
	PendingPredictedModifier.Reset();

	QueuedServerModifiers.Empty();

	for (const TSharedPtr<FPredictedModifier>& PredictedModifier : QueuedPredictedModifiers)
	{
		PredictedModifier->Promise.Set();
	}
	QueuedPredictedModifiers.Empty();
}

void FVoxelSculptHeightNetworkClient::ProcessQueue()
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

	UpdateData();
}

bool FVoxelSculptHeightNetworkClient::ProcessQueue_ShouldContinue()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	ensure(!PendingServerModifier || !PendingPredictedModifier);

	if (PendingServerModifier)
	{
		if (!PendingServerModifier->Future.IsComplete())
		{
			LOG_VOXEL_NET(Verbose, "FVoxelSculptHeightNetworkClient: Waiting for server modifier to complete");
			return false;
		}

		const TVoxelOptional<FVoxelSculptHeightContext> Context = GetContext();
		if (!ensure(Context))
		{
			return false;
		}

		const TVoxelBulkRef<FVoxelSculptHeightData> NewServerData(PendingServerModifier->Future.GetSharedValueChecked());

		ServerData = NewServerData;

		if (!ensure(ServerData.GetHash() == PendingServerModifier->ExpectedHash))
		{
			LOG_VOXEL(Error, "Hash mismatch: expected %s after modifier, got %s",
				*PendingServerModifier->ExpectedHash.ToString(),
				*ServerData.GetHash().ToString());
		}

		if (PredictedDatas.Num() > 0)
		{
			const FPredictedData& PredictedData = PredictedDatas[0];

			if (PredictedData.ModifierGuid == PendingServerModifier->ModifierGuid)
			{
				// Modifier got replicated from the server, we can remove it

				// If we got here, it means our original prediction was incorrect, otherwise we'd just have used it
				ensure(PredictedData.NewData.GetHash() != ServerData.GetHash());

				PredictedDatas.RemoveAt(0);
			}
		}

		// Remove the modifier from QueuedPredictedModifiers in case it wasn't processed yet
		for (auto It = QueuedPredictedModifiers.CreateIterator(); It; ++It)
		{
			FPredictedModifier& PredictedModifier = **It;

			if (PredictedModifier.ModifierGuid == PendingServerModifier->ModifierGuid)
			{
				PredictedModifier.Promise.Set();
				It.RemoveCurrent();
			}
		}

		// Re-apply all predicted diffs

		TVoxelBulkRef<FVoxelSculptHeightData> OldData = NewServerData;

		for (auto It = PredictedDatas.CreateIterator(); It; ++It)
		{
			const TVoxelBulkPtr<FVoxelSculptHeightData> NewData = It->Diff->TryApply(
				OldData,
				Context->TargetPrecision);

			if (!NewData)
			{
				// Conflict, rollback
				It.RemoveCurrent();
				continue;
			}

			It->NewData = NewData.ToBulkRef();
			OldData = NewData.ToBulkRef();
		}

		LOG_VOXEL_NET(Verbose, "FVoxelSculptHeightNetworkClient: Server modifier applied in %s (queued for %s)",
			*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - PendingServerModifier->StartTime),
			*FVoxelUtilities::SecondsToString(PendingServerModifier->StartTime - PendingServerModifier->QueueTime));

		PendingServerModifier.Reset();
		return true;
	}

	if (PendingPredictedModifier)
	{
		if (!PendingPredictedModifier->Future.IsComplete())
		{
			LOG_VOXEL_NET(Verbose, "FVoxelSculptHeightNetworkClient: Waiting for predicted modifier to complete");
			return false;
		}

		const TVoxelBulkRef<FVoxelSculptHeightData> NewData(PendingPredictedModifier->Future.GetSharedValueChecked());

		PredictedDatas.Add(FPredictedData
		{
			PendingPredictedModifier->ModifierGuid,
			FVoxelSculptHeightDiff::Create(PendingPredictedModifier->OldData, NewData),
			NewData
		});

		LOG_VOXEL_NET(Verbose, "FVoxelSculptHeightNetworkClient: Predicted modifier applied in %s (queued for %s)",
			*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - PendingPredictedModifier->StartTime),
			*FVoxelUtilities::SecondsToString(PendingPredictedModifier->StartTime - PendingPredictedModifier->QueueTime));

		PendingPredictedModifier->Promise.Set();
		PendingPredictedModifier.Reset();
		return true;
	}

	if (QueuedServerModifiers.Num() > 0)
	{
		const FQueuedServerModifier ServerModifier = MoveTemp(QueuedServerModifiers[0]);
		QueuedServerModifiers.RemoveAt(0);

		ApplyServerModifier(ServerModifier);
		return true;
	}

	if (QueuedPredictedModifiers.Num() > 0)
	{
		if (!ServerData)
		{
			// Not loaded yet
			LOG_VOXEL_NET(Verbose, "FVoxelSculptHeightNetworkClient: Waiting for server data");
			return false;
		}

		const TVoxelOptional<FVoxelSculptHeightContext> Context = GetContext();
		if (!ensure(Context))
		{
			return false;
		}

		const TSharedPtr<FPredictedModifier> PredictedModifier = QueuedPredictedModifiers[0];
		QueuedPredictedModifiers.RemoveAt(0);

		const TVoxelBulkRef<FVoxelSculptHeightData> OldData = INLINE_LAMBDA
		{
			if (PredictedDatas.Num() > 0)
			{
				return PredictedDatas.Last().NewData;
			}

			return ServerData.ToBulkRef();
		};

		const TVoxelFuture<const FVoxelSculptHeightData> Future = OldData->ApplyModifier(
			BulkLoader,
			*Context,
			OldData.GetHash(),
			PredictedModifier->ModifierRuntime.ToSharedRef());

		ensure(!PendingPredictedModifier);
		PendingPredictedModifier.Emplace(FPendingPredictedModifier
		{
			PredictedModifier->QueueTime,
			FPlatformTime::Seconds(),
			PredictedModifier->ModifierGuid,
			OldData,
			PredictedModifier->Promise,
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

void FVoxelSculptHeightNetworkClient::ApplyServerModifier(const FQueuedServerModifier& Modifier)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!ensure(ServerData))
	{
		return;
	}

	const FVoxelSculptHeightServerToClientMessage_ApplyModifier& Message = *Modifier.Message;

	ensure(ServerData.GetHash() == Message.HashBefore);

	const TVoxelBulkPtr<FVoxelHeightSerializedModifier> SerializedModifier = TVoxelBulkPtr<FVoxelHeightSerializedModifier>::LoadFromBytes(Message.ModifierData);
	if (!ensure(SerializedModifier) ||
		!ensure(SerializedModifier->Modifier))
	{
		return;
	}

	const TSharedPtr<IVoxelHeightModifierRuntime> ModifierRuntime = SerializedModifier->Modifier->GetRuntime();
	if (!ensure(ModifierRuntime))
	{
		return;
	}

	if (PredictedDatas.Num() > 0)
	{
		const FPredictedData& PredictedData = PredictedDatas[0];

		if (PredictedData.ModifierGuid == Message.ModifierGuid &&
			PredictedData.NewData.GetHash() == Message.HashAfter)
		{
			// Perfectly predicted
			ServerData = PredictedData.NewData;
			PredictedDatas.RemoveAt(0);
			return;
		}
	}

	const TVoxelOptional<FVoxelSculptHeightContext> Context = GetContext();
	if (!ensure(Context))
	{
		return;
	}

	const TVoxelFuture<const FVoxelSculptHeightData> Future = ServerData->ApplyModifier(
		BulkLoader,
		*Context,
		ServerData.GetHash(),
		ModifierRuntime.ToSharedRef());

	ensure(!PendingServerModifier);
	PendingServerModifier.Emplace(FPendingServerModifier
	{
		Modifier.QueueTime,
		FPlatformTime::Seconds(),
		Message.ModifierGuid,
		Message.HashAfter,
		Future
	});

	FVoxelFuture(Future).Then_GameThread(MakeWeakPtrLambda(this, [this]
	{
		ProcessQueue();
	}));
}