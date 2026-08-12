// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "Shape/VoxelShapeStamp.h"
#include "VoxelStampCustomization.h"
#include "SVoxelShapeStampTypeSelector.h"
#include "Graphs/VoxelVolumeGraphStamp.h"

class FVoxelShapeStampCustomization : public FVoxelStampCustomization
{
public:
	virtual void CustomizeChildren(
		const TSharedRef<IPropertyHandle> PropertyHandle,
		IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		ChildBuilder.AddCustomRow(INVTEXT("ShapeType"))
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVoxelShapeStampTypeSelector)
			.Handle(PropertyHandle->GetChildHandleStatic(FVoxelShapeStamp, Shape))
			.PropertyUtilities(CustomizationUtils.GetPropertyUtilities())
		];

		FVoxelStampCustomization::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);
	}
};

DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelShapeStamp, FVoxelShapeStampCustomization);