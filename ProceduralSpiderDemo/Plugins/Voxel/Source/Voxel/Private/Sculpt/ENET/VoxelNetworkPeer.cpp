// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNetworkPeer.h"
#include "VoxelNetworkPacket.h"
#include "VoxelNetworkChannel.h"
#include "Sculpt/ENET/VoxelNetworkLog.h"

namespace Voxel::Network
{

FPeer::FDetailedStats FPeer::GetStats() const
{
	VOXEL_FUNCTION_COUNTER();

	int32 PacketsSent = 0;
	int32 PacketsLost = 0;
	int32 PacketsWaitingForAck = 0;
	FTimespan Lag;
	SentCommands.GetStats(
		PacketsSent,
		PacketsLost,
		PacketsWaitingForAck,
		Lag);

	return FDetailedStats
	{
		PacketsSent,
		PacketsLost,
		CommandsToSend.WithPayload.Num_Slow() + CommandsToSend.WithoutPayload.Num_Slow(),
		PacketsWaitingForAck,
		DropRate.Value,
		RTTHandler.GetSmoothedRTT(),
		RTTHandler.GetSmoothedRTTVariance(),
		RTTHandler.GetSpotRTT(),
		RTTHandler.GetSpotRTTVariance(),
		Lag
	};
}

TSharedPtr<FChannel> FPeer::FindChannel(const uint8 ChannelId)
{
	if (!ensure(ChannelId != InvalidChannelId))
	{
		return {};
	}
	if (!ensure(InternalChannels.IsValidIndex(ChannelId)))
	{
		return {};
	}

	return InternalChannels[ChannelId];
}

TSharedRef<FChannel> FPeer::FindOrAddChannel(const uint8 ChannelId)
{
	check(ChannelId != InvalidChannelId);

	if (!InternalChannels.IsValidIndex(ChannelId))
	{
		InternalChannels.SetNum(ChannelId + 1);
	}

	if (!InternalChannels[ChannelId])
	{
		InternalChannels[ChannelId] = MakeShared<FChannel>(
			FString::Printf(TEXT("%s Channel %d"), *Name, ChannelId),
			ChannelId);
	}

	return InternalChannels[ChannelId].ToSharedRef();
}

void FPeer::Send(
	const uint8 ChannelId,
	const TConstVoxelArrayView<uint8> Data)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FChannel> Channel = FindOrAddChannel(ChannelId);

	LOG_VOXEL_NET_TRANSPORT(Verbose, "FPeer::Send: Sending %d bytes on %s",
		Data.Num(),
		*Channel->GetName());

	if (!ensure(Data.Num() <= MaxMessageSize) ||
		!ensure(Context->MTU > sizeof(FPacketHeader) + sizeof(FCommand)) ||
		!ensure(Context->MTU <= MaxMTU))
	{
		return;
	}

	const int32 MaxFragmentLength = Context->MTU - sizeof(FPacketHeader) - sizeof(TCommand<ECommand::SendFragment>);

	const int32 NumFragments = FMath::DivideAndRoundUp(Data.Num(), MaxFragmentLength);
	if (!ensure(NumFragments <= MaxFragmentsPerMessage))
	{
		return;
	}

	const uint16 StartSequenceNumber = uint16(Channel->Outgoing.SequenceNumber + 1);

	LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FPeer::Send: Fragmenting into %d fragments, start seq %d",
		NumFragments,
		StartSequenceNumber);

	int32 FragmentOffset = 0;
	for (int32 FragmentIndex = 0; FragmentIndex < NumFragments; FragmentIndex++)
	{
		const int32 FragmentLength = FMath::Min(MaxFragmentLength, Data.Num() - FragmentOffset);

		FSendFragment SendFragment;
		SendFragment.StartSequenceNumber = StartSequenceNumber;
		SendFragment.MessageSize = Data.Num();
		SendFragment.NumFragments = NumFragments;
		SendFragment.FragmentIndex = FragmentIndex;
		SendFragment.FragmentOffset = FragmentOffset;
		SendFragment.FragmentLength = FragmentLength;

		TCommand<ECommand::SendFragment> Command;
		Command.Header.ChannelId = Channel->ChannelId;
		Command.Set(SendFragment);

		TUniquePtr<FOutgoingCommand> OutgoingCommand = MakeUnique<FOutgoingCommand>();
		OutgoingCommand->Payload = Data.Slice(FragmentOffset, FragmentLength).Array();
		OutgoingCommand->Command = Command;
		QueueOutgoingCommand(MoveTemp(OutgoingCommand));

		FragmentOffset += FragmentLength;
	}
	check(FragmentOffset == Data.Num());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FPeer::QueueOutgoingCommand(TUniquePtr<FOutgoingCommand> OutgoingCommand)
{
	BytesSentToPeer += OutgoingCommand->Command.View().Num() + OutgoingCommand->Payload.Num();

	const uint8 ChannelId = OutgoingCommand->Command.Header.ChannelId;

	if (ChannelId == InvalidChannelId)
	{
		OutgoingSequenceNumber++;
		OutgoingCommand->Command.Header.SequenceNumber = OutgoingSequenceNumber;
	}
	else
	{
		const TSharedPtr<FChannel> Channel = FindChannel(ChannelId);
		if (!ensure(Channel))
		{
			return;
		}

		Channel->Outgoing.SequenceNumber++;
		OutgoingCommand->Command.Header.SequenceNumber = Channel->Outgoing.SequenceNumber;
	}

	CommandsToSend.Add(MoveTemp(OutgoingCommand));
}

