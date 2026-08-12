// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Buffer/VoxelDoubleBuffers.h"

FString FVoxelDoubleVector2D::ToString() const
{
	return FString::Printf(TEXT("X=%3.3f Y=%3.3f"), X, Y);
}

FString FVoxelDoubleVector::ToString() const
{
	return FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f"), X, Y, Z);
}

FString FVoxelDoubleLinearColor::ToString() const
{
	return FString::Printf(TEXT("(R=%f,G=%f,B=%f,A=%f)"), R, G, B, A);
}