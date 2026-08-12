// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelBuffer.h"
#include "Texture/VoxelTexture.h"
#include "Texture/VoxelTextureData.h"

VOXEL_CUSTOMIZE_CLASS(UVoxelTexture)(IDetailLayoutBuilder& DetailLayout)
{
	DetailLayout.EditCategory("Config")
	.AddCustomRow(INVTEXT("Buffer Type"))
	.NameContent()
	[
		SNew(SVoxelDetailText)
		.Text(INVTEXT("Buffer Type"))
	]
	.ValueContent()
	[
		SNew(SVoxelDetailText)
		.Text_Lambda([WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout)]
		{
			if (!ensure(WeakObjects.Num() == 1))
			{
				return FText();
			}

			const UVoxelTexture* Object = WeakObjects[0].Resolve();
			if (!ensure(Object))
			{
				return FText();
			}

			const TSharedPtr<const FVoxelTextureData> Data = Object->GetData();
			if (!Data ||
				!ensure(Data->Buffer))
			{
				return INVTEXT("unset");
			}

			return Data->Buffer->GetStruct()->GetDisplayNameText();
		})
	];

	DetailLayout.EditCategory("Config")
	.AddCustomRow(INVTEXT("Size"))
	.NameContent()
	[
		SNew(SVoxelDetailText)
		.Text(INVTEXT("Size"))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SVoxelDetailText)
			.Text_Lambda([WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout)]
			{
				if (!ensure(WeakObjects.Num() == 1))
				{
					return FText();
				}

				const UVoxelTexture* Object = WeakObjects[0].Resolve();
				if (!ensure(Object))
				{
					return FText();
				}

				const TSharedPtr<const FVoxelTextureData> Data = Object->GetData();
				if (!Data ||
					!ensure(Data->Buffer))
				{
					return INVTEXT("unset");
				}

				return FText::FromString(FString::Printf(TEXT("%d x %d"), Data->SizeX, Data->SizeY));
			})
		]
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.Padding(8.f, 0.f, 0.f, 0.f)
		.AutoWidth()
		[
			SNew(SBox)
			.Visibility_Lambda([WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout)]
			{
				if (!ensure(WeakObjects.Num() == 1))
				{
					return EVisibility::Collapsed;
				}

				const UVoxelTexture* Object = WeakObjects[0].Resolve();
				if (!ensure(Object))
				{
					return EVisibility::Collapsed;
				}

				const TSharedPtr<const FVoxelTextureData> Data = Object->GetData();
				if (!Data ||
					!ensure(Data->Buffer))
				{
					return EVisibility::Collapsed;
				}

				if (FMath::IsPowerOfTwo(Data->SizeX) &&
					FMath::IsPowerOfTwo(Data->SizeY))
				{
					return EVisibility::Collapsed;
				}

				return EVisibility::Visible;
			})
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Warning"))
				]
				+ SHorizontalBox::Slot()
				.Padding(4.f, 0.f, 0.f, 0.f)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(SVoxelDetailText)
					.Text(INVTEXT("Sampling non-power-of-two voxel textures is 2-5x slower"))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			]
		]
	];

	DetailLayout.EditCategory("Config")
	.AddCustomRow(INVTEXT("Allocated Size"))
	.NameContent()
	[
		SNew(SVoxelDetailText)
		.Text(INVTEXT("Allocated Size"))
	]
	.ValueContent()
	[
		SNew(SVoxelDetailText)
		.Text_Lambda([WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout)]
		{
			if (!ensure(WeakObjects.Num() == 1))
			{
				return FText();
			}

			const UVoxelTexture* Object = WeakObjects[0].Resolve();
			if (!ensure(Object))
			{
				return FText();
			}

			const TSharedPtr<const FVoxelTextureData> Data = Object->GetData();
			if (!Data ||
				!ensure(Data->Buffer))
			{
				return INVTEXT("unset");
			}

			return FVoxelUtilities::BytesToText(Data->Buffer->GetAllocatedSize());
		})
	];
}