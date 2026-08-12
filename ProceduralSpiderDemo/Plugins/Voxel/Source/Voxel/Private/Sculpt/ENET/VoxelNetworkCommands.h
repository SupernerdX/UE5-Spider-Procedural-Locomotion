// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNetworkTypes.h"
#include "VoxelNetworkPacket.h"

namespace Voxel::Network
{

enum class ECommand : uint8
{
	SendFragment,
	Acknowledge,
	Ping,
	BandwidthLimit
};

struct FCommandHeader
{
	ECommand Command;
	uint8 ChannelId;
	uint16 SequenceNumber;

	FCommandHeader()
	{
		FMemory::Memzero(*this);
	}
};
checkStatic(sizeof(FCommandHeader) == 4);

template<ECommand Command>
struct TCommandData;

struct FSendFragment
{
	uint16 StartSequenceNumber = 0;
	int32 MessageSize = 0;
	int32 NumFragments = 0;
	int32 FragmentIndex = 0;
	int32 FragmentOffset = 0;
	int32 FragmentLength = 0;
};

template<>
struct TCommandData<ECommand::SendFragment>
{
public:
	struct FUnalignedPacked
	{
		uint8 Data[16];
	};
	FUnalignedPacked PackedData;

	TCommandData()
	{
		FMemory::Memzero(*this);
	}

	FORCEINLINE void Set(const FSendFragment Value)
	{
		FPacked Packed;
		Packed.StartSequenceNumber = Value.StartSequenceNumber;
		Packed.MessageSize = FVoxelUtilities::CheckBits<MaxMessageSizeLog2>(Value.MessageSize);
		Packed.NumFragments = FVoxelUtilities::CheckBits<MaxFragmentsPerMessageLog2>(Value.NumFragments);
		Packed.FragmentIndex = FVoxelUtilities::CheckBits<MaxFragmentsPerMessageLog2>(Value.FragmentIndex);
		Packed.FragmentOffset = FVoxelUtilities::CheckBits<MaxMessageSizeLog2>(Value.FragmentOffset);
		Packed.FragmentLength = FVoxelUtilities::CheckBits<MaxMTULog2>(Value.FragmentLength);
		PackedData = ReinterpretCastRef<FUnalignedPacked>(Packed);
	}
	FORCEINLINE FSendFragment Get() const
	{
		const FPacked Packed = ReinterpretCastRef_Unaligned<FPacked>(PackedData);

		return FSendFragment
		{
			uint16(Packed.StartSequenceNumber),
			int32(Packed.MessageSize),
			int32(Packed.NumFragments),
			int32(Packed.FragmentIndex),
			int32(Packed.FragmentOffset),
			int32(Packed.FragmentLength)
		};
	}

private:
	struct FPacked
	{
		uint64 StartSequenceNumber : 16;
		uint64 MessageSize : MaxMessageSizeLog2;
		uint64 NumFragments : MaxFragmentsPerMessageLog2;
		uint64 FragmentIndex : MaxFragmentsPerMessageLog2;
		uint64 FragmentOffset : MaxMessageSizeLog2;
		uint64 FragmentLength : MaxMTULog2;
	};
};
checkStatic(sizeof(TCommandData<ECommand::SendFragment>) == 16);

template<>
struct TCommandData<ECommand::Acknowledge>
{
	FPackedTime ReceivedSentTime;
};

template<>
struct TCommandData<ECommand::Ping>
{
};

template<>
struct TCommandData<ECommand::BandwidthLimit>
{
	uint32 IncomingBandwidth = 0;
	uint32 OutgoingBandwidth = 0;
};

FORCEINLINE int32 GetCommandSizeSafe(const ECommand Command)
{
	switch (Command)
	{
	default: return -1;

#define CASE(X) case ECommand::X: return sizeof(FCommandHeader) + sizeof(TCommandData<ECommand::X>);

	CASE(SendFragment);
	CASE(Acknowledge);
	CASE(Ping);
	CASE(BandwidthLimit);

#undef CASE
	}
}

struct FCommandBase
{
	FCommandHeader Header;
};

template<ECommand Command>
struct TCommand : FCommandBase, TCommandData<Command>
{
	TCommand()
	{
		FMemory::Memzero(*this);
		Header.Command = Command;
	}
};

struct FCommand
{
	FORCEINLINE FCommand()
	{
		FMemory::Memzero(Data);
	}
	template<ECommand Command>
	FORCEINLINE FCommand(const TCommand<Command>& Value)
	{
		FMemory::Memzero(Data);
		FMemory::Memcpy(this, &Value, sizeof(TCommand<Command>));

		checkVoxelSlow(Header.Command == Command);
	}

	template<ECommand Command>
	FORCEINLINE void SetData(const TCommandData<Command>& Value)
	{
		checkVoxelSlow(offsetof(FCommand, Data) == sizeof(FCommandHeader));

		Header.Command = Command;
		FMemory::Memcpy(&Data, &Value, sizeof(TCommandData<Command>));
	}

	FORCEINLINE TConstVoxelArrayView<uint8> View() const
	{
		const int32 Size = GetCommandSizeSafe(Header.Command);
		checkVoxelSlow(Size > 0);
		return MakeByteVoxelArrayView(*this).Slice(0, Size);
	}

	template<ECommand Command>
	FORCEINLINE TCommand<Command>& Get()
	{
		checkVoxelSlow(Header.Command == Command);
		return *reinterpret_cast<TCommand<Command>*>(this);
	}
	template<ECommand Command>
	FORCEINLINE const TCommand<Command>& Get() const
	{
		checkVoxelSlow(Header.Command == Command);
		return *reinterpret_cast<const TCommand<Command>*>(this);
	}

	FCommandHeader Header;

private:
	union FData
	{
		FData()
		{
		}

		TCommandData<ECommand::SendFragment> SendFragment;
		TCommandData<ECommand::Acknowledge> Acknowledge;
		TCommandData<ECommand::Ping> Ping;
		TCommandData<ECommand::BandwidthLimit> BandwidthLimit;
	};
	FData Data;
};
}