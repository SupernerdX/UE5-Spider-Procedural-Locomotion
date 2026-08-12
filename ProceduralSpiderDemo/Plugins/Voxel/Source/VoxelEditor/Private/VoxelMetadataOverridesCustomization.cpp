// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelMetadataOverrides.h"

VOXEL_CUSTOMIZE_STRUCT_CHILDREN(FVoxelMetadataOverrides)(const TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ChildBuilder.AddProperty(PropertyHandle->GetChildHandleStatic(FVoxelMetadataOverrides, Overrides))
	.DisplayName(PropertyHandle->GetPropertyDisplayName())
	.ToolTip(PropertyHandle->GetToolTipText())
	.ShouldAutoExpand(true);
}