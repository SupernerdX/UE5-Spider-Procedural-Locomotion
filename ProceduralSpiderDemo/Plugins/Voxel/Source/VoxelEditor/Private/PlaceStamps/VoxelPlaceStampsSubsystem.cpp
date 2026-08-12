// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPlaceStampsSubsystem.h"

#include "VoxelSettings.h"
#include "VoxelStampActor.h"
#include "VoxelEditorSettings.h"
#include "SVoxelPlaceStampsTab.h"

#include "LevelEditor.h"
#include "StatusBarSubsystem.h"
#include "ContentBrowserModule.h"
#include "WidgetDrawerConfig.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Framework/Docking/LayoutExtender.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelPlaceStampsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	StructOnScope = {};
	DrawerWidget = {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelPlaceStampsSubsystem::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);

	UVoxelPlaceStampsSubsystem* Subsystem = Cast<UVoxelPlaceStampsSubsystem>(InThis);
	if (!Subsystem ||
		!Subsystem->StructOnScope)
	{
		return;
	}

	Subsystem->StructOnScope->AddReferencedObjects(Collector);
}

TSharedPtr<SVoxelPlaceStampsTab> UVoxelPlaceStampsSubsystem::GetDrawerWidget()
{
	if (!DrawerWidget)
	{
		DrawerWidget = SNew(SVoxelPlaceStampsTab, nullptr);
	}

	return DrawerWidget;
}

