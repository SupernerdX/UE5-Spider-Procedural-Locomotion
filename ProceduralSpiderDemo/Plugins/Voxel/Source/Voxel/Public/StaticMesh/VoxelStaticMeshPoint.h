// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "MeshTypes.h"

struct alignas(8) VOXEL_API FVoxelStaticMeshPoint
{
	FTriangleID TriangleId;

	union
	{
		struct
		{
			uint32 BarycentricX : 10;
			uint32 BarycentricY : 10;
			uint32 BarycentricZ : 10;
		};

		uint32 Raw;
	};

	FORCEINLINE FVoxelStaticMeshPoint()
	{
		TriangleId = -1;
		Raw = 0;
	}

	FORCEINLINE FVoxelStaticMeshPoint(
		const FTriangleID TriangleId,
		const FVector3f& Barycentrics)
		: TriangleId(TriangleId)
	{
		checkVoxelSlow(FMath::IsNearlyEqual(Barycentrics.X + Barycentrics.Y + Barycentrics.Z, 1.f, 0.01f));

		const FIntVector IntBarycentrics = FVoxelUtilities::Clamp(
			FVoxelUtilities::FloorToInt(Barycentrics * 1023.999f),
			0,
			1023);

		BarycentricX = IntBarycentrics.X;
		BarycentricY = IntBarycentrics.Y;
		BarycentricZ = IntBarycentrics.Z;
	}

	FORCEINLINE FVector3f GetBarycentric() const
	{
		return FVector3f(
			BarycentricX / 1023.f,
			BarycentricY / 1023.f,
			BarycentricZ / 1023.f);
	}

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FVoxelStaticMeshPoint& Point)
	{
		return Ar << ReinterpretCastRef<uint64>(Point);
	}
};