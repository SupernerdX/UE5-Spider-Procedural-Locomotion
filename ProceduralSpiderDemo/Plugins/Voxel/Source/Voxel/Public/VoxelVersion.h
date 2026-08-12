// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

using FVoxelVersion = DECLARE_VOXEL_VERSION
(
	FirstVersion,
	AddVoxelStaticMesh,
	FixVoxelEdGraphMigration,
	RemoveSculptStamps,
	AddExternalSculptSaves,
	AddVolumeSculptDistanceType,
	AddBulkDataToSaveAssets,
	ChangeBoundsExtension,
	AddResolveSmartSurfaceTypeToPCGQueryProjection,
	AddBulkDataToHeightSaveAssets
);

constexpr FVoxelGuid GVoxelCustomVersionGUID = VOXEL_GUID("897612303D0244CCBA7E5839DB2E7231");