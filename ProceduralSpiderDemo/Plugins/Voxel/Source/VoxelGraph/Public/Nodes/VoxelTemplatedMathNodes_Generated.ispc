// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

export void VoxelNode_VoxelComputeNode_Acos_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = acos(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Acos_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = acos(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_AcosDegrees_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = 180.f / PI * acos(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_AcosDegrees_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = 180.f / PI * acos(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Asin_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = asin(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Asin_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = asin(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_AsinDegrees_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = 180.f / PI * asin(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_AsinDegrees_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = 180.f / PI * asin(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Atan2_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Y = LoadDouble(InputBuffers[0], Index);
		const double X = LoadDouble(InputBuffers[1], Index);
		double ReturnValue;

		ReturnValue = atan2(Y, X);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Atan2_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Y = LoadFloat(InputBuffers[0], Index);
		const float X = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		ReturnValue = atan2(Y, X);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Atan2Degrees_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Y = LoadDouble(InputBuffers[0], Index);
		const double X = LoadDouble(InputBuffers[1], Index);
		double ReturnValue;

		ReturnValue = 180.f / PI * atan2(Y, X);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Atan2Degrees_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Y = LoadFloat(InputBuffers[0], Index);
		const float X = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		ReturnValue = 180.f / PI * atan2(Y, X);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Atan_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = atan(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Atan_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = atan(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_AtanDegrees_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = 180.f / PI * atan(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_AtanDegrees_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = 180.f / PI * atan(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Cos_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = cos(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Cos_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = cos(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_CosDegrees_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = cos(PI / 180.f * Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_CosDegrees_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = cos(PI / 180.f * Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_DegreesToRadians_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = A * (PI / 180.f);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_DegreesToRadians_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = A * (PI / 180.f);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Frac_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = Value - floor(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Frac_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = Value - floor(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_GetPI_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		double ReturnValue;

		ReturnValue = PI;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_GetPI_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		float ReturnValue;

		ReturnValue = PI;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Power_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Base = LoadDouble(InputBuffers[0], Index);
		const double Exp = LoadDouble(InputBuffers[1], Index);
		double ReturnValue;

		ReturnValue = pow(abs(Base), Exp);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Power_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Base = LoadFloat(InputBuffers[0], Index);
		const float Exp = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		ReturnValue = pow(abs(Base), Exp);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_RadiansToDegrees_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = A * (180.f / PI);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_RadiansToDegrees_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = A * (180.f / PI);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Sin_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = sin(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Sin_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = sin(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_SinDegrees_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = sin(PI / 180.f * Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_SinDegrees_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = sin(PI / 180.f * Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Sqrt_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = sqrt(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Sqrt_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = sqrt(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Tan_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = tan(Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Tan_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = tan(Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_TanDegrees_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double Value = LoadDouble(InputBuffers[0], Index);
		double ReturnValue;

		ReturnValue = tan(PI / 180.f * Value);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_TanDegrees_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		float ReturnValue;

		ReturnValue = tan(PI / 180.f * Value);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}