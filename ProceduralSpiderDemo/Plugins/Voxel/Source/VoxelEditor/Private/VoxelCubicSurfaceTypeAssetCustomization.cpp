// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "Surface/VoxelCubicSurfaceTypeAsset.h"
#include "Surface/VoxelCubicTextureTemplate.h"
#include "Engine/Texture2D.h"

class FVoxelCubicSurfaceTypeAssetCustomization
	: public FVoxelDetailCustomization
{
public:
	static TVoxelArray<UVoxelCubicSurfaceTypeAsset*> GetObjectsBeingCustomized(IDetailLayoutBuilder& DetailLayout)
	{
		return FVoxelEditorUtilities::GetObjectsBeingCustomized<UVoxelCubicSurfaceTypeAsset>(DetailLayout);
	}

	static UVoxelCubicSurfaceTypeAsset* GetUniqueObjectBeingCustomized(IDetailLayoutBuilder& DetailLayout)
	{
		return FVoxelEditorUtilities::GetUniqueObjectBeingCustomized<UVoxelCubicSurfaceTypeAsset>(DetailLayout);
	}

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
	{
		// Hide default map properties - we'll show custom UI
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicSurfaceTypeAsset, Textures));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicSurfaceTypeAsset, Scalars));
		DetailLayout.HideProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicSurfaceTypeAsset, Vectors));

		RefreshDelegate = FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout);

		// Get the object being customized
		UVoxelCubicSurfaceTypeAsset* Asset = GetUniqueObjectBeingCustomized(DetailLayout);
		if (!Asset)
		{
			return;
		}

		// Ensure maps are synced with template before displaying
		Asset->SyncWithTemplate();

		// Cache asset and template for change detection
		CachedAsset = Asset;
		CachedTemplate = Asset->Template;

		// Config category - show Template picker
		IDetailCategoryBuilder& ConfigCategory = DetailLayout.EditCategory("Config", INVTEXT("Config"), ECategoryPriority::Important);

		// If no template, show info message
		if (!Asset->Template)
		{
			ConfigCategory
			.AddCustomRow(INVTEXT("No Template"))
			.WholeRowContent()
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.f)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Info"))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					SNew(SVoxelDetailText)
					.Text(INVTEXT("Select a Template to configure texture slots"))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			];

			return;
		}

		UVoxelCubicTextureTemplate* Template = Asset->Template;

		// Subscribe to template changes to refresh the UI
		TemplateChangedHandle = Template->OnTemplateChanged.AddSP(
			this,
			&FVoxelCubicSurfaceTypeAssetCustomization::OnTemplateChanged);

		// Get property handles
		const TSharedRef<IPropertyHandle> TexturesHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicSurfaceTypeAsset, Textures));
		const TSharedRef<IPropertyHandle> ScalarsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicSurfaceTypeAsset, Scalars));
		const TSharedRef<IPropertyHandle> VectorsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelCubicSurfaceTypeAsset, Vectors));

		// Texture Slots section
		if (Template->TextureSlots.Num() > 0)
		{
			IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Texture Slots", INVTEXT("Texture Slots"), ECategoryPriority::Default);

			for (const FVoxelCubicTextureSlot& Slot : Template->TextureSlots)
			{
				const TSharedPtr<IPropertyHandle> ValueHandle = FVoxelEditorUtilities::FindMapValuePropertyHandle(*TexturesHandle, Slot.Guid);
				if (!ValueHandle ||
					!ValueHandle->IsValidHandle())
				{
					continue;
				}

				const TSharedPtr<IPropertyHandle> TextureHandle = ValueHandle->GetChildHandleStatic(FVoxelCubicTextureEntry, Texture);
				if (!TextureHandle ||
					!TextureHandle->IsValidHandle())
				{
					continue;
				}

				const FGuid SlotGuid = Slot.Guid;

				Category
				.AddCustomRow(FText::FromName(Slot.Name))
				.OverrideResetToDefault(FResetToDefaultOverride::Create(
					MakeAttributeLambda(MakeWeakPtrLambda(this, [this, SlotGuid]
					{
						return CanResetTexture(SlotGuid);
					})),
					MakeWeakPtrDelegate(this, [this, SlotGuid, ValueHandle]
					{
						ResetTexture(SlotGuid, ValueHandle);
					}),
					false))
				.NameContent()
				[
					SNew(SVoxelDetailText)
					.Text(FText::FromName(Slot.Name))
				]
				.ValueContent()
				.MinDesiredWidth(200.f)
				[
					TextureHandle->CreatePropertyValueWidget()
				];
			}
		}

		// Scalar Slots section
		if (Template->ScalarSlots.Num() > 0)
		{
			IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Scalar Slots", INVTEXT("Scalar Slots"), ECategoryPriority::Default);

			for (const FVoxelCubicScalarSlot& Slot : Template->ScalarSlots)
			{
				const TSharedPtr<IPropertyHandle> ValueHandle = FVoxelEditorUtilities::FindMapValuePropertyHandle(*ScalarsHandle, Slot.Guid);
				if (!ValueHandle ||
					!ValueHandle->IsValidHandle())
				{
					continue;
				}

				const TSharedPtr<IPropertyHandle> ScalarHandle = ValueHandle->GetChildHandleStatic(FVoxelCubicScalarEntry, Value);
				if (!ScalarHandle ||
					!ScalarHandle->IsValidHandle())
				{
					continue;
				}

				const FGuid SlotGuid = Slot.Guid;

				Category
				.AddCustomRow(FText::FromName(Slot.Name))
				.OverrideResetToDefault(FResetToDefaultOverride::Create(
					MakeAttributeLambda(MakeWeakPtrLambda(this, [this, SlotGuid]
					{
						return CanResetScalar(SlotGuid);
					})),
					MakeWeakPtrDelegate(this, [this, SlotGuid, ValueHandle]
					{
						ResetScalar(SlotGuid, ValueHandle);
					}),
					false))
				.NameContent()
				[
					SNew(SVoxelDetailText)
					.Text(FText::FromName(Slot.Name))
				]
				.ValueContent()
				.MinDesiredWidth(125.f)
				[
					ScalarHandle->CreatePropertyValueWidget()
				];
			}
		}

		// Vector Slots section
		if (Template->VectorSlots.Num() > 0)
		{
			IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Vector Slots", INVTEXT("Vector Slots"), ECategoryPriority::Default);

			for (const FVoxelCubicVectorSlot& Slot : Template->VectorSlots)
			{
				const TSharedPtr<IPropertyHandle> ValueHandle = FVoxelEditorUtilities::FindMapValuePropertyHandle(*VectorsHandle, Slot.Guid);
				if (!ValueHandle ||
					!ValueHandle->IsValidHandle())
				{
					continue;
				}

				const TSharedPtr<IPropertyHandle> VectorHandle = ValueHandle->GetChildHandleStatic(FVoxelCubicVectorEntry, Value);
				if (!VectorHandle ||
					!VectorHandle->IsValidHandle())
				{
					continue;
				}

				const FGuid SlotGuid = Slot.Guid;

				Category
				.AddCustomRow(FText::FromName(Slot.Name))
				.OverrideResetToDefault(FResetToDefaultOverride::Create(
					MakeAttributeLambda(MakeWeakPtrLambda(this, [this, SlotGuid]
					{
						return CanResetVector(SlotGuid);
					})),
					MakeWeakPtrDelegate(this, [this, SlotGuid, ValueHandle]
					{
						ResetVector(SlotGuid, ValueHandle);
					}),
					false))
				.NameContent()
				[
					SNew(SVoxelDetailText)
					.Text(FText::FromName(Slot.Name))
				]
				.ValueContent()
				.MinDesiredWidth(200.f)
				[
					VectorHandle->CreatePropertyValueWidget()
				];
			}
		}
	}

