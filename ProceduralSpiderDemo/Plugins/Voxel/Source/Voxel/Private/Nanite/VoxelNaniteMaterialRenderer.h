// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Surface/VoxelSurfaceType.h"
#include "MegaMaterial/VoxelRenderMaterial.h"

struct FVoxelRenderSubsystem;
class FVoxelNaniteMesh;
class FVoxelMegaMaterialProxy;
class FVoxelNaniteMaterialRendererImpl;

class FVoxelNaniteMaterialRenderer : public TSharedFromThis<FVoxelNaniteMaterialRenderer>
{
public:
	explicit FVoxelNaniteMaterialRenderer(const TSharedRef<FVoxelMegaMaterialProxy>& MegaMaterialProxy);

public:
	TSharedPtr<FVoxelMaterialInstanceRef> GetMaterialInstance(FVoxelMaterialRenderIndex RenderIndex) const;

	void PrepareRender(TVoxelSet<TSharedPtr<const FVoxelNaniteMesh>>&& NewMeshes);

	void UpdateRender(
		const FVoxelRenderSubsystem& Subsystem,
		const FTransform& NewLocalToWorld);

private:
	const TSharedRef<FVoxelNaniteMaterialRendererImpl> Impl;

	TVoxelSet<TSharedPtr<const FVoxelNaniteMesh>> Meshes;
	TVoxelArray<FVoxelSurfaceType> UsedSurfaceTypes;
};