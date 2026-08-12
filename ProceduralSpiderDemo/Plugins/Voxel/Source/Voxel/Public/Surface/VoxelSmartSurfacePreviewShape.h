// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSmartSurfacePreviewShape.generated.h"

struct FVoxelStampRef;
class UVoxelSurfaceTypeInterface;

UENUM()
enum class EVoxelSmartSurfacePreviewShape : uint8
{
	// Sets the preview mesh to a sphere primitive.
	Sphere UMETA(Icon = "MaterialEditor.SetSpherePreview"),
	// Sets the preview mesh to a plane primitive.
	Plane UMETA(Icon = "MaterialEditor.SetPlanePreview"),
};

class VOXEL_API FVoxelSmartSurfacePreviewShapeUtilities
{
public:
	static FVoxelStampRef MakeStamp(
		EVoxelSmartSurfacePreviewShape Type,
		UVoxelSurfaceTypeInterface* SurfaceType);
};