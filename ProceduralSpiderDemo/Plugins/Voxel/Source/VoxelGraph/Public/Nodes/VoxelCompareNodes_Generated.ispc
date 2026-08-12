// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

export void VoxelNode_VoxelComputeNode_Greater_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A > B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Greater_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A > B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Greater_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A > B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Greater_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A > B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_GreaterEqual_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A >= B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_GreaterEqual_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A >= B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_GreaterEqual_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A >= B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_GreaterEqual_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A >= B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Less_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A < B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Less_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A < B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Less_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A < B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Less_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A < B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_LessEqual_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A <= B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_LessEqual_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A <= B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_LessEqual_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A <= B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_LessEqual_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A <= B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}