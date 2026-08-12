// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

VOXEL_API DECLARE_LOG_CATEGORY_EXTERN(LogVoxelNet, Log, All);
VOXEL_API DECLARE_LOG_CATEGORY_EXTERN(LogVoxelNetTransport, Fatal, All);

#define LOG_VOXEL_NET(Verbosity, Format, ...) UE_LOG(LogVoxelNet, Verbosity, TEXT(Format), ##__VA_ARGS__)
#define LOG_VOXEL_NET_TRANSPORT(Verbosity, Format, ...) UE_LOG(LogVoxelNetTransport, Verbosity, TEXT("%s: " Format), *GetName(), ##__VA_ARGS__)