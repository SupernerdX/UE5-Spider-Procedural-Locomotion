// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"
#include "VoxelSampleMesh.isph"

export void VoxelNode_SampleVoxelStaticMesh(
	const uniform float PositionX[],
	const uniform float PositionY[],
	const uniform float PositionZ[],
	uniform float Result[],
	const uniform int32 Num,
	const uniform float Scale,
	const uniform bool bUseTricubic,
	const uniform int3& Size,
	const uniform float VoxelSize,
	const uniform float3& MeshOrigin,
	const uniform float MeshDistances[])
{
	const uniform float MaxPositionX = Size.x - 2.0001f;
	const uniform float MaxPositionY = Size.y - 2.0001f;
	const uniform float MaxPositionZ = Size.z - 2.0001f;

	const uniform float VoxelSizeAndScale = VoxelSize * Scale;
	const uniform float InvVoxelSizeAndScale = 1.f / VoxelSizeAndScale;

	const uniform int32 SizeX = Size.x;
	const uniform int32 SizeXY = Size.x * Size.y;
	const uniform int32 SizeXYZ = Size.x * Size.y * Size.z;

	FOREACH(Index, 0, Num)
	{
		const varying float3 Position = MakeFloat3(PositionX[Index], PositionY[Index], PositionZ[Index]) * InvVoxelSizeAndScale - MeshOrigin;

		const varying float Distance = SampleMeshDistances(
			Position.x,
			Position.y,
			Position.z,
			MaxPositionX,
			MaxPositionY,
			MaxPositionZ,
			SizeX,
			SizeXY,
			SizeXYZ,
			Size,
			bUseTricubic,
			MeshDistances);

		Result[Index] = Distance * VoxelSizeAndScale;
	}
}

