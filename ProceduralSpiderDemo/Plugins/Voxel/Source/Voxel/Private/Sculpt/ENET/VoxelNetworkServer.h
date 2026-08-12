// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class IVoxelNetworkRemoteClient;
struct FVoxelNetworkClientToServerMessage;
struct FVoxelNetworkServerToClientMessage;

class IVoxelNetworkServer : public TSharedFromThis<IVoxelNetworkServer>
{
public:
	IVoxelNetworkServer() = default;
	virtual ~IVoxelNetworkServer();

	VOXEL_COUNT_INSTANCES();

public:
	FName GetNetworkId() const
	{
		return PrivateNetworkId;
	}

	void Register(
		const UWorld& World,
		FName NetworkId);

	void Unregister();

protected:
	virtual TSharedRef<IVoxelNetworkRemoteClient> CreateClientImpl() = 0;

	void ForeachClient(TVoxelFunctionRef<void(IVoxelNetworkRemoteClient& Client)> Lambda);
	void BroadcastMessage(const TSharedRef<const FVoxelNetworkServerToClientMessage>& Message);

private:
	FName PrivateNetworkId;
	FSimpleMulticastDelegate OnUnregister;
	TVoxelArray<TWeakPtr<IVoxelNetworkRemoteClient>> WeakClients;

	TSharedRef<IVoxelNetworkRemoteClient> CreateClient();

	friend class FVoxelPeerManager;
};

class IVoxelNetworkRemoteClient : public TSharedFromThis<IVoxelNetworkRemoteClient>
{
public:
	IVoxelNetworkRemoteClient() = default;
	virtual ~IVoxelNetworkRemoteClient() = default;

	VOXEL_COUNT_INSTANCES();

	FName GetNetworkId() const
	{
		ensure(!PrivateNetworkId.IsNone());
		return PrivateNetworkId;
	}

protected:
	void SendMessage(const TSharedRef<const FVoxelNetworkServerToClientMessage>& Message);

	virtual void OnConnect() = 0;
	virtual void OnMessageReceived(const TSharedRef<FVoxelNetworkClientToServerMessage>& Message) = 0;

private:
	FName PrivateNetworkId;
	TVoxelUniqueFunction<void(TVoxelArray<uint8>)> SendMessageBytes;
	TVoxelMap<int32, const UScriptStruct*> ClientToServerMessage_IndexToStruct;
	TVoxelMap<const UScriptStruct*, int32> ServerToClientMessage_StructToIndex;

	void OnMessageBytesReceived(TConstVoxelArrayView<uint8> Bytes);

	friend class IVoxelNetworkServer;
	friend class FVoxelPeerManager;
	friend class FVoxelChannelManager;
};