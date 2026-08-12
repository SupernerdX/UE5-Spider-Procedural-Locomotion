// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelChannel.h"
#include "UnrealEngine.h"
#include "Engine/NetDriver.h"
#include "Engine/GameInstance.h"
#include "Engine/NetConnection.h"
#include "Engine/PackageMapClient.h"
#include "Sculpt/Volume/VoxelSculptVolume.h"
#include "Compression/OodleDataCompressionUtil.h"
#include "Sculpt/ENET/VoxelNetworkClient.h"
#include "Sculpt/ENET/VoxelNetworkLog.h"
#include "Sculpt/ENET/VoxelNetworkServer.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	for (const UClass* Class : GetDerivedClasses<UNetDriver>())
	{
		UNetDriver& NetDriver = *Class->GetDefaultObject<UNetDriver>();

		FChannelDefinition ChannelDefinition;
		ChannelDefinition.ChannelName = "Voxel";
		ChannelDefinition.ClassName = FName(UVoxelChannel::StaticClass()->GetPathName());
		ChannelDefinition.StaticChannelIndex = -1;
		ChannelDefinition.bTickOnCreate = true;
		ChannelDefinition.bServerOpen = true;
		ChannelDefinition.bClientOpen = true;
		ChannelDefinition.bInitialServer = true;
		ChannelDefinition.bInitialClient = true;
		NetDriver.ChannelDefinitions.Add(ChannelDefinition);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelChannelManager::OnMessageReceived(const TConstVoxelArrayView<uint8> Data)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	ensure(!Client || !RemoteClient);

	if (Client)
	{
		Client->OnMessageBytesReceived(Data);
		return;
	}

	if (RemoteClient)
	{
		RemoteClient->OnMessageBytesReceived(Data);
		return;
	}

	LOG_VOXEL_NET(Verbose, "%s: FVoxelChannelManager::OnMessageReceived: %s: Message queued, waiting for client",
		*PeerName,
		*NetworkId.ToString());

	QueuedMessages.Add(Data.Array());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelPeerManager> FVoxelPeerManager::Create(
	const TSharedRef<Voxel::Network::FHost>& Host,
	UVoxelChannel& Channel)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelPeerManager> Result = MakeShareable(new FVoxelPeerManager(Host, &Channel));

	ConstCast(Result->Peer) = Host->ConnectToPeer(
		ensure(Channel.Connection) && ensure(Channel.Connection->Driver)
		? Channel.Connection->Driver->LowLevelGetNetworkNumber()
		: "ERROR",
		MakeWeakPtrLambda(Result, [&Result = *Result](const TConstVoxelArrayView<uint8> Data)
		{
			Result.SendPacket(Data);
		}),
		MakeWeakPtrLambda(Result, [&Result = *Result](const uint8 ChannelId, TVoxelArray<uint8>&& Data)
		{
			Result.OnMessageReceived(ChannelId, MoveTemp(Data));
		}));

	return Result;
}

void FVoxelPeerManager::ReceiveBunch(FInBunch& Bunch)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const int64 BitsLeft = Bunch.GetBitsLeft();
	if (!ensure(BitsLeft % 8 == 0))
	{
		return;
	}

	TVoxelArray<uint8> Data;
	FVoxelUtilities::SetNumFast(Data, BitsLeft / 8);

	Bunch.Serialize(Data.GetData(), Data.Num());

	if (!ensure(!Bunch.GetError()))
	{
		return;
	}

	ReceivedPackets.Add(MoveTemp(Data));
}

FVoxelPeerManager::~FVoxelPeerManager()
{
	if (const TSharedPtr<Voxel::Network::FHost> Host = WeakHost.Pin())
	{
		Host->RemovePeer(Peer);
	}
}

