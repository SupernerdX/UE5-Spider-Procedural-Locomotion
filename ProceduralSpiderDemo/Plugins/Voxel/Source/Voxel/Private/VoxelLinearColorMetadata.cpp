// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelLinearColorMetadata.h"

DEFINE_VOXEL_FACTORY(UVoxelLinearColorMetadata);

FVoxelPinValue UVoxelLinearColorMetadata::GetDefaultValue() const
{
	return FVoxelPinValue::Make(DefaultValue);
}

TVoxelOptional<EVoxelMetadataMaterialType> UVoxelLinearColorMetadata::GetMaterialType() const
{
	switch (GPUPacking)
	{
	default: ensure(false); return EVoxelMetadataMaterialType::Float32_4;
	case EVoxelFloatMetadataPacking::OneByte: return EVoxelMetadataMaterialType::Float8_4;
	case EVoxelFloatMetadataPacking::TwoBytes: return EVoxelMetadataMaterialType::Float16_4;
	case EVoxelFloatMetadataPacking::FourBytes: return EVoxelMetadataMaterialType::Float32_4;
	}
}