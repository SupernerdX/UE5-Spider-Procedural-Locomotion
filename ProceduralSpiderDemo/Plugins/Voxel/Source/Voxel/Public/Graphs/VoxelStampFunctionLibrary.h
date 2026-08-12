// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelStampBehavior.h"
#include "VoxelHeightBlendMode.h"
#include "VoxelVolumeBlendMode.h"
#include "VoxelFunctionLibrary.h"
#include "VoxelFloatMetadataRef.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Surface/VoxelSurfaceTypeBuffer.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "VoxelStampFunctionLibrary.generated.h"

UCLASS()
class VOXEL_API UVoxelStampFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Surface Type")
	FVoxelSurfaceTypeBlendBuffer BlendSurfaceTypes(
		const FVoxelSurfaceTypeBlendBuffer& A,
		const FVoxelSurfaceTypeBlendBuffer& B,
		const FVoxelFloatBuffer& Alpha) const;

	// Returns the weight of Type in Blend
	// Will be 0 if the surface is not present
	UFUNCTION(Category = "Surface Type")
	FVoxelFloatBuffer GetSurfaceTypeWeight(
		const FVoxelSurfaceTypeBlendBuffer& Blend,
		const FVoxelSurfaceTypeBuffer& Type) const;

public:
	UFUNCTION(Category = "Stamp", meta = (AllowList = "Height,Volume,HeightSpline,VolumeSpline"))
	FVoxelSeed GetStampSeed() const;

public:
	UFUNCTION(Category = "Stamp", meta = (AllowList = "Height"))
	float GetHeightSmoothness() const;

	UFUNCTION(Category = "Stamp", meta = (AllowList = "Height"))
	EVoxelHeightBlendMode GetHeightBlendMode() const;

	UFUNCTION(Category = "Stamp", meta = (AllowList = "Height"))
	bool IsHeightOverrideBlendMode() const;

public:
	UFUNCTION(Category = "Stamp", meta = (AllowList = "Volume"))
	float GetVolumeSmoothness() const;

	UFUNCTION(Category = "Stamp", meta = (AllowList = "Volume"))
	EVoxelVolumeBlendMode GetVolumeBlendMode() const;

	UFUNCTION(Category = "Stamp", meta = (AllowList = "Volume"))
	bool IsVolumeOverrideBlendMode() const;

public:
	UFUNCTION(Category = "Stamp")
	void QueryHeight(
		UPARAM(DisplayName = "Position (World Space)") const FVoxelDoubleVector2DBuffer& Position,
		const FVoxelWeakStackHeightLayer& Layer,
		FVoxelFloatBuffer& Height,
		bool& bIsValid) const;

	UFUNCTION(Category = "Stamp")
	void QueryDistance(
		UPARAM(DisplayName = "Position (World Space)") const FVoxelDoubleVectorBuffer& Position,
		const FVoxelWeakStackVolumeLayer& Layer,
		FVoxelFloatBuffer& Distance,
		bool& bIsValid) const;

public:
	UFUNCTION(Category = "Stamp")
	void QueryHeightSurfaceType(
		UPARAM(DisplayName = "Position (World Space)") const FVoxelDoubleVector2DBuffer& Position,
		const FVoxelWeakStackHeightLayer& Layer,
		FVoxelFloatBuffer& Height,
		FVoxelSurfaceTypeBlendBuffer& SurfaceType,
		bool& bIsValid) const;

	UFUNCTION(Category = "Stamp")
	void QueryVolumeSurfaceType(
		UPARAM(DisplayName = "Position (World Space)") const FVoxelDoubleVectorBuffer& Position,
		const FVoxelWeakStackVolumeLayer& Layer,
		FVoxelFloatBuffer& Distance,
		FVoxelSurfaceTypeBlendBuffer& SurfaceType,
		bool& bIsValid) const;

public:
	UFUNCTION(Category = "Stamp")
	void QueryHeightMetadata(
		UPARAM(DisplayName = "Position (World Space)") const FVoxelDoubleVector2DBuffer& Position,
		const FVoxelWeakStackHeightLayer& Layer,
		const FVoxelFloatMetadataRef& Metadata,
		FVoxelFloatBuffer& Height,
		FVoxelFloatBuffer& Value,
		bool& bIsValid) const;

	UFUNCTION(Category = "Stamp")
	void QueryVolumeMetadata(
		UPARAM(DisplayName = "Position (World Space)") const FVoxelDoubleVectorBuffer& Position,
		const FVoxelWeakStackVolumeLayer& Layer,
		const FVoxelFloatMetadataRef& Metadata,
		FVoxelFloatBuffer& Distance,
		FVoxelFloatBuffer& Value,
		bool& bIsValid) const;

public:
	// Returns true if there are any stamps affecting the given layer within the bounds of the provided positions
	// Use this for per-chunk early-out optimizations (e.g., skip expensive gradient computation if no stamps exist)
	// BehaviorMask is a bitmask; if "Affect All" is selected, any stamp will qualify, not only stamps which Affect All
	UFUNCTION(Category = "Stamp")
	bool HasStampsInBounds(
		UPARAM(DisplayName = "Position (World Space)") const FVoxelDoubleVectorBuffer& Position,
		const FVoxelWeakStackLayer& Layer,
		EVoxelStampBehavior BehaviorMask = EVoxelStampBehavior::AffectAll) const;
};