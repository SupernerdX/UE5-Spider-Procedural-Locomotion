// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "Surface/VoxelCubicTextureTemplate.h"

VOXEL_CUSTOMIZE_CLASS(UVoxelCubicTextureTemplate)(IDetailLayoutBuilder& DetailLayout)
{
	// Hide default array properties - we'll show custom UI instead
	DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicTextureTemplate, TextureSlots));
	DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicTextureTemplate, ScalarSlots));
	DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicTextureTemplate, VectorSlots));

	const TSharedRef<IPropertyHandle> TextureSlotsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicTextureTemplate, TextureSlots));
	const TSharedRef<IPropertyHandle> ScalarSlotsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicTextureTemplate, ScalarSlots));
	const TSharedRef<IPropertyHandle> VectorSlotsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicTextureTemplate, VectorSlots));

	const TSharedPtr<IPropertyUtilities> PropUtilities = DetailLayout.GetPropertyUtilities();

	// Helper to build slot section UI
	const auto BuildSlotSection = [&](
		IDetailCategoryBuilder& Category,
		const TSharedRef<IPropertyHandle>& ArrayHandle,
		const FText& SlotTypeName,
		const FName NamePropertyName,
		const FName ValuePropertyName)
	{
		if (!ArrayHandle->IsValidHandle())
		{
			return;
		}

		const TSharedPtr<IPropertyHandleArray> ArrayHandleArray = ArrayHandle->AsArray();
		if (!ArrayHandleArray)
		{
			return;
		}

		uint32 NumElements = 0;
		ArrayHandleArray->GetNumElements(NumElements);

		// Add a row for each slot
		for (uint32 Index = 0; Index < NumElements; Index++)
		{
			const TSharedPtr<IPropertyHandle> ElementHandle = ArrayHandle->GetChildHandle(Index);
			if (!ElementHandle.IsValid())
			{
				continue;
			}

			const TSharedPtr<IPropertyHandle> NameHandle = ElementHandle->GetChildHandle(NamePropertyName);
			const TSharedPtr<IPropertyHandle> ValueHandle = ElementHandle->GetChildHandle(ValuePropertyName);

			if (!NameHandle ||
				!NameHandle->IsValidHandle() ||
				!ValueHandle ||
				!ValueHandle->IsValidHandle())
			{
				continue;
			}

			Category
			.AddCustomRow(FText::Format(INVTEXT("{0} {1}"), SlotTypeName, FText::AsNumber(Index)))
			.NameContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					NameHandle->CreatePropertyValueWidget()
				]
			]
			.ValueContent()
			.MinDesiredWidth(200.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					ValueHandle->CreatePropertyValueWidget()
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.f, 0.f, 0.f, 0.f)
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.ContentPadding(0)
					.ToolTipText(INVTEXT("Remove this slot"))
					.OnClicked_Lambda([ArrayHandleArray, Index, PropUtilities]
					{
						ArrayHandleArray->DeleteItem(Index);
						PropUtilities->RequestForceRefresh();
						return FReply::Handled();
					})
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.Delete"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			];
		}

		// Add "Add Slot" button at the bottom
		Category
		.AddCustomRow(FText::Format(INVTEXT("Add {0}"), SlotTypeName))
		.WholeRowContent()
		.HAlign(HAlign_Left)
		[
			SNew(SBox)
			.Padding(FMargin(0.f, 4.f, 0.f, 4.f))
			[
				SNew(SButton)
				.Text(FText::Format(INVTEXT("+ Add {0}"), SlotTypeName))
				.OnClicked_Lambda([ArrayHandleArray, PropUtilities]
				{
					uint32 NumElements = 0;
					ArrayHandleArray->GetNumElements(NumElements);
					ArrayHandleArray->AddItem();
					PropUtilities->RequestForceRefresh();
					return FReply::Handled();
				})
			]
		];
	};

	// Texture Slots section
	{
		IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Texture Slots", INVTEXT("Texture Slots"), ECategoryPriority::Important);
		BuildSlotSection(
			Category,
			TextureSlotsHandle,
			INVTEXT("Texture Slot"),
			GET_MEMBER_NAME_STATIC(FVoxelCubicTextureSlot, Name),
			GET_MEMBER_NAME_STATIC(FVoxelCubicTextureSlot, DefaultTexture));
	}

	// Scalar Slots section
	{
		IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Scalar Slots", INVTEXT("Scalar Slots"), ECategoryPriority::Important);
		BuildSlotSection(
			Category,
			ScalarSlotsHandle,
			INVTEXT("Scalar Slot"),
			GET_MEMBER_NAME_STATIC(FVoxelCubicScalarSlot, Name),
			GET_MEMBER_NAME_STATIC(FVoxelCubicScalarSlot, DefaultValue));
	}

	// Vector Slots section
	{
		IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Vector Slots", INVTEXT("Vector Slots"), ECategoryPriority::Important);
		BuildSlotSection(
			Category,
			VectorSlotsHandle,
			INVTEXT("Vector Slot"),
			GET_MEMBER_NAME_STATIC(FVoxelCubicVectorSlot, Name),
			GET_MEMBER_NAME_STATIC(FVoxelCubicVectorSlot, DefaultValue));
	}

	// Material category comes after slots
	DetailLayout.EditCategory("Material", INVTEXT("Material"), ECategoryPriority::Default);
	DetailLayout.EditCategory("Meshing", INVTEXT("Meshing"), ECategoryPriority::Default);
	DetailLayout.EditCategory("Rendering", INVTEXT("Rendering"), ECategoryPriority::Default);
}