export void VoxelNode_SampleVoxelStaticMesh_ApplyMetadata_Float(
	const uniform float PositionX[],
	const uniform float PositionY[],
	const uniform float PositionZ[],
	uniform float Result[],
	const uniform int32 Num,
	const uniform float Scale,
	const uniform int3& Size,
	const uniform float VoxelSize,
	const uniform float3& MeshOrigin,
	const uniform int32 Indices[],
	const uniform float MeshValues[])
{
	const uniform float MaxPositionX = Size.x - 2.0001;
	const uniform float MaxPositionY = Size.y - 2.0001;
	const uniform float MaxPositionZ = Size.z - 2.0001;

	const uniform float VoxelSizeAndScale = VoxelSize * Scale;
	const uniform float InvVoxelSizeAndScale = 1.f / VoxelSizeAndScale;

	const uniform int32 SizeX = Size.x;
	const uniform int32 SizeXY = Size.x * Size.y;

	FOREACH(Index, 0, Num)
	{
		varying float3 Position = MakeFloat3(PositionX[Index], PositionY[Index], PositionZ[Index]) * InvVoxelSizeAndScale - MeshOrigin;

		Position.x = clamp(Position.x, 1.f, MaxPositionX);
		Position.y = clamp(Position.y, 1.f, MaxPositionY);
		Position.z = clamp(Position.z, 1.f, MaxPositionZ);

		const varying float MinXf = floor(Position.x);
		const varying float MinYf = floor(Position.y);
		const varying float MinZf = floor(Position.z);

		const varying float AlphaX = Position.x - MinXf;
		const varying float AlphaY = Position.y - MinYf;
		const varying float AlphaZ = Position.z - MinZf;

		const varying int32 MinX = (int32)MinXf;
		const varying int32 MinY = (int32)MinYf;
		const varying int32 MinZ = (int32)MinZf;

		check(0 <= MinX && MinX + 1 < Size.x);
		check(0 <= MinY && MinY + 1 < Size.y);
		check(0 <= MinZ && MinZ + 1 < Size.z);

		IGNORE_PERF_WARNING
		const int32 Index000 = Indices[(MinX + 0) + SizeX * (MinY + 0) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index001 = Indices[(MinX + 1) + SizeX * (MinY + 0) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index010 = Indices[(MinX + 0) + SizeX * (MinY + 1) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index011 = Indices[(MinX + 1) + SizeX * (MinY + 1) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index100 = Indices[(MinX + 0) + SizeX * (MinY + 0) + SizeXY * (MinZ + 1)];
		IGNORE_PERF_WARNING
		const int32 Index101 = Indices[(MinX + 1) + SizeX * (MinY + 0) + SizeXY * (MinZ + 1)];
		IGNORE_PERF_WARNING
		const int32 Index110 = Indices[(MinX + 0) + SizeX * (MinY + 1) + SizeXY * (MinZ + 1)];
		IGNORE_PERF_WARNING
		const int32 Index111 = Indices[(MinX + 1) + SizeX * (MinY + 1) + SizeXY * (MinZ + 1)];

		const float NewValue = TrilinearInterpolation(
			IGNORE_PERF_WARNING
			MeshValues[Index000],
			IGNORE_PERF_WARNING
			MeshValues[Index001],
			IGNORE_PERF_WARNING
			MeshValues[Index010],
			IGNORE_PERF_WARNING
			MeshValues[Index011],
			IGNORE_PERF_WARNING
			MeshValues[Index100],
			IGNORE_PERF_WARNING
			MeshValues[Index101],
			IGNORE_PERF_WARNING
			MeshValues[Index110],
			IGNORE_PERF_WARNING
			MeshValues[Index111],
			AlphaX,
			AlphaY,
			AlphaZ);

		Result[Index] = NewValue;
	}
}

export void VoxelNode_SampleVoxelStaticMesh_ApplyMetadata_LinearColor(
	const uniform float PositionX[],
	const uniform float PositionY[],
	const uniform float PositionZ[],
	uniform float ResultR[],
	uniform float ResultG[],
	uniform float ResultB[],
	uniform float ResultA[],
	const uniform int32 Num,
	const uniform float Scale,
	const uniform int3& Size,
	const uniform float VoxelSize,
	const uniform float3& MeshOrigin,
	const uniform int32 Indices[],
	const uniform float MeshValuesR[],
	const uniform float MeshValuesG[],
	const uniform float MeshValuesB[],
	const uniform float MeshValuesA[])
{
	const uniform float MaxPositionX = Size.x - 2.0001;
	const uniform float MaxPositionY = Size.y - 2.0001;
	const uniform float MaxPositionZ = Size.z - 2.0001;

	const uniform float VoxelSizeAndScale = VoxelSize * Scale;
	const uniform float InvVoxelSizeAndScale = 1.f / VoxelSizeAndScale;

	const uniform int32 SizeX = Size.x;
	const uniform int32 SizeXY = Size.x * Size.y;

	FOREACH(Index, 0, Num)
	{
		varying float3 Position = MakeFloat3(PositionX[Index], PositionY[Index], PositionZ[Index]) * InvVoxelSizeAndScale - MeshOrigin;

		Position.x = clamp(Position.x, 1.f, MaxPositionX);
		Position.y = clamp(Position.y, 1.f, MaxPositionY);
		Position.z = clamp(Position.z, 1.f, MaxPositionZ);

		const varying float MinXf = floor(Position.x);
		const varying float MinYf = floor(Position.y);
		const varying float MinZf = floor(Position.z);

		const varying float AlphaX = Position.x - MinXf;
		const varying float AlphaY = Position.y - MinYf;
		const varying float AlphaZ = Position.z - MinZf;

		const varying int32 MinX = (int32)MinXf;
		const varying int32 MinY = (int32)MinYf;
		const varying int32 MinZ = (int32)MinZf;

		check(0 <= MinX && MinX + 1 < Size.x);
		check(0 <= MinY && MinY + 1 < Size.y);
		check(0 <= MinZ && MinZ + 1 < Size.z);

		IGNORE_PERF_WARNING
		const int32 Index000 = Indices[(MinX + 0) + SizeX * (MinY + 0) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index001 = Indices[(MinX + 1) + SizeX * (MinY + 0) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index010 = Indices[(MinX + 0) + SizeX * (MinY + 1) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index011 = Indices[(MinX + 1) + SizeX * (MinY + 1) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index100 = Indices[(MinX + 0) + SizeX * (MinY + 0) + SizeXY * (MinZ + 1)];
		IGNORE_PERF_WARNING
		const int32 Index101 = Indices[(MinX + 1) + SizeX * (MinY + 0) + SizeXY * (MinZ + 1)];
		IGNORE_PERF_WARNING
		const int32 Index110 = Indices[(MinX + 0) + SizeX * (MinY + 1) + SizeXY * (MinZ + 1)];
		IGNORE_PERF_WARNING
		const int32 Index111 = Indices[(MinX + 1) + SizeX * (MinY + 1) + SizeXY * (MinZ + 1)];

		{
			const float NewValueR = TrilinearInterpolation(
				IGNORE_PERF_WARNING
				MeshValuesR[Index000],
				IGNORE_PERF_WARNING
				MeshValuesR[Index001],
				IGNORE_PERF_WARNING
				MeshValuesR[Index010],
				IGNORE_PERF_WARNING
				MeshValuesR[Index011],
				IGNORE_PERF_WARNING
				MeshValuesR[Index100],
				IGNORE_PERF_WARNING
				MeshValuesR[Index101],
				IGNORE_PERF_WARNING
				MeshValuesR[Index110],
				IGNORE_PERF_WARNING
				MeshValuesR[Index111],
				AlphaX,
				AlphaY,
				AlphaZ);

			IGNORE_PERF_WARNING
			ResultR[Index] = NewValueR;
		}

		{
			const float NewValueG = TrilinearInterpolation(
				IGNORE_PERF_WARNING
				MeshValuesG[Index000],
				IGNORE_PERF_WARNING
				MeshValuesG[Index001],
				IGNORE_PERF_WARNING
				MeshValuesG[Index010],
				IGNORE_PERF_WARNING
				MeshValuesG[Index011],
				IGNORE_PERF_WARNING
				MeshValuesG[Index100],
				IGNORE_PERF_WARNING
				MeshValuesG[Index101],
				IGNORE_PERF_WARNING
				MeshValuesG[Index110],
				IGNORE_PERF_WARNING
				MeshValuesG[Index111],
				AlphaX,
				AlphaY,
				AlphaZ);

			IGNORE_PERF_WARNING
			ResultG[Index] = NewValueG;
		}

		{
			const float NewValueB = TrilinearInterpolation(
				IGNORE_PERF_WARNING
				MeshValuesB[Index000],
				IGNORE_PERF_WARNING
				MeshValuesB[Index001],
				IGNORE_PERF_WARNING
				MeshValuesB[Index010],
				IGNORE_PERF_WARNING
				MeshValuesB[Index011],
				IGNORE_PERF_WARNING
				MeshValuesB[Index100],
				IGNORE_PERF_WARNING
				MeshValuesB[Index101],
				IGNORE_PERF_WARNING
				MeshValuesB[Index110],
				IGNORE_PERF_WARNING
				MeshValuesB[Index111],
				AlphaX,
				AlphaY,
				AlphaZ);

			IGNORE_PERF_WARNING
			ResultB[Index] = NewValueB;
		}

		{
			const float NewValueA = TrilinearInterpolation(
				IGNORE_PERF_WARNING
				MeshValuesA[Index000],
				IGNORE_PERF_WARNING
				MeshValuesA[Index001],
				IGNORE_PERF_WARNING
				MeshValuesA[Index010],
				IGNORE_PERF_WARNING
				MeshValuesA[Index011],
				IGNORE_PERF_WARNING
				MeshValuesA[Index100],
				IGNORE_PERF_WARNING
				MeshValuesA[Index101],
				IGNORE_PERF_WARNING
				MeshValuesA[Index110],
				IGNORE_PERF_WARNING
				MeshValuesA[Index111],
				AlphaX,
				AlphaY,
				AlphaZ);

			IGNORE_PERF_WARNING
			ResultA[Index] = NewValueA;
		}
	}
}

export void VoxelNode_SampleVoxelStaticMesh_ApplyMetadata_Normal(
	const uniform float PositionX[],
	const uniform float PositionY[],
	const uniform float PositionZ[],
	uniform FVoxelOctahedron Result[],
	const uniform int32 Num,
	const uniform float Scale,
	const uniform int3& Size,
	const uniform float VoxelSize,
	const uniform float3& MeshOrigin,
	const uniform int32 Indices[],
	const uniform FVoxelOctahedron MeshValues[])
{
	const uniform float MaxPositionX = Size.x - 2.0001;
	const uniform float MaxPositionY = Size.y - 2.0001;
	const uniform float MaxPositionZ = Size.z - 2.0001;

	const uniform float VoxelSizeAndScale = VoxelSize * Scale;
	const uniform float InvVoxelSizeAndScale = 1.f / VoxelSizeAndScale;

	const uniform int32 SizeX = Size.x;
	const uniform int32 SizeXY = Size.x * Size.y;

	FOREACH(Index, 0, Num)
	{
		varying float3 Position = MakeFloat3(PositionX[Index], PositionY[Index], PositionZ[Index]) * InvVoxelSizeAndScale - MeshOrigin;

		Position.x = clamp(Position.x, 1.f, MaxPositionX);
		Position.y = clamp(Position.y, 1.f, MaxPositionY);
		Position.z = clamp(Position.z, 1.f, MaxPositionZ);

		const varying float MinXf = floor(Position.x);
		const varying float MinYf = floor(Position.y);
		const varying float MinZf = floor(Position.z);

		const varying float AlphaX = Position.x - MinXf;
		const varying float AlphaY = Position.y - MinYf;
		const varying float AlphaZ = Position.z - MinZf;

		const varying int32 MinX = (int32)MinXf;
		const varying int32 MinY = (int32)MinYf;
		const varying int32 MinZ = (int32)MinZf;

		check(0 <= MinX && MinX + 1 < Size.x);
		check(0 <= MinY && MinY + 1 < Size.y);
		check(0 <= MinZ && MinZ + 1 < Size.z);

		IGNORE_PERF_WARNING
		const int32 Index000 = Indices[(MinX + 0) + SizeX * (MinY + 0) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index001 = Indices[(MinX + 1) + SizeX * (MinY + 0) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index010 = Indices[(MinX + 0) + SizeX * (MinY + 1) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index011 = Indices[(MinX + 1) + SizeX * (MinY + 1) + SizeXY * (MinZ + 0)];
		IGNORE_PERF_WARNING
		const int32 Index100 = Indices[(MinX + 0) + SizeX * (MinY + 0) + SizeXY * (MinZ + 1)];
		IGNORE_PERF_WARNING
		const int32 Index101 = Indices[(MinX + 1) + SizeX * (MinY + 0) + SizeXY * (MinZ + 1)];
		IGNORE_PERF_WARNING
		const int32 Index110 = Indices[(MinX + 0) + SizeX * (MinY + 1) + SizeXY * (MinZ + 1)];
		IGNORE_PERF_WARNING
		const int32 Index111 = Indices[(MinX + 1) + SizeX * (MinY + 1) + SizeXY * (MinZ + 1)];

		const float3 NewValue = normalize(TrilinearInterpolation(
			IGNORE_PERF_WARNING
			OctahedronToUnitVector(MeshValues[Index000]),
			IGNORE_PERF_WARNING
			OctahedronToUnitVector(MeshValues[Index001]),
			IGNORE_PERF_WARNING
			OctahedronToUnitVector(MeshValues[Index010]),
			IGNORE_PERF_WARNING
			OctahedronToUnitVector(MeshValues[Index011]),
			IGNORE_PERF_WARNING
			OctahedronToUnitVector(MeshValues[Index100]),
			IGNORE_PERF_WARNING
			OctahedronToUnitVector(MeshValues[Index101]),
			IGNORE_PERF_WARNING
			OctahedronToUnitVector(MeshValues[Index110]),
			IGNORE_PERF_WARNING
			OctahedronToUnitVector(MeshValues[Index111]),
			AlphaX,
			AlphaY,
			AlphaZ));

		IGNORE_PERF_WARNING
		Result[Index] = MakeOctahedron(UnitVectorToOctahedron(NewValue));
	}
}