private:
	void OnTemplateChanged()
	{
		RefreshDelegate.ExecuteIfBound();
	}

	bool CanResetTexture(const FGuid& SlotGuid) const
	{
		const UVoxelCubicSurfaceTypeAsset* Asset = CachedAsset.Resolve();
		if (!Asset)
		{
			return false;
		}

		const FVoxelCubicTextureEntry* Entry = Asset->Textures.Find(SlotGuid);
		if (!Entry)
		{
			return false;
		}

		return Entry->bIsEdited;
	}

	bool CanResetScalar(const FGuid& SlotGuid) const
	{
		const UVoxelCubicSurfaceTypeAsset* Asset = CachedAsset.Resolve();
		if (!Asset)
		{
			return false;
		}

		const FVoxelCubicScalarEntry* Entry = Asset->Scalars.Find(SlotGuid);
		if (!Entry)
		{
			return false;
		}

		return Entry->bIsEdited;
	}

	bool CanResetVector(const FGuid& SlotGuid) const
	{
		const UVoxelCubicSurfaceTypeAsset* Asset = CachedAsset.Resolve();
		if (!Asset)
		{
			return false;
		}

		const FVoxelCubicVectorEntry* Entry = Asset->Vectors.Find(SlotGuid);
		if (!Entry)
		{
			return false;
		}

		return Entry->bIsEdited;
	}

	void ResetTexture(const FGuid& SlotGuid, const TSharedPtr<IPropertyHandle>& ValueHandle) const
	{
		if (!ValueHandle ||
			!ValueHandle->IsValidHandle())
		{
			return;
		}

		UVoxelCubicSurfaceTypeAsset* Asset = CachedAsset.Resolve();
		const UVoxelCubicTextureTemplate* Template = CachedTemplate.Resolve();
		if (!Asset ||
			!Template)
		{
			return;
		}

		const FScopedTransaction Transaction(INVTEXT("Reset Texture to Default"));

		const TSharedPtr<IPropertyHandle> TextureHandle = ValueHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelCubicTextureEntry, Texture));

		if (TextureHandle &&
			TextureHandle->IsValidHandle())
		{
			const UTexture2D* DefaultTexture = Template->GetDefaultTexture(SlotGuid);
			TextureHandle->SetValue(static_cast<const UObject*>(DefaultTexture));
		}

		// Directly set bIsEdited on the asset - property handles don't work reliably for nested struct members
		if (FVoxelCubicTextureEntry* Entry = Asset->Textures.Find(SlotGuid))
		{
			Asset->Modify();
			Entry->bIsEdited = false;
		}
	}

	void ResetScalar(const FGuid& SlotGuid, const TSharedPtr<IPropertyHandle>& ValueHandle) const
	{
		if (!ValueHandle ||
			!ValueHandle->IsValidHandle())
		{
			return;
		}

		UVoxelCubicSurfaceTypeAsset* Asset = CachedAsset.Resolve();
		const UVoxelCubicTextureTemplate* Template = CachedTemplate.Resolve();
		if (!Asset ||
			!Template)
		{
			return;
		}

		const FScopedTransaction Transaction(INVTEXT("Reset Scalar to Default"));

		const TSharedPtr<IPropertyHandle> ScalarHandle = ValueHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelCubicScalarEntry, Value));

		if (ScalarHandle &&
			ScalarHandle->IsValidHandle())
		{
			ScalarHandle->SetValue(Template->GetDefaultScalar(SlotGuid));
		}

		// Directly set bIsEdited on the asset - property handles don't work reliably for nested struct members
		if (FVoxelCubicScalarEntry* Entry = Asset->Scalars.Find(SlotGuid))
		{
			Asset->Modify();
			Entry->bIsEdited = false;
		}
	}

	void ResetVector(const FGuid& SlotGuid, const TSharedPtr<IPropertyHandle>& ValueHandle) const
	{
		if (!ValueHandle ||
			!ValueHandle->IsValidHandle())
		{
			return;
		}

		UVoxelCubicSurfaceTypeAsset* Asset = CachedAsset.Resolve();
		const UVoxelCubicTextureTemplate* Template = CachedTemplate.Resolve();
		if (!Asset ||
			!Template)
		{
			return;
		}

		const FScopedTransaction Transaction(INVTEXT("Reset Vector to Default"));

		const TSharedPtr<IPropertyHandle> VectorHandle = ValueHandle->GetChildHandleStatic(FVoxelCubicVectorEntry, Value);

		if (VectorHandle &&
			VectorHandle->IsValidHandle())
		{
			FVoxelEditorUtilities::SetStructPropertyValue(VectorHandle, Template->GetDefaultVector(SlotGuid));
		}

		// Directly set bIsEdited on the asset - property handles don't work reliably for nested struct members
		if (FVoxelCubicVectorEntry* Entry = Asset->Vectors.Find(SlotGuid))
		{
			Asset->Modify();
			Entry->bIsEdited = false;
		}
	}

private:
	TVoxelObjectPtr<UVoxelCubicSurfaceTypeAsset> CachedAsset;
	TVoxelObjectPtr<UVoxelCubicTextureTemplate> CachedTemplate;
	FSimpleDelegate RefreshDelegate;
	FDelegateHandle TemplateChangedHandle;
};

DEFINE_VOXEL_CLASS_LAYOUT(UVoxelCubicSurfaceTypeAsset, FVoxelCubicSurfaceTypeAssetCustomization);