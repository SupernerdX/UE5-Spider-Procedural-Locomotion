// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

UENUM()
enum class EVoxelMegaMaterialTarget : uint8
{
	NonNanite,
	NaniteWPO,
	NaniteDisplacement,
	NaniteMaterialSelection,
	Lumen,

	Max UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EVoxelMegaMaterialTarget, EVoxelMegaMaterialTarget::Max);

FORCEINLINE const TCHAR* LexToString(const EVoxelMegaMaterialTarget Target)
{
	switch (Target)
	{
	default: ensure(false);
	case EVoxelMegaMaterialTarget::NonNanite: return TEXT("NonNanite");
	case EVoxelMegaMaterialTarget::NaniteWPO: return TEXT("NaniteWPO");
	case EVoxelMegaMaterialTarget::NaniteDisplacement: return TEXT("NaniteDisplacement");
	case EVoxelMegaMaterialTarget::NaniteMaterialSelection: return TEXT("NaniteMaterialSelection");
	case EVoxelMegaMaterialTarget::Lumen: return TEXT("Lumen");
	}
}