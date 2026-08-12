// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class VOXELEDITOR_API FVoxelStampCustomization : public FVoxelPropertyTypeCustomizationBase
{
public:
	virtual void CustomizeHeader(
		TSharedRef<IPropertyHandle> PropertyHandle,
		FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
	}

	virtual void CustomizeChildren(
		TSharedRef<IPropertyHandle> PropertyHandle,
		IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

public:
	virtual void CustomizePropertyRow(
		TSharedRef<IPropertyHandle> BasePropertyHandle,
		FProperty* Property,
		IDetailPropertyRow& PropertyRow) {}
};