// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNetworkTypes.h"
#include "VoxelNetworkCommands.h"

namespace Voxel::Network
{

constexpr uint8 InvalidChannelId = 0xFF;

// FreeWindows is critical to prevent unsafe wraparounds
// It ensures our total window space is at most 32k, meaning we can detect old packets
struct FWindow
{
	static constexpr int32 Size = 4096;
	static constexpr int32 NumWindows = (1 << 16) / Size;
	static constexpr int32 FreeWindows = NumWindows / 2;
};

struct FIncomingCommand : TNode<FIncomingCommand>
{
	FCommand Command;
	int32 NumFragments = 0;
	int32 NumFragmentsReceived = 0;
	FVoxelBitArray ReceivedFragments;
	TVoxelArray<uint8> Data;

	FORCEINLINE uint16 GetSequenceNumber() const
	{
		return Command.Header.SequenceNumber;
	}
	FORCEINLINE int32 GetUnwrappedSequenceNumber(const uint16 SequenceNumberHead) const
	{
		return
			SequenceNumberHead <= Command.Header.SequenceNumber
			? Command.Header.SequenceNumber
			: Command.Header.SequenceNumber + (1 << 16);
	}
};

class FChannel
{
public:
	const FString Name;
	const uint8 ChannelId;

	explicit FChannel(
		const FString& Name,
		uint8 ChannelId);

	const FString& GetName() const
	{
		return Name;
	}

public:
	struct FOutgoing
	{
		uint16 SequenceNumber = 0;
		TVoxelStaticArray<uint16, FWindow::NumWindows> Windows{ ForceInit };
	};
	FOutgoing Outgoing;

	struct FIncoming
	{
	public:
		const FChannel& Channel;
		uint16 SequenceNumberHead = 0;

		explicit FIncoming(const FChannel& Channel)
			: Channel(Channel)
		{
		}

	public:
		const FString& GetName() const
		{
			return Channel.GetName();
		}

		FIncomingCommand* FindCommand(uint16 SequenceNumberToFind);
		FIncomingCommand* InsertCommand(TUniquePtr<FIncomingCommand> CommandToInsert);

		void Check();
		void DispatchCommands(FOnMessageReceived OnMessageReceived);

	private:
		// Sorted by SequenceNumber
		TList<FIncomingCommand> Commands;
	};
	FIncoming Incoming{ *this };
};

}