// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSmartSurfacePreviewShape.h"
#include "VoxelStampRef.h"
#include "Shape/VoxelPlaneShape.h"
#include "Shape/VoxelShapeStamp.h"
#include "Shape/VoxelSphereShape.h"

FVoxelStampRef FVoxelSmartSurfacePreviewShapeUtilities::MakeStamp(
	const EVoxelSmartSurfacePreviewShape Type,
	UVoxelSurfaceTypeInterface* SurfaceType)
{
	switch (Type)
	{
	default:
	{
		ensure(false);
		return {};
	}
	case EVoxelSmartSurfacePreviewShape::Sphere:
	{
		FVoxelSphereShape Shape;
		Shape.Radius = 100.f;

		FVoxelShapeStamp Stamp;
		Stamp.Shape = Shape;
		Stamp.SurfaceType = SurfaceType;
		return FVoxelStampRef::New(Stamp);
	}
	case EVoxelSmartSurfacePreviewShape::Plane:
	{
		FVoxelPlaneShape Shape;
		Shape.Size = FVector2D(20000.f);

		FVoxelShapeStamp Stamp;
		Stamp.Shape = Shape;
		Stamp.SurfaceType = SurfaceType;
		return FVoxelStampRef::New(Stamp);
	}
	}
}