// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNetworkHost.h"
#include "VoxelNetworkPacket.h"
#include "VoxelNetworkCommands.h"
#include "VoxelNetworkLog.h"

namespace Voxel::Network
{

FHost::FHost(const FString& Name)
	: Name(Name)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FHost::Tick(const TConstVoxelArrayView<FReceivedPackets> AllReceivedPackets)
{
	VOXEL_FUNCTION_COUNTER();

	Context->Time = FTime::Now();

	UpdateBandwidthThrottle();
	SendOutgoingCommands(true);

	for (const FReceivedPackets& ReceivedPackets : AllReceivedPackets)
	{
		HandlePackets(ReceivedPackets);
	}

	SendOutgoingCommands(true);
}

TSharedRef<FPeer> FHost::ConnectToPeer(
	const FString& PeerName,
	const FSendPacket& SendPacket,
	const FOnMessageReceived& OnMessageReceived)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(Context->IncomingBandwidthLimit > 0);
	ensure(Context->OutgoingBandwidthLimit > 0);

	const int32 PeerIndex = Peers.Emplace(SharedRef_Null);

	const TSharedRef<FPeer> Peer = MakeShared<FPeer>(
		FString::Printf(TEXT("%s %s [Peer %d]"), *Name, *PeerName, PeerIndex),
		PeerIndex,
		SendPacket,
		OnMessageReceived,
		Context);

	Peers[PeerIndex] = Peer;

	LOG_VOXEL_NET_TRANSPORT(Log, "FHost::ConnectToPeer: Created %s with bandwidth limits In=%d Out=%d",
		*Peer->GetName(),
		Context->IncomingBandwidthLimit,
		Context->OutgoingBandwidthLimit);

	TCommandData<ECommand::BandwidthLimit> Command;
	Command.IncomingBandwidth = Context->IncomingBandwidthLimit;
	Command.OutgoingBandwidth = Context->OutgoingBandwidthLimit;
	Peer->QueueSystemCommand(Command);

	return Peer;
}