void FVoxelPeerManager::RegisterClient(const TSharedRef<IVoxelNetworkClient>& Client)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const FName NetworkId = Client->GetNetworkId();
	if (!ensure(!NetworkId.IsNone()))
	{
		return;
	}

	LOG_VOXEL_NET(Verbose, "%s: FVoxelPeerManager::RegisterClient: %s",
		*Peer->GetName(),
		*NetworkId.ToString());

	if (!NetworkIdToChannelId.Contains(NetworkId))
	{
		LOG_VOXEL_NET(Verbose, "%s: FVoxelPeerManager::RegisterClient: %s: Channel not yet registered, adding to pending",
			*Peer->GetName(),
			*NetworkId.ToString());

		NetworkIdToPendingClient.Add_EnsureNew(NetworkId, Client);
		return;
	}

	const uint8 ChannelId = NetworkIdToChannelId[NetworkId];

	FVoxelChannelManager& Channel = *Channels[ChannelId];
	ensure(Channel.NetworkId == NetworkId);

	ensure(!Channel.Client);
	Channel.Client = Client;

	ensure(!Client->SendMessageBytes);
	Client->SendMessageBytes = MakeWeakPtrLambda(this, [this, ChannelId](const TConstVoxelArrayView<uint8> DataToSend)
	{
		check(IsInGameThread());
		Peer->Send(ChannelId, DataToSend);
	});

	LOG_VOXEL_NET(Verbose, "%s: FVoxelPeerManager::RegisterClient: %s: Dispatching %d queued messages",
		*Peer->GetName(),
		*NetworkId.ToString(),
		Channel.QueuedMessages.Num());

	for (const TVoxelArray<uint8>& Message : Channel.QueuedMessages)
	{
		Client->OnMessageBytesReceived(Message);
	}
	Channel.QueuedMessages.Empty();
}

void FVoxelPeerManager::RegisterServer(const TSharedRef<IVoxelNetworkServer>& Server)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const FName NetworkId = Server->GetNetworkId();
	if (!ensure(!NetworkId.IsNone()))
	{
		return;
	}

	LOG_VOXEL_NET(Log, "%s: FVoxelPeerManager::RegisterServer: %s",
		*Peer->GetName(),
		*NetworkId.ToString());

	if (!ensure(!NetworkIdToChannelId.Contains(NetworkId)))
	{
		LOG_VOXEL_NET(Warning, "%s: FVoxelPeerManager::RegisterServer: %s: Already registered",
			*Peer->GetName(),
			*NetworkId.ToString());

		return;
	}

	int32 ChannelId = 0;
	while (ChannelId < 256)
	{
		if (!Channels.IsValidIndex(ChannelId))
		{
			Channels.Emplace();
		}

		if (!Channels[ChannelId])
		{
			break;
		}

		ChannelId++;
	}

	TSharedPtr<FVoxelChannelManager>& Channel = Channels[ChannelId];
	if (!ensure(!Channel))
	{
		LOG_VOXEL(Error, "Ran out of channel IDs");
		return;
	}

	Channel = MakeShared<FVoxelChannelManager>(NetworkId, Peer->GetName());

	Channel->RemoteClient = Server->CreateClient();
	Channel->RemoteClient->SendMessageBytes = MakeWeakPtrLambda(this, [this, ChannelId](const TConstVoxelArrayView<uint8> DataToSend)
	{
		check(IsInGameThread());
		Peer->Send(ChannelId, DataToSend);
	});

	NetworkIdToChannelId.Add_EnsureNew(NetworkId, ChannelId);

	LOG_VOXEL_NET(Verbose, "%s: FVoxelPeerManager::RegisterServer: %s: Assigned channel ID %d",
		*Peer->GetName(),
		*NetworkId.ToString(),
		ChannelId);

	// Send registration
	{
		FVoxelWriter Writer;
		Writer << Magic0;
		Writer << Magic1;
		Writer << Magic2;
		Writer << RegisterChannelMagic;
		Writer << NetworkId;
		Peer->Send(ChannelId, Writer.Bytes);

		LOG_VOXEL_NET(Verbose, "%s: FVoxelPeerManager::RegisterServer: %s: Sent registration packet",
			*Peer->GetName(),
			*NetworkId.ToString());
	}

	Server->OnUnregister.Add(MakeWeakPtrDelegate(this, [this, ChannelId, NetworkId]
	{
		check(IsInGameThread());

		LOG_VOXEL_NET(Log, "%s: FVoxelPeerManager::UnregisterServer: %s",
			*Peer->GetName(),
			*NetworkId.ToString());

		if (!ensure(NetworkIdToChannelId.FindRef(NetworkId) == ChannelId))
		{
			return;
		}

		// Send unregistration
		{
			FVoxelWriter Writer;
			Writer << Magic0;
			Writer << Magic1;
			Writer << Magic2;
			Writer << UnregisterChannelMagic;
			Writer << NetworkId;
			Peer->Send(ChannelId, Writer.Bytes);

			LOG_VOXEL_NET(Verbose, "%s: FVoxelPeerManager::UnregisterServer: %s: Sent unregistration packet",
				*Peer->GetName(),
				*NetworkId.ToString());
		}

		ensure(NetworkIdToChannelId.Remove(NetworkId));

		Channels[ChannelId].Reset();
	}));

	// Do this once registration is sent
	Channel->RemoteClient->OnConnect();
}

