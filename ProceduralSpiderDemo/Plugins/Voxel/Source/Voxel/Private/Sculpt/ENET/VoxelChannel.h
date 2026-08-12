// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Engine/ActorChannel.h"
#include "Sculpt/ENET/VoxelNetworkHost.h"
#include "Sculpt/ENET/VoxelNetworkPeer.h"
#include "VoxelChannel.generated.h"

class IVoxelNetworkRemoteClient;
class IVoxelNetworkServer;
class IVoxelNetworkClient;
class UVoxelChannel;
class AVoxelSculptVolume;

class FVoxelChannelManager
{
public:
	const FName NetworkId;
	const FString PeerName;
	TSharedPtr<IVoxelNetworkClient> Client;
	TSharedPtr<IVoxelNetworkRemoteClient> RemoteClient;
	TVoxelArray<TVoxelArray<uint8>> QueuedMessages;

	FVoxelChannelManager(
		const FName NetworkId,
		const FString& PeerName)
		: NetworkId(NetworkId)
		, PeerName(PeerName)
	{
	}

	void OnMessageReceived(TConstVoxelArrayView<uint8> Data);
};

class FVoxelPeerManager : public TSharedFromThis<FVoxelPeerManager>
{
public:
	const TWeakPtr<Voxel::Network::FHost> WeakHost;
	const TVoxelObjectPtr<UVoxelChannel> WeakChannel;
	const TSharedRef<Voxel::Network::FPeer> Peer = SharedRef_Null;
	TVoxelArray<TVoxelArray<uint8>> ReceivedPackets;

	static TSharedRef<FVoxelPeerManager> Create(
		const TSharedRef<Voxel::Network::FHost>& Host,
		UVoxelChannel& Channel);

	void ReceiveBunch(FInBunch& Bunch);

	~FVoxelPeerManager();

public:
	void RegisterClient(const TSharedRef<IVoxelNetworkClient>& Client);
	void RegisterServer(const TSharedRef<IVoxelNetworkServer>& Server);

private:
	static constexpr uint64 Magic0 = 0xF219B49E8C5048B2;
	static constexpr uint64 Magic1 = 0x95DA76AB009FC5D8;
	static constexpr uint64 Magic2 = 0x1D13B213604245D4;
	static constexpr uint64 RegisterChannelMagic = 0xA97C7AD0E16253C7;
	static constexpr uint64 UnregisterChannelMagic = 0xD8E7D37F7EC647DD;

	bool bFirstPacketSent = false;
	TVoxelMap<FName, TSharedPtr<IVoxelNetworkClient>> NetworkIdToPendingClient;
	TVoxelMap<FName, uint8> NetworkIdToChannelId;
	TVoxelArray<TSharedPtr<FVoxelChannelManager>> Channels;

	explicit FVoxelPeerManager(
		const TWeakPtr<Voxel::Network::FHost>& WeakHost,
		const TVoxelObjectPtr<UVoxelChannel> WeakChannel)
		: WeakHost(WeakHost)
		, WeakChannel(WeakChannel)
	{
	}

	void SendPacket(TConstVoxelArrayView<uint8> Data);

	void OnMessageReceived(
		uint8 ChannelId,
		TVoxelArray<uint8> Data);
};

class FVoxelNetworkManager : public FVoxelTicker
{
public:
	static TSharedPtr<FVoxelNetworkManager> Get(const UWorld& World);

	//~ Begin FVoxelTicker Interface
	virtual void Tick() override;
	//~ End FVoxelTicker Interface

public:
	void RegisterClient(const TSharedRef<IVoxelNetworkClient>& Client);
	void RegisterServer(const TSharedRef<IVoxelNetworkServer>& Server);

private:
	const TVoxelObjectPtr<UGameInstance> WeakGameInstance;
	TVoxelArray<TWeakPtr<IVoxelNetworkClient>> WeakClients;
	TVoxelArray<TWeakPtr<IVoxelNetworkServer>> WeakServers;

	explicit FVoxelNetworkManager(const TVoxelObjectPtr<UGameInstance> WeakGameInstance)
		: WeakGameInstance(WeakGameInstance)
	{
	}

private:
	TSharedPtr<Voxel::Network::FHost> Host;
	TVoxelArray<TSharedPtr<FVoxelPeerManager>> PeerManagers;
};

UCLASS()
class VOXEL_API UVoxelChannel : public UChannel
{
	GENERATED_BODY()

public:
	TWeakPtr<FVoxelPeerManager> WeakPeerManager;

	UVoxelChannel();

	//~ Begin UChannel Interface
	virtual void ReceivedBunch(FInBunch& Bunch) override;
	//~ End UChannel Interface

public:
	static UVoxelChannel* FindChannel(UNetConnection& Connection);

	static void GetChannels(
		const UWorld* World,
		TVoxelInlineArray<UVoxelChannel*, 16>& OutChannels);
};