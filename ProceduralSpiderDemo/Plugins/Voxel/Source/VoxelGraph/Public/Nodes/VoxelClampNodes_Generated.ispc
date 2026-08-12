// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

export void VoxelNode_VoxelComputeNode_Clamp_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		const double Min = LoadDouble(InputBuffers[1], Index);
		const double Max = LoadDouble(InputBuffers[2], Index);
		double ReturnValue;

		ReturnValue = clamp(Value, Min, Max);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Clamp_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		const float Min = LoadFloat(InputBuffers[1], Index);
		const float Max = LoadFloat(InputBuffers[2], Index);
		float ReturnValue;

		ReturnValue = clamp(Value, Min, Max);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Clamp_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 Value = LoadInt32(InputBuffers[0], Index);
		const int32 Min = LoadInt32(InputBuffers[1], Index);
		const int32 Max = LoadInt32(InputBuffers[2], Index);
		int32 ReturnValue;

		ReturnValue = clamp(Value, Min, Max);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Clamp_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 Value = LoadInt64(InputBuffers[0], Index);
		const int64 Min = LoadInt64(InputBuffers[1], Index);
		const int64 Max = LoadInt64(InputBuffers[2], Index);
		int64 ReturnValue;

		ReturnValue = clamp(Value, Min, Max);

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_MappedRangeValueClamped_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		const double InMin = LoadDouble(InputBuffers[1], Index);
		const double InMax = LoadDouble(InputBuffers[2], Index);
		const double OutMin = LoadDouble(InputBuffers[3], Index);
		const double OutMax = LoadDouble(InputBuffers[4], Index);
		double ReturnValue;

		ReturnValue = lerp(OutMin, OutMax, clamp((Value - InMin) / (InMax - InMin), 0.d, 1.d));

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_MappedRangeValueClamped_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		const float InMin = LoadFloat(InputBuffers[1], Index);
		const float InMax = LoadFloat(InputBuffers[2], Index);
		const float OutMin = LoadFloat(InputBuffers[3], Index);
		const float OutMax = LoadFloat(InputBuffers[4], Index);
		float ReturnValue;

		ReturnValue = lerp(OutMin, OutMax, clamp((Value - InMin) / (InMax - InMin), 0.f, 1.f));

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_MappedRangeValueUnclamped_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		const double InMin = LoadDouble(InputBuffers[1], Index);
		const double InMax = LoadDouble(InputBuffers[2], Index);
		const double OutMin = LoadDouble(InputBuffers[3], Index);
		const double OutMax = LoadDouble(InputBuffers[4], Index);
		double ReturnValue;

		ReturnValue = lerp(OutMin, OutMax, (Value - InMin) / (InMax - InMin));

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_MappedRangeValueUnclamped_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		const float InMin = LoadFloat(InputBuffers[1], Index);
		const float InMax = LoadFloat(InputBuffers[2], Index);
		const float OutMin = LoadFloat(InputBuffers[3], Index);
		const float OutMax = LoadFloat(InputBuffers[4], Index);
		float ReturnValue;

		ReturnValue = lerp(OutMin, OutMax, (Value - InMin) / (InMax - InMin));

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}