void FVoxelPeerManager::SendPacket(const TConstVoxelArrayView<uint8> Data)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	UVoxelChannel* Channel = WeakChannel.Resolve();
	if (!ensureVoxelSlow(Channel))
	{
		return;
	}

	FOutBunch Bunch(Channel, false);
	Bunch.Serialize(ConstCast(Data.GetData()), Data.Num());

	if (!bFirstPacketSent)
	{
		// Reliable bunch to open channel
		bFirstPacketSent = true;
		Bunch.bReliable = true;
	}

	Channel->SendBunch(&Bunch, false);
}

void FVoxelPeerManager::OnMessageReceived(
	const uint8 ChannelId,
	const TVoxelArray<uint8> Data)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (Channels.Num() <= ChannelId)
	{
		Channels.SetNum(int32(ChannelId) + 1);
	}
	TSharedPtr<FVoxelChannelManager>& Channel = Channels[ChannelId];

	constexpr int32 MagicSize = 4 * sizeof(uint64);

	if (Data.Num() >= MagicSize)
	{
		// See if we have a magic packet

		const TConstVoxelArrayView<uint64> Start = Data.View().LeftOf(MagicSize).ReinterpretAs<uint64>();

		if (Start[0] == Magic0 &&
			Start[1] == Magic1 &&
			Start[2] == Magic2)
		{
			if (Start[3] == RegisterChannelMagic)
			{
				FVoxelReader Reader(Data.View().RightOf(MagicSize));

				FName NetworkId;
				Reader << NetworkId;

				if (!ensure(Reader.IsAtEndWithoutError()))
				{
					return;
				}

				LOG_VOXEL_NET(Log, "%s: FVoxelPeerManager::OnMessageReceived: Received RegisterChannel for %s on channel %d",
					*Peer->GetName(),
					*NetworkId.ToString(),
					ChannelId);

				ensure(!Channel);
				Channel = MakeShared<FVoxelChannelManager>(NetworkId, Peer->GetName());

				TSharedPtr<IVoxelNetworkClient> Client;
				if (NetworkIdToPendingClient.RemoveAndCopyValue(NetworkId, Client))
				{
					LOG_VOXEL_NET(Verbose, "%s: FVoxelPeerManager::OnMessageReceived: Found pending client for %s",
						*Peer->GetName(),
						*NetworkId.ToString());

					Channel->Client = Client;

					ensure(!Client->SendMessageBytes);
					Client->SendMessageBytes = MakeWeakPtrLambda(this, [this, ChannelId](const TConstVoxelArrayView<uint8> DataToSend)
					{
						check(IsInGameThread());
						Peer->Send(ChannelId, DataToSend);
					});
				}

				NetworkIdToChannelId.Add_EnsureNew(NetworkId, ChannelId);

				return;
			}
			else if (Start[3] == UnregisterChannelMagic)
			{
				FVoxelReader Reader(Data.View().RightOf(MagicSize));

				FName NetworkId;
				Reader << NetworkId;

				if (!ensure(Reader.IsAtEndWithoutError()))
				{
					return;
				}

				LOG_VOXEL_NET(Log, "%s: FVoxelPeerManager::OnMessageReceived: Received UnregisterChannel for %s on channel %d",
					*Peer->GetName(),
					*NetworkId.ToString(),
					ChannelId);

				if (!ensure(Channel) ||
					!ensure(Channel->NetworkId == NetworkId))
				{
					return;
				}

				uint8 RemovedChannelId = 0;
				ensure(NetworkIdToChannelId.RemoveAndCopyValue(NetworkId, RemovedChannelId));
				ensure(ChannelId == RemovedChannelId);

				Channel.Reset();

				return;
			}
			else
			{
				ensure(false);
			}
		}
	}

	if (!ensure(Channel))
	{
		return;
	}

	Channel->OnMessageReceived(Data);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelMap<TVoxelObjectPtr<UGameInstance>, TSharedPtr<FVoxelNetworkManager>> GVoxelGameInstanceToNetworkSubsystem;

