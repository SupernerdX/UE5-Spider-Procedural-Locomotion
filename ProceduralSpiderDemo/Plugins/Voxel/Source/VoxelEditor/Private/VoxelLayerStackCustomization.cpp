// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelLayer.h"
#include "VoxelLayerStack.h"

VOXEL_CONSOLE_VARIABLE(
	VOXELEDITOR_API, bool, GVoxelAllowDefaultLayersEdits, false,
	"voxel.AllowDefaultLayersEdits",
	"Allow editing default layer assets");

VOXEL_CUSTOMIZE_CLASS(UVoxelLayerStack)(IDetailLayoutBuilder& DetailLayout)
{
	if (GVoxelAllowDefaultLayersEdits)
	{
		return;
	}

	TVoxelArray<UVoxelLayerStack*> Objects = GetObjectsBeingCustomized(DetailLayout);
	if (Objects.Num() != 1)
	{
		return;
	}

	UVoxelLayerStack* Object = Objects[0];
	if (!ensure(Object) ||
		!Object->GetPathName().StartsWith("/Voxel/"))
	{
		return;
	}

	TArray<FName> CategoryNames;
	DetailLayout.GetCategoryNames(CategoryNames);

		{
		IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Info");
		Category.AddCustomRow(INVTEXT("Disabled"))
		.WholeRowContent()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.f)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.Warning"))
				.ColorAndOpacity(FStyleColors::Warning)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailLayout.GetDetailFont())
				.Text(INVTEXT("This is a default Voxel Plugin asset, so it cannot be edited. To make changes to your stack, make a new asset."))
			]
		];
		}

	for (const FName CategoryName : CategoryNames)
	{
		IDetailCategoryBuilder& Category = DetailLayout.EditCategory(CategoryName);
		TArray<TSharedRef<IPropertyHandle>> PropertyHandles;
		Category.GetDefaultProperties(PropertyHandles);

		for (const TSharedRef<IPropertyHandle>& PropertyHandle : PropertyHandles)
		{
			if (!PropertyHandle->IsValidHandle())
			{
				continue;
			}

			Category.AddProperty(PropertyHandle)
			.EditCondition(false, {});
		}
	}
}

VOXEL_CUSTOMIZE_CLASS(UVoxelLayer)(IDetailLayoutBuilder& DetailLayout)
{
	if (GVoxelAllowDefaultLayersEdits)
	{
		return;
	}

	TVoxelArray<UVoxelLayer*> Objects = GetObjectsBeingCustomized(DetailLayout);
	if (Objects.Num() != 1)
	{
		return;
	}

	UVoxelLayer* Object = Objects[0];
	if (!ensure(Object) ||
		!Object->GetPathName().StartsWith("/Voxel/"))
	{
		return;
	}

	TArray<FName> CategoryNames;
	DetailLayout.GetCategoryNames(CategoryNames);

		{
		IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Info");
		Category.AddCustomRow(INVTEXT("Disabled"))
		.WholeRowContent()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.f)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.Warning"))
				.ColorAndOpacity(FStyleColors::Warning)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailLayout.GetDetailFont())
				.Text(INVTEXT("This is a default Voxel Plugin asset, so it cannot be edited. To make changes to your layers, make a new asset."))
			]
		];
		}

	for (const FName CategoryName : CategoryNames)
	{
		IDetailCategoryBuilder& Category = DetailLayout.EditCategory(CategoryName);
		TArray<TSharedRef<IPropertyHandle>> PropertyHandles;
		Category.GetDefaultProperties(PropertyHandles);

		for (const TSharedRef<IPropertyHandle>& PropertyHandle : PropertyHandles)
		{
			if (!PropertyHandle->IsValidHandle())
			{
				continue;
			}

			Category.AddProperty(PropertyHandle)
			.EditCondition(false, {});
		}
	}
}