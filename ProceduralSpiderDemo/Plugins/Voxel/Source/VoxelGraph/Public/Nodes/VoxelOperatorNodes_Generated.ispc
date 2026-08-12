// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

export void VoxelNode_VoxelComputeNode_Abs_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = abs(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Abs_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = abs(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Abs_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 Value = LoadInt32(InputBuffers[0], Index);
		int32 ReturnValue;

		ReturnValue = abs(Value);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Abs_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 Value = LoadInt64(InputBuffers[0], Index);
		int64 ReturnValue;

		ReturnValue = abs(Value);

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Add_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		double ReturnValue;

		ReturnValue = A + B;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Add_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		ReturnValue = A + B;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Add_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		ReturnValue = A + B;

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Add_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		int64 ReturnValue;

		ReturnValue = A + B;

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Divide_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		double ReturnValue;

		ReturnValue = A / B;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Divide_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		ReturnValue = B != 0.f ? A / B : A > 0.f ? BIG_NUMBER : A < 0.f ? -BIG_NUMBER : 1.f;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Divide_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		IGNORE_PERF_WARNING
		ReturnValue = B != 0 ? A / B : 0;

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Divide_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		int64 ReturnValue;

		IGNORE_PERF_WARNING
		ReturnValue = B != 0 ? A / B : 0;

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Max_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		double ReturnValue;

		ReturnValue = max(A, B);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Max_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		ReturnValue = max(A, B);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Max_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		ReturnValue = max(A, B);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Max_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		int64 ReturnValue;

		ReturnValue = max(A, B);

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Min_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		double ReturnValue;

		ReturnValue = min(A, B);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Min_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		ReturnValue = min(A, B);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Min_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		ReturnValue = min(A, B);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Min_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		int64 ReturnValue;

		ReturnValue = min(A, B);

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Modulus_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		double ReturnValue;

		IGNORE_PERF_WARNING
		ReturnValue = B != 0 ? (A - floor(A / B) * B) : 0;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Modulus_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		IGNORE_PERF_WARNING
		ReturnValue = B != 0 ? (A - floor(A / B) * B) : 0;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Modulus_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		IGNORE_PERF_WARNING
		ReturnValue = B != 0 ? A % B : 0;

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Modulus_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		int64 ReturnValue;

		IGNORE_PERF_WARNING
		ReturnValue = B != 0 ? A % B : 0;

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Multiply_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		double ReturnValue;

		ReturnValue = A * B;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Multiply_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		ReturnValue = A * B;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Multiply_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		ReturnValue = A * B;

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Multiply_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		int64 ReturnValue;

		ReturnValue = A * B;

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_OneMinus_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = 1 - Value;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_OneMinus_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = 1 - Value;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Sign_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = Value < 0.d ? -1.d : Value > 0.f ? 1.d : 0.d;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Sign_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = Value < 0.f ? -1.f : Value > 0.f ? 1.f : 0.f;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Sign_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 Value = LoadInt32(InputBuffers[0], Index);
		int32 ReturnValue;

		ReturnValue = Value < 0 ? -1 : Value > 0 ? 1 : 0;

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Sign_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 Value = LoadInt64(InputBuffers[0], Index);
		int64 ReturnValue;

		ReturnValue = Value < 0 ? -1 : Value > 0 ? 1 : 0;

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Subtract_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		double ReturnValue;

		ReturnValue = A - B;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Subtract_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		ReturnValue = A - B;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Subtract_Int32(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		ReturnValue = A - B;

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Subtract_Int64(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int64 A = LoadInt64(InputBuffers[0], Index);
		const int64 B = LoadInt64(InputBuffers[1], Index);
		int64 ReturnValue;

		ReturnValue = A - B;

		StoreInt64(OutputBuffers[0], Index, ReturnValue);
	}
}