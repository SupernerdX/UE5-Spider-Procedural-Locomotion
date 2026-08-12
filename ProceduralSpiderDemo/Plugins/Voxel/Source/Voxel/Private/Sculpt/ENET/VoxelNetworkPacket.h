// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNetworkTypes.h"

namespace Voxel::Network
{

struct FPacketHeader
{
	FPackedTime SentTime;

	FPacketHeader()
	{
		FMemory::Memzero(*this);
	}
};
checkStatic(sizeof(FPacketHeader) == 2);

constexpr int32 MaxMTU = 1500;
// Max 6 windows, otherwise we get stuck
constexpr int32 MaxFragmentsPerMessage = 6 * 4096;
constexpr int32 MaxMessageSize = MaxFragmentsPerMessage * MaxMTU;

constexpr int32 MaxMTULog2 = FVoxelUtilities::CeilLog2<MaxMTU + 1>();
constexpr int32 MaxMessageSizeLog2 = FVoxelUtilities::CeilLog2<MaxMessageSize + 1>();
constexpr int32 MaxFragmentsPerMessageLog2 = FVoxelUtilities::CeilLog2<MaxFragmentsPerMessage + 1>();

struct FPacketWriter
{
	const int32 MTU;
	bool bOverflow = false;
	TVoxelFixedArray<uint8, MaxMTU> Bytes;

	explicit FPacketWriter(const int32 MTU)
		: MTU(MTU)
	{
		checkVoxelSlow(0 <= MTU && MTU <= MaxMTU);
	}

	FORCEINLINE bool CanWrite(const int32 Size) const
	{
		return Bytes.Num() + Size <= MTU;
	}
	FORCEINLINE void Write(const TConstVoxelArrayView<uint8> Data)
	{
		checkVoxelSlow(CanWrite(Data.Num()));
		Bytes.Append(Data);
	}

	template<typename T>
	requires std::is_trivially_destructible_v<T>
	FORCEINLINE bool CanWrite() const
	{
		return CanWrite(sizeof(T));
	}
	template<typename T>
	requires
	(
		std::is_trivially_destructible_v<T> &&
		!TIsTArrayView_V<T>
	)
	FORCEINLINE void Write(const T& Value)
	{
		this->Write(MakeByteVoxelArrayView(Value));
	}
};

}