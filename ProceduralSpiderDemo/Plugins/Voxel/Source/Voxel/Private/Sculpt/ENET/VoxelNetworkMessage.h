// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/ENET/VoxelChannel.h"
#include "VoxelNetworkMessage.generated.h"

struct FVoxelNetworkMessageHeader
{
	uint8 Index : 6;
	uint8 bHasHash : 1;
	uint8 bHasStructName : 1;
};

USTRUCT()
struct FVoxelNetworkMessage : public FVoxelVirtualStruct
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	virtual void Serialize(FArchive& Ar) VOXEL_PURE_VIRTUAL();

public:
	static TSharedPtr<FVoxelNetworkMessage> FromBytes(
		TConstVoxelArrayView<uint8> Bytes,
		TVoxelMap<int32, const UScriptStruct*>& IndexToStruct);

	TVoxelArray<uint8> ToBytes(TVoxelMap<const UScriptStruct*, int32>& StructToIndex) const;
};

USTRUCT()
struct FVoxelNetworkClientToServerMessage : public FVoxelNetworkMessage
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()
};

USTRUCT()
struct FVoxelNetworkServerToClientMessage : public FVoxelNetworkMessage
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()
};