// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkHint.h"
#include "Bulk/VoxelBulkPtr.h"
#include "Sculpt/ENET/VoxelNetworkMessage.h"
#include "VoxelSculptHeightNetworkMessages.generated.h"

struct FVoxelSculptHeightData;

USTRUCT()
struct FVoxelSculptHeightServerToClientMessage_InitialLoad : public FVoxelNetworkServerToClientMessage
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	TVoxelBulkPtr<FVoxelSculptHeightData> Data;

	//~ Begin FVoxelNetworkMessage Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelNetworkMessage Interface
};

USTRUCT()
struct FVoxelSculptHeightServerToClientMessage_SendBulkData : public FVoxelNetworkServerToClientMessage
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	FVoxelBulkHash Hash;
	TVoxelArray<uint8> BulkData;

	//~ Begin FVoxelNetworkMessage Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelNetworkMessage Interface
};

USTRUCT()
struct FVoxelSculptHeightServerToClientMessage_ApplyModifier : public FVoxelNetworkServerToClientMessage
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	FGuid ModifierGuid;
	TVoxelArray<uint8> ModifierData;
	FVoxelBulkHash HashBefore;
	FVoxelBulkHash HashAfter;

	//~ Begin FVoxelNetworkMessage Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelNetworkMessage Interface
};

USTRUCT()
struct FVoxelSculptHeightServerToClientMessage_SetSculptData : public FVoxelNetworkServerToClientMessage
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	TVoxelBulkPtr<FVoxelSculptHeightData> Data;

	//~ Begin FVoxelNetworkMessage Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelNetworkMessage Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FVoxelSculptHeightClientToServerMessage_RequestBulkData : public FVoxelNetworkClientToServerMessage
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	FVoxelBulkHash Hash;
	FVoxelBulkHint Hint;

	//~ Begin FVoxelNetworkMessage Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelNetworkMessage Interface
};

USTRUCT()
struct FVoxelSculptHeightClientToServerMessage_ApplyModifier : public FVoxelNetworkClientToServerMessage
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	// TODO: Smart diffing: store a list of TArray<uint8> that we already replicated
	// Try to find the array with the same exact size with the least differences
	// Then send that hash + a diff (ranges + changed bytes)
	// Have a timeout of 30s on TArrays - keep them in memory for 30s, but stop using them after 15s
	// (but always keep at least one in memory per array size)

	FGuid ModifierGuid;
	TVoxelArray<uint8> ModifierData;

	//~ Begin FVoxelNetworkMessage Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelNetworkMessage Interface
};