TSharedPtr<TStructOnScope<FVoxelPlaceStampDefaults>> UVoxelPlaceStampsSubsystem::GetStructOnScope()
{
	if (!StructOnScope)
	{
		StructOnScope = MakeShared<TStructOnScope<FVoxelPlaceStampDefaults>>();

		FString DefaultValue;
		GConfig->GetString(TEXT("VoxelPlaceStamps"), TEXT("Defaults"), DefaultValue, GEditorPerProjectIni);

		FVoxelPlaceStampDefaults Defaults;
		if (!FVoxelUtilities::PropertyFromText_Direct(
			*FVoxelUtilities::MakeStructProperty(StaticStructFast<FVoxelPlaceStampDefaults>()),
			DefaultValue,
			reinterpret_cast<void*>(&Defaults),
			nullptr))
		{
			Defaults.HeightLayer = GetDefault<UVoxelSettings>()->DefaultHeightLayer.LoadSynchronous();
			Defaults.VolumeLayer = GetDefault<UVoxelSettings>()->DefaultVolumeLayer.LoadSynchronous();
		}

		StructOnScope->InitializeAs<FVoxelPlaceStampDefaults>(Defaults);
	}

	return StructOnScope;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelPlaceStampsSubsystem::UpdateActors(const TArray<AActor*>& Actors)
{
	const UVoxelPlaceStampsSubsystem* PlaceStampsSubsystem = GEditor->GetEditorSubsystem<UVoxelPlaceStampsSubsystem>();
	if (!PlaceStampsSubsystem ||
		!PlaceStampsSubsystem->StructOnScope)
	{
		return;
	}

	for (AActor* Actor : Actors)
	{
		AVoxelStampActor* StampActor = Cast<AVoxelStampActor>(Actor);
		if (!StampActor)
		{
			continue;
		}

		(*PlaceStampsSubsystem->StructOnScope)->ApplyOnStamp(StampActor->GetStamp());
		StampActor->UpdateStamp();
	}
}

void UVoxelPlaceStampsSubsystem::SaveDefaults()
{
	const UVoxelPlaceStampsSubsystem* PlaceStampsSubsystem = GEditor->GetEditorSubsystem<UVoxelPlaceStampsSubsystem>();
	if (!PlaceStampsSubsystem ||
		!PlaceStampsSubsystem->StructOnScope)
	{
		return;
	}

	const FVoxelStructView View(*PlaceStampsSubsystem->StructOnScope);

	const FString Value =
		FVoxelUtilities::PropertyToText_Direct(
			*FVoxelUtilities::MakeStructProperty(StaticStructFast<FVoxelPlaceStampDefaults>()),
			View.GetStructMemory(),
			nullptr);
	GConfig->SetString(TEXT("VoxelPlaceStamps"), TEXT("Defaults"), *Value, GEditorPerProjectIni);
}

void UVoxelPlaceStampsSubsystem::RegisterDrawer()
{
	UStatusBarSubsystem* StatusBarSubsystem = GEditor->GetEditorSubsystem<UStatusBarSubsystem>();
	UVoxelPlaceStampsSubsystem* PlaceStampsSubsystem = GEditor->GetEditorSubsystem<UVoxelPlaceStampsSubsystem>();
	if (!StatusBarSubsystem ||
		!PlaceStampsSubsystem)
	{
		return;
	}

	FWidgetDrawerConfig Config("PlaceVoxelStamps");
	Config.ButtonText = INVTEXT("Voxel Stamps");
	Config.Icon = FAppStyle::GetBrush("LevelEditor.Tabs.PlacementBrowser");
	Config.ToolTipText = INVTEXT("Opens a temporary window with available voxel stamps to place into world (Shift+Space)");
	Config.GetDrawerContentDelegate.BindWeakLambda(PlaceStampsSubsystem, [PlaceStampsSubsystem]() -> TSharedRef<SWidget>
	{
		return PlaceStampsSubsystem->GetDrawerWidget().ToSharedRef();
	});
	Config.OnDrawerOpenedDelegate.BindWeakLambda(PlaceStampsSubsystem, [PlaceStampsSubsystem](FName StatusBarName)
	{
		PlaceStampsSubsystem->GetDrawerWidget()->FocusSearchBox();
	});

	StatusBarSubsystem->RegisterDrawer("LevelEditor.StatusBar", MoveTemp(Config), 1);
}

void UVoxelPlaceStampsSubsystem::UnregisterDrawer()
{
	UStatusBarSubsystem* StatusBarSubsystem = GEditor->GetEditorSubsystem<UStatusBarSubsystem>();
	if (!StatusBarSubsystem)
	{
		return;
	}

	StatusBarSubsystem->UnregisterDrawer("LevelEditor.StatusBar", "PlaceVoxelStamps");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelPlaceStampsCommands : public TVoxelCommands<FVoxelPlaceStampsCommands>
{
public:
	TSharedPtr<FUICommandInfo> TogglePlaceStampsDrawer;

	virtual void RegisterCommands() override;
};

DEFINE_VOXEL_COMMANDS(FVoxelPlaceStampsCommands);

void FVoxelPlaceStampsCommands::RegisterCommands()
{
#if PLATFORM_MAC
	VOXEL_UI_COMMAND(TogglePlaceStampsDrawer, "Open Place Stamps Drawer", "Opens a temporary window with available voxel stamps to place into world (Shift+Space)", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Shift, EKeys::SpaceBar));
#else
	VOXEL_UI_COMMAND(TogglePlaceStampsDrawer, "Open Place Stamps Drawer", "Opens a temporary window with available voxel stamps to place into world (Shift+Space)", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Shift, EKeys::SpaceBar));
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FLevelEditorModule& Module = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	Module.OnTabManagerChanged().AddLambda([]
	{
		const FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();

		LevelEditorTabManager->RegisterTabSpawner("PlaceVoxelStamps", FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
		{
			TSharedRef<SDockTab> DockTab =
				SNew(SDockTab)
				.Label(INVTEXT("Place Voxel Stamps"));

			DockTab->SetContent(SNew(SVoxelPlaceStampsTab, DockTab));

			return DockTab;
		}))
		.SetDisplayName(INVTEXT("Place Voxel Stamps"))
		.SetTooltipText({})
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.PlacementBrowser"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory());
	});

	Module.OnRegisterLayoutExtensions().AddLambda([](FLayoutExtender& Extender)
	{
		Extender.ExtendLayout(
			LevelEditorTabIds::PlacementBrowser,
			ELayoutExtensionPosition::After,
			FTabManager::FTab(FTabId("PlaceVoxelStamps"), ETabState::ClosedTab));
	});

	FVoxelUtilities::DelayedCall([]
	{
		if (GetDefault<UVoxelEditorSettings>()->bEnablePlaceStampsDrawer)
		{
			UVoxelPlaceStampsSubsystem::RegisterDrawer();
		}
	});

	const auto TogglePlaceStampsDrawer = []
	{
		if (UStatusBarSubsystem* StatusBarSubsystem = GEditor->GetEditorSubsystem<UStatusBarSubsystem>())
		{
			StatusBarSubsystem->TryToggleDrawer("PlaceVoxelStamps");
		}
	};

	const FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetGlobalLevelEditorActions()->MapAction(
		FVoxelPlaceStampsCommands::Get().TogglePlaceStampsDrawer,
		MakeLambdaDelegate(TogglePlaceStampsDrawer));

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.GetAllContentBrowserCommandExtenders().Add(MakeLambdaDelegate([TogglePlaceStampsDrawer](TSharedRef<FUICommandList> OutCommandList, FOnContentBrowserGetSelection InGetSelectionDelegate)
	{
		OutCommandList->MapAction(
			FVoxelPlaceStampsCommands::Get().TogglePlaceStampsDrawer,
			MakeLambdaDelegate(TogglePlaceStampsDrawer));
	}));
}