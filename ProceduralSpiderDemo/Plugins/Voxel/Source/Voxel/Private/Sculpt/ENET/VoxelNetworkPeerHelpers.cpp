// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNetworkPeerHelpers.h"
#include "Sculpt/ENET/VoxelNetworkLog.h"
#include "Sculpt/ENET/VoxelNetworkPeer.h"

namespace Voxel::Network
{

const FString& FRTTHandler::GetName() const
{
	return Peer.Name;
}

void FRTTHandler::Update(
	const FTime Time,
	const FTimespan PacketRTT)
{
	if (LastRTTUpdateTime.IsNull())
	{
		RTT = PacketRTT;
		RTTVariance = PacketRTT / 2;
		LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FRTTHandler::Update: Initial RTT=%dms Variance=%dms",
			RTT.Milliseconds,
			RTTVariance.Milliseconds);
	}
	else
	{
		const FTimespan Delta = PacketRTT - RTT;

		RTT += Delta / 8;

		RTTVariance -= RTTVariance / 4;
		RTTVariance += Delta.Abs() / 4;

		LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FRTTHandler::Update: PacketRTT=%dms SpotRTT=%dms SpotVariance=%dms",
			PacketRTT.Milliseconds,
			RTT.Milliseconds,
			RTTVariance.Milliseconds);
	}

	MinRTT = FMath::Min(MinRTT, RTT);
	HighestRTTVariance = FMath::Max(HighestRTTVariance, RTTVariance);

	if (Time - LastRTTUpdateTime >= FTimespan::FromSeconds(5))
	{
		LastRTTUpdateTime = Time;

		LastRTT = MinRTT;
		LastRTTVariance = FMath::Max(HighestRTTVariance, FTimespan::FromMilliseconds(1));

		LOG_VOXEL_NET_TRANSPORT(Verbose, "FRTTHandler::Update: Updated smoothed RTT=%dms Variance=%dms",
			LastRTT.Milliseconds,
			LastRTTVariance.Milliseconds);

		MinRTT = RTT;
		HighestRTTVariance = RTTVariance;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const FString& FSentCommands::GetName() const
{
	return Peer.Name;
}

void FSentCommands::Add(TUniquePtr<FOutgoingCommand> Command)
{
	VOXEL_FUNCTION_COUNTER();

	Check();
	ON_SCOPE_EXIT
	{
		Check();
	};

	PacketsSent++;
	InFlightSize += Command->Payload.Num();

	for (FOutgoingCommand& OtherCommand : Commands.Reverse())
	{
		if (OtherCommand.GetTimeoutTime() <= Command->GetTimeoutTime())
		{
			OtherCommand.List_InsertAfter(MoveTemp(Command));
			return;
		}
	}

	Commands.PushFront(MoveTemp(Command));
}

TUniquePtr<FOutgoingCommand> FSentCommands::TryRemove(
	const uint8 ChannelId,
	const uint16 SequenceNumber)
{
	VOXEL_FUNCTION_COUNTER();

	for (FOutgoingCommand& Command : Commands)
	{
		if (Command.Command.Header.ChannelId == ChannelId &&
			Command.Command.Header.SequenceNumber == SequenceNumber)
		{
			InFlightSize -= Command.Payload.Num();
			checkVoxelSlow(InFlightSize >= 0);

			return Command.List_Remove();
		}
	}

	return {};
}

void FSentCommands::CheckTimeouts(
	const FTime Time,
	const TVoxelFunctionRef<void(TUniquePtr<FOutgoingCommand>)> QueueCommand)
{
	VOXEL_FUNCTION_COUNTER();

	Check();

	int32 NumTimedOut = 0;
	for (auto It = Commands.CreateIterator(); It; ++It)
	{
		FOutgoingCommand& OutgoingCommand = *It;

		if (Time < OutgoingCommand.GetTimeoutTime())
		{
			if (VOXEL_DEBUG)
			{
				while (It)
				{
					check(Time < It->GetTimeoutTime());
					++It;
				}
			}
			break;
		}

		PacketsLost++;
		NumTimedOut++;

		LOG_VOXEL_NET_TRANSPORT(Verbose, "FSentCommands::CheckTimeouts: Packet timed out (channel %d seq %d, attempt %d, timeout %dms)",
			OutgoingCommand.Command.Header.ChannelId,
			OutgoingCommand.Command.Header.SequenceNumber,
			OutgoingCommand.SendAttempts,
			OutgoingCommand.RoundTripTimeout.Milliseconds);

		OutgoingCommand.RoundTripTimeout *= 2;
		if (!ensureVoxelSlow(OutgoingCommand.RoundTripTimeout < FTimespan::FromSeconds(1)))
		{
			LOG_VOXEL_NET_TRANSPORT(Warning, "FSentCommands::CheckTimeouts: Timeout exceeded 1 second (%dms)",
				OutgoingCommand.RoundTripTimeout.Milliseconds);
		}

		InFlightSize -= OutgoingCommand.Payload.Num();
		checkVoxelSlow(InFlightSize >= 0);

		QueueCommand(It.RemoveCurrent());
	}

	if (NumTimedOut > 0)
	{
		LOG_VOXEL_NET_TRANSPORT(Verbose, "FSentCommands::CheckTimeouts: %d packets timed out (total lost: %d, sent: %d)",
			NumTimedOut,
			PacketsLost,
			PacketsSent);
	}
}

void FSentCommands::GetStats(
	int32& OutPacketsSent,
	int32& OutPacketsLost,
	int32& OutPacketsQueued,
	FTimespan& OutLag) const
{
	VOXEL_FUNCTION_COUNTER();

	const FTime Time = FTime::Now();

	int32 NumPackets = 0;
	FTimespan MaxLag;

	for (const FOutgoingCommand& Command : Commands)
	{
		NumPackets++;
		MaxLag = FMath::Max(MaxLag, Time - Command.FirstSendTime);
	}

	OutPacketsSent = PacketsSent;
	OutPacketsLost = PacketsLost;
	OutPacketsQueued = NumPackets;
	OutLag = MaxLag;
}

void FSentCommands::Check() const
{
	if (!VOXEL_DEBUG)
	{
		return;
	}

	int64 InFlightActualSize = 0;

	FTime Timeout;
	for (const FOutgoingCommand& Command : Commands)
	{
		check(Timeout <= Command.GetTimeoutTime());
		Timeout = Command.GetTimeoutTime();

		InFlightActualSize += Command.Payload.Num();
	}
	check(InFlightSize == InFlightActualSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FCommandsToSent::Add(TUniquePtr<FOutgoingCommand> OutgoingCommand)
{
	if (OutgoingCommand->HasPayload())
	{
		WithPayload.PushBack(MoveTemp(OutgoingCommand));
	}
	else
	{
		WithoutPayload.PushBack(MoveTemp(OutgoingCommand));
	}
}

TUniquePtr<FOutgoingCommand> FCommandsToSent::TryRemove(
	const uint8 ChannelId,
	const uint16 SequenceNumber)
{
	VOXEL_FUNCTION_COUNTER();

	for (FOutgoingCommand& Command : WithPayload)
	{
		if (Command.SendAttempts == 0)
		{
			break;
		}

		if (Command.Command.Header.ChannelId == ChannelId &&
			Command.Command.Header.SequenceNumber == SequenceNumber)
		{
			return Command.List_Remove();
		}
	}

	for (FOutgoingCommand& Command : WithoutPayload)
	{
		if (Command.SendAttempts == 0)
		{
			break;
		}

		if (Command.Command.Header.ChannelId == ChannelId &&
			Command.Command.Header.SequenceNumber == SequenceNumber)
		{
			return Command.List_Remove();
		}
	}

	return {};
}

}