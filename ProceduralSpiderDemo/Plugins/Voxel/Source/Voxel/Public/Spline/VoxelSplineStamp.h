// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FSplineCurves;
struct FVoxelSplineMetadataRuntime;

namespace ispc
{
#define __ISPC_ENUM_EInterpMode__
#define __ISPC_STRUCT_FSegment__

enum EInterpMode
{
	InterpMode_Constant = 0,
	InterpMode_Linear = 1,
	InterpMode_Cubic = 2
};
struct FSegment
{
	float Key;
	float NextKey;
	float3 Value;
	float3 NextValue;
	float3 LeaveTangent;
	float3 ArriveTangent;
	EInterpMode InterpMode;
};
}

struct VOXEL_API FVoxelSplineSegment
{
	FVoxelBox Bounds;
	ispc::FSegment Segment;

	FInterpCurvePoint<FQuat> RotationA;
	FInterpCurvePoint<FQuat> RotationB;
	FInterpCurvePoint<FVector> PositionA;
	FInterpCurvePoint<FVector> PositionB;
	FInterpCurvePoint<FVector> ScaleA;
	FInterpCurvePoint<FVector> ScaleB;

	TVoxelMap<FGuid, TPair<float, float>> GuidToFloatValue;
	TVoxelMap<FGuid, TPair<FVector2f, FVector2f>> GuidToVector2DValue;
	TVoxelMap<FGuid, TPair<FVector3f, FVector3f>> GuidToVectorValue;

	FORCEINLINE bool operator==(const FVoxelSplineSegment& Other) const
	{
		return
			Bounds == Other.Bounds &&
			RotationA == Other.RotationA &&
			RotationB == Other.RotationB &&
			PositionA == Other.PositionA &&
			PositionB == Other.PositionB &&
			ScaleA == Other.ScaleA &&
			ScaleB == Other.ScaleB &&
			GuidToFloatValue.OrderIndependentEqual(Other.GuidToFloatValue) &&
			GuidToVector2DValue.OrderIndependentEqual(Other.GuidToVector2DValue) &&
			GuidToVectorValue.OrderIndependentEqual(Other.GuidToVectorValue);
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelSplineSegment& InSegment)
	{
		return GetTypeHash(InSegment.PositionA.OutVal);
	}

	static TVoxelArray<FVoxelSplineSegment> Create(
		const FSplineCurves& SplineCurves,
		const FVoxelSplineMetadataRuntime& MetadataRuntime);
};