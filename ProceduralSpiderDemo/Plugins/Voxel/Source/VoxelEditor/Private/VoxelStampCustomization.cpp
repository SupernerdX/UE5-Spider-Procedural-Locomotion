// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampCustomization.h"
#include "VoxelStamp.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"

DEFINE_VOXEL_STRUCT_LAYOUT_RECURSIVE(FVoxelStamp, FVoxelStampCustomization);

void FVoxelStampCustomization::CustomizeChildren(
	const TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FVoxelEditorUtilities::EnableRealtime();

	uint32 NumChildren = 0;
	PropertyHandle->GetNumChildren(NumChildren);

	TVoxelArray<TSharedPtr<IPropertyHandle>> AdvancedHandles;
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ChildIndex++)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex);
		if (!ChildHandle ||
			!ChildHandle->IsValidHandle())
		{
			continue;
		}

		const FProperty* Property = ChildHandle->GetProperty();

		if (Property)
		{
			if (Property->GetOwnerStruct() == StaticStructFast<FVoxelStamp>() ||
				Property->GetOwnerStruct() == StaticStructFast<FVoxelHeightStamp>() ||
				Property->GetOwnerStruct() == StaticStructFast<FVoxelVolumeStamp>())
			{
				continue;
			}
		}

		if (Property &&
			Property->HasAnyPropertyFlags(CPF_AdvancedDisplay))
		{
			AdvancedHandles.Add(ChildHandle);
			continue;
		}

		CustomizePropertyRow(
			PropertyHandle, 
			ChildHandle->GetProperty(),
			ChildBuilder.AddProperty(ChildHandle.ToSharedRef()));
	}

	if (AdvancedHandles.Num() > 0)
	{
		IDetailCategoryBuilder& ParentCategory = ChildBuilder.GetParentCategory();
		for (const TSharedPtr<IPropertyHandle>& Handle : AdvancedHandles)
		{
			CustomizePropertyRow(
				PropertyHandle,
				Handle->GetProperty(),
				ParentCategory.AddProperty(Handle.ToSharedRef(), EPropertyLocation::Advanced));
		}
	}
}