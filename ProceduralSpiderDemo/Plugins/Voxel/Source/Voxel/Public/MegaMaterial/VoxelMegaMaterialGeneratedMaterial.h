// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "MegaMaterial/VoxelMegaMaterialState.h"
#include "VoxelMegaMaterialGeneratedMaterial.generated.h"

class FGlobalComponentRecreateRenderStateContext;

USTRUCT()
struct FVoxelMegaMaterialTargetGeneratedMaterial
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UMaterial> Material;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceConstant> Instance;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FVoxelMegaMaterialTargetState State;
#endif

#if WITH_EDITOR
	void Update(
		const UObject& ErrorOwner,
		const FVoxelMegaMaterialTargetState& NewState,
		TSharedPtr<FGlobalComponentRecreateRenderStateContext>& Context);
#endif
};

USTRUCT()
struct FVoxelMegaMaterialMaterialGeneratedMaterial
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UMaterial> Material;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceConstant> Instance;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FVoxelMegaMaterialMaterialState State;
#endif

#if WITH_EDITOR
	void Update(
		const UObject& ErrorOwner,
		const FVoxelMegaMaterialMaterialState& NewState,
		TSharedPtr<FGlobalComponentRecreateRenderStateContext>& Context);
#endif
};