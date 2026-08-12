// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

export void VoxelNode_VoxelComputeNode_CrossProduct_2D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double2 V1 = MakeDouble2(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index));
		const double2 V2 = MakeDouble2(LoadDouble(InputBuffers[2], Index), LoadDouble(InputBuffers[3], Index));
		double ReturnValue;

		ReturnValue = V1.x * V2.y - V1.y * V2.x;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_CrossProduct_2D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 V1 = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float2 V2 = MakeFloat2(LoadFloat(InputBuffers[2], Index), LoadFloat(InputBuffers[3], Index));
		float ReturnValue;

		ReturnValue = V1.x * V2.y - V1.y * V2.x;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_CrossProduct_3D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double3 V1 = MakeDouble3(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index), LoadDouble(InputBuffers[2], Index));
		const double3 V2 = MakeDouble3(LoadDouble(InputBuffers[3], Index), LoadDouble(InputBuffers[4], Index), LoadDouble(InputBuffers[5], Index));
		double3 ReturnValue;

		ReturnValue = cross(V1, V2);

		StoreDouble(OutputBuffers[0], Index, ReturnValue.x);
		StoreDouble(OutputBuffers[1], Index, ReturnValue.y);
		StoreDouble(OutputBuffers[2], Index, ReturnValue.z);
	}
}

export void VoxelNode_VoxelComputeNode_CrossProduct_3D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 V1 = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		const float3 V2 = MakeFloat3(LoadFloat(InputBuffers[3], Index), LoadFloat(InputBuffers[4], Index), LoadFloat(InputBuffers[5], Index));
		float3 ReturnValue;

		ReturnValue = cross(V1, V2);

		StoreFloat(OutputBuffers[0], Index, ReturnValue.x);
		StoreFloat(OutputBuffers[1], Index, ReturnValue.y);
		StoreFloat(OutputBuffers[2], Index, ReturnValue.z);
	}
}

export void VoxelNode_VoxelComputeNode_Distance2D_3D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double3 V1 = MakeDouble3(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index), LoadDouble(InputBuffers[2], Index));
		const double3 V2 = MakeDouble3(LoadDouble(InputBuffers[3], Index), LoadDouble(InputBuffers[4], Index), LoadDouble(InputBuffers[5], Index));
		double ReturnValue;

		ReturnValue = distance(MakeFloat2(V1.x, V1.y), MakeFloat2(V2.x, V2.y));

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Distance2D_3D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 V1 = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		const float3 V2 = MakeFloat3(LoadFloat(InputBuffers[3], Index), LoadFloat(InputBuffers[4], Index), LoadFloat(InputBuffers[5], Index));
		float ReturnValue;

		ReturnValue = distance(MakeFloat2(V1.x, V1.y), MakeFloat2(V2.x, V2.y));

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Distance_2D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double2 V1 = MakeDouble2(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index));
		const double2 V2 = MakeDouble2(LoadDouble(InputBuffers[2], Index), LoadDouble(InputBuffers[3], Index));
		double ReturnValue;

		ReturnValue = distance(V1, V2);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Distance_2D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 V1 = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float2 V2 = MakeFloat2(LoadFloat(InputBuffers[2], Index), LoadFloat(InputBuffers[3], Index));
		float ReturnValue;

		ReturnValue = distance(V1, V2);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Distance_3D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double3 V1 = MakeDouble3(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index), LoadDouble(InputBuffers[2], Index));
		const double3 V2 = MakeDouble3(LoadDouble(InputBuffers[3], Index), LoadDouble(InputBuffers[4], Index), LoadDouble(InputBuffers[5], Index));
		double ReturnValue;

		ReturnValue = distance(V1, V2);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Distance_3D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 V1 = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		const float3 V2 = MakeFloat3(LoadFloat(InputBuffers[3], Index), LoadFloat(InputBuffers[4], Index), LoadFloat(InputBuffers[5], Index));
		float ReturnValue;

		ReturnValue = distance(V1, V2);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_DotProduct_2D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double2 V1 = MakeDouble2(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index));
		const double2 V2 = MakeDouble2(LoadDouble(InputBuffers[2], Index), LoadDouble(InputBuffers[3], Index));
		double ReturnValue;

		ReturnValue = V1.x * V2.x + V1.y * V2.y;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_DotProduct_2D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 V1 = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		const float2 V2 = MakeFloat2(LoadFloat(InputBuffers[2], Index), LoadFloat(InputBuffers[3], Index));
		float ReturnValue;

		ReturnValue = V1.x * V2.x + V1.y * V2.y;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_DotProduct_3D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double3 V1 = MakeDouble3(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index), LoadDouble(InputBuffers[2], Index));
		const double3 V2 = MakeDouble3(LoadDouble(InputBuffers[3], Index), LoadDouble(InputBuffers[4], Index), LoadDouble(InputBuffers[5], Index));
		double ReturnValue;

		ReturnValue = V1.x * V2.x + V1.y * V2.y + V1.z * V2.z;

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_DotProduct_3D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 V1 = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		const float3 V2 = MakeFloat3(LoadFloat(InputBuffers[3], Index), LoadFloat(InputBuffers[4], Index), LoadFloat(InputBuffers[5], Index));
		float ReturnValue;

		ReturnValue = V1.x * V2.x + V1.y * V2.y + V1.z * V2.z;

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Length_2D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double2 Vector = MakeDouble2(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index));
		double ReturnValue;

		ReturnValue = length(Vector);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Length_2D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 Vector = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		float ReturnValue;

		ReturnValue = length(Vector);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Length_3D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double3 Vector = MakeDouble3(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index), LoadDouble(InputBuffers[2], Index));
		double ReturnValue;

		ReturnValue = length(Vector);

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Length_3D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Vector = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		float ReturnValue;

		ReturnValue = length(Vector);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_LengthXY_3D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double3 Vector = MakeDouble3(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index), LoadDouble(InputBuffers[2], Index));
		double ReturnValue;

		ReturnValue = length(MakeFloat2(Vector.x, Vector.y));

		StoreDouble(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_LengthXY_3D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Vector = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		float ReturnValue;

		ReturnValue = length(MakeFloat2(Vector.x, Vector.y));

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_Normalize_2D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double2 Vector = MakeDouble2(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index));
		double2 ReturnValue;

		ReturnValue = normalize(Vector);

		StoreDouble(OutputBuffers[0], Index, ReturnValue.x);
		StoreDouble(OutputBuffers[1], Index, ReturnValue.y);
	}
}

