// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelSculptHeightNetworkMessages.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"

void FVoxelSculptHeightServerToClientMessage_InitialLoad::Serialize(FArchive& Ar)
{
	Data.ShallowSerialize(Ar);
}

void FVoxelSculptHeightServerToClientMessage_SendBulkData::Serialize(FArchive& Ar)
{
	Ar << Hash;
	Ar << BulkData;
}

void FVoxelSculptHeightServerToClientMessage_ApplyModifier::Serialize(FArchive& Ar)
{
	Ar << ModifierGuid;
	Ar << ModifierData;
	Ar << HashBefore;
	Ar << HashAfter;
}

void FVoxelSculptHeightServerToClientMessage_SetSculptData::Serialize(FArchive& Ar)
{
	Data.ShallowSerialize(Ar);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptHeightClientToServerMessage_RequestBulkData::Serialize(FArchive& Ar)
{
	Ar << Hash;
	Hint.Serialize(Ar);
}

void FVoxelSculptHeightClientToServerMessage_ApplyModifier::Serialize(FArchive& Ar)
{
	Ar << ModifierGuid;
	Ar << ModifierData;
}