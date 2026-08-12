// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelObjectPinType.h"
#include "VoxelFunctionLibrary.h"
#include "VoxelHeightmapUtilities.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Heightmap/VoxelHeightmap.h"
#include "Surface/VoxelSurfaceType.h"
#include "VoxelHeightmapFunctionLibrary.generated.h"

struct FVoxelHeightmap_HeightData;

USTRUCT()
struct VOXEL_API FVoxelHeightmapRef
{
	GENERATED_BODY()

	TVoxelObjectPtr<UVoxelHeightmap> Object;
	float ScaleXY = 0;
	FVoxelSurfaceType DefaultSurfaceType;
	float ScaleZ = 0;
	float OffsetZ = 0;
	TSharedPtr<const FVoxelHeightmap_HeightData> HeightData;
	TVoxelArray<FVoxelHeightmapUtilities::FWeightmap> Weightmaps;
};

DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelHeightmapRef);

USTRUCT()
struct VOXEL_API FVoxelHeightmapRefPinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelHeightmapRef, UVoxelHeightmap);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelHeightmapFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Heightmap")
	FIntPoint GetHeightmapSize(const FVoxelHeightmapRef& Heightmap) const;
};