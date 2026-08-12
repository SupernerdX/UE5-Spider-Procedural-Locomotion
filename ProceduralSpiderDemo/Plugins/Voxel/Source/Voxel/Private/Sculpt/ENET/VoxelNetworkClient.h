// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelNetworkClientToServerMessage;
struct FVoxelNetworkServerToClientMessage;

class IVoxelNetworkClient : public TSharedFromThis<IVoxelNetworkClient>
{
public:
	VOXEL_COUNT_INSTANCES();

	IVoxelNetworkClient() = default;
	virtual ~IVoxelNetworkClient() = default;

public:
	void Register(
		const UWorld& World,
		FName NetworkId);

	FName GetNetworkId() const
	{
		return PrivateNetworkId;
	}

protected:
	void SendMessage(const TSharedRef<const FVoxelNetworkClientToServerMessage>& Message);

	virtual void OnRegistered() {}
	virtual void OnMessageReceived(const TSharedRef<FVoxelNetworkServerToClientMessage>& Message) = 0;

private:
	FName PrivateNetworkId;
	TVoxelUniqueFunction<void(TVoxelArray<uint8>)> SendMessageBytes;
	TVoxelMap<int32, const UScriptStruct*> ServerToClientMessage_IndexToStruct;
	TVoxelMap<const UScriptStruct*, int32> ClientToServerMessage_StructToIndex;

	void OnMessageBytesReceived(TConstVoxelArrayView<uint8> Bytes);

	friend class FVoxelPeerManager;
	friend class FVoxelChannelManager;
};