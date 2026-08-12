// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNetworkChannel.h"
#include "VoxelNetworkLog.h"

namespace Voxel::Network
{

FChannel::FChannel(
	const FString& Name,
	const uint8 ChannelId)
	: Name(Name)
	, ChannelId(ChannelId)
{
	ensure(ChannelId != InvalidChannelId);
}

FIncomingCommand* FChannel::FIncoming::FindCommand(const uint16 SequenceNumberToFind)
{
	VOXEL_FUNCTION_COUNTER();
	Check();

	// Sequence numbers are uint16 (0-65535) and wrap around when they exceed 65535.
	// SequenceNumberHead marks the last processed command.

	const int32 UnwrappedSequenceNumberToFind =
		SequenceNumberHead <= SequenceNumberToFind
		? SequenceNumberToFind
		: SequenceNumberToFind + (1 << 16);

	LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FChannel::FIncoming::FindCommand: Looking for sequence %d (unwrapped: %d), head: %d",
		SequenceNumberToFind,
		UnwrappedSequenceNumberToFind,
		SequenceNumberHead);

	// Go down from highest to lowest
	for (FIncomingCommand& Command : Commands.Reverse())
	{
		const int32 UnwrappedSequenceNumber = Command.GetUnwrappedSequenceNumber(SequenceNumberHead);

		if (UnwrappedSequenceNumber < UnwrappedSequenceNumberToFind)
		{
			// Too low
			LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FChannel::FIncoming::FindCommand: Not found (too low)");
			return nullptr;
		}

		if (UnwrappedSequenceNumber > UnwrappedSequenceNumberToFind)
		{
			// Too high, keep going down
			continue;
		}

		checkVoxelSlow(UnwrappedSequenceNumber == UnwrappedSequenceNumberToFind);
		LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FChannel::FIncoming::FindCommand: Found command");
		return &Command;
	}

	LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FChannel::FIncoming::FindCommand: Not found (empty list)");
	return nullptr;
}

FIncomingCommand* FChannel::FIncoming::InsertCommand(TUniquePtr<FIncomingCommand> CommandToInsert)
{
	VOXEL_FUNCTION_COUNTER();

	Check();
	ON_SCOPE_EXIT
	{
		Check();
	};

	const int32 UnwrappedSequenceNumberToInsert = CommandToInsert->GetUnwrappedSequenceNumber(SequenceNumberHead);

	LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FChannel::FIncoming::InsertCommand: Inserting sequence %d (unwrapped: %d), head: %d",
		CommandToInsert->GetSequenceNumber(),
		UnwrappedSequenceNumberToInsert,
		SequenceNumberHead);

	if (!ensureVoxelSlow(UnwrappedSequenceNumberToInsert != SequenceNumberHead))
	{
		// Duplicate
		LOG_VOXEL_NET_TRANSPORT(Warning, "FChannel::FIncoming::InsertCommand: Duplicate command at sequence head %d",
			SequenceNumberHead);

		return nullptr;
	}

	// Go down from highest to lowest
	for (FIncomingCommand& Command : Commands.Reverse())
	{
		const int32 UnwrappedSequenceNumber = Command.GetUnwrappedSequenceNumber(SequenceNumberHead);

		if (UnwrappedSequenceNumber < UnwrappedSequenceNumberToInsert)
		{
			// Too low, insert here
			return Command.List_InsertAfter(MoveTemp(CommandToInsert));
		}

		if (UnwrappedSequenceNumber > UnwrappedSequenceNumberToInsert)
		{
			// Too high, keep going down
			continue;
		}

		checkVoxelSlow(UnwrappedSequenceNumber == UnwrappedSequenceNumberToInsert);

		// Duplicate
		LOG_VOXEL_NET_TRANSPORT(Warning, "FChannel::FIncoming::InsertCommand: Duplicate command at sequence %d",
			CommandToInsert->GetSequenceNumber());

		ensure(false);
		return nullptr;
	}

	LOG_VOXEL_NET_TRANSPORT(VeryVerbose, "FChannel::FIncoming::InsertCommand: Inserted at front");
	return Commands.PushFront(MoveTemp(CommandToInsert));
}

void FChannel::FIncoming::Check()
{
	if (!VOXEL_DEBUG)
	{
		return;
	}

	VOXEL_FUNCTION_COUNTER();

	int32 SequenceNumber = -1;
	for (const FIncomingCommand& Command : Commands)
	{
		const int32 UnwrappedSequenceNumber = Command.GetUnwrappedSequenceNumber(SequenceNumberHead);

		check(SequenceNumber < UnwrappedSequenceNumber);
		SequenceNumber = UnwrappedSequenceNumber;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FChannel::FIncoming::DispatchCommands(const FOnMessageReceived OnMessageReceived)
{
	VOXEL_FUNCTION_COUNTER();

	Check();
	ON_SCOPE_EXIT
	{
		Check();
	};

	int32 NumDispatched = 0;
	for (auto It = Commands.CreateIterator(); It; ++It)
	{
		FIncomingCommand& Command = *It;

		if (Command.GetSequenceNumber() != uint16(SequenceNumberHead + 1) ||
			Command.NumFragmentsReceived != Command.NumFragments)
		{
			break;
		}

		if (Command.NumFragments > 0)
		{
			SequenceNumberHead = uint16(Command.GetSequenceNumber() + Command.NumFragments - 1);

			LOG_VOXEL_NET_TRANSPORT(Verbose, "FChannel::FIncoming::DispatchCommands: Dispatching fragmented message (seq %d, %d fragments, %d bytes)",
				Command.GetSequenceNumber(),
				Command.NumFragments,
				Command.Data.Num());
		}
		else
		{
			SequenceNumberHead = Command.GetSequenceNumber();

			LOG_VOXEL_NET_TRANSPORT(Verbose, "FChannel::FIncoming::DispatchCommands: Dispatching message (seq %d, %d bytes)",
				Command.GetSequenceNumber(),
				Command.Data.Num());
		}

		OnMessageReceived(
			Command.Command.Header.ChannelId,
			MoveTemp(Command.Data));

		It.RemoveCurrent();
		NumDispatched++;
	}

	if (NumDispatched > 0)
	{
		LOG_VOXEL_NET_TRANSPORT(Verbose, "FChannel::FIncoming::DispatchCommands: Dispatched %d messages, new head: %d",
			NumDispatched,
			SequenceNumberHead);
	}
}

}