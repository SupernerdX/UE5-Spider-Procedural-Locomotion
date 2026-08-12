// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelTool.h"
#include "VoxelHeightTool.generated.h"

struct FVoxelHeightModifier;

UCLASS(Abstract)
class VOXEL_API UVoxelHeightTool : public UVoxelTool
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<FVoxelHeightModifier> GetModifier(float StrengthMultiplier) const VOXEL_PURE_VIRTUAL({});
};