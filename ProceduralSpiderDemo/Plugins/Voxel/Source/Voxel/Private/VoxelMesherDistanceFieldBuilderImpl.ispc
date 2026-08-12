// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

FORCEINLINE uint8 QuantizeDistance(
	const uniform float LocalToVolumeScale,
	const uniform float DistanceFieldToVolumeScaleBiasX,
	const uniform float DistanceFieldToVolumeScaleBiasY,
	const varying float Distance)
{
	// Transform to the tracing shader Volume space
	const float VolumeSpaceDistance = Distance * LocalToVolumeScale;
	// Transform to the Distance Field texture's space
	const float RescaledDistance = (VolumeSpaceDistance - DistanceFieldToVolumeScaleBiasY) / DistanceFieldToVolumeScaleBiasX;

	return clamp((int32)floor(RescaledDistance * 255.0f + .5f), 0, 255);
}

export void VoxelMesherDistanceFieldBuilder_ComputeVolumeDistanceFields(
	const uniform int32 BrickPositionAX,
	const uniform int32 BrickPositionAY,
	const uniform int32 BrickPositionAZ,
	const uniform int32 SizeX,
	const uniform int32 SizeY,
	const uniform int32 SizeZ,
	const uniform int32 BoundsMinX,
	const uniform int32 BoundsMinY,
	const uniform int32 BoundsMinZ,
	const uniform float VoxelSize,
	const uniform float TexelSize,
	const uniform int32 DistancesSize,
	const uniform float Distances[],
	const uniform float MeshDistanceFieldBias,
	const uniform float LocalToVolumeScale,
	const uniform float DistanceFieldToVolumeScaleBiasX,
	const uniform float DistanceFieldToVolumeScaleBiasY,
	const uniform int32 BrickSize,
	const uniform int32 UniqueDataBrickSize,
	uniform uint8 OutDistances[])
{
	const uniform int3 BrickPositionA = MakeInt3(BrickPositionAX, BrickPositionAY, BrickPositionAZ);

	for (uniform int32 BaseZ = 0; BaseZ < BrickSize; BaseZ++)
	{
		for (uniform int32 BaseY = 0; BaseY < BrickSize; BaseY++)
		{
			FOREACH(X, 0, BrickSize)
			{
				const varying int32 Index = X + BaseY * BrickSize + BaseZ * BrickSize * BrickSize;

				varying int3 BrickPositionB = MakeInt3(X, BaseY, BaseZ);
				varying int3 DistancePosition = BrickPositionA * UniqueDataBrickSize + BrickPositionB;
				varying float3 QueryPosition = MakeFloat3(DistancePosition.x - 1, DistancePosition.y - 1, DistancePosition.z - 1) * TexelSize;
				varying float3 MeshPosition = QueryPosition + 1;
				MeshPosition = clamp(MeshPosition, 0.0f, DistancesSize - 0.0001f);

				varying int32 MinX = (int32)floor(MeshPosition.x - BoundsMinX);
				varying int32 MinY = (int32)floor(MeshPosition.y - BoundsMinY);
				varying int32 MinZ = (int32)floor(MeshPosition.z - BoundsMinZ);
				varying int32 MaxX = MinX + 1;
				varying int32 MaxY = MinY + 1;
				varying int32 MaxZ = MinZ + 1;

				const varying bool bOutOfBounds =
					MinX >= SizeX |
					MinY >= SizeY | 
					MinZ >= SizeZ |
					MaxX >= SizeX |
					MaxY >= SizeY |
					MaxZ >= SizeZ |
					MinX < 0 |
					MinY < 0 | 
					MinZ < 0 |
					MaxX < 0 |
					MaxY < 0 |
					MaxZ < 0;
				if (bOutOfBounds)
				{
					continue;
				}

				const varying float AlphaX = MeshPosition.x - floor(MeshPosition.x);
				const varying float AlphaY = MeshPosition.y - floor(MeshPosition.y);
				const varying float AlphaZ = MeshPosition.z - floor(MeshPosition.z);

				const varying int32 BaseZ0 = MinZ * SizeX * SizeY;
				const varying int32 BaseZ1 = MaxZ * SizeX * SizeY;
				const varying int32 BaseY0 = MinY * SizeX;
				const varying int32 BaseY1 = MaxY * SizeX;
				
				const varying int32 Index000 = MinX + BaseY0 + BaseZ0;
				const varying int32 Index001 = MaxX + BaseY0 + BaseZ0;
				const varying int32 Index010 = MinX + BaseY1 + BaseZ0;
				const varying int32 Index011 = MaxX + BaseY1 + BaseZ0;
				const varying int32 Index100 = MinX + BaseY0 + BaseZ1;
				const varying int32 Index101 = MaxX + BaseY0 + BaseZ1;
				const varying int32 Index110 = MinX + BaseY1 + BaseZ1;
				const varying int32 Index111 = MaxX + BaseY1 + BaseZ1;

				IGNORE_PERF_WARNING
				const varying float Distance000 = Distances[Index000];
				IGNORE_PERF_WARNING
				const varying float Distance001 = Distances[Index001];
				IGNORE_PERF_WARNING
				const varying float Distance010 = Distances[Index010];
				IGNORE_PERF_WARNING
				const varying float Distance011 = Distances[Index011];
				IGNORE_PERF_WARNING
				const varying float Distance100 = Distances[Index100];
				IGNORE_PERF_WARNING
				const varying float Distance101 = Distances[Index101];
				IGNORE_PERF_WARNING
				const varying float Distance110 = Distances[Index110];
				IGNORE_PERF_WARNING
				const varying float Distance111 = Distances[Index111];

				const varying bool bHasNaN =
					isnan(Distance000) |
					isnan(Distance001) | 
					isnan(Distance010) |
					isnan(Distance011) |
					isnan(Distance100) |
					isnan(Distance101) | 
					isnan(Distance110) |
					isnan(Distance111);

				if (bHasNaN)
				{
					continue;
				}

				varying float Distance =
					TrilinearInterpolation(
						Distance000,
						Distance001,
						Distance010,
						Distance011,
						Distance100,
						Distance101,
						Distance110,
						Distance111,
						AlphaX,
						AlphaY,
						AlphaZ);

				Distance /= VoxelSize;
			 	Distance += MeshDistanceFieldBias;

				const varying uint8 NewDistance = QuantizeDistance(LocalToVolumeScale, DistanceFieldToVolumeScaleBiasX, DistanceFieldToVolumeScaleBiasY, Distance);
				OutDistances[Index] = min(OutDistances[Index], NewDistance);
			}
		}
	}
}

