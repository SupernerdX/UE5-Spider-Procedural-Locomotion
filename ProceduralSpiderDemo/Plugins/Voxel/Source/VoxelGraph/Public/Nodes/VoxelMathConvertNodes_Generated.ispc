// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

export void VoxelNode_VoxelComputeNode_Ceil_DoubleToDouble(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = ceil(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Ceil_DoubleToInt32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		int32 ReturnValue;

		ReturnValue = (int32)ceil(Value);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Ceil_DoubleToInt64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		int64 ReturnValue;

		ReturnValue = (int64)ceil(Value);

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Ceil_FloatToFloat(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = ceil(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Ceil_FloatToInt32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		int32 ReturnValue;

		ReturnValue = (int32)ceil(Value);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Ceil_FloatToInt64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		int64 ReturnValue;

		ReturnValue = (int64)ceil(Value);

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Floor_DoubleToDouble(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = floor(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Floor_DoubleToInt32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		int32 ReturnValue;

		ReturnValue = (int32)floor(Value);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Floor_DoubleToInt64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		int64 ReturnValue;

		ReturnValue = (int64)floor(Value);

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Floor_FloatToFloat(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = floor(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Floor_FloatToInt32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		int32 ReturnValue;

		ReturnValue = (int32)floor(Value);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Floor_FloatToInt64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		int64 ReturnValue;

		ReturnValue = (int64)floor(Value);

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Round_DoubleToDouble(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = round(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Round_DoubleToInt32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		int32 ReturnValue;

		ReturnValue = (int32)round(Value);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Round_DoubleToInt64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		int64 ReturnValue;

		ReturnValue = (int64)round(Value);

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Round_FloatToFloat(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = round(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Round_FloatToInt32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		int32 ReturnValue;

		ReturnValue = (int32)round(Value);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Round_FloatToInt64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		int64 ReturnValue;

		ReturnValue = (int64)round(Value);

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}