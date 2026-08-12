// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

export void VoxelNode_VoxelComputeNode_BilinearInterpolation(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 Alpha = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float X0Y0 = LoadFloat(InputBuffers[2], Index);
		const float X1Y0 = LoadFloat(InputBuffers[3], Index);
		const float X0Y1 = LoadFloat(InputBuffers[4], Index);
		const float X1Y1 = LoadFloat(InputBuffers[5], Index);
		float ReturnValue;

		ReturnValue = BilinearInterpolation(X0Y0, X1Y0, X0Y1, X1Y1, Alpha.x, Alpha.y);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Bitwise_And(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		ReturnValue = A & B;

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Bitwise_Not(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		int32 ReturnValue;

		ReturnValue = ~A;

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Bitwise_Or(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		ReturnValue = A | B;

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Bitwise_Xor(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 A = LoadInt32(InputBuffers[0], Index);
		const int32 B = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		ReturnValue = A ^ B;

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_BooleanNOR(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const bool A = LoadBool(InputBuffers[0], Index);
		const bool B = LoadBool(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = !(A || B);

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_BooleanNOT(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const bool A = LoadBool(InputBuffers[0], Index);
		bool ReturnValue;

		ReturnValue = !A;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_BooleanXOR(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const bool A = LoadBool(InputBuffers[0], Index);
		const bool B = LoadBool(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A != B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_BreakQuaternion(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float4 Quat = MakeFloat4(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index), LoadFloat(InputBuffers[3], Index));
		float Roll;
		float Pitch;
		float Yaw;

		MakeEulerFromQuaternion(Quat, Pitch, Yaw, Roll);

		StoreFloat(OutputBuffers[0], Index, Roll);
		StoreFloat(OutputBuffers[1], Index, Pitch);
		StoreFloat(OutputBuffers[2], Index, Yaw);
	}
}

export void VoxelNode_VoxelComputeNode_DistanceToColor(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Distance = LoadFloat(InputBuffers[0], Index);
		const float Scale = LoadFloat(InputBuffers[1], Index);
		float4 Color;

		Color = GetDistanceFieldColor(Distance / Scale);

		StoreFloat(OutputBuffers[0], Index, Color.x);
		StoreFloat(OutputBuffers[1], Index, Color.y);
		StoreFloat(OutputBuffers[2], Index, Color.z);
		StoreFloat(OutputBuffers[3], Index, Color.w);
	}
}

export void VoxelNode_VoxelComputeNode_IsFinite(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		bool ReturnValue;

		ReturnValue = IsFinite(Value);

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_IsVoid(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		bool ReturnValue;

		ReturnValue = isnan(Value);

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_LeftShift(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 Value = LoadInt32(InputBuffers[0], Index);
		const int32 Shift = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		IGNORE_PERF_WARNING
		ReturnValue = Value << clamp(Shift, 0, 31);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_MakeQuaternion(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Roll = LoadFloat(InputBuffers[0], Index);
		const float Pitch = LoadFloat(InputBuffers[1], Index);
		const float Yaw = LoadFloat(InputBuffers[2], Index);
		float4 ReturnValue;

		ReturnValue = MakeQuaternionFromEuler(Pitch, Yaw, Roll);

		StoreFloat(OutputBuffers[0], Index, ReturnValue.x);
		StoreFloat(OutputBuffers[1], Index, ReturnValue.y);
		StoreFloat(OutputBuffers[2], Index, ReturnValue.z);
		StoreFloat(OutputBuffers[3], Index, ReturnValue.w);
	}
}

export void VoxelNode_VoxelComputeNode_MakeRotationFromX(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 X = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		float4 ReturnValue;

		ReturnValue = MakeQuaternionFromX(X);

		StoreFloat(OutputBuffers[0], Index, ReturnValue.x);
		StoreFloat(OutputBuffers[1], Index, ReturnValue.y);
		StoreFloat(OutputBuffers[2], Index, ReturnValue.z);
		StoreFloat(OutputBuffers[3], Index, ReturnValue.w);
	}
}

export void VoxelNode_VoxelComputeNode_MakeRotationFromY(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Y = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		float4 ReturnValue;

		ReturnValue = MakeQuaternionFromY(Y);

		StoreFloat(OutputBuffers[0], Index, ReturnValue.x);
		StoreFloat(OutputBuffers[1], Index, ReturnValue.y);
		StoreFloat(OutputBuffers[2], Index, ReturnValue.z);
		StoreFloat(OutputBuffers[3], Index, ReturnValue.w);
	}
}

export void VoxelNode_VoxelComputeNode_MakeRotationFromZ(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Z = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		float4 ReturnValue;

		ReturnValue = MakeQuaternionFromZ(Z);

		StoreFloat(OutputBuffers[0], Index, ReturnValue.x);
		StoreFloat(OutputBuffers[1], Index, ReturnValue.y);
		StoreFloat(OutputBuffers[2], Index, ReturnValue.z);
		StoreFloat(OutputBuffers[3], Index, ReturnValue.w);
	}
}

export void VoxelNode_VoxelComputeNode_ReplaceVoid(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float Value = LoadFloat(InputBuffers[0], Index);
		const float Fallback = LoadFloat(InputBuffers[1], Index);
		float ReturnValue;

		ReturnValue = isnan(Value) ? Fallback : Value;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_RightShift(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const int32 Value = LoadInt32(InputBuffers[0], Index);
		const int32 Shift = LoadInt32(InputBuffers[1], Index);
		int32 ReturnValue;

		IGNORE_PERF_WARNING
		ReturnValue = Value >> clamp(Shift, 0, 31);

		StoreInt32(OutputBuffers[0], Index, ReturnValue);
	}
}