// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelFloatMetadata.h"

DEFINE_VOXEL_FACTORY(UVoxelFloatMetadata);

FVoxelPinValue UVoxelFloatMetadata::GetDefaultValue() const
{
	return FVoxelPinValue::Make(DefaultValue);
}

TVoxelOptional<EVoxelMetadataMaterialType> UVoxelFloatMetadata::GetMaterialType() const
{
	switch (GPUPacking)
	{
	default: ensure(false); return EVoxelMetadataMaterialType::Float32_1;
	case EVoxelFloatMetadataPacking::OneByte: return EVoxelMetadataMaterialType::Float8_1;
	case EVoxelFloatMetadataPacking::TwoBytes: return EVoxelMetadataMaterialType::Float16_1;
	case EVoxelFloatMetadataPacking::FourBytes: return EVoxelMetadataMaterialType::Float32_1;
	}
}