TSharedPtr<FVoxelNetworkManager> FVoxelNetworkManager::Get(const UWorld& World)
{
	check(IsInGameThread());

	UGameInstance* GameInstance = World.GetGameInstance();
	if (!ensure(GameInstance))
	{
		return {};
	}

	TSharedPtr<FVoxelNetworkManager>& Result = GVoxelGameInstanceToNetworkSubsystem.FindOrAdd(GameInstance);
	if (!Result)
	{
		Result = MakeShareable(new FVoxelNetworkManager(GameInstance));
	}
	return Result;
}

void FVoxelNetworkManager::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	const UGameInstance* GameInstance = WeakGameInstance.Resolve();
	if (!GameInstance)
	{
		TSharedPtr<FVoxelNetworkManager> This;
		GVoxelGameInstanceToNetworkSubsystem.RemoveAndCopyValue(WeakGameInstance, This);
		ensure(This.Get() == this);
		return;
	}

	const UWorld* World = GameInstance->GetWorld();
	if (!ensureVoxelSlow(World))
	{
		return;
	}

	TVoxelArray<TSharedPtr<FVoxelPeerManager>> NewPeerManagers;
	NewPeerManagers.Reserve(PeerManagers.Num());
	ON_SCOPE_EXIT
	{
		PeerManagers = MoveTemp(NewPeerManagers);
	};

	TVoxelInlineArray<UVoxelChannel*, 16> Channels;
	UVoxelChannel::GetChannels(World, Channels);

	if (Channels.Num() == 0)
	{
		Host.Reset();
		return;
	}

	if (!Host)
	{
		Host = MakeShared<Voxel::Network::FHost>(GetDebugStringForWorld(World));
		Host->Context->MTU = 1000;
	}

	TVoxelInlineArray<Voxel::Network::FReceivedPackets, 16> AllReceivedPackets;
	for (UVoxelChannel* Channel : Channels)
	{
		TSharedPtr<FVoxelPeerManager> PeerManager = Channel->WeakPeerManager.Pin();
		if (!PeerManager)
		{
			PeerManager = FVoxelPeerManager::Create(Host.ToSharedRef(), *Channel);
			Channel->WeakPeerManager = PeerManager;

			for (auto It = WeakClients.CreateIterator(); It; ++It)
			{
				const TSharedPtr<IVoxelNetworkClient> Client = It->Pin();
				if (!Client)
				{
					It.RemoveCurrentSwap();
					continue;
				}

				PeerManager->RegisterClient(Client.ToSharedRef());
			}

			for (auto It = WeakServers.CreateIterator(); It; ++It)
			{
				const TSharedPtr<IVoxelNetworkServer> Server = It->Pin();
				if (!Server)
				{
					It.RemoveCurrentSwap();
					continue;
				}

				PeerManager->RegisterServer(Server.ToSharedRef());
			}
		}
		NewPeerManagers.Add(PeerManager);

		AllReceivedPackets.Add(Voxel::Network::FReceivedPackets
		{
			PeerManager->Peer,
			MoveTemp(PeerManager->ReceivedPackets)
		});
	}

	Host->Tick(AllReceivedPackets);
}

void FVoxelNetworkManager::RegisterClient(const TSharedRef<IVoxelNetworkClient>& Client)
{
	check(IsInGameThread());

	for (const TSharedPtr<FVoxelPeerManager>& PeerManager : PeerManagers)
	{
		PeerManager->RegisterClient(Client);
	}

	WeakClients.Add(Client);
}

