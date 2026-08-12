// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelTool.h"
#include "VoxelVolumeTool.generated.h"

class UVoxelMetadata;
struct FVoxelVolumeModifier;

UCLASS(Abstract)
class VOXEL_API UVoxelVolumeTool : public UVoxelTool
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<FVoxelVolumeModifier> GetModifier(float StrengthMultiplier) const VOXEL_PURE_VIRTUAL({});
};