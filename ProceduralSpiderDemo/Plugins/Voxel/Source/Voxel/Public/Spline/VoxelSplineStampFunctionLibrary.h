// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFunctionLibrary.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Spline/VoxelSplineParameters.h"
#include "VoxelSplineStampFunctionLibrary.generated.h"

UCLASS()
class VOXEL_API UVoxelSplineStampFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	// In a height spline will return GetClosestSplineKey2D
	// In a volume spline will return GetClosestSplineKey3D
	// Might be inaccurate if Position is further than MaxWidth away from the spline
	UFUNCTION(Category = "Stamp|Spline", meta = (AllowList = "HeightSpline, VolumeSpline"))
	FVoxelFloatBuffer GetClosestSplineKeyGeneric(const FVoxelVectorBuffer& Position) const;

	// Might be inaccurate if Position is further than MaxWidth away from the spline
	UFUNCTION(Category = "Stamp|Spline", meta = (AllowList = "HeightSpline, VolumeSpline"))
	FVoxelFloatBuffer GetClosestSplineKey2D(UPARAM(meta = (PositionPin)) const FVoxelVector2DBuffer& Position) const;

	// Might be inaccurate if Position is further than MaxWidth away from the spline
	UFUNCTION(Category = "Stamp|Spline", meta = (AllowList = "HeightSpline, VolumeSpline"))
	FVoxelFloatBuffer GetClosestSplineKey3D(UPARAM(meta = (PositionPin)) const FVoxelVectorBuffer& Position) const;

public:
	UFUNCTION(Category = "Stamp|Spline", meta = (AllowList = "HeightSpline, VolumeSpline"))
	float GetSplineLength() const;

	// Returns the last valid spline key (at the spline's end point)
	UFUNCTION(Category = "Stamp|Spline", meta = (AllowList = "HeightSpline, VolumeSpline"))
	float GetLastSplineKey() const;

	UFUNCTION(Category = "Stamp|Spline", meta = (AllowList = "HeightSpline, VolumeSpline"))
	FVoxelVectorBuffer GetPositionAlongSpline(UPARAM(meta = (SplineKeyPin)) const FVoxelFloatBuffer& SplineKey) const;

	UFUNCTION(Category = "Stamp|Spline", meta = (AllowList = "HeightSpline, VolumeSpline"))
	FVoxelFloatBuffer GetHeightAlongSpline(UPARAM(meta = (SplineKeyPin)) const FVoxelFloatBuffer& SplineKey) const;

	// Return how far along the spline we currently are
	// Between 0 and GetSplineLength
	UFUNCTION(Category = "Stamp|Spline", meta = (AllowList = "HeightSpline, VolumeSpline"))
	FVoxelFloatBuffer GetDistanceAlongSpline(UPARAM(meta = (SplineKeyPin)) const FVoxelFloatBuffer& SplineKey) const;

	UFUNCTION(Category = "Stamp|Spline", meta = (AllowList = "HeightSpline, VolumeSpline", AdvancedDisplay = "bQueryPosition, bQueryRotation, bQueryScale"))
	FVoxelTransformBuffer GetTransformAlongSpline(UPARAM(meta = (SplineKeyPin)) const FVoxelFloatBuffer& SplineKey) const;

public:
	UFUNCTION(Category = "Stamp|Spline", DisplayName = "Get Spline Parameter Value (float)", meta = (AllowList = "HeightSpline, VolumeSpline"))
	FVoxelFloatBuffer GetFloatSplineParameterValue(
		const FVoxelFloatSplineParameter& Parameter,
		UPARAM(meta = (SplineKeyPin)) const FVoxelFloatBuffer& SplineKey) const;

	UFUNCTION(Category = "Stamp|Spline", DisplayName = "Get Spline Parameter Value (Vector 2D)", meta = (AllowList = "HeightSpline, VolumeSpline"))
	FVoxelVector2DBuffer GetVector2DSplineParameterValue(
		const FVoxelVector2DSplineParameter& Parameter,
		UPARAM(meta = (SplineKeyPin)) const FVoxelFloatBuffer& SplineKey) const;

	UFUNCTION(Category = "Stamp|Spline", DisplayName = "Get Spline Parameter Value (Vector)", meta = (AllowList = "HeightSpline, VolumeSpline"))
	FVoxelVectorBuffer GetVectorSplineParameterValue(
		const FVoxelVectorSplineParameter& Parameter,
		UPARAM(meta = (SplineKeyPin)) const FVoxelFloatBuffer& SplineKey) const;
};