export void VoxelNode_VoxelComputeNode_Normalize_2D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 Vector = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		float2 ReturnValue;

		ReturnValue = normalize(Vector);

		StoreFloat(OutputBuffers[0], Index, ReturnValue.x);
		StoreFloat(OutputBuffers[1], Index, ReturnValue.y);
	}
}

export void VoxelNode_VoxelComputeNode_Normalize_3D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double3 Vector = MakeDouble3(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index), LoadDouble(InputBuffers[2], Index));
		double3 ReturnValue;

		ReturnValue = normalize(Vector);

		StoreDouble(OutputBuffers[0], Index, ReturnValue.x);
		StoreDouble(OutputBuffers[1], Index, ReturnValue.y);
		StoreDouble(OutputBuffers[2], Index, ReturnValue.z);
	}
}

export void VoxelNode_VoxelComputeNode_Normalize_3D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Vector = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		float3 ReturnValue;

		ReturnValue = normalize(Vector);

		StoreFloat(OutputBuffers[0], Index, ReturnValue.x);
		StoreFloat(OutputBuffers[1], Index, ReturnValue.y);
		StoreFloat(OutputBuffers[2], Index, ReturnValue.z);
	}
}

export void VoxelNode_VoxelComputeNode_NormalToSlope_2D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double2 Normal = MakeDouble2(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index));
		double Slope;

		const double3 Normalized = normalize(MakeDouble3(Normal.x, Normal.y, 1));
		Slope = acos(Normalized.z) * (180.f / PI);

		StoreDouble(OutputBuffers[0], Index, Slope);
	}
}

export void VoxelNode_VoxelComputeNode_NormalToSlope_2D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float2 Normal = MakeFloat2(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index));
		float Slope;

		const float3 Normalized = normalize(MakeFloat3(Normal.x, Normal.y, 1));
		Slope = acos(Normalized.z) * (180.f / PI);

		StoreFloat(OutputBuffers[0], Index, Slope);
	}
}

export void VoxelNode_VoxelComputeNode_NormalToSlope_3D_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double3 Normal = MakeDouble3(LoadDouble(InputBuffers[0], Index), LoadDouble(InputBuffers[1], Index), LoadDouble(InputBuffers[2], Index));
		double Slope;

		const double3 Normalized = normalize(Normal);
		Slope = acos(Normalized.z) * (180.f / PI);

		StoreDouble(OutputBuffers[0], Index, Slope);
	}
}

export void VoxelNode_VoxelComputeNode_NormalToSlope_3D_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float3 Normal = MakeFloat3(LoadFloat(InputBuffers[0], Index), LoadFloat(InputBuffers[1], Index), LoadFloat(InputBuffers[2], Index));
		float Slope;

		const float3 Normalized = normalize(Normal);
		Slope = acos(Normalized.z) * (180.f / PI);

		StoreFloat(OutputBuffers[0], Index, Slope);
	}
}