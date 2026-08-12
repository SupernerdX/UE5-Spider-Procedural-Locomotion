// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkPtr.h"
#include "Sculpt/ENET/VoxelNetworkMessage.h"
#include "VoxelSculptVolumeNetworkMessages.generated.h"

struct FVoxelSculptVolumeData;

USTRUCT()
struct FVoxelSculptVolumeServerToClientMessage_InitialLoad : public FVoxelNetworkServerToClientMessage
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	TVoxelBulkPtr<FVoxelSculptVolumeData> Data;

	//~ Begin FVoxelNetworkMessage Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelNetworkMessage Interface
};

USTRUCT()
struct FVoxelSculptVolumeServerToClientMessage_SendBulkData : public FVoxelNetworkServerToClientMessage
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
struct FVoxelSculptVolumeServerToClientMessage_ApplyModifier : public FVoxelNetworkServerToClientMessage
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
struct FVoxelSculptVolumeServerToClientMessage_SetSculptData : public FVoxelNetworkServerToClientMessage
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	TVoxelBulkPtr<FVoxelSculptVolumeData> Data;

	//~ Begin FVoxelNetworkMessage Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelNetworkMessage Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FVoxelSculptVolumeClientToServerMessage_RequestBulkData : public FVoxelNetworkClientToServerMessage
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
struct FVoxelSculptVolumeClientToServerMessage_ApplyModifier : public FVoxelNetworkClientToServerMessage
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