void FHost::RemovePeer(const TSharedRef<FPeer>& Peer)
{
	if (!ensure(Peers.IsValidIndex(Peer->PeerIndex)) ||
		!ensure(Peers[Peer->PeerIndex] == Peer))
	{
		return;
	}

	LOG_VOXEL_NET_TRANSPORT(Log, "FHost::RemovePeer: Removing %s", *Peer->GetName());

	// Make sure to flush first
	Flush();

	Peers.RemoveAt(Peer->PeerIndex);

	bRecalculateBandwidthLimits = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FHost::Flush()
{
	VOXEL_FUNCTION_COUNTER();

	Context->Time = FTime::Now();
	SendOutgoingCommands(false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FHost::UpdateBandwidthThrottle()
{
	ensure(Context->IncomingBandwidthLimit > 0);
	ensure(Context->OutgoingBandwidthLimit > 0);

	const FTimespan ElapsedTime = Context->Time - LastBandwidthThrottleTime;

	// Throttle every 1000ms OR every 1000 packets, whichever comes first
	// This ensures responsive throttling even at high packet rates
	if (ElapsedTime < FTimespan::FromMilliseconds(1000) &&
		PacketsSinceLastThrottle < 1000)
	{
		return;
	}

	LastBandwidthThrottleTime = Context->Time;
	PacketsSinceLastThrottle = 0;

	{
		// Calculate total bytes sent and allowed in this period
		double TotalBytesAllowed = Context->OutgoingBandwidthLimit * ElapsedTime.Seconds();
		ensure(TotalBytesAllowed > 0);

		int32 TotalBytesSent = 0;
		for (const TSharedRef<FPeer>& Peer : Peers)
		{
			TotalBytesSent += Peer->BytesSentToPeer;
		}

		TVoxelInlineArray<TSharedRef<FPeer>, 32> PeersToProcess;
		for (const TSharedRef<FPeer>& Peer : Peers)
		{
			if (Peer->bInitialBandwidthLimitReceived)
			{
				PeersToProcess.Add(Peer);
			}
		}

		// Iteratively handle peers that exceed their individual bandwidth limits
		bool bNeedsAdjustment = true;
		while (PeersToProcess.Num() > 0 && bNeedsAdjustment)
		{
			bNeedsAdjustment = false;

			// Calculate global throttle ratio (0.0 to 1.0)
			double GlobalThrottleRatio;
			if (TotalBytesSent <= TotalBytesAllowed)
			{
				GlobalThrottleRatio = 1;
			}
			else
			{
				GlobalThrottleRatio = TotalBytesAllowed / TotalBytesSent;
			}

			for (auto It = PeersToProcess.CreateIterator(); It; ++It)
			{
				const TSharedRef<FPeer> Peer = *It;
				ensure(Peer->IncomingBandwidthLimit > 0);

				const double PeerBytesAllowed = Peer->IncomingBandwidthLimit * ElapsedTime.Seconds();
				const double PeerThrottledBytes = GlobalThrottleRatio * Peer->BytesSentToPeer;

				// Skip if peer's throttled amount is within their bandwidth limit
				if (PeerThrottledBytes <= PeerBytesAllowed)
				{
					continue;
				}

				// Peer exceeded their bandwidth - calculate individual throttle limit

				Peer->DropRate.SetMin(1 - PeerBytesAllowed / Peer->BytesSentToPeer);

				bNeedsAdjustment = true;
				TotalBytesAllowed -= PeerBytesAllowed;
				TotalBytesSent -= Peer->BytesSentToPeer;

				It.RemoveCurrent();
			}
		}

		// Apply global throttle to remaining peers
		double DropRate;
		if (TotalBytesSent <= TotalBytesAllowed)
		{
			DropRate = 0;
		}
		else
		{
			DropRate = 1 - TotalBytesAllowed / TotalBytesSent;
		}

		for (const TSharedRef<FPeer>& Peer : PeersToProcess)
		{
			Peer->DropRate.SetMin(DropRate);
		}
	}

	// Reset stats
	for (const TSharedRef<FPeer>& Peer : Peers)
	{
		Peer->BytesSentToPeer = 0;
	}

	if (bRecalculateBandwidthLimits ||
		LastIncomingBandwidthLimit != Context->IncomingBandwidthLimit ||
		LastOutgoingBandwidthLimit != Context->OutgoingBandwidthLimit)
	{
		bRecalculateBandwidthLimits = false;

		LastIncomingBandwidthLimit = Context->IncomingBandwidthLimit;
		LastOutgoingBandwidthLimit = Context->OutgoingBandwidthLimit;

		TVoxelInlineArray<TSharedRef<FPeer>, 32> PeersToProcess;
		for (const TSharedRef<FPeer>& Peer : Peers)
		{
			if (Peer->bInitialBandwidthLimitReceived)
			{
				PeersToProcess.Add(Peer);
			}
		}

		int32 BandwidthLimitPerPeer = 0;
		{
			bool bNeedsAdjustment = true;
			int32 GlobalBandwidthLimit = Context->IncomingBandwidthLimit;

			while (PeersToProcess.Num() > 0 && bNeedsAdjustment)
			{
				bNeedsAdjustment = false;
				BandwidthLimitPerPeer = GlobalBandwidthLimit / PeersToProcess.Num();

				for (auto It = PeersToProcess.CreateIterator(); It; ++It)
				{
					const TSharedRef<FPeer>& Peer = *It;
					ensure(Peer->OutgoingBandwidthLimit > 0);

					if (Peer->OutgoingBandwidthLimit >= BandwidthLimitPerPeer)
					{
						continue;
					}

					bNeedsAdjustment = true;
					GlobalBandwidthLimit -= Peer->OutgoingBandwidthLimit;

					It.RemoveCurrent();
				}
			}
		}

		for (const TSharedRef<FPeer>& Peer : Peers)
		{
			if (!Peer->bInitialBandwidthLimitReceived)
			{
				continue;
			}

			TCommandData<ECommand::BandwidthLimit> Command;
			Command.OutgoingBandwidth = Context->OutgoingBandwidthLimit;

			if (PeersToProcess.Contains(Peer))
			{
				Command.IncomingBandwidth = BandwidthLimitPerPeer;
			}
			else
			{
				Command.IncomingBandwidth = Peer->OutgoingBandwidthLimit;
			}

			Peer->QueueSystemCommand(Command);
		}
	}
}

void FHost::SendOutgoingCommands(const bool bCheckForTimeouts)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelInlineArray<TSharedRef<FPeer>, 32> PeersToProcess;
	for (const TSharedRef<FPeer>& Peer : Peers)
	{
		PeersToProcess.Add(Peer);
	}

	while (PeersToProcess.Num() > 0)
	{
		for (auto It = PeersToProcess.CreateIterator(); It; ++It)
		{
			const TSharedRef<FPeer>& Peer = *It;

			FPacketWriter PacketWriter(Context->MTU);

			ON_SCOPE_EXIT
			{
				if (!PacketWriter.bOverflow)
				{
					It.RemoveCurrentSwap();
				}
			};

			if (!ensure(Context->MTU > sizeof(FPacketHeader) + sizeof(FCommand)))
			{
				return;
			}

			// Write header
			{
				FPacketHeader Header;
				Header.SentTime = FPackedTime::Pack(Context->Time);
				PacketWriter.Write(Header);
			}

			while (!Peer->Acknowledgements.IsEmpty())
			{
				if (!PacketWriter.CanWrite<TCommand<ECommand::Acknowledge>>())
				{
					PacketWriter.bOverflow = true;
					break;
				}

				const TUniquePtr<FAcknowledgement> Acknowledgement = Peer->Acknowledgements.PopFront();

				TCommand<ECommand::Acknowledge> Acknowledge;
				Acknowledge.Header.ChannelId = Acknowledgement->Command.Header.ChannelId;
				Acknowledge.Header.SequenceNumber = Acknowledgement->Command.Header.SequenceNumber;
				Acknowledge.ReceivedSentTime = Acknowledgement->SentTime;
				PacketWriter.Write(Acknowledge);
			}

			if (bCheckForTimeouts)
			{
				Peer->SentCommands.CheckTimeouts(Context->Time, [&](TUniquePtr<FOutgoingCommand> Command)
				{
					Peer->CommandsToSend.Add(MoveTemp(Command));
				});
			}

			Peer->WriteOutgoingCommands(PacketWriter);

			if (Peer->SentCommands.IsEmpty() &&
				Context->bAllowPings &&
				Context->Time - Peer->LastReceiveTime >= FTimespan::FromMilliseconds(500) &&
				PacketWriter.CanWrite<TCommand<ECommand::Ping>>())
			{
				TCommandData<ECommand::Ping> Command;
				Peer->QueueSystemCommand(Command);

				Peer->WriteOutgoingCommands(PacketWriter);
			}

			if (PacketWriter.Bytes.Num() == sizeof(FPacketHeader))
			{
				// Nothing written
				continue;
			}

			Peer->LastSendTime = Context->Time;

			Peer->SendPacket(PacketWriter.Bytes);
			PacketsSinceLastThrottle++;
		}
	}
}

void FHost::HandlePackets(const FReceivedPackets& ReceivedPackets)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FPeer> Peer = ReceivedPackets.Peer;

	if (!ensure(Peers.IsValidIndex(Peer->PeerIndex)) ||
		!ensure(Peers[Peer->PeerIndex] == Peer))
	{
		return;
	}

	for (const TVoxelArray<uint8>& PacketData : ReceivedPackets.Packets)
	{
		HandlePacket(Peer, PacketData);
	}
}

void FHost::HandlePacket(
	const TSharedRef<FPeer>& Peer,
	const TConstVoxelArrayView<uint8> PacketData)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(PacketData.Num() >= sizeof(FPacketHeader)))
	{
		return;
	}
	const FPacketHeader PacketHeader = PacketData.LeftOf(sizeof(FPacketHeader)).ReinterpretAsSingle<FPacketHeader>();

	TConstVoxelArrayView<uint8> Data = PacketData.RightOf(sizeof(FPacketHeader));
	ensure(Data.Num() > 0);

	ON_SCOPE_EXIT
	{
		ensure(Data.Num() == 0);
	};

	while (Data.Num() > 0)
	{
		if (!ensure(Data.Num() >= sizeof(FCommandHeader)))
		{
			return;
		}

		FCommand Command;
		{
			const FCommandHeader Header = Data.LeftOf(sizeof(FCommandHeader)).ReinterpretAsSingle<FCommandHeader>();

			const int32 CommandSize = GetCommandSizeSafe(Header.Command);
			if (!ensure(CommandSize > 0))
			{
				return;
			}

			FVoxelUtilities::Memcpy(
				MakeByteVoxelArrayView(Command).LeftOf(CommandSize),
				Data.LeftOf(CommandSize));

			Data = Data.RightOf(CommandSize);
		}

		switch (Command.Header.Command)
		{
		default:
		{
			ensure(false);
			return;
		}
		case ECommand::Acknowledge:
		{
			LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FHost::HandlePacket: Received Acknowledge for channel %d seq %d",
				Command.Header.ChannelId,
				Command.Header.SequenceNumber);

			Peer->HandleAcknowledge(Command.Get<ECommand::Acknowledge>());
		}
		break;
		case ECommand::Ping:
		{
			LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FHost::HandlePacket: Received Ping");
		}
		break;
		case ECommand::SendFragment:
		{
			const FSendFragment Fragment = Command.Get<ECommand::SendFragment>().Get();

			LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FHost::HandlePacket: Received SendFragment channel %d seq %d fragment %d/%d",
				Command.Header.ChannelId,
				Fragment.StartSequenceNumber,
				Fragment.FragmentIndex + 1,
				Fragment.NumFragments);

			Peer->HandleSendFragment(Command.Get<ECommand::SendFragment>(), Data);
		}
		break;
		case ECommand::BandwidthLimit:
		{
			Peer->IncomingBandwidthLimit = Command.Get<ECommand::BandwidthLimit>().IncomingBandwidth;
			Peer->OutgoingBandwidthLimit = Command.Get<ECommand::BandwidthLimit>().OutgoingBandwidth;

			LOG_VOXEL_NET_TRANSPORT(Log, "FHost::HandlePacket: Received BandwidthLimit In=%d Out=%d",
				Peer->IncomingBandwidthLimit,
				Peer->OutgoingBandwidthLimit);

			if (!Peer->bInitialBandwidthLimitReceived)
			{
				Peer->bInitialBandwidthLimitReceived = true;
				bRecalculateBandwidthLimits = true;
			}
		}
		break;
		}

		if (Command.Header.Command != ECommand::Acknowledge)
		{
			Peer->QueueAcknowledgement(Command, PacketHeader.SentTime);
		}
	}
}

}