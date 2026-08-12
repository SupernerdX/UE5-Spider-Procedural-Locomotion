// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

#define Type uint8
#define TypeToFloat(Value) (Value / (float)MAX_uint8)
#include "VoxelTextureFunctionLibraryImpl.isph"
#undef Type
#undef TypeToFloat

#define Type uint16
#define TypeToFloat(Value) (Value / (float)MAX_uint16)
#include "VoxelTextureFunctionLibraryImpl.isph"
#undef Type
#undef TypeToFloat

#define Type float
#define TypeToFloat(Value) Value
#include "VoxelTextureFunctionLibraryImpl.isph"
#undef Type
#undef TypeToFloat