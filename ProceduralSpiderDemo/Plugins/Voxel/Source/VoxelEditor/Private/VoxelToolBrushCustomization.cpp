// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "Sculpt/VoxelToolBrush.h"

class FVoxelToolBrushCustomization : public FVoxelPropertyTypeCustomizationBase
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
	}

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		uint32 NumChildren = 0;
		ensure(PropertyHandle->GetNumChildren(NumChildren) == FPropertyAccess::Success);

		for (uint32 Index = 0; Index < NumChildren; Index++)
		{
			TSharedPtr<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(Index);
			if (!ChildHandle ||
				!ChildHandle->IsValidHandle())
			{
				continue;
			}

			ChildBuilder.AddProperty(ChildHandle.ToSharedRef());
		}
	}
};

DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelToolBrush, FVoxelToolBrushCustomization);
DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelCircularBrush, FVoxelToolBrushCustomization);
DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelAlphaBrush, FVoxelToolBrushCustomization);
DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelPatternBrush, FVoxelToolBrushCustomization);