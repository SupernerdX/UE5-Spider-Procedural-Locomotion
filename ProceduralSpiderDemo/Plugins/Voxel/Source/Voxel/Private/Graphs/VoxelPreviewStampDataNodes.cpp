// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelPreviewStampDataNodes.h"
#include "Graphs/VoxelStampGraphParameters.h"

void FVoxelPreviewDataNode_BaseStampPreviewData::Query_WithPosition(
	const FVoxelGraphQueryImpl& Query,
	FVoxelGraphQueryImpl& OutQuery) const
{
	OutQuery.AddParameter<FVoxelGraphParameters::FLOD>().Value = LODPin.GetSynchronous(Query);
	OutQuery.AddParameter<FVoxelGraphParameters::FStampSeed>(StampSeedPin.GetSynchronous(Query));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewDataNode_HeightPreviewData::Query_WithPosition(
	const FVoxelGraphQueryImpl& Query,
	FVoxelGraphQueryImpl& OutQuery) const
{
	Super::Query_WithPosition(Query, OutQuery);

	FVoxelGraphParameters::FHeightStamp& HeightStamp = OutQuery.AddParameter<FVoxelGraphParameters::FHeightStamp>();
	HeightStamp.BlendMode = BlendModePin.GetSynchronous(Query);
	HeightStamp.Smoothness = SmoothnessPin.GetSynchronous(Query);

	TVoxelMap<FVoxelMetadataRef, TPinRef_Input<FVoxelBuffer>> Metadata;
	for (const auto& Pin : MetadataPins)
	{
		Metadata.Add_EnsureNew(Pin.MetadataRef, Pin.PinRef);
	}
	OutQuery.AddParameter<FVoxelGraphParameters::FHeightStampPreviewData>(
		Query,
		HeightPin,
		SurfaceTypePin,
		MoveTemp(Metadata));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewDataNode_VolumePreviewData::Query_WithPosition(
	const FVoxelGraphQueryImpl& Query,
	FVoxelGraphQueryImpl& OutQuery) const
{
	Super::Query_WithPosition(Query, OutQuery);

	FVoxelGraphParameters::FVolumeStamp& VolumeStamp = OutQuery.AddParameter<FVoxelGraphParameters::FVolumeStamp>();
	VolumeStamp.BlendMode = BlendModePin.GetSynchronous(Query);
	VolumeStamp.Smoothness = SmoothnessPin.GetSynchronous(Query);

	TVoxelMap<FVoxelMetadataRef, TPinRef_Input<FVoxelBuffer>> Metadata;
	for (const auto& Pin : MetadataPins)
	{
		Metadata.Add_EnsureNew(Pin.MetadataRef, Pin.PinRef);
	}

	OutQuery.AddParameter<FVoxelGraphParameters::FVolumeStampPreviewData>(
		Query,
		DistancePin,
		SurfaceTypePin,
		MoveTemp(Metadata));
}