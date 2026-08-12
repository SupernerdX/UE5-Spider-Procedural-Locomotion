// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNetworkTypes.h"
#include "VoxelNetworkPeer.h"

namespace Voxel::Network
{

struct FReceivedPackets
{
	const TSharedRef<FPeer> Peer;
	const TVoxelArray<TVoxelArray<uint8>> Packets;

	FReceivedPackets(
		const TSharedRef<FPeer>& Peer,
		TVoxelArray<TVoxelArray<uint8>>&& Packets)
		: Peer(Peer)
		, Packets(MoveTemp(Packets))
	{
	}
};

class FHost
{
public:
	const FString Name;
	const TSharedRef<FContext> Context = MakeShared<FContext>();

	explicit FHost(const FString& Name);

	const FString& GetName() const
	{
		return Name;
	}

public:
	void Tick(TConstVoxelArrayView<FReceivedPackets> AllReceivedPackets);

	TSharedRef<FPeer> ConnectToPeer(
		const FString& PeerName,
		const FSendPacket& SendPacket,
		const FOnMessageReceived& OnMessageReceived);

	void RemovePeer(const TSharedRef<FPeer>& Peer);

private:
	bool bRecalculateBandwidthLimits = false;
	FTime LastBandwidthThrottleTime;
	int32 PacketsSinceLastThrottle = 0;
	int32 LastIncomingBandwidthLimit = 0;
	int32 LastOutgoingBandwidthLimit = 0;
	TVoxelSparseArray<TSharedRef<FPeer>> Peers;

	void Flush();

private:
	void UpdateBandwidthThrottle();
	void SendOutgoingCommands(bool bCheckForTimeouts);

	void HandlePackets(const FReceivedPackets& ReceivedPackets);

	void HandlePacket(
		const TSharedRef<FPeer>& Peer,
		TConstVoxelArrayView<uint8> PacketData);
};

}