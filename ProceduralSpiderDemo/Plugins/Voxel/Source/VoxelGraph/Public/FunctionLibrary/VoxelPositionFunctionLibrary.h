// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFunctionLibrary.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelGraphPositionParameter.h"
#include "VoxelPositionFunctionLibrary.generated.h"

UENUM(BlueprintType, meta = (OverrideEnumSize = 100))
enum class EVoxelPositionSpace : uint8
{
	LocalSpace,
	WorldSpace
};

UCLASS()
class VOXELGRAPH_API UVoxelPositionFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Misc", meta = (ShowInPinShortList, NotTemplate))
	FVoxelVector2DBuffer GetPosition2D(
		UPARAM(meta = (HidePinLabel)) EVoxelPositionSpace Space) const;

	UFUNCTION(Category = "Misc", meta = (ShowInPinShortList, NotTemplate))
	FVoxelVectorBuffer GetPosition3D(
		UPARAM(meta = (HidePinLabel)) EVoxelPositionSpace Space,
		UPARAM(meta = (AdvancedDisplay)) bool bFallbackTo2D = false) const;

public:
	UFUNCTION(Category = "Misc", DisplayName = "Get Position 2D (double)", meta = (ShowInPinShortList, NotTemplate))
	FVoxelDoubleVector2DBuffer GetPosition2D_Double(
		UPARAM(meta = (HidePinLabel)) EVoxelPositionSpace Space) const;

	UFUNCTION(Category = "Misc", DisplayName = "Get Position 3D (double)", meta = (ShowInPinShortList, NotTemplate))
	FVoxelDoubleVectorBuffer GetPosition3D_Double(
		UPARAM(meta = (HidePinLabel)) EVoxelPositionSpace Space,
		UPARAM(meta = (AdvancedDisplay)) bool bFallbackTo2D = false) const;
};