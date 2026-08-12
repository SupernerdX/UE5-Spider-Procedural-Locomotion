// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"
#include "VoxelNoiseNodesImpl.isph"

export void VoxelNode_VoxelComputeNode_CellularNoise2D(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 Position = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float Amplitude = LoadFloat(InputBuffers[2], Index);
		const float FeatureScale = LoadFloat(InputBuffers[3], Index);
		const float Jitter = LoadFloat(InputBuffers[4], Index);
		const int32 Seed = LoadInt32(InputBuffers[5], Index);
		float Value;

		Value = GetCellularNoise2D(Seed, Position / FeatureScale, Jitter) * Amplitude;

		StoreFloat(OutputBuffers[0], Index, Value);
	}
}

export void VoxelNode_VoxelComputeNode_CellularNoise3D(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Position = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		const float Amplitude = LoadFloat(InputBuffers[3], Index);
		const float FeatureScale = LoadFloat(InputBuffers[4], Index);
		const float Jitter = LoadFloat(InputBuffers[5], Index);
		const int32 Seed = LoadInt32(InputBuffers[6], Index);
		float Value;

		Value = GetCellularNoise3D(Seed, Position / FeatureScale, Jitter) * Amplitude;

		StoreFloat(OutputBuffers[0], Index, Value);
	}
}

export void VoxelNode_VoxelComputeNode_MixSeeds(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		ReturnValue = MurmurHash32(MurmurHash32(A) ^ B);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_PerlinNoise2D(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 Position = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float Amplitude = LoadFloat(InputBuffers[2], Index);
		const float FeatureScale = LoadFloat(InputBuffers[3], Index);
		const int32 Seed = LoadInt32(InputBuffers[4], Index);
		float Value;

		Value = GetPerlin2D(Seed, Position / FeatureScale) * Amplitude;

		StoreFloat(OutputBuffers[0], Index, Value);
	}
}

export void VoxelNode_VoxelComputeNode_PerlinNoise3D(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Position = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		const float Amplitude = LoadFloat(InputBuffers[3], Index);
		const float FeatureScale = LoadFloat(InputBuffers[4], Index);
		const int32 Seed = LoadInt32(InputBuffers[5], Index);
		float Value;

		Value = GetPerlin3D(Seed, Position / FeatureScale) * Amplitude;

		StoreFloat(OutputBuffers[0], Index, Value);
	}
}

export void VoxelNode_VoxelComputeNode_SimplexNoise2D(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 Position = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float Amplitude = LoadFloat(InputBuffers[2], Index);
		const float FeatureScale = LoadFloat(InputBuffers[3], Index);
		const int32 Seed = LoadInt32(InputBuffers[4], Index);
		float Value;

		Value = GetSimplex2D(Seed, Position / FeatureScale) * Amplitude;

		StoreFloat(OutputBuffers[0], Index, Value);
	}
}

export void VoxelNode_VoxelComputeNode_SimplexNoise3D(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Position = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		const float Amplitude = LoadFloat(InputBuffers[3], Index);
		const float FeatureScale = LoadFloat(InputBuffers[4], Index);
		const int32 Seed = LoadInt32(InputBuffers[5], Index);
		float Value;

		Value = GetSimplex3D(Seed, Position / FeatureScale) * Amplitude;

		StoreFloat(OutputBuffers[0], Index, Value);
	}
}

export void VoxelNode_VoxelComputeNode_TrueDistanceCellularNoise2D(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 Position = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float Amplitude = LoadFloat(InputBuffers[2], Index);
		const float FeatureScale = LoadFloat(InputBuffers[3], Index);
		const float Jitter = LoadFloat(InputBuffers[4], Index);
		const int32 Seed = LoadInt32(InputBuffers[5], Index);
		float Value;
		float2 CellPosition;
		float2 FirstNeighborPosition;
		float FirstNeighborDistance;
		float2 SecondNeighborPosition;
		float SecondNeighborDistance;
		float2 ThirdNeighborPosition;
		float ThirdNeighborDistance;

		Value = GetTrueDistanceCellularNoise2D(Seed, Position / FeatureScale, Jitter, &CellPosition, &FirstNeighborPosition, &SecondNeighborPosition, SecondNeighborDistance, &ThirdNeighborPosition, ThirdNeighborDistance);
		FirstNeighborPosition = FirstNeighborPosition * FeatureScale;
		FirstNeighborDistance = Value * FeatureScale;
		SecondNeighborPosition = SecondNeighborPosition * FeatureScale;
		SecondNeighborDistance = SecondNeighborDistance * FeatureScale;
		ThirdNeighborPosition = ThirdNeighborPosition * FeatureScale;
		ThirdNeighborDistance = ThirdNeighborDistance * FeatureScale;
		Value = Value * Amplitude;
		CellPosition = CellPosition * FeatureScale;

		StoreFloat(OutputBuffers[0], Index, Value);
		StoreFloat(OutputBuffers[1], Index, CellPosition.x);
		StoreFloat(OutputBuffers[2], Index, CellPosition.y);
		StoreFloat(OutputBuffers[3], Index, FirstNeighborPosition.x);
		StoreFloat(OutputBuffers[4], Index, FirstNeighborPosition.y);
		StoreFloat(OutputBuffers[5], Index, FirstNeighborDistance);
		StoreFloat(OutputBuffers[6], Index, SecondNeighborPosition.x);
		StoreFloat(OutputBuffers[7], Index, SecondNeighborPosition.y);
		StoreFloat(OutputBuffers[8], Index, SecondNeighborDistance);
		StoreFloat(OutputBuffers[9], Index, ThirdNeighborPosition.x);
		StoreFloat(OutputBuffers[10], Index, ThirdNeighborPosition.y);
		StoreFloat(OutputBuffers[11], Index, ThirdNeighborDistance);
	}
}

