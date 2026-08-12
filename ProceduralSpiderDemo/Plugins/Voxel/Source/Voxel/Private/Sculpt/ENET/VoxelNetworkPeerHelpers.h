// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNetworkCommands.h"

namespace Voxel::Network
{

struct FAcknowledgement : TNode<FAcknowledgement>
{
	FCommand Command;
	FPackedTime SentTime;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FOutgoingCommand : TNode<FOutgoingCommand>
{
	FCommand Command;
	TVoxelArray<uint8> Payload;

	FTime FirstSendTime;
	FTime SendTime;
	int32 SendAttempts = 0;
	FTimespan RoundTripTimeout;

	FORCEINLINE FTime GetTimeoutTime() const
	{
		return SendTime + RoundTripTimeout;
	}
	FORCEINLINE bool HasPayload() const
	{
		return Payload.Num() > 0;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FDropRate
{
	// Current drop rate based on RTT
	// 1 is drop all, 0 is drop none
	double Value = 0;
	// Minimum drop rate to apply to respect bandwidth limits on both sides
	double MinValue = 0;
	// Never drop ALL packets
	static constexpr double MaxValue = 31 / 32.;

	void SetMin(const double MinDropRate)
	{
		MinValue = FMath::Clamp(MinDropRate, 0, MaxValue);
		Set(Value);
	}
	void Set(const double DropRate)
	{
		Value = FMath::Clamp(DropRate, MinValue, MaxValue);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FRTTHandler
{
public:
	const FPeer& Peer;

	explicit FRTTHandler(const FPeer& Peer)
		: Peer(Peer)
	{
	}

	const FString& GetName() const;

public:
	FORCEINLINE FTimespan GetSpotRTT() const
	{
		return RTT;
	}
	FORCEINLINE FTimespan GetSpotRTTVariance() const
	{
		return RTTVariance;
	}

	FORCEINLINE FTimespan GetSmoothedRTT() const
	{
		return LastRTT;
	}
	FORCEINLINE FTimespan GetSmoothedRTTVariance() const
	{
		return LastRTTVariance;
	}

	void Update(
		FTime Time,
		FTimespan PacketRTT);

private:
	FTime LastRTTUpdateTime;

	FTimespan MinRTT = FTimespan::FromMilliseconds(500);
	FTimespan HighestRTTVariance;

	FTimespan LastRTT = FTimespan::FromMilliseconds(500);
	FTimespan LastRTTVariance;

	FTimespan RTT = FTimespan::FromMilliseconds(500);
	FTimespan RTTVariance;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FSentCommands
{
public:
	const FPeer& Peer;

	explicit FSentCommands(const FPeer& Peer)
		: Peer(Peer)
	{
	}

	const FString& GetName() const;

public:
	FORCEINLINE bool IsEmpty() const
	{
		return Commands.IsEmpty();
	}
	FORCEINLINE int32 GetInFlightSize() const
	{
		return InFlightSize;
	}

public:
	void Add(TUniquePtr<FOutgoingCommand> Command);

	TUniquePtr<FOutgoingCommand> TryRemove(
		uint8 ChannelId,
		uint16 SequenceNumber);

	void CheckTimeouts(
		const FTime Time,
		const TVoxelFunctionRef<void(TUniquePtr<FOutgoingCommand>)> QueueCommand);

	void GetStats(
		int32& OutPacketsSent,
		int32& OutPacketsLost,
		int32& OutPacketsQueued,
		FTimespan& OutLag) const;

private:
	// Sorted by timeout
	TList<FOutgoingCommand> Commands;
	int32 InFlightSize = 0;

	int32 PacketsSent = 0;
	int32 PacketsLost = 0;

	FTime LastPacketLossComputeTime;

	void Check() const;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FCommandsToSent
{
	TList<FOutgoingCommand> WithPayload;
	TList<FOutgoingCommand> WithoutPayload;

	void Add(TUniquePtr<FOutgoingCommand> OutgoingCommand);

	TUniquePtr<FOutgoingCommand> TryRemove(
		uint8 ChannelId,
		uint16 SequenceNumber);
};
}