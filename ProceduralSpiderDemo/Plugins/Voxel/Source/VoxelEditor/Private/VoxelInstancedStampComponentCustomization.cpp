// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelInstancedStampComponent.h"
#include "Heightmap/VoxelHeightmapStamp.h"

VOXEL_INITIALIZE_STYLE(VoxelInstancedStampStyle)
{
	const FButtonStyle Button = FAppStyle::GetWidgetStyle<FButtonStyle>("Button");
	const FSlateColor SelectionColor = FAppStyle::GetSlateColor("SelectionColor");
	const FSlateColor SelectionColor_Inactive = FAppStyle::GetSlateColor("SelectionColor_Inactive");
	const FSlateColor SelectionColor_Pressed = FAppStyle::GetSlateColor("SelectionColor_Pressed");

	Set("InstancedStampDetails.SelectFirst", FButtonStyle(Button)
		.SetNormal(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectFirst", CoreStyleConstants::Icon20x20))
		.SetHovered(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectFirst", CoreStyleConstants::Icon20x20, SelectionColor))
		.SetPressed(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectFirst", CoreStyleConstants::Icon20x20, SelectionColor_Pressed))
		.SetDisabled(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectFirst", CoreStyleConstants::Icon20x20, SelectionColor_Inactive)));

	Set("InstancedStampDetails.SelectPrev", FButtonStyle(Button)
		.SetNormal(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectPrevious", CoreStyleConstants::Icon20x20))
		.SetHovered(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectPrevious", CoreStyleConstants::Icon20x20, SelectionColor))
		.SetPressed(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectPrevious", CoreStyleConstants::Icon20x20, SelectionColor_Pressed))
		.SetDisabled(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectPrevious", CoreStyleConstants::Icon20x20, SelectionColor_Inactive)));

	Set("InstancedStampDetails.SelectNext", FButtonStyle(Button)
		.SetNormal(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectNext", CoreStyleConstants::Icon20x20))
		.SetHovered(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectNext", CoreStyleConstants::Icon20x20, SelectionColor))
		.SetPressed(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectNext", CoreStyleConstants::Icon20x20, SelectionColor_Pressed))
		.SetDisabled(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectNext", CoreStyleConstants::Icon20x20, SelectionColor_Inactive)));

	Set("InstancedStampDetails.SelectLast", FButtonStyle(Button)
		.SetNormal(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectLast", CoreStyleConstants::Icon20x20))
		.SetHovered(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectLast", CoreStyleConstants::Icon20x20, SelectionColor))
		.SetPressed(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectLast", CoreStyleConstants::Icon20x20, SelectionColor_Pressed))
		.SetDisabled(CORE_IMAGE_BRUSH_SVG("Starship/Splines/Spline_SelectLast", CoreStyleConstants::Icon20x20, SelectionColor_Inactive)));

	const auto CoreImageBrushSVG = [](const FString& RelativePath)
	{
		return FPaths::EngineContentDir() / TEXT("Slate") / RelativePath + ".svg";
	};

	Set("InstancedStampDetails.Add", FButtonStyle(Button)
		.SetNormal(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/plus"), CoreStyleConstants::Icon20x20))
		.SetHovered(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/plus"), CoreStyleConstants::Icon20x20, SelectionColor))
		.SetPressed(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/plus"), CoreStyleConstants::Icon20x20, SelectionColor_Pressed))
		.SetDisabled(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/plus"), CoreStyleConstants::Icon20x20, SelectionColor_Inactive)));

	Set("InstancedStampDetails.Remove", FButtonStyle(Button)
		.SetNormal(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/minus"), CoreStyleConstants::Icon20x20))
		.SetHovered(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/minus"), CoreStyleConstants::Icon20x20, SelectionColor))
		.SetPressed(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/minus"), CoreStyleConstants::Icon20x20, SelectionColor_Pressed))
		.SetDisabled(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/minus"), CoreStyleConstants::Icon20x20, SelectionColor_Inactive)));

	Set("InstancedStampDetails.Duplicate", FButtonStyle(Button)
		.SetNormal(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/Duplicate"), CoreStyleConstants::Icon20x20))
		.SetHovered(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/Duplicate"), CoreStyleConstants::Icon20x20, SelectionColor))
		.SetPressed(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/Duplicate"), CoreStyleConstants::Icon20x20, SelectionColor_Pressed))
		.SetDisabled(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/Duplicate"), CoreStyleConstants::Icon20x20, SelectionColor_Inactive)));

	Set("InstancedStampDetails.Clear", FButtonStyle(Button)
		.SetNormal(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/delete-outline"), CoreStyleConstants::Icon20x20))
		.SetHovered(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/delete-outline"), CoreStyleConstants::Icon20x20, SelectionColor))
		.SetPressed(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/delete-outline"), CoreStyleConstants::Icon20x20, SelectionColor_Pressed))
		.SetDisabled(FSlateVectorImageBrush(CoreImageBrushSVG("Starship/Common/delete-outline"), CoreStyleConstants::Icon20x20, SelectionColor_Inactive)));
}

class FVoxelInstancedStampComponentCustomization : public FVoxelDetailCustomization
{
public:
	static TVoxelArray<UVoxelInstancedStampComponent*> GetObjectsBeingCustomized(IDetailLayoutBuilder& DetailLayout) { return FVoxelEditorUtilities::GetObjectsBeingCustomized<UVoxelInstancedStampComponent>(DetailLayout); }
	static UVoxelInstancedStampComponent* GetUniqueObjectBeingCustomized(IDetailLayoutBuilder& DetailLayout) { return FVoxelEditorUtilities::GetUniqueObjectBeingCustomized<UVoxelInstancedStampComponent>(DetailLayout); }
	static TVoxelArray<TVoxelObjectPtr<UVoxelInstancedStampComponent>> GetWeakObjectsBeingCustomized(IDetailLayoutBuilder& DetailLayout) { return TVoxelArray<TVoxelObjectPtr<UVoxelInstancedStampComponent>>(GetObjectsBeingCustomized(DetailLayout)); }

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
	{
		FVoxelEditorUtilities::EnableRealtime();

		DetailLayout.HideCategory("Activation");
		DetailLayout.HideCategory("Rendering");
		DetailLayout.HideCategory("Cooking");
		DetailLayout.HideCategory("Physics");
		DetailLayout.HideCategory("LOD");
		DetailLayout.HideCategory("AssetUserData");
		DetailLayout.HideCategory("Navigation");

		FVoxelEditorUtilities::HideAndMoveToCategory(DetailLayout, "Tags", "Misc", { GET_MEMBER_NAME_STATIC(UActorComponent, ComponentTags) }, false);

		CreateControlsSection(DetailLayout);
		CreateStampSection(DetailLayout);

		const IDetailCategoryBuilder& ConfigCategory = DetailLayout.EditCategory("Config", INVTEXT("Config"), ECategoryPriority::Uncommon);
		IDetailCategoryBuilder& MiscCategory = DetailLayout.EditCategory("Misc", INVTEXT("Misc"), ECategoryPriority::Uncommon);
		MiscCategory.SetSortOrder(ConfigCategory.GetSortOrder() + 2);
	}

private:
	static void CreateControlsSection(IDetailLayoutBuilder& DetailLayout)
	{
		TVoxelArray<TVoxelObjectPtr<UVoxelInstancedStampComponent>> WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout);
		if (WeakObjects.Num() > 1)
		{
			return;
		}

		const TSharedRef<IPropertyHandle> StampsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelInstancedStampComponent, PrivateStamps), UVoxelInstancedStampComponent::StaticClass());
		const TSharedRef<IPropertyHandle> ActiveInstanceHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelInstancedStampComponent, ActiveInstance), UVoxelInstancedStampComponent::StaticClass());

		IDetailCategoryBuilder& ControlsCategory = DetailLayout.EditCategory("Controls", INVTEXT("Controls"), ECategoryPriority::Important);

		ControlsCategory.AddCustomRow(INVTEXT("Instances count"))
		.WholeRowContent()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SVoxelDetailText)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				.Text(INVTEXT("Instances count: "))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SVoxelDetailText)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.Text_Lambda([StampsHandle]
				{
					uint32 NumInstances = 0;
					StampsHandle->GetNumChildren(NumInstances);
					return FText::AsNumber(NumInstances);
				})
			]
		];

		ControlsCategory.AddCustomRow(INVTEXT("Array controls"))
		.WholeRowContent()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			.Clipping(EWidgetClipping::ClipToBounds)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(FSlateColor::UseForeground())
				.ButtonStyle(FVoxelEditorStyle::Get(), "InstancedStampDetails.Add")
				.ToolTipText(INVTEXT("Add instance"))
				.OnClicked_Lambda([StampsHandle, ActiveInstanceHandle, WeakObjects, PropUtilities = DetailLayout.GetPropertyUtilities()]
				{
					if (!StampsHandle->IsValidHandle() ||
						!ActiveInstanceHandle->IsValidHandle())
					{
						return FReply::Handled();
					}

					uint32 ActiveInstanceIndex = 0;
					if (!ensure(ActiveInstanceHandle->GetValue(ActiveInstanceIndex) == FPropertyAccess::Success))
					{
						return FReply::Handled();
					}

					const TSharedPtr<IPropertyHandleArray> StampsArrayHandle = StampsHandle->AsArray();

					uint32 NumElements = 0;
					StampsArrayHandle->GetNumElements(NumElements);

					for (const TVoxelObjectPtr<UVoxelInstancedStampComponent>& WeakObject : WeakObjects)
					{
						UVoxelInstancedStampComponent* Object = WeakObject.Resolve();
						if (!Object)
						{
							continue;
						}

						FVoxelTransaction Transaction(Object, "Add instance");
						Object->ActiveInstance = NumElements;
						Object->AddStamp(FVoxelStampRef::New(FVoxelHeightmapStamp()));
					}

					PropUtilities->RequestForceRefresh();

					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(FSlateColor::UseForeground())
				.ButtonStyle(FVoxelEditorStyle::Get(), "InstancedStampDetails.Remove")
				.ToolTipText(INVTEXT("Remove instance"))
				.IsEnabled_Lambda([StampsHandle]
				{
					if (!StampsHandle->IsValidHandle())
					{
						return false;
					}

					uint32 NumInstances = 0;
					StampsHandle->GetNumChildren(NumInstances);

					return NumInstances > 0;
				})
				.OnClicked_Lambda([StampsHandle, ActiveInstanceHandle, PropUtilities = DetailLayout.GetPropertyUtilities()]
				{
					if (!StampsHandle->IsValidHandle() ||
						!ActiveInstanceHandle->IsValidHandle())
					{
						return FReply::Handled();
					}

					uint32 ActiveInstanceIndex = 0;
					if (!ensure(ActiveInstanceHandle->GetValue(ActiveInstanceIndex) == FPropertyAccess::Success))
					{
						return FReply::Handled();
					}

					const TSharedPtr<IPropertyHandleArray> StampsArrayHandle = StampsHandle->AsArray();
					StampsArrayHandle->DeleteItem(ActiveInstanceIndex);

					PropUtilities->RequestForceRefresh();

					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(FSlateColor::UseForeground())
				.ButtonStyle(FVoxelEditorStyle::Get(), "InstancedStampDetails.Duplicate")
				.ToolTipText(INVTEXT("Duplicate instance"))
				.IsEnabled_Lambda([StampsHandle]
				{
					if (!StampsHandle->IsValidHandle())
					{
						return false;
					}

					uint32 NumInstances = 0;
					StampsHandle->GetNumChildren(NumInstances);

					return NumInstances > 0;
				})
				.OnClicked_Lambda([WeakObjects, StampsHandle, ActiveInstanceHandle, PropUtilities = DetailLayout.GetPropertyUtilities()]
				{
					if (!StampsHandle->IsValidHandle() ||
						!ActiveInstanceHandle->IsValidHandle())
					{
						return FReply::Handled();
					}

					uint32 ActiveInstanceIndex = 0;
					if (!ensure(ActiveInstanceHandle->GetValue(ActiveInstanceIndex) == FPropertyAccess::Success))
					{
						return FReply::Handled();
					}

					for (const TVoxelObjectPtr<UVoxelInstancedStampComponent>& WeakObject : WeakObjects)
					{
						UVoxelInstancedStampComponent* Object = WeakObject.Resolve();
						if (!Object)
						{
							continue;
						}

						FVoxelTransaction Transaction(Object, "Duplicate stamp");
						Object->DuplicateStamp(ActiveInstanceIndex);
					}

					const TSharedPtr<IPropertyHandleArray> StampsArrayHandle = StampsHandle->AsArray();
					StampsArrayHandle->DuplicateItem(ActiveInstanceIndex);

					PropUtilities->RequestForceRefresh();

					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(FSlateColor::UseForeground())
				.ButtonStyle(FVoxelEditorStyle::Get(), "InstancedStampDetails.Clear")
				.ToolTipText(INVTEXT("Clear instances"))
				.IsEnabled_Lambda([StampsHandle]
				{
					if (!StampsHandle->IsValidHandle())
					{
						return false;
					}

					uint32 NumInstances = 0;
					StampsHandle->GetNumChildren(NumInstances);

					return NumInstances > 0;
				})
				.OnClicked_Lambda([StampsHandle, ActiveInstanceHandle, WeakObjects, PropUtilities = DetailLayout.GetPropertyUtilities()]
				{
					if (!StampsHandle->IsValidHandle() ||
						!ActiveInstanceHandle->IsValidHandle())
					{
						return FReply::Handled();
					}

					for (const TVoxelObjectPtr<UVoxelInstancedStampComponent>& WeakObject : WeakObjects)
					{
						UVoxelInstancedStampComponent* Object = WeakObject.Resolve();
						if (!Object)
						{
							continue;
						}

						FVoxelTransaction Transaction(Object, "Clear instances");
						Object->ActiveInstance = 0;
						Object->ClearStamps();
					}

					PropUtilities->RequestForceRefresh();

					return FReply::Handled();
				})
			]
		];

		ControlsCategory.AddCustomRow(INVTEXT("Navigation"))
		.WholeRowContent()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			.Clipping(EWidgetClipping::ClipToBounds)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SButton)
				.ButtonStyle(FVoxelEditorStyle::Get(), "InstancedStampDetails.SelectFirst")
				.ContentPadding(2.0f)
				.ToolTipText(INVTEXT("Select first instance"))
				.IsEnabled_Lambda([ActiveInstanceHandle]
				{
					if (!ActiveInstanceHandle->IsValidHandle())
					{
						return false;
					}

					uint32 ActiveInstance = 0;
					ActiveInstanceHandle->GetValue(ActiveInstance);

					return ActiveInstance > 0;
				})
				.OnClicked_Lambda([ActiveInstanceHandle]
				{
					if (!ActiveInstanceHandle->IsValidHandle())
					{
						return FReply::Handled();
					}

					ActiveInstanceHandle->SetValue(0);

					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SButton)
				.ButtonStyle(FVoxelEditorStyle::Get(), "InstancedStampDetails.SelectPrev")
				.ContentPadding(2.0f)
				.ToolTipText(INVTEXT("Select previous instance"))
				.IsEnabled_Lambda([ActiveInstanceHandle]
				{
					if (!ActiveInstanceHandle->IsValidHandle())
					{
						return false;
					}

					uint32 ActiveInstance = 0;
					ActiveInstanceHandle->GetValue(ActiveInstance);

					return ActiveInstance > 0;
				})
				.OnClicked_Lambda([ActiveInstanceHandle]
				{
					if (!ActiveInstanceHandle->IsValidHandle())
					{
						return FReply::Handled();
					}

					uint32 ActiveInstance = 0;
					ActiveInstanceHandle->GetValue(ActiveInstance);
					ActiveInstanceHandle->SetValue(FMath::Max(0u, ActiveInstance - 1));

					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SButton)
				.ButtonStyle(FVoxelEditorStyle::Get(), "InstancedStampDetails.SelectNext")
				.ContentPadding(2.0f)
				.ToolTipText(INVTEXT("Select next instance"))
				.IsEnabled_Lambda([StampsHandle, ActiveInstanceHandle]
				{
					if (!ActiveInstanceHandle->IsValidHandle() ||
						!StampsHandle->IsValidHandle())
					{
						return false;
					}

					uint32 ActiveInstance = 0;
					ActiveInstanceHandle->GetValue(ActiveInstance);
					uint32 NumInstances = 0;
					StampsHandle->GetNumChildren(NumInstances);

					return
						NumInstances > 0 &&
						NumInstances - 1 > ActiveInstance;
				})
				.OnClicked_Lambda([StampsHandle, ActiveInstanceHandle]
				{
					if (!ActiveInstanceHandle->IsValidHandle() ||
						!StampsHandle->IsValidHandle())
					{
						return FReply::Handled();
					}

					uint32 ActiveInstance = 0;
					ActiveInstanceHandle->GetValue(ActiveInstance);
					uint32 NumInstances = 0;
					StampsHandle->GetNumChildren(NumInstances);
					ActiveInstanceHandle->SetValue(FMath::Min(NumInstances - 1, ActiveInstance + 1));

					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SButton)
				.ButtonStyle(FVoxelEditorStyle::Get(), "InstancedStampDetails.SelectLast")
				.ContentPadding(2.0f)
				.ToolTipText(INVTEXT("Select last instance"))
				.IsEnabled_Lambda([StampsHandle, ActiveInstanceHandle]
				{
					if (!ActiveInstanceHandle->IsValidHandle() ||
						!StampsHandle->IsValidHandle())
					{
						return false;
					}

					uint32 ActiveInstance = 0;
					ActiveInstanceHandle->GetValue(ActiveInstance);
					uint32 NumInstances = 0;
					StampsHandle->GetNumChildren(NumInstances);

					return
						NumInstances > 0 &&
						NumInstances - 1 > ActiveInstance;
				})
				.OnClicked_Lambda([StampsHandle, ActiveInstanceHandle]
				{
					if (!ActiveInstanceHandle->IsValidHandle() ||
						!StampsHandle->IsValidHandle())
					{
						return FReply::Handled();
					}

					uint32 NumInstances = 0;
					StampsHandle->GetNumChildren(NumInstances);
					ActiveInstanceHandle->SetValue(NumInstances - 1);

					return FReply::Handled();
				})
			]
		];

		uint32 NumInstances = 0;
		StampsHandle->GetNumChildren(NumInstances);

		if (NumInstances > 0)
		{
			ActiveInstanceHandle->SetInstanceMetaData("UIMin", "0");
			ActiveInstanceHandle->SetInstanceMetaData("ClampMin", "0");
			ActiveInstanceHandle->SetInstanceMetaData("UIMax", LexToString(FMath::Max(0, int32(NumInstances) - 1)));
			ActiveInstanceHandle->SetInstanceMetaData("ClampMax", LexToString(FMath::Max(0, int32(NumInstances) - 1)));
			ControlsCategory.AddProperty(ActiveInstanceHandle);
			ActiveInstanceHandle->SetOnPropertyValueChanged(MakeLambdaDelegate([PropUtilities = DetailLayout.GetPropertyUtilities()]
			{
				PropUtilities->RequestForceRefresh();
			}));
		}
	}

	static void CreateStampSection(IDetailLayoutBuilder& DetailLayout)
	{
		const TSharedRef<IPropertyHandle> StampsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelInstancedStampComponent, PrivateStamps), UVoxelInstancedStampComponent::StaticClass());
		StampsHandle->MarkHiddenByCustomization();

		const TSharedRef<IPropertyHandle> ActiveInstanceHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelInstancedStampComponent, ActiveInstance), UVoxelInstancedStampComponent::StaticClass());
		ActiveInstanceHandle->MarkHiddenByCustomization();

		uint32 NumInstances = 0;
		if (StampsHandle->GetNumChildren(NumInstances) != FPropertyAccess::Success)
		{
			IDetailCategoryBuilder& StampCategory = DetailLayout.EditCategory("Stamp", INVTEXT("Stamp"), ECategoryPriority::TypeSpecific);
			StampCategory.AddCustomRow(INVTEXT("Multiple Instances"))
			.WholeRowContent()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SVoxelDetailText)
				.ColorAndOpacity(FStyleColors::Warning)
				.Text(INVTEXT("Cannot edit multiple instanced stamp components"))
			];
			return;
		}

		if (NumInstances == 0)
		{
			return;
		}

		uint32 ActiveInstanceIndex = 0;
		ensure(ActiveInstanceHandle->GetValue(ActiveInstanceIndex) == FPropertyAccess::Success);

		if (ActiveInstanceIndex >= NumInstances)
		{
			ActiveInstanceIndex = FMath::Max(0u, NumInstances - 1);
			ActiveInstanceHandle->SetValue(ActiveInstanceIndex);
		}

		const TSharedPtr<IPropertyHandle> ChildHandle = StampsHandle->GetChildHandle(ActiveInstanceIndex);
		if (!ensure(ChildHandle) ||
			!ensure(ChildHandle->IsValidHandle()))
		{
			return;
		}

		IDetailCategoryBuilder& ConfigCategory = DetailLayout.EditCategory("Config", INVTEXT("Config"), ECategoryPriority::TypeSpecific);
		ConfigCategory.AddProperty(ChildHandle);
	}
};

DEFINE_VOXEL_CLASS_LAYOUT(UVoxelInstancedStampComponent, FVoxelInstancedStampComponentCustomization);