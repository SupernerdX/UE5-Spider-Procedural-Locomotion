// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNormalMetadata.h"

DEFINE_VOXEL_FACTORY(UVoxelNormalMetadata);

FVoxelPinValue UVoxelNormalMetadata::GetDefaultValue() const
{
	return FVoxelPinValue::Make(DefaultValue);
}

TVoxelOptional<EVoxelMetadataMaterialType> UVoxelNormalMetadata::GetMaterialType() const
{
	return EVoxelMetadataMaterialType::Normal;
}