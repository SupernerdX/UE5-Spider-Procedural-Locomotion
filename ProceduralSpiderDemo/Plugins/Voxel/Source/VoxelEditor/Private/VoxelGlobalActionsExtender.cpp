// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGlobalActionsExtender.h"
#include "VoxelPCGTracker.h"
#include "VoxelVersionEditor.h"
#include "PCGCommon.h"
#include "ToolMenus.h"
#include "EditorModes.h"
#include "LevelEditor.h"
#include "PCGSubsystem.h"
#include "EditorModeManager.h"
#include "VoxelPluginVersion.h"
#include "VoxelEditorSettings.h"
#include "VoxelConsoleCommandsTab.h"
#include "VoxelTerminalGraphRuntime.h"

VOXEL_INITIALIZE_STYLE(VoxelGlobalActions)
{
	Set("VoxelIcon.DebugMode", new IMAGE_BRUSH("UIIcons/VoxelIcon_40x", CoreStyleConstants::Icon16x16, FLinearColor(FColorList::Orange)));

	const FCheckBoxStyle FavoriteToggleStyle = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(FSlateRoundedBoxBrush(FStyleColors::Dropdown, FVector4(4.0f, 0.0f, 0.0f, 4.0f)))
		.SetUncheckedPressedImage(FSlateRoundedBoxBrush(FStyleColors::Hover, FVector4(4.0f, 0.0f, 0.0f, 4.0f)))
		.SetUncheckedHoveredImage(FSlateRoundedBoxBrush(FStyleColors::Hover, FVector4(4.0f, 0.0f, 0.0f, 4.0f)))
		.SetCheckedImage(FSlateRoundedBoxBrush(FStyleColors::Primary, FVector4(4.0f, 0.0f, 0.0f, 4.0f)))
		.SetCheckedPressedImage(FSlateRoundedBoxBrush(FStyleColors::PrimaryHover, FVector4(4.0f, 0.0f, 0.0f, 4.0f)))
		.SetCheckedHoveredImage(FSlateRoundedBoxBrush(FStyleColors::PrimaryPress, FVector4(4.0f, 0.0f, 0.0f, 4.0f)));
	Set("VoxelModeButton", FavoriteToggleStyle);

};

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	if (!GetDefault<UVoxelEditorSettings>()->bEnableToolbarActions)
	{
		return;
	}

	FVoxelGlobalActionsExtender::RegisterMenu();
}

