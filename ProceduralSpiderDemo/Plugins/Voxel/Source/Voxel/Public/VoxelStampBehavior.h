// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampBehavior.generated.h"

UENUM(BlueprintType, meta = (Bitflags, VoxelSegmentedEnum, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EVoxelStampBehavior : uint8
{
	None = 0 UMETA(HideInUI),

	// Affect Shape
	AffectShape = 1 << 0 UMETA(Icon = "ClassIcon.Cube"),
	// Affect Surface Type
	AffectSurfaceType = 1 << 1 UMETA(Icon = "ClassIcon.Material"),
	// Affect Metadata
	AffectMetadata = 1 << 2 UMETA(Icon = "Kismet.Tabs.Palette"),

	AffectAll = AffectShape | AffectSurfaceType | AffectMetadata UMETA(HideInUI),
	AffectShapeAndSurfaceType = AffectShape | AffectSurfaceType UMETA(HideInUI),
	AffectShapeAndMetadata = AffectShape | AffectMetadata UMETA(HideInUI),
	AffectSurfaceTypeAndMetadata = AffectSurfaceType | AffectMetadata UMETA(HideInUI),
};
ENUM_CLASS_FLAGS(EVoxelStampBehavior);