void FVoxelNetworkManager::RegisterServer(const TSharedRef<IVoxelNetworkServer>& Server)
{
	check(IsInGameThread());

	for (const TSharedPtr<FVoxelPeerManager>& PeerManager : PeerManagers)
	{
		PeerManager->RegisterServer(Server);
	}

	WeakServers.Add(Server);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelChannel::UVoxelChannel()
{
	ChName = "Voxel";
}

void UVoxelChannel::ReceivedBunch(FInBunch& Bunch)
{
	VOXEL_FUNCTION_COUNTER();

	if (Broken ||
		bTornOff)
	{
		return;
	}

	TSharedPtr<FVoxelPeerManager> PeerManager = WeakPeerManager.Pin();
	if (!PeerManager)
	{
		const UWorld* World = GetWorld();
		if (!ensure(World))
		{
			return;
		}

		FVoxelNetworkManager::Get(*World)->Tick();

		PeerManager = WeakPeerManager.Pin();

		if (!ensureVoxelSlow(PeerManager))
		{
			return;
		}
	}

	PeerManager->ReceiveBunch(Bunch);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelMap<TVoxelObjectPtr<UNetConnection>, int32> GNetConnectionToVoxelChannel;

UVoxelChannel* UVoxelChannel::FindChannel(UNetConnection& Connection)
{
	VOXEL_FUNCTION_COUNTER();

	int32 Index = -1;
	if (const int32* IndexPtr = GNetConnectionToVoxelChannel.Find(Connection))
	{
		Index = *IndexPtr;
	}

	if (!Connection.Channels.IsValidIndex(Index) ||
		!ensureVoxelSlow(Cast<UVoxelChannel>(Connection.Channels[Index])))
	{
		VOXEL_SCOPE_COUNTER("Iterate Channels");

		for (int32 ChannelIndex = 0; ChannelIndex < Connection.Channels.Num(); ChannelIndex++)
		{
			const TObjectPtr<UChannel> ChannelPtr = Connection.Channels[ChannelIndex];
			if (!ChannelPtr)
			{
				continue;
			}

			const UChannel* Channel = ResolveObjectPtrFast(ChannelPtr);
			if (!Channel ||
				!Channel->IsA<UVoxelChannel>())
			{
				continue;
			}

			for (auto It = GNetConnectionToVoxelChannel.CreateIterator(); It; ++It)
			{
				if (!It.Key().IsValid_Slow())
				{
					It.RemoveCurrent();
				}
			}
			GNetConnectionToVoxelChannel.FindOrAdd(Connection) = ChannelIndex;

			Index = ChannelIndex;
			break;
		}
	}

	if (!Connection.Channels.IsValidIndex(Index) ||
		!ensureVoxelSlow(Cast<UVoxelChannel>(Connection.Channels[Index])))
	{
		return nullptr;
	}

	UVoxelChannel* Channel = CastChecked<UVoxelChannel>(Connection.Channels[Index]);
	ensure(Channel->ChIndex == Index);
	return Channel;
}

void UVoxelChannel::GetChannels(
	const UWorld* World,
	TVoxelInlineArray<UVoxelChannel*, 16>& OutChannels)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensureVoxelSlow(World))
	{
		return;
	}

	if (World->GetNetMode() == NM_Standalone)
	{
		return;
	}

	const UNetDriver* NetDriver = World->GetNetDriver();
	if (!NetDriver)
	{
		return;
	}

	if (World->GetNetMode() == NM_Client)
	{
		const APlayerController* Controller = World->GetFirstPlayerController();
		if (!ensureVoxelSlow(Controller))
		{
			return;
		}

		UNetConnection* Connection = Controller->GetNetConnection();
		if (!ensureVoxelSlow(Connection))
		{
			return;
		}

		UVoxelChannel* Channel = FindChannel(*Connection);
		if (!ensureVoxelSlow(Channel))
		{
			return;
		}

		OutChannels.Add(Channel);
	}
	else
	{
		ensure(
			World->GetNetMode() == NM_DedicatedServer ||
			World->GetNetMode() == NM_ListenServer);

		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			const APlayerController* Controller = Iterator->Get();
			if (!ensureVoxelSlow(Controller) ||
				Controller->IsLocalController())
			{
				continue;
			}

			UNetConnection* Connection = Controller->GetNetConnection();
			if (!ensureVoxelSlow(Connection))
			{
				continue;
			}

			UVoxelChannel* Channel = FindChannel(*Connection);
			if (!ensureVoxelSlow(Channel))
			{
				continue;
			}

			OutChannels.Add(Channel);
		}
	}
}