void FVoxelGlobalActionsExtender::RegisterMenu()
{
	UToolMenu* ToolBar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.AssetsToolBar");

	FToolMenuSection& Section = ToolBar->FindOrAddSection("Content");

	FToolMenuEntry& NewEntry = Section.AddDynamicEntry("VoxelPluginGlobalActions", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
	{
		{
			InSection.AddEntry(
				FToolMenuEntry::InitWidget(
					"VoxelMode",
					SNew(SCheckBox)
					.Style(&FVoxelEditorStyle::GetWidgetStyle<FCheckBoxStyle>("VoxelModeButton"))
					.Padding(FMargin(6.f, 0.f, 4.f, 0.f))
					.IsChecked_Lambda([]
					{
						return
							GLevelEditorModeTools().IsModeActive("VoxelToolEdMode")
							? ECheckBoxState::Checked
							: ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([](ECheckBoxState)
					{
						if (GLevelEditorModeTools().IsModeActive("VoxelToolEdMode"))
						{
							GLevelEditorModeTools().DeactivateMode("VoxelToolEdMode");
							GLevelEditorModeTools().ActivateDefaultMode();
							return;
						}

						GLevelEditorModeTools().DeactivateAllModes();
						GLevelEditorModeTools().ActivateMode("VoxelToolEdMode");
					})
					[
						SNew(SBox)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SImage)
#if VOXEL_DEBUG == 1
							.Image(FVoxelEditorStyle::GetBrush("VoxelIcon.DebugMode"))
#else
							.Image(FVoxelEditorStyle::GetBrush("VoxelIcon"))
#endif
							.DesiredSizeOverride(FVector2D(24.f))
						]
					],
					{},
					false,
					true,
					false,
					INVTEXT("Toggle voxel mode.")));
		}

		{
			FToolMenuEntry Entry =
				FToolMenuEntry::InitToolBarButton(
					"Refresh",
					MakeLambdaDelegate([]
					{
						Voxel::RefreshAll();
					}),
					{},
					INVTEXT("Force the regeneration of all voxel worlds in the level."),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Refresh"));
			Entry.StyleNameOverride = FName("Toolbar.BackplateCenter");
			InSection.AddEntry(Entry);
		}

		FToolMenuEntry ComboEntry =
			FToolMenuEntry::InitComboButton(
				"Voxel",
				FUIAction(),
				FNewMenuDelegate::CreateLambda([](FMenuBuilder& Menu)
				{
					Menu.BeginSection("Voxel", INVTEXT("Voxel"));
					Menu.AddMenuEntry(
						INVTEXT("Open Place Voxel Stamps"),
						{},
						FSlateIcon(FVoxelEditorStyle::GetStyleSetName(), "VoxelIcon"),
						MakeLambdaDelegate([]
						{
							const FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
							const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();

							LevelEditorTabManager->TryInvokeTab(FTabId("PlaceVoxelStamps"));
						}));
					Menu.EndSection();

					Menu.BeginSection("Editor UX", INVTEXT("Editor UX"));
					Menu.AddMenuEntry(
						INVTEXT("Show Selected Stamp Bounds"),
						INVTEXT("Show the bounds of the selected stamp"),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "ShowFlagsMenu.BSP"),
						FUIAction(
							MakeLambdaDelegate([]
							{
								static IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(TEXT("voxel.stamp.ShowSelectedBounds"));
								if (!ensure(ConsoleVariable))
								{
									return;
								}

								ConsoleVariable->Set(!ConsoleVariable->GetBool(), ECVF_SetByConsole);
							}),
							{},
							MakeLambdaDelegate([]
							{
								static IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(TEXT("voxel.stamp.ShowSelectedBounds"));
								if (!ensure(ConsoleVariable))
								{
									return false;
								}

								return ConsoleVariable->GetBool();
							})),
						{},
						EUserInterfaceActionType::ToggleButton
					);
					Menu.EndSection();

					Menu.BeginSection("Technical", INVTEXT("Technical"));
#if VOXEL_STATS
					Menu.AddMenuEntry(
						MakeAttributeLambda([]
						{
							return
								UE::Trace::IsTracing()
									? INVTEXT("Stop Voxel Trace")
									: INVTEXT("Start Voxel Trace");
						}),
						{},
						FSlateIcon(/* FTraceToolsStyle::GetStyleSetName() */"TraceToolsStyle", "ToggleTraceButton.RecordTraceCenter.StatusBar"),
						MakeLambdaDelegate([]
						{
							if (UE::Trace::IsTracing())
							{
								GEditor->Exec(nullptr, TEXT("voxel.StopInsights"));
							}
							else
							{
								GEditor->Exec(nullptr, TEXT("voxel.StartInsights"));
							}
						}));
#endif

					Menu.AddMenuEntry(
						INVTEXT("Recompile voxel graphs"),
						INVTEXT("Will recompile all Voxel Graphs in the project and open all the ones with any errors or warnings"),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Recompile.Small"),
						MakeLambdaDelegate([]
						{
							GVoxelGraphEditorInterface->CompileAll();
						}));

					Menu.AddMenuEntry(
						INVTEXT("Upgrade voxel assets"),
						INVTEXT("Checkout and save any voxel asset that needs to be upgraded"),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.ArrowUp"),
						MakeLambdaDelegate([]
						{
							FVoxelVersionUtilities::UpgradeVoxelAssets();
						}));

					Menu.EndSection();

					Menu.BeginSection("PCG", INVTEXT("PCG"));
					Menu.AddMenuEntry(
						INVTEXT("Refresh Runtime PCG"),
						INVTEXT("Refreshes all runtime PCG components."),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Refresh"),
						FUIAction(
							MakeLambdaDelegate([]
							{
								if (UPCGSubsystem* PCGSubsystem = UPCGSubsystem::GetInstance(GEditor->GetEditorWorldContext().World()))
								{
									PCGSubsystem->RefreshAllRuntimeGenComponents();
								}
							})),
						{}
					);
					Menu.AddMenuEntry(
						INVTEXT("Disable Voxel PCG Refreshes"),
						INVTEXT("Restricts changes to the Voxel World from automatically invalidating PCG components, so PCG will only regenerate voxel-related nodes if told to manually."),
						FSlateIcon(FVoxelEditorStyle::GetStyleSetName(), "VoxelIcon"),
						FUIAction(
							MakeLambdaDelegate([]
							{
								GVoxelDisablePCGRegen = !GVoxelDisablePCGRegen;
							}),
							{},
							MakeLambdaDelegate([]
							{
								return GVoxelDisablePCGRegen;
							})),
						{},
						EUserInterfaceActionType::ToggleButton
					);
					Menu.AddMenuEntry(
						INVTEXT("Disable PCG Globally"),
						INVTEXT("Toggles PCG processing on/off and will cancel tasks depending on settings.\nUse Ctrl to pause and cancel all tasks."),
						FSlateIcon("PCGEditorStyle", "PCG.Editor.Pause"),
						FUIAction(
							MakeLambdaDelegate([]
							{
								const bool bWasPaused = PCGSystemSwitches::CVarPausePCGExecution.GetValueOnAnyThread();

								if (!bWasPaused)
								{
									bool bShouldCancelAll = false;

									if (FSlateApplication::Get().GetModifierKeys().IsControlDown())
									{
										bShouldCancelAll = true;
									}

									if (bShouldCancelAll)
									{
										if (UPCGSubsystem* PCGSubsystem = UPCGSubsystem::GetInstance(GEditor->GetEditorWorldContext().World()))
										{
											PCGSubsystem->CancelAllGeneration();
										}
									}
								}

								PCGSystemSwitches::CVarPausePCGExecution->Set(!bWasPaused);
							}),
							{},
							MakeLambdaDelegate([]
							{
								return PCGSystemSwitches::CVarPausePCGExecution.GetValueOnAnyThread();
							})),
						{},
						EUserInterfaceActionType::ToggleButton
					);
					Menu.EndSection();

					Menu.BeginSection("Debug", INVTEXT("Debug"));
					Menu.AddMenuEntry(
						INVTEXT("Console Variables"),
						INVTEXT("View, edit and execute voxel console variables."),
						FSlateIcon(FVoxelEditorStyle::GetStyleSetName(), "VoxelIcon"),
						MakeLambdaDelegate([]
						{
							GVoxelCommandsTabManager->OpenGlobalCommands();
						})
					);
					Menu.EndSection();

					Menu.BeginSection("Info", INVTEXT("Info"));

					const TSharedRef<SWidget> VersionWidget =
						SNew(SBox)
						.Padding(4.f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(STextBlock)
								.Text(INVTEXT("Voxel Plugin Version: "))
								.ColorAndOpacity(FSlateColor::UseForeground())
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(STextBlock)
								.Text(FText::FromString(FVoxelUtilities::GetPluginVersion().ToString_UserFacing()))
								.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							]
						];
					Menu.AddWidget(VersionWidget, {}, true);
					Menu.EndSection();
				}),
				INVTEXT("Voxel"),
				INVTEXT("Voxel Plugin configuration"));
		ComboEntry.StyleNameOverride = FName("Toolbar.BackplateRightCombo");
		InSection.AddEntry(ComboEntry);
	}));
	NewEntry.InsertPosition.Position = EToolMenuInsertType::First;

	UToolMenus::Get()->RefreshMenuWidget("LevelEditor.LevelEditorToolBar.AssetsToolBar");
}

void FVoxelGlobalActionsExtender::UnregisterMenu()
{
	UToolMenus::Get()->RemoveEntry(
		"LevelEditor.LevelEditorToolBar.AssetsToolBar",
		"Content",
		"VoxelPluginGlobalActions");

	UToolMenus::Get()->RefreshMenuWidget("LevelEditor.LevelEditorToolBar.AssetsToolBar");
}