export void VoxelNode_VoxelComputeNode_TrueDistanceCellularNoise3D(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Position = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		const float Amplitude = LoadFloat(InputBuffers[3], Index);
		const float FeatureScale = LoadFloat(InputBuffers[4], Index);
		const float Jitter = LoadFloat(InputBuffers[5], Index);
		const int32 Seed = LoadInt32(InputBuffers[6], Index);
		float Value;
		float3 CellPosition;
		float3 FirstNeighborPosition;
		float FirstNeighborDistance;
		float3 SecondNeighborPosition;
		float SecondNeighborDistance;
		float3 ThirdNeighborPosition;
		float ThirdNeighborDistance;

		Value = GetTrueDistanceCellularNoise3D(Seed, Position / FeatureScale, Jitter, &CellPosition, &FirstNeighborPosition, &SecondNeighborPosition, SecondNeighborDistance, &ThirdNeighborPosition, ThirdNeighborDistance);
		FirstNeighborPosition = FirstNeighborPosition * FeatureScale;
		FirstNeighborDistance = Value * FeatureScale;
		SecondNeighborPosition = SecondNeighborPosition * FeatureScale;
		SecondNeighborDistance = SecondNeighborDistance * FeatureScale;
		ThirdNeighborPosition = ThirdNeighborPosition * FeatureScale;
		ThirdNeighborDistance = ThirdNeighborDistance * FeatureScale;
		Value = Value * Amplitude;
		CellPosition = CellPosition * FeatureScale;

		StoreFloat(OutputBuffers[0], Index, Value);
		StoreFloat(OutputBuffers[1], Index, CellPosition.x);
		StoreFloat(OutputBuffers[2], Index, CellPosition.y);
		StoreFloat(OutputBuffers[3], Index, CellPosition.z);
		StoreFloat(OutputBuffers[4], Index, FirstNeighborPosition.x);
		StoreFloat(OutputBuffers[5], Index, FirstNeighborPosition.y);
		StoreFloat(OutputBuffers[6], Index, FirstNeighborPosition.z);
		StoreFloat(OutputBuffers[7], Index, FirstNeighborDistance);
		StoreFloat(OutputBuffers[8], Index, SecondNeighborPosition.x);
		StoreFloat(OutputBuffers[9], Index, SecondNeighborPosition.y);
		StoreFloat(OutputBuffers[10], Index, SecondNeighborPosition.z);
		StoreFloat(OutputBuffers[11], Index, SecondNeighborDistance);
		StoreFloat(OutputBuffers[12], Index, ThirdNeighborPosition.x);
		StoreFloat(OutputBuffers[13], Index, ThirdNeighborPosition.y);
		StoreFloat(OutputBuffers[14], Index, ThirdNeighborPosition.z);
		StoreFloat(OutputBuffers[15], Index, ThirdNeighborDistance);
	}
}

export void VoxelNode_VoxelComputeNode_ValueNoise2D(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 Position = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float Amplitude = LoadFloat(InputBuffers[2], Index);
		const float FeatureScale = LoadFloat(InputBuffers[3], Index);
		const int32 Seed = LoadInt32(InputBuffers[4], Index);
		float Value;

		Value = GetValue2D(Seed, Position / FeatureScale) * Amplitude;

		StoreFloat(OutputBuffers[0], Index, Value);
	}
}

export void VoxelNode_VoxelComputeNode_ValueNoise3D(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Position = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		const float Amplitude = LoadFloat(InputBuffers[3], Index);
		const float FeatureScale = LoadFloat(InputBuffers[4], Index);
		const int32 Seed = LoadInt32(InputBuffers[5], Index);
		float Value;

		Value = GetValue3D(Seed, Position / FeatureScale) * Amplitude;

		StoreFloat(OutputBuffers[0], Index, Value);
	}
}