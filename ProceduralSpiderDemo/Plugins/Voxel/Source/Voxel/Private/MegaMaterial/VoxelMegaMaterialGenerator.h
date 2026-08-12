// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "MaterialExpressionIO.h"
#include "MegaMaterial/VoxelMegaMaterialGeneratedData.h"

#if WITH_EDITOR
class FVoxelMegaMaterialGenerator
{
public:
	static bool GenerateMaterialForTarget(
		TVoxelObjectPtr<const UObject> ErrorOwner,
		const FVoxelMegaMaterialTargetState& State,
		UMaterial& NewMaterial,
		UMaterialInstanceConstant& NewMaterialInstance);

	static bool GenerateMaterial(
		TVoxelObjectPtr<const UObject> ErrorOwner,
		const FVoxelMegaMaterialMaterialState& State,
		UMaterial& NewMaterial,
		UMaterialInstanceConstant& NewMaterialInstance);

	static bool ApplyBlendSmoothness(
		const TMap<FVoxelMaterialRenderIndex, FVoxelSurfaceTypeState>& IndexToSurfaceType,
		const UMaterial& NewMaterial,
		UMaterialInstanceConstant& MaterialInstance);

public:
	static TVoxelArray<UVoxelMetadata*> GetUsedMetadatas(const UMaterial& Material);
	static TVoxelArray<UVoxelMetadata*> GetUsedMetadatas(const UMaterialFunction& MaterialFunction);

private:
	static bool AddAttributePostProcess(
		TVoxelObjectPtr<const UObject> ErrorOwner,
		const FVoxelMegaMaterialState& MegaMaterial,
		UMaterial& Material);

	static void WrapInComment(
		UMaterial& Material,
		const FVoxelIntBox2D& Bounds,
		const FString& Comment);

	static bool ShouldDuplicateFunction(const UMaterialExpression& Expression);
};
#endif