// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelMaterialGraphShapeToolbar.h"
#include "Styling/SlateIconFinder.h"

#if VOXEL_ENGINE_VERSION >= 506
#include "ToolMenu.h"
#include "ViewportToolbar/UnrealEdViewportToolbar.h"
#endif

void SVoxelMaterialGraphShapeToolbar::Construct(const FArguments& Args)
{
	TypeAttribute = Args._Type;
	OnTypeChanged = Args._OnTypeChanged;

	FToolBarBuilder ToolbarBuilder(nullptr, FMultiBoxCustomization::None, nullptr, true);

	ToolbarBuilder.SetStyle(&FAppStyle::Get(), "LegacyViewportMenu");
	ToolbarBuilder.SetLabelVisibility(EVisibility::Collapsed);
	ToolbarBuilder.SetIsFocusable(false);
	
	ToolbarBuilder.BeginSection("Preview");
	{
		const UEnum* Enum = StaticEnum<EVoxelSmartSurfacePreviewShape>();
		for (int32 Index = 0; Index < Enum->NumEnums() - 1; Index++)
		{
			if (Enum->HasMetaData(TEXT("Hidden"), Index))
			{
				continue;
			}

			const EVoxelSmartSurfacePreviewShape Type = EVoxelSmartSurfacePreviewShape(Enum->GetValueByIndex(Index));
			ToolbarBuilder.AddToolBarButton(
				FUIAction(
					MakeLambdaDelegate([this, Type]
					{
						OnTypeChanged.ExecuteIfBound(Type);
					}),
					{},
					MakeLambdaDelegate([this, Type]
					{
						return TypeAttribute.Get() == Type;
					})),
				{},
				Enum->GetDisplayNameTextByIndex(Index),
				Enum->GetToolTipTextByIndex(Index),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), *Enum->GetMetaData(TEXT("Icon"), Index)),
				EUserInterfaceActionType::ToggleButton);
		}
	}
	ToolbarBuilder.EndSection();

	ChildSlot
	[
		SNew(SBorder)
#if VOXEL_ENGINE_VERSION == 506
		.Visibility_Lambda([]
		{
			return
				UE::UnrealEd::ShowOldViewportToolbars()
				? EVisibility::Visible
				: EVisibility::Collapsed;
		})
#endif
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		.ForegroundColor(FAppStyle::GetSlateColor("DefaultForeground"))
		.HAlign(HAlign_Right)
		[
			ToolbarBuilder.MakeWidget()
		]
	];

	SViewportToolBar::Construct(SViewportToolBar::FArguments());
}

#if VOXEL_ENGINE_VERSION >= 506
void SVoxelMaterialGraphShapeToolbar::ExtendToolbar(
	UToolMenu& ToolMenu,
	TDelegate<void(EVoxelSmartSurfacePreviewShape)> OnTypeChanged,
	TDelegate<EVoxelSmartSurfacePreviewShape()> GetType)
{
	FToolMenuSection& Section = ToolMenu.FindOrAddSection("Right");
	Section.AddEntry(
		FToolMenuEntry::InitSubMenu(
		"Preview",
		INVTEXT("Preview"),
		{},
		MakeLambdaDelegate([OnTypeChanged, GetType](UToolMenu* Submenu)
		{
			FToolMenuSection& InnerSection = Submenu->AddSection("Preview");
			FToolMenuEntryToolBarData ToolbarData;
			ToolbarData.BlockGroupName = "PreviewMeshOptions";
			ToolbarData.ResizeParams.ClippingPriority = 2000;
			ToolbarData.LabelOverride = FText();

			const UEnum* Enum = StaticEnum<EVoxelSmartSurfacePreviewShape>();
			for (int32 Index = 0; Index < Enum->NumEnums() - 1; Index++)
			{
				if (Enum->HasMetaData(TEXT("Hidden"), Index))
				{
					continue;
				}

				const EVoxelSmartSurfacePreviewShape Type = EVoxelSmartSurfacePreviewShape(Enum->GetValueByIndex(Index));
				FToolMenuEntry& Entry = InnerSection.AddMenuEntry(
					Enum->GetNameByIndex(Index),
					Enum->GetDisplayNameTextByIndex(Index),
					Enum->GetToolTipTextByIndex(Index),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), *Enum->GetMetaData(TEXT("Icon"), Index)),
					FUIAction(
						MakeLambdaDelegate([Type, OnTypeChanged]
						{
							OnTypeChanged.Execute(Type);
						}),
						{},
						MakeLambdaDelegate([Type, GetType]
						{
							return GetType.Execute() == Type;
						})),
					EUserInterfaceActionType::RadioButton);

				Entry.SetShowInToolbarTopLevel(true);
				Entry.ToolBarData = ToolbarData;
			}
		}),
		false,
		FSlateIconFinder::FindIconForClass(UStaticMesh::StaticClass())));
}
#endif
