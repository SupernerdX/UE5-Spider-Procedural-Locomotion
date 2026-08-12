// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "StaticMesh/VoxelStaticMeshPoint.h"
#include "StaticMesh/VoxelStaticMeshSettings.h"

struct VOXEL_API FVoxelMeshVoxelizer
{
public:
	static void Voxelize(
		const FVoxelStaticMeshSettings& Settings,
		const TVoxelArray<FVector3f>& Vertices,
		const TVoxelArray<int32>& Indices,
		const TVoxelArray<FTriangleID>& TriangleIds,
		const FVector3f& Origin,
		const FIntVector& Size,
		TVoxelArray<float>& Distances,
		TVoxelArray<float>& ClosestX,
		TVoxelArray<float>& ClosestY,
		TVoxelArray<float>& ClosestZ,
		TVoxelMap<FVector3f, FVoxelStaticMeshPoint>& ClosestToPoint,
		int32& NumLeaks);

private:
	// Calculate twice signed area of triangle (0,0)-(A.X,A.Y)-(B.X,B.Y)
	// Return an SOS-determined sign (-1, +1, or 0 only if it's a truly degenerate triangle)
	static int32 Orientation(
		const FVector2d& A,
		const FVector2d& B,
		double& TwiceSignedArea);

	// Robust test of (x0,y0) in the triangle (x1,y1)-(x2,y2)-(x3,y3)
	// If true is returned, the barycentric coordinates are set in a,b,c.
	static bool PointInTriangle2D(
		const FVector2d& Point,
		FVector2d A,
		FVector2d B,
		FVector2d C,
		double& AlphaA, double& AlphaB, double& AlphaC);
};