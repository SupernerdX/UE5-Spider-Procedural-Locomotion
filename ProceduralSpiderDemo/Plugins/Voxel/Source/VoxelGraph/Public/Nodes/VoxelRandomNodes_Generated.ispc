// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

enum EVoxelRandomVectorType{	EVoxelRandomVectorType_Uniform = 0,
	EVoxelRandomVectorType_Free = 1,
	EVoxelRandomVectorType_LockXY = 2,
	EVoxelRandomVectorType_EVoxelRandomVectorType_MAX = 3,
};
export void VoxelNode_VoxelComputeNode_RandFloat(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 Seed = LoadInt32(InputBuffers[0], Index);
		const float2 Range = MakeFloat2(LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		float ReturnValue;

		ReturnValue = RandRange(Seed, Range);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_RandomRotator(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 Roll = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float2 Pitch = MakeFloat2(LoadFloat(InputBuffers[2], Index), LoadFloat(InputBuffers[3], Index));
		const float2 Yaw = MakeFloat2(LoadFloat(InputBuffers[4], Index), LoadFloat(InputBuffers[5], Index));
		const int32 Seed = LoadInt32(InputBuffers[6], Index);
		float4 ReturnValue;

		ReturnValue = MakeQuaternionFromEuler(RandRange(MurmurHash32(Seed, 0), Pitch), RandRange(MurmurHash32(Seed, 1), Yaw), RandRange(MurmurHash32(Seed, 2), Roll));

		StoreFloat(OutputBuffers[0], Index, ReturnValue.x);
		StoreFloat(OutputBuffers[1], Index, ReturnValue.y);
		StoreFloat(OutputBuffers[2], Index, ReturnValue.z);
		StoreFloat(OutputBuffers[3], Index, ReturnValue.w);
	}
}

export void VoxelNode_VoxelComputeNode_RandomVector(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 RangeX = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float2 RangeY = MakeFloat2(LoadFloat(InputBuffers[2], Index), LoadFloat(InputBuffers[3], Index));
		const float2 RangeZ = MakeFloat2(LoadFloat(InputBuffers[4], Index), LoadFloat(InputBuffers[5], Index));
		const uint8 Type = LoadByte(InputBuffers[6], Index);
		const int32 Seed = LoadInt32(InputBuffers[7], Index);
		float3 ReturnValue;

		switch (Type)
		{
			default:
			{
				check(false);
				ReturnValue.x = 0.f;
				ReturnValue.y = 0.f;
				ReturnValue.z = 0.f;
				break;
			}
			case EVoxelRandomVectorType_Uniform:
			{
				ReturnValue.x = RandRange(Seed, RangeX);
				ReturnValue.y = ReturnValue.x;
				ReturnValue.z = ReturnValue.x;
				break;
			}
			case EVoxelRandomVectorType_Free:
			{
				ReturnValue.x = RandRange(MurmurHash32(Seed, 0), RangeX);
				ReturnValue.y = RandRange(MurmurHash32(Seed, 1), RangeY);
				ReturnValue.z = RandRange(MurmurHash32(Seed, 2), RangeZ);
				break;
			}
			case EVoxelRandomVectorType_LockXY:
			{
				ReturnValue.x = RandRange(MurmurHash32(Seed, 0), RangeX);
				ReturnValue.y = ReturnValue.x;
				ReturnValue.z = RandRange(MurmurHash32(Seed, 1), RangeZ);
				break;
			}
		}
		;

		StoreFloat(OutputBuffers[0], Index, ReturnValue.x);
		StoreFloat(OutputBuffers[1], Index, ReturnValue.y);
		StoreFloat(OutputBuffers[2], Index, ReturnValue.z);
	}
}