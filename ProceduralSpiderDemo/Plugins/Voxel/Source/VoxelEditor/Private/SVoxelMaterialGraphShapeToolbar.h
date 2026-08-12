// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "SViewportToolBar.h"
#include "Surface/VoxelSmartSurfacePreviewShape.h"

class SVoxelMaterialGraphShapeToolbar : public SViewportToolBar
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ATTRIBUTE(EVoxelSmartSurfacePreviewShape, Type)
		SLATE_EVENT(TDelegate<void(EVoxelSmartSurfacePreviewShape)>, OnTypeChanged)
	};

	void Construct(const FArguments& Args);

#if VOXEL_ENGINE_VERSION >= 506
	static void ExtendToolbar(
		UToolMenu& ToolMenu,
		TDelegate<void(EVoxelSmartSurfacePreviewShape)> OnTypeChanged,
		TDelegate<EVoxelSmartSurfacePreviewShape()> GetType);
#endif

private:
	TAttribute<EVoxelSmartSurfacePreviewShape> TypeAttribute;
	TDelegate<void(EVoxelSmartSurfacePreviewShape)> OnTypeChanged;
};
