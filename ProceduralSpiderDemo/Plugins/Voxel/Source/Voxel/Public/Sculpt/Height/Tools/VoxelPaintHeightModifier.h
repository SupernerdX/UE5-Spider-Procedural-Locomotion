// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataOverrides.h"
#include "Surface/VoxelSurfaceType.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Sculpt/VoxelSculptMode.h"
#include "Sculpt/Height/VoxelHeightModifier.h"
#include "VoxelPaintHeightModifier.generated.h"

class FVoxelToolRuntimeBrush;

USTRUCT()
struct VOXEL_API FVoxelPaintHeightModifier : public FVoxelHeightModifier
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY()
	FVector2D Center = FVector2D(ForceInit);

	UPROPERTY()
	TObjectPtr<UVoxelSurfaceTypeInterface> SurfaceTypeToPaint;

	UPROPERTY()
	FVoxelMetadataOverrides MetadatasToPaint;

	UPROPERTY()
	float Radius = 0.f;

	UPROPERTY()
	float Strength = 0.f;

	UPROPERTY()
	EVoxelSculptMode Mode = {};

	UPROPERTY()
	FVoxelToolBrush Brush;

public:
	//~ Begin FVoxelHeightModifier Interface
	virtual TSharedPtr<IVoxelHeightModifierRuntime> GetRuntime() const override;
	//~ End FVoxelHeightModifier Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelPaintHeightModifierRuntime : public IVoxelHeightModifierRuntime
{
public:
	const FVector2D Center;
	const float Radius;
	const float Strength;
	const EVoxelSculptMode Mode;
	const FVoxelSurfaceType SurfaceType;
	const TSharedRef<FVoxelRuntimeMetadataOverrides> MetadataOverrides;
	const TSharedRef<const FVoxelToolRuntimeBrush> Brush;

	FVoxelPaintHeightModifierRuntime(
		const FVector2D& Center,
		const float Radius,
		const float Strength,
		const EVoxelSculptMode Mode,
		const FVoxelSurfaceType& SurfaceType,
		const TSharedRef<FVoxelRuntimeMetadataOverrides>& MetadataOverrides,
		const TSharedRef<const FVoxelToolRuntimeBrush>& Brush)
		: Center(Center)
		, Radius(Radius)
		, Strength(Strength)
		, Mode(Mode)
		, SurfaceType(SurfaceType)
		, MetadataOverrides(MetadataOverrides)
		, Brush(Brush)
	{
	}

public:
	//~ Begin IVoxelHeightModifierRuntime Interface
	virtual FVoxelBox2D GetBounds() const override;
	virtual void Apply(FVoxelHeightSculptCanvasWriter& Writer) const override;
	//~ End IVoxelHeightModifierRuntime Interface
};
