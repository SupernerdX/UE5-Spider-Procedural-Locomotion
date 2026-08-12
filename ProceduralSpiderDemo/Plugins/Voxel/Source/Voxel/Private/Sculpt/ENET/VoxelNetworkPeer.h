// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNetworkChannel.h"
#include "VoxelNetworkPeerHelpers.h"

namespace Voxel::Network
{

class FPeer
{
public:
	const FString Name;
	const int32 PeerIndex;
	const FSendPacket SendPacket;
	const FOnMessageReceived OnMessageReceived;
	const TSharedRef<FContext> Context;

	FPeer(
		const FString& Name,
		const int32 PeerIndex,
		const FSendPacket& SendPacket,
		const FOnMessageReceived& OnMessageReceived,
		const TSharedRef<FContext>& Context)
		: Name(Name)
		, PeerIndex(PeerIndex)
		, SendPacket(SendPacket)
		, OnMessageReceived(OnMessageReceived)
		, Context(Context)
	{
	}

	const FString& GetName() const
	{
		return Name;
	}

public:
	struct FDetailedStats
	{
		int32 PacketsSent = 0;
		int32 PacketsLost = 0;
		int32 PacketsQueued = 0;
		int32 PacketsWaitingForAck = 0;

		double DropRate = 0;

		FTimespan RTT;
		FTimespan RTTVariance;

		// Unsmoothed
		FTimespan SpotRTT;
		FTimespan SpotRTTVariance;

		// How far back the oldest unacknowledged packet is
		// Should disconnect if >= 30s
		FTimespan Lag;
	};
	FDetailedStats GetStats() const;

public:
	int32 IncomingBandwidthLimit = 0;
	int32 OutgoingBandwidthLimit = 0;
	bool bInitialBandwidthLimitReceived = false;

	FTime LastSendTime;
	FTime LastReceiveTime;

	int32 BytesSentToPeer = 0;
	uint16 OutgoingSequenceNumber = 0;

	FDropRate DropRate;
	FRTTHandler RTTHandler{ *this };
	FSentCommands SentCommands{ *this };
	FCommandsToSent CommandsToSend;
	TList<FAcknowledgement> Acknowledgements;
	TVoxelArray<TSharedPtr<FChannel>> InternalChannels;

	TSharedPtr<FChannel> FindChannel(const uint8 ChannelId);
	TSharedRef<FChannel> FindOrAddChannel(const uint8 ChannelId);

	void Send(
		uint8 ChannelId,
		TConstVoxelArrayView<uint8> Data);

public:
	template<ECommand Command>
	void QueueSystemCommand(const TCommandData<Command>& CommandData)
	{
		TUniquePtr<FOutgoingCommand> OutgoingCommand = MakeUnique<FOutgoingCommand>();
		OutgoingCommand->Command.Header.ChannelId = InvalidChannelId;
		OutgoingCommand->Command.SetData(CommandData);
		QueueOutgoingCommand(MoveTemp(OutgoingCommand));
	}

	void QueueOutgoingCommand(TUniquePtr<FOutgoingCommand> OutgoingCommand);

	void QueueAcknowledgement(
		const FCommand& Command,
		const FPackedTime SentTime);

public:
	void WriteOutgoingCommands(FPacketWriter& PacketWriter);

public:
	void HandleAcknowledge(const TCommand<ECommand::Acknowledge>& Command);

	void HandleSendFragment(
		const TCommand<ECommand::SendFragment>& Command,
		TConstVoxelArrayView<uint8>& Data);
};

}