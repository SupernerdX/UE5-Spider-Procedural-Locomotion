// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "PCGCallVoxelGraph.h"
#include "SVoxelGraphPinTypeComboBox.h"

VOXEL_CUSTOMIZE_STRUCT_HEADER(FVoxelPCGObjectAttributeType)(const TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow
	.NameContent()
	.MinDesiredWidth(350.f)
	.MaxDesiredWidth(350.f)
	[
		SNew(SBox)
		.MinDesiredWidth(350.f)
		.HAlign(HAlign_Fill)
		[
			PropertyHandle->GetChildHandleStatic(FVoxelPCGObjectAttributeType, Attribute)->CreatePropertyValueWidgetWithCustomization(nullptr)
		]
	]
	.ValueContent()
	.MinDesiredWidth(125.f)
	.MaxDesiredWidth(125.f)
	[
		SNew(SVoxelPinTypeComboBox)
		.AllowedTypes_Lambda([]
		{
			FVoxelPinTypeSet PinTypes;
			FVoxelPinTypeSet::AllObjects().Iterate([&](const FVoxelPinType& Type)
			{
				if (Type.IsBuffer() ||
					Type.Is<FVoxelSoftObjectPath>() ||
					Type.Is<FVoxelSoftClassPath>())
				{
					return;
				}

				PinTypes.Add(Type);
			});
			return PinTypes;
		})
		.OnTypeChanged_Lambda([=](const FVoxelPinType& Type)
		{
			FVoxelEditorUtilities::SetStructPropertyValue(PropertyHandle->GetChildHandleStatic(FVoxelPCGObjectAttributeType, Type), Type);
		})
		.CurrentType_Lambda([WeakHandle = MakeWeakPtr(PropertyHandle)]() -> FVoxelPinType
		{
			const TSharedPtr<IPropertyHandle> Handle = WeakHandle.Pin();
			if (!Handle)
			{
				return {};
			}

			return FVoxelEditorUtilities::GetStructPropertyValue<FVoxelPinType>(Handle->GetChildHandleStatic(FVoxelPCGObjectAttributeType, Type));
		})
	];
}