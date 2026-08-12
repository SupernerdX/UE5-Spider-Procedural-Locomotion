// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelToolToolkit.h"
#include "VoxelToolEdMode.h"
#include "VoxelStampComponent.h"
#include "Sculpt/Height/Tools/VoxelHeightTool.h"
#include "Sculpt/Height/VoxelHeightSculptStamp.h"
#include "Sculpt/Volume/VoxelVolumeSculptStamp.h"
#include "Sculpt/Volume/Tools/VoxelVolumeTool.h"

#include "EditorModes.h"
#include "EditorModeManager.h"
#include "Styling/ToolBarStyle.h"
#include "IStructureDetailsView.h"
#include "Widgets/Layout/SHeader.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "Sculpt/Height/VoxelSculptHeight.h"
#include "Toolkits/AssetEditorModeUILayer.h"

VOXEL_INITIALIZE_STYLE(Voxel)
{
	Set("Voxel.AngleTool", new IMAGE_BRUSH("Voxel/Voxel_AngleTool_x40", CoreStyleConstants::Icon20x20));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelToolToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, const TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);

	StructOnScope = MakeShared<TStructOnScope<FVoxelToolSculptActorSelector>>();

	AVoxelSculptActorBase* Actor = nullptr;
	if (const TSharedPtr<FVoxelToolEdMode> EdMode = GetEdMode())
	{
		Actor = EdMode->GetSculptActor();
	}

	if (!Actor)
	{
		if (USelection* SelectedActors = GEditor->GetSelectedActors())
		{
			TArray<AActor*> Actors;
			SelectedActors->GetSelectedObjects(Actors);

			for (AActor* OtherActor : Actors)
			{
				Actor = Cast<AVoxelSculptActorBase>(OtherActor);

				if (Actor)
				{
					break;
				}
			}
		}

		if (Actor)
		{
			const TSharedPtr<FVoxelToolEdMode> EdMode = GetEdMode();
			if (ensure(EdMode))
			{
				EdMode->SetSculptActor(Actor);
			}
			else
			{
				Actor = nullptr;
			}
		}
	}

	StructOnScope->InitializeAs<FVoxelToolSculptActorSelector>(FVoxelToolSculptActorSelector{ Actor });

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs Args;
	Args.bAllowSearch = false;
	Args.bShowOptions = false;
	Args.bHideSelectionTip = true;
	Args.bShowPropertyMatrixButton = false;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
	const FStructureDetailsViewArgs StructArgs;

	ActorDetailsView = PropertyEditorModule.CreateStructureDetailView(Args, StructArgs, StructOnScope);
	ActorDetailsView->GetOnFinishedChangingPropertiesDelegate().Add(MakeWeakPtrDelegate(this, [this](const FPropertyChangedEvent& PropertyChangedEvent)
	{
		const TSharedPtr<FVoxelToolEdMode> EdMode = GetEdMode();
		if (!ensure(EdMode))
		{
			return;
		}

		if (EdMode->ActiveTool)
		{
			EdMode->ActiveTool->CallExit(false);
		}

		EdMode->SetSculptActor((*StructOnScope)->Actor);
		UpdatePrimaryModePanel();
	}));

	SetCurrentPalette("Sculpt");
}

FEdMode* FVoxelToolToolkit::GetEditorMode() const
{
	if (const TSharedPtr<FVoxelToolEdMode> EdMode = GetEdMode())
	{
		return EdMode.Get();
	}

	return nullptr;
}

FName FVoxelToolToolkit::GetToolkitFName() const
{
	return "VoxelToolEdMode";
}

FText FVoxelToolToolkit::GetBaseToolkitName() const
{
	return INVTEXT("Voxel Sculpt");
}

FText FVoxelToolToolkit::GetActiveToolDisplayName() const
{
	return INVTEXT("FVoxelToolToolkit::GetActiveToolDisplayName");
}

FText FVoxelToolToolkit::GetActiveToolMessage() const
{
	return INVTEXT("FVoxelToolToolkit::GetActiveToolMessage");
}

void FVoxelToolToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add("Sculpt");
}

FText FVoxelToolToolkit::GetToolPaletteDisplayName(const FName Palette) const
{
	if (Palette == STATIC_FNAME("Sculpt"))
	{
		return INVTEXT("Sculpt");
	}

	return FModeToolkit::GetToolPaletteDisplayName(Palette);
}

void FVoxelToolToolkit::OnToolPaletteChanged(const FName PaletteName)
{
	FModeToolkit::OnToolPaletteChanged(PaletteName);
}