export void VoxelMesherDistanceFieldBuilder_ComputeHeightDistanceFields(
	const uniform int32 BrickPositionAX,
	const uniform int32 BrickPositionAY,
	const uniform int32 SizeX,
	const uniform int32 SizeY,
	const uniform int32 BoundsMinX,
	const uniform int32 BoundsMinY,
	const uniform float VoxelSize,
	const uniform float TexelSize,
	const uniform int32 DistancesSize,
	const uniform float Distances[],
	const uniform float MeshDistanceFieldBias,
	const uniform float LocalToVolumeScale,
	const uniform float DistanceFieldToVolumeScaleBiasX,
	const uniform float DistanceFieldToVolumeScaleBiasY,
	const uniform int32 NumBricksPerChunk,
	const uniform float StartZ,
	const uniform int32 BrickSize,
	const uniform int32 UniqueDataBrickSize,
	uniform uint8* uniform OutDistanceFields[])
{
	const uniform int2 BrickPositionA = MakeInt2(BrickPositionAX, BrickPositionAY);

	for (uniform int32 BaseY = 0; BaseY < BrickSize; BaseY++)
	{
		FOREACH(X, 0, BrickSize)
		{
			varying int2 BrickPositionB = MakeInt2(X, BaseY);
			varying int2 DistancePosition = BrickPositionA * UniqueDataBrickSize + BrickPositionB;
			varying float2 QueryPosition = MakeFloat2(DistancePosition.x - 1, DistancePosition.y - 1) * TexelSize;
			varying float2 MeshPosition = QueryPosition + 1;
			MeshPosition = clamp(MeshPosition, 0.0f, DistancesSize - 0.0001f);

			varying int32 MinX = (int32)floor(MeshPosition.x - BoundsMinX);
			varying int32 MinY = (int32)floor(MeshPosition.y - BoundsMinY);
			varying int32 MaxX = MinX + 1;
			varying int32 MaxY = MinY + 1;

			const varying bool bOutOfBounds =
				MinX >= SizeX |
				MinY >= SizeY | 
				MaxX >= SizeX |
				MaxY >= SizeY |
				MinX < 0 |
				MinY < 0 | 
				MaxX < 0 |
				MaxY < 0;
			if (bOutOfBounds)
			{
				continue;
			}

			const varying float AlphaX = MeshPosition.x - floor(MeshPosition.x);
			const varying float AlphaY = MeshPosition.y - floor(MeshPosition.y);

			const varying int32 BaseY0 = MinY * SizeX;
			const varying int32 BaseY1 = MaxY * SizeX;
			
			const varying int32 Index00 = MinX + BaseY0;
			const varying int32 Index01 = MaxX + BaseY0;
			const varying int32 Index10 = MinX + BaseY1;
			const varying int32 Index11 = MaxX + BaseY1;

			IGNORE_PERF_WARNING
			const varying float Height00 = Distances[Index00];
			IGNORE_PERF_WARNING
			const varying float Height01 = Distances[Index01];
			IGNORE_PERF_WARNING
			const varying float Height10 = Distances[Index10];
			IGNORE_PERF_WARNING
			const varying float Height11 = Distances[Index11];

			const varying bool bHasNaN =
				isnan(Height00) |
				isnan(Height01) | 
				isnan(Height10) |
				isnan(Height11);

			if (bHasNaN)
			{
				continue;
			}

			varying float Height =
				BilinearInterpolation(
					Height00,
					Height01,
					Height10,
					Height11,
					AlphaX,
					AlphaY);

			const varying float HeightGradientX = 
				BilinearInterpolation(
					Height01 - Height00,
					Height01 - Height00,
					Height11 - Height10,
					Height11 - Height10,
					AlphaX,
					AlphaY) / VoxelSize;

			const varying float HeightGradientY = 
				BilinearInterpolation(
					Height10 - Height00,
					Height11 - Height01,
					Height10 - Height00,
					Height11 - Height01,
					AlphaX,
					AlphaY) / VoxelSize;

			const varying float NormalInverseLength = rsqrt(1.0f + HeightGradientX * HeightGradientX + HeightGradientY * HeightGradientY);

			const varying int32 IndexXY = X + BaseY * BrickSize;

			for (uniform int32 BrickPositionAZ = 0; BrickPositionAZ < NumBricksPerChunk; BrickPositionAZ++)
			{
				for (int32 Z = 0; Z < BrickSize; Z++)
				{
					const int32 ZDistance = BrickPositionAZ * UniqueDataBrickSize + Z;
					const float ZPosition = (ZDistance - 1) * TexelSize;

					float ZMeshPosition = ZPosition + 1;
					ZMeshPosition = clamp(ZMeshPosition, 0.f, DistancesSize - 0.0001f);

					const varying float QueryZ = StartZ + (VoxelSize * ZMeshPosition);
					const varying float VerticalDistance = QueryZ - Height;

					varying float Distance = VerticalDistance * NormalInverseLength;

					Distance /= VoxelSize;
					Distance += MeshDistanceFieldBias;

					const varying int32 Index = IndexXY + Z * BrickSize * BrickSize;
					const varying uint8 NewDistance = QuantizeDistance(LocalToVolumeScale, DistanceFieldToVolumeScaleBiasX, DistanceFieldToVolumeScaleBiasY, Distance);
					OutDistanceFields[BrickPositionAZ][Index] = min(OutDistanceFields[BrickPositionAZ][Index], NewDistance);
				}
			}
		}
	}
}