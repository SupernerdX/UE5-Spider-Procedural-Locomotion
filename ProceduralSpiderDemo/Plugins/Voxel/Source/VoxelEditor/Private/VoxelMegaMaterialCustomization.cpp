// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "MegaMaterial/VoxelMegaMaterialGeneratedData.h"
#include "AssetViewUtils.h"
#include "Materials/Material.h"

VOXEL_CUSTOMIZE_CLASS(UVoxelMegaMaterial)(IDetailLayoutBuilder& DetailLayout)
{
	// Top category
	DetailLayout.EditCategory("Materials");
	DetailLayout.EditCategory("Global");

	const TSharedRef<IPropertyHandle> SurfaceTypesHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelMegaMaterial, SurfaceTypes), UVoxelMegaMaterial::StaticClass());

	if (!SurfaceTypesHandle->IsExpanded())
	{
		SurfaceTypesHandle->SetExpanded(true);
	}

	const auto AddButton = [&](
		IDetailCategoryBuilder& CategoryBuilder,
		const EVoxelMegaMaterialTarget Target,
		auto IsEnabled)
	{
		CategoryBuilder
		.AddCustomRow(INVTEXT("Show Generated Material"))
		.Visibility(MakeAttributeLambda([IsEnabled, WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout)]() -> EVisibility
		{
			for (const TVoxelObjectPtr<UVoxelMegaMaterial>& WeakObject : WeakObjects)
			{
				const UVoxelMegaMaterial* Object = WeakObject.Resolve();
				if (!ensure(Object))
				{
					continue;
				}

				if (!IsEnabled(*Object))
				{
					return EVisibility::Collapsed;
				}
			}

			return EVisibility::Visible;
		}))
		.NameContent()
		[
			SNew(SVoxelDetailText)
			.Text(INVTEXT("Show Generated Material"))
		]
		.ValueContent()
		[
			SNew(SButton)
			.Text(INVTEXT("Show"))
			.HAlign(HAlign_Center)
			.OnClicked_Lambda([Target, WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout)]
			{
				for (const TVoxelObjectPtr<UVoxelMegaMaterial>& WeakObject : WeakObjects)
				{
					UVoxelMegaMaterial* Object = WeakObject.Resolve();
					if (!ensure(Object))
					{
						continue;
					}

					UMaterialInterface* Material = Object->GetProxy()->GetTargetMaterial(Target).Resolve();
					if (!Material)
					{
						continue;
					}

					AssetViewUtils::OpenEditorForAsset(Material);
				}

				return FReply::Handled();
			})
		];
	};

	{
		IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Non-Nanite");
		Category.AddProperty(GET_MEMBER_NAME_CHECKED(UVoxelMegaMaterial, NonNaniteMaterialType), UVoxelMegaMaterial::StaticClass());
		AddButton(Category, EVoxelMegaMaterialTarget::NonNanite, [](const UVoxelMegaMaterial& MegaMaterial)
		{
			return MegaMaterial.NonNaniteMaterialType == EVoxelMegaMaterialGenerationType::Generated;
		});
	}

	{
		IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Nanite");
		Category.AddProperty(GET_MEMBER_NAME_CHECKED(UVoxelMegaMaterial, NaniteDisplacementMaterialType), UVoxelMegaMaterial::StaticClass());
		AddButton(Category, EVoxelMegaMaterialTarget::NaniteMaterialSelection, [](const UVoxelMegaMaterial& MegaMaterial)
		{
			return MegaMaterial.NaniteDisplacementMaterialType == EVoxelMegaMaterialGenerationType::Generated;
		});
	}

	{
		IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Lumen");
		Category.AddProperty(GET_MEMBER_NAME_CHECKED(UVoxelMegaMaterial, LumenMaterialType), UVoxelMegaMaterial::StaticClass());
		AddButton(Category, EVoxelMegaMaterialTarget::Lumen, [](const UVoxelMegaMaterial& MegaMaterial)
		{
			return MegaMaterial.LumenMaterialType == EVoxelMegaMaterialGenerationType::Generated;
		});
	}

	IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Misc");

	Category
	.AddCustomRow(INVTEXT("Rebuild"))
	.NameContent()
	[
		SNew(SVoxelDetailText)
		.Text(INVTEXT("Trigger Rebuild"))
	]
	.ValueContent()
	[
		SNew(SButton)
		.Text(INVTEXT("Rebuild"))
		.ToolTipText(INVTEXT("Queue a rebuild of the generated material. This is done automatically, only use this if you are having issues"))
		.HAlign(HAlign_Center)
		.OnClicked_Lambda([WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout)]
		{
			for (const TVoxelObjectPtr<UVoxelMegaMaterial>& WeakObject : WeakObjects)
			{
				UVoxelMegaMaterial* Object = WeakObject.Resolve();
				if (!ensure(Object))
				{
					continue;
				}

				Object->GetGeneratedData_EditorOnly().ForceRebuild();
			}

			return FReply::Handled();
		})
	];

	const TSharedRef<int32> IndexToShow = MakeShared<int32>(1);

	Category
	.AddCustomRow(INVTEXT("Show Generated"))
	.NameContent()
	[
		SNew(SVoxelDetailText)
		.Text(INVTEXT("Show Generated Material"))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SVoxelDetailText)
			.Text(INVTEXT("Index to show"))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.MinDesiredWidth(75.f)
			.MaxDesiredWidth(75.f)
			.Padding(10, 0, 10, 0)
			[
				SNew(SSpinBox<int32>)
				.MinValue(1)
				.MaxValue_Lambda([WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout)]
				{
					int32 MaxIndex = MAX_int32;
					for (const TVoxelObjectPtr<UVoxelMegaMaterial>& WeakObject : WeakObjects)
					{
						const UVoxelMegaMaterial* Object = WeakObject.Resolve();
						if (!ensure(Object))
						{
							continue;
						}

						int32 ObjectMaxIndex;
						if (Object->CubicTextureTemplate)
						{
							// Cubic materials: index 1 = opaque/masked, index 2 = translucent
							ObjectMaxIndex = 2;
						}
						else
						{
							ObjectMaxIndex = Object->SurfaceTypes.Num() - 1;
						}
						MaxIndex = FMath::Min(MaxIndex, ObjectMaxIndex);
					}
					return MaxIndex;
				})
				.Value_Lambda([=]
				{
					return *IndexToShow;
				})
				.OnValueChanged_Lambda([=](const int32 NewValue)
				{
					*IndexToShow = NewValue;
				})
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(INVTEXT("Show"))
			.HAlign(HAlign_Center)
			.OnClicked_Lambda([IndexToShow, WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout)]
			{
				for (const TVoxelObjectPtr<UVoxelMegaMaterial>& WeakObject : WeakObjects)
				{
					UVoxelMegaMaterial* Object = WeakObject.Resolve();
					if (!ensure(Object))
					{
						continue;
					}

					const TVoxelMap<FVoxelMaterialRenderIndex, TVoxelObjectPtr<UMaterialInterface>> IndexToMaterial = Object->GetProxy()->GetMaterialIndexToMaterial();
					if (!ensureVoxelSlow(IndexToMaterial.Contains(FVoxelMaterialRenderIndex(*IndexToShow))))
					{
						continue;
					}

					UMaterialInterface* Material = IndexToMaterial[FVoxelMaterialRenderIndex(*IndexToShow)].Resolve_Ensured();
					if (!Material)
					{
						continue;
					}

					AssetViewUtils::OpenEditorForAsset(Material);
				}

				return FReply::Handled();
			})
		]
	];

}