TSharedRef<SWidget> FVoxelToolToolkit::CreatePaletteWidget(TSharedPtr<FUICommandList> InCommandList, FName InToolbarCustomizationName, FName InPaletteName)
{
	const TSharedPtr<FVoxelToolEdMode> EdMode = GetEdMode();
	if (!ensure(EdMode))
	{
		return SNullWidget::NullWidget;
	}

	TSharedRef<SVerticalBox> VerticalBox =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			ActorDetailsView->GetWidget().ToSharedRef()
		];

	AVoxelSculptActorBase* Actor = (*StructOnScope)->Actor;

	if (!Actor)
	{
		EdMode->SetSculptActor(nullptr);

		VerticalBox->AddSlot()
		.Padding(4.f)
		.HAlign(HAlign_Center)
		.AutoHeight()
		[
			SNew(SVoxelDetailText)
			.ColorAndOpacity(FStyleColors::Warning)
			.Text(INVTEXT("Please select a Voxel Sculpt Actor to start sculpting"))
		];

		OnChangeTool(nullptr);
		return VerticalBox;
	}

	const UClass* TargetToolClass =
		Actor->IsA<AVoxelSculptHeight>()
		? UVoxelHeightTool::StaticClass()
		: UVoxelVolumeTool::StaticClass();

	FUniformToolBarBuilder ToolbarBuilder(InCommandList, FMultiBoxCustomization(InToolbarCustomizationName));
	ToolbarBuilder.SetStyle(&FAppStyle::Get(), "PaletteToolBar");

	if (EdMode->ActiveTool &&
		!EdMode->ActiveToolClass->IsChildOf(TargetToolClass))
	{
		EdMode->ActiveTool = nullptr;
		EdMode->ActiveToolClass = nullptr;
	}

	for (auto& It : EdMode->GetClassToTool())
	{
		if (!It.Key->IsChildOf(TargetToolClass))
		{
			continue;
		}

		FSlateIcon Icon = FSlateIcon(FVoxelEditorStyle::GetStyleSetName(), "VoxelEdMode");
		FName StyleName = FAppStyle::GetAppStyleSetName();
		if (It.Key->HasMetaData(STATIC_FNAME("StyleSet")))
		{
			StyleName = FName(It.Key->GetMetaData(STATIC_FNAME("StyleSet")));
		}

		if (It.Key->HasMetaData(STATIC_FNAME("Icon")))
		{
			Icon = FSlateIcon(StyleName, FName(It.Key->GetMetaData(STATIC_FNAME("Icon"))));
		}

		ToolbarBuilder.AddToolBarButton(
			FUIAction(
				MakeWeakPtrDelegate(this, [this, Class = It.Key]
				{
					OnChangeTool(Class);
				}),
				{},
				MakeWeakPtrDelegate(this, [this, Class = It.Key]
				{
					return IsToolActive(Class);
				})),
			{},
			It.Key->GetDisplayNameText(),
			It.Key->GetToolTipText(),
			Icon,
			EUserInterfaceActionType::ToggleButton);

		if (!IsValid(EdMode->ActiveTool))
		{
			EdMode->ActiveTool = It.Value;
			EdMode->ActiveToolClass = It.Key;
		}
	}

	VerticalBox->AddSlot()
	.AutoHeight()
	[
		ToolbarBuilder.MakeWidget()
	];

	if (DetailsView)
	{
		DetailsView->SetObject(EdMode->ActiveTool);
	}

	EdMode->bReinitializeTools = false;

	return VerticalBox;
}

void FVoxelToolToolkit::RequestModeUITabs()
{
	const TSharedPtr<FAssetEditorModeUILayer> PinnedModeUILayerPtr = ModeUILayer.Pin();
	if (!PinnedModeUILayerPtr)
	{
		return;
	}

	PrimaryTabInfo.OnSpawnTab = MakeWeakPtrDelegate(this, [this](const FSpawnTabArgs& Args)
	{
		TSharedRef<SDockTab> Tab = CreatePrimaryModePanel(Args);
		Tab->SetOnTabClosed(MakeLambdaDelegate([](TSharedRef<SDockTab>)
		{
			if (GLevelEditorModeTools().IsModeActive("VoxelToolEdMode"))
			{
				GLevelEditorModeTools().DeactivateMode("VoxelToolEdMode");
				GLevelEditorModeTools().ActivateDefaultMode();
			}
		}));
		return Tab;
	});
	PrimaryTabInfo.TabLabel = INVTEXT("Mode Toolbox");
	PrimaryTabInfo.TabTooltip = INVTEXT("Open the Modes tab, which contains the active editor mode's settings.");
	PinnedModeUILayerPtr->SetModePanelInfo(UAssetEditorUISubsystem::TopLeftTabID, PrimaryTabInfo);
}

TSharedPtr<SWidget> FVoxelToolToolkit::GetInlineContent() const
{
	return
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			DetailsView.ToSharedRef()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Text(INVTEXT("Exit Sculpt Mode"))
			.OnClicked_Lambda([]
			{
				GLevelEditorModeTools().DeactivateMode("VoxelToolEdMode");
				GLevelEditorModeTools().ActivateDefaultMode();
				return FReply::Handled();
			})
		];
}

void FVoxelToolToolkit::OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	FModeToolkit::OnToolStarted(Manager, Tool);
}

void FVoxelToolToolkit::OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	FModeToolkit::OnToolEnded(Manager, Tool);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelToolToolkit::SwitchSculptActor(AVoxelSculptActorBase* NewSculptActor) const
{
	(*StructOnScope)->Actor = NewSculptActor;
	ActorDetailsView->GetDetailsView()->ForceRefresh();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelToolToolkit::IsToolActive(const TSubclassOf<UVoxelTool>& Class) const
{
	const TSharedPtr<FVoxelToolEdMode> EdMode = GetEdMode();
	if (!EdMode)
	{
		return false;
	}

	return EdMode->ActiveToolClass == Class;
}

void FVoxelToolToolkit::OnChangeTool(const TSubclassOf<UVoxelTool>& Class) const
{
	const TSharedPtr<FVoxelToolEdMode> EdMode = GetEdMode();
	if (!ensure(EdMode))
	{
		return;
	}

	if (EdMode->ActiveTool)
	{
		EdMode->ActiveTool->CallExit(false);
	}

	EdMode->ActiveTool = EdMode->GetTool(Class);
	EdMode->ActiveToolClass = Class;

	if (DetailsView)
	{
		DetailsView->SetObject(EdMode->ActiveTool);
	}
}

void FVoxelToolToolkit::OnStampComponentChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
}

TSharedPtr<FVoxelToolEdMode> FVoxelToolToolkit::GetEdMode() const
{
	return WeakEdMode.Pin();
}