// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelSculptVolumeNetworkMessages.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"

void FVoxelSculptVolumeServerToClientMessage_InitialLoad::Serialize(FArchive& Ar)
{
	Data.ShallowSerialize(Ar);
}

void FVoxelSculptVolumeServerToClientMessage_SendBulkData::Serialize(FArchive& Ar)
{
	Ar << Hash;
	Ar << BulkData;
}

void FVoxelSculptVolumeServerToClientMessage_ApplyModifier::Serialize(FArchive& Ar)
{
	Ar << ModifierGuid;
	Ar << ModifierData;
	Ar << HashBefore;
	Ar << HashAfter;
}

void FVoxelSculptVolumeServerToClientMessage_SetSculptData::Serialize(FArchive& Ar)
{
	Data.ShallowSerialize(Ar);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptVolumeClientToServerMessage_RequestBulkData::Serialize(FArchive& Ar)
{
	Ar << Hash;
	Hint.Serialize(Ar);
}

void FVoxelSculptVolumeClientToServerMessage_ApplyModifier::Serialize(FArchive& Ar)
{
	Ar << ModifierGuid;
	Ar << ModifierData;
}