void FPeer::QueueAcknowledgement(
	const FCommand& Command,
	const FPackedTime SentTime)
{
	if (Command.Header.ChannelId != InvalidChannelId)
	{
		const TSharedPtr<FChannel> Channel = FindChannel(Command.Header.ChannelId);
		if (!ensure(Channel))
		{
			return;
		}

		const uint16 SequenceNumber = Command.Header.SequenceNumber;

		const int32 UnwrappedSequenceNumber =
			Channel->Incoming.SequenceNumberHead <= SequenceNumber
			? SequenceNumber
			: SequenceNumber + (1 << 16);

		const int32 Window = UnwrappedSequenceNumber / FWindow::Size;
		const int32 ActiveWindow = Channel->Incoming.SequenceNumberHead / FWindow::Size;

		if (Window == ActiveWindow + FWindow::FreeWindows - 1 ||
			Window == ActiveWindow + FWindow::FreeWindows)
		{
			// This packet is too far ahead, don't send a ack as we could end up acking something we haven't received yet
			// Only check ActiveWindow + 7 and + 8 as further ahead might be wrapped around acks for packets we already received
			// eg if current window is 10 and this ack window is 9 (but we still need to ack it), its unwrapped window will be 25
			ensureVoxelSlow(false);
			return;
		}
	}

	BytesSentToPeer += sizeof(TCommand<ECommand::Acknowledge>);

	TUniquePtr<FAcknowledgement> Acknowledgement = MakeUnique<FAcknowledgement>();
	Acknowledgement->Command = Command;
	Acknowledgement->SentTime = SentTime;
	Acknowledgements.PushBack(MoveTemp(Acknowledgement));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FPeer::WriteOutgoingCommands(FPacketWriter& PacketWriter)
{
	VOXEL_FUNCTION_COUNTER();

	bool bWindowWrap = false;
	bool bCanProcessPayloads = true;
	TList<FOutgoingCommand>::FIterator WithoutPayloadIt = CommandsToSend.WithoutPayload.CreateIterator();
	TList<FOutgoingCommand>::FIterator WithPayloadIt = CommandsToSend.WithPayload.CreateIterator();

	while (true)
	{
		FOutgoingCommand* OutgoingCommand;
		if (WithoutPayloadIt)
		{
			// Process system messages first
			OutgoingCommand = &WithoutPayloadIt.Get();
			++WithoutPayloadIt;

			checkVoxelSlow(!OutgoingCommand->HasPayload());
		}
		else if (
			bCanProcessPayloads &&
			WithPayloadIt)
		{
			// Process payloads if we are allowed to (ie network not overwhelmed)
			OutgoingCommand = &WithPayloadIt.Get();
			++WithPayloadIt;

			checkVoxelSlow(OutgoingCommand->HasPayload());
		}
		else
		{
			return;
		}

		const int32 CommandSize = OutgoingCommand->Command.View().Num();

		if (!PacketWriter.CanWrite(CommandSize + OutgoingCommand->Payload.Num()))
		{
			PacketWriter.bOverflow = true;
			return;
		}

		// Check window size
		if (OutgoingCommand->HasPayload())
		{
			int32 WindowSize = FMath::Min(IncomingBandwidthLimit, Context->OutgoingBandwidthLimit) * 10 / 128;
			WindowSize = FMath::Clamp(WindowSize, 4096, 32 * 1024 * 1024);
			WindowSize = FMath::CeilToInt32((1 - DropRate.Value) * WindowSize);
			WindowSize = FMath::Max(WindowSize, PacketWriter.MTU);

			if (SentCommands.GetInFlightSize() + OutgoingCommand->Payload.Num() > WindowSize)
			{
				bCanProcessPayloads = false;
				continue;
			}
		}

		const uint8 ChannelId = OutgoingCommand->Command.Header.ChannelId;

		TSharedPtr<FChannel> Channel;
		if (ChannelId != InvalidChannelId)
		{
			// Only SendFragment can have a payload, acknowledgments are handled elsewhere
			ensure(OutgoingCommand->HasPayload());

			Channel = FindChannel(ChannelId);

			if (!ensure(Channel))
			{
				return;
			}
		}

		const int32 Window = OutgoingCommand->Command.Header.SequenceNumber / FWindow::Size;

		if (Channel)
		{
			if (bWindowWrap)
			{
				continue;
			}

			if (OutgoingCommand->SendAttempts == 0)
			{
				if (OutgoingCommand->Command.Header.SequenceNumber % FWindow::Size == 0)
				{
					// Check if previous window is full
					const int32 PreviousWindow = FVoxelUtilities::PositiveMod(Window - 1, FWindow::NumWindows);
					if (Channel->Outgoing.Windows[PreviousWindow] >= FWindow::Size)
					{
						bWindowWrap = true;
						continue;
					}

					// Check if any of the next FreeWindows + 2 windows are in use
					const bool bTooManyWindowsActive = INLINE_LAMBDA
					{
						for (int32 Index = 0; Index < FWindow::FreeWindows + 2; Index++)
						{
							const int32 WindowIndex = (Window + Index) % FWindow::NumWindows;
							if (Channel->Outgoing.Windows[WindowIndex] > 0)
							{
								return true;
							}
						}
						return false;
					};

					if (bTooManyWindowsActive)
					{
						bWindowWrap = true;
						continue;
					}
				}

				Channel->Outgoing.Windows[Window]++;
			}
		}

		if (OutgoingCommand->SendAttempts == 0)
		{
			OutgoingCommand->FirstSendTime = Context->Time;
		}

		OutgoingCommand->SendTime = Context->Time;
		OutgoingCommand->SendAttempts++;

		if (OutgoingCommand->RoundTripTimeout.IsNull())
		{
			OutgoingCommand->RoundTripTimeout = RTTHandler.GetSpotRTT() + 4 * RTTHandler.GetSpotRTTVariance();
		}

		PacketWriter.Write(OutgoingCommand->Command.View());
		PacketWriter.Write(OutgoingCommand->Payload);

		SentCommands.Add(OutgoingCommand->List_Remove());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FPeer::HandleAcknowledge(const TCommand<ECommand::Acknowledge>& Command)
{
	const FTime ReceivedSentTime = Command.ReceivedSentTime.Unpack(Context->Time);
	if (!ensureVoxelSlow(ReceivedSentTime <= Context->Time))
	{
		LOG_VOXEL_NET_TRANSPORT(Warning, "FPeer::HandleAcknowledge: Invalid sent time for channel %d seq %d",
			Command.Header.ChannelId,
			Command.Header.SequenceNumber);

		return;
	}

	const FTimespan PacketRTT = FMath::Max(Context->Time - ReceivedSentTime, FTimespan::FromMilliseconds(1));

	LastReceiveTime = Context->Time;

	RTTHandler.Update(Context->Time, PacketRTT);

	LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FPeer::HandleAcknowledge: channel %d seq %d, RTT=%dms",
		Command.Header.ChannelId,
		Command.Header.SequenceNumber,
		PacketRTT.Milliseconds);

	const FTimespan RTT = RTTHandler.GetSmoothedRTT();
	const FTimespan RTTVariance = RTTHandler.GetSmoothedRTTVariance();

	if (RTT <= RTTVariance)
	{
		// If the variance is HIGHER than the average, the network is extremely unstable
		// Give up doing anything fancy, just send as much as we can
		DropRate.Set(0);
	}
	else if (PacketRTT <= RTT)
	{
		// RTT is decreasing, reduce drop rate
		DropRate.Set(DropRate.Value - 0.06);
	}
	else if (PacketRTT > RTT + 2 * RTTVariance)
	{
		// RTT is increasing, increase drop rate
		DropRate.Set(DropRate.Value + 0.06);
	}

	const uint8 ChannelId = Command.Header.ChannelId;
	const uint16 SequenceNumber = Command.Header.SequenceNumber;

	if (!SentCommands.TryRemove(ChannelId, SequenceNumber) &&
		!CommandsToSend.TryRemove(ChannelId, SequenceNumber))
	{
		// Failed to find the source of this acknowledgment, likely sent twice
		LOG_VOXEL_NET_TRANSPORT(Verbose, "FPeer::HandleAcknowledge: Command not found for channel %d seq %d",
			ChannelId,
			SequenceNumber);

		return;
	}

	if (ChannelId != InvalidChannelId)
	{
		const TSharedPtr<FChannel> Channel = FindChannel(ChannelId);
		if (!ensure(Channel))
		{
			return;
		}

		const int32 Window = SequenceNumber / FWindow::Size;
		if (ensure(Channel->Outgoing.Windows[Window] > 0))
		{
			Channel->Outgoing.Windows[Window]--;
		}
	}
}

void FPeer::HandleSendFragment(
	const TCommand<ECommand::SendFragment>& Command,
	TConstVoxelArrayView<uint8>& Data)
{
	if (!ensure(Command.Header.ChannelId != InvalidChannelId))
	{
		LOG_VOXEL_NET_TRANSPORT(Warning, "FPeer::HandleSendFragment: Invalid channel ID");
		return;
	}

	const uint16 SequenceNumber = uint16(Command.Get().StartSequenceNumber);
	const int32 MessageSize = Command.Get().MessageSize;
	const int32 NumFragments = Command.Get().NumFragments;
	const int32 FragmentIndex = Command.Get().FragmentIndex;
	const int32 FragmentOffset = Command.Get().FragmentOffset;
	const int32 FragmentLength = Command.Get().FragmentLength;

	LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FPeer::HandleSendFragment: channel %d seq %d fragment %d/%d (%d bytes at offset %d, total %d)",
		Command.Header.ChannelId,
		SequenceNumber,
		FragmentIndex + 1,
		NumFragments,
		FragmentLength,
		FragmentOffset,
		MessageSize);

	if (!ensure(FragmentLength <= Data.Num()))
	{
		LOG_VOXEL_NET_TRANSPORT(Warning, "FPeer::HandleSendFragment: Fragment length %d exceeds data size %d",
			FragmentLength,
			Data.Num());

		return;
	}

	const TConstVoxelArrayView<uint8> PacketData = Data.LeftOf(FragmentLength);
	Data = Data.RightOf(FragmentLength);

	if (!ensure(MessageSize <= MaxMessageSize) ||
		!ensure(NumFragments <= MaxFragmentsPerMessage) ||
		!ensure(FragmentIndex < NumFragments) ||
		!ensure(FragmentOffset + FragmentLength <= MessageSize))
	{
		LOG_VOXEL_NET_TRANSPORT(Warning, "FPeer::HandleSendFragment: Invalid fragment parameters (MessageSize=%d/%d, NumFragments=%d/%d, FragmentIndex=%d, FragmentOffset=%d, FragmentLength=%d)",
			MessageSize,
			MaxMessageSize,
			NumFragments,
			MaxFragmentsPerMessage,
			FragmentIndex,
			FragmentOffset,
			FragmentLength);

		return;
	}

	// Create channel if needed
	const TSharedRef<FChannel> Channel = FindOrAddChannel(Command.Header.ChannelId);

	{
		const int32 UnwrappedSequenceNumber =
			Channel->Incoming.SequenceNumberHead <= SequenceNumber
			? SequenceNumber
			: SequenceNumber + (1 << 16);

		const int32 Window = UnwrappedSequenceNumber / FWindow::Size;
		const int32 ActiveWindow = Channel->Incoming.SequenceNumberHead / FWindow::Size;

		if (!ensureVoxelSlow(Window >= ActiveWindow) ||
			!ensureVoxelSlow(Window < ActiveWindow + FWindow::FreeWindows - 1))
		{
			LOG_VOXEL_NET_TRANSPORT(Warning, "FPeer::HandleSendFragment: Fragment outside valid window range (window %d, active %d)",
				Window,
				ActiveWindow);

			return;
		}
	}

	FIncomingCommand* StartCommand = Channel->Incoming.FindCommand(SequenceNumber);
	if (StartCommand)
	{
		if (!ensure(StartCommand->Command.Header.Command == ECommand::SendFragment) ||
			!ensure(StartCommand->NumFragments == NumFragments) ||
			!ensure(StartCommand->Data.Num() == MessageSize))
		{
			return;
		}
	}

	if (!StartCommand)
	{
		if (SequenceNumber == Channel->Incoming.SequenceNumberHead)
		{
			// Already received
			LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FPeer::HandleSendFragment: Duplicate message %d (already completed)",
				SequenceNumber);

			return;
		}

		TUniquePtr<FIncomingCommand> IncomingCommand = MakeUnique<FIncomingCommand>();
		IncomingCommand->Command = Command;
		IncomingCommand->Command.Header.SequenceNumber = SequenceNumber;
		IncomingCommand->NumFragments = NumFragments;
		IncomingCommand->NumFragmentsReceived = 0;
		IncomingCommand->ReceivedFragments.SetNum(NumFragments, false);
		FVoxelUtilities::SetNumZeroed(IncomingCommand->Data, MessageSize);

		StartCommand = Channel->Incoming.InsertCommand(MoveTemp(IncomingCommand));

		if (!ensure(StartCommand))
		{
			// Logic is broken (the FindCommand above should have succeeded then)
			return;
		}
	}

	if (StartCommand->ReceivedFragments[FragmentIndex])
	{
		// Already received
		LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FPeer::HandleSendFragment: Duplicate fragment %d (already received)",
			FragmentIndex);

		return;
	}

	StartCommand->NumFragmentsReceived++;
	StartCommand->ReceivedFragments[FragmentIndex] = true;

	checkVoxelSlow(StartCommand->NumFragmentsReceived == StartCommand->ReceivedFragments.CountSetBits());

	FVoxelUtilities::Memcpy(
		StartCommand->Data.View().Slice(FragmentOffset, FragmentLength),
		PacketData);

	if (StartCommand->NumFragmentsReceived == StartCommand->NumFragments)
	{
		LOG_VOXEL_NET_TRANSPORT(Verbose, "FPeer::HandleSendFragment: Message complete (%d/%d fragments), dispatching",
			StartCommand->NumFragmentsReceived,
			StartCommand->NumFragments);

		Channel->Incoming.DispatchCommands(OnMessageReceived);
	}
	else
	{
		LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FPeer::HandleSendFragment: Received fragment %d/%d (total %d/%d)",
			FragmentIndex + 1,
			NumFragments,
			StartCommand->NumFragmentsReceived,
			StartCommand->NumFragments);
	}
}

}