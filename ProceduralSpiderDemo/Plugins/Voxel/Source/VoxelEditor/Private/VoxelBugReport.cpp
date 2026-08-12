// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelBugReport.h"
#include "SVoxelBugReport.h"
#include "ToolMenus.h"
#include "VoxelEditorSettings.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserCommandExtender>& Extenders = ContentBrowserModule.GetAllContentBrowserCommandExtenders();
	Extenders.Add(MakeLambdaDelegate([](TSharedRef<FUICommandList> CommandList, FOnContentBrowserGetSelection GetSelectionDelegate)
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu");
		FToolMenuSection& Section = Menu->FindOrAddSection("AssetContextReferences");
		Section.AddDynamicEntry("VoxelReportBugCommands", MakeLambdaDelegate([=](FToolMenuSection& InSection)
		{
			if (!GetDefault<UVoxelEditorSettings>()->bEnableContentBrowserActions)
			{
				return;
			}

			InSection.AddMenuEntry(
				"ReportVoxelPluginBug",
				INVTEXT("Report Voxel Plugin Bug"),
				INVTEXT("Upload this asset and its dependencies to report a Voxel Plugin bug"),
				FSlateIcon("RigVMEditorStyle", "RigVM.Bug.Dot"),
				FUIAction(MakeLambdaDelegate([=]
				{
					const FContentBrowserModule& LocalContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

					TArray<FAssetData> SelectedAssets;
					LocalContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

					if (SelectedAssets.Num() == 0)
					{
						VOXEL_MESSAGE(Error, "No assets selected");
						return;
					}

					MakeShared<FVoxelBugReport>()->Open(SelectedAssets);
				})));
		}));
	}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelBugReport::Open(const TArray<FAssetData>& SelectedAssets)
{
	VOXEL_FUNCTION_COUNTER();

	IAssetRegistry& AssetRegistry = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	if (AssetRegistry.IsLoadingAssets())
	{
		AssetRegistry.WaitForCompletion();
	}

	{
		FScopedSlowTask SlowTask(SelectedAssets.Num(), INVTEXT("Gathering Dependencies..."));
		SlowTask.MakeDialog();

		for (const FAssetData& SelectedAsset : SelectedAssets)
		{
			SlowTask.EnterProgressFrame();

			AddPackage(SelectedAsset.PackageName);
		}
	}

	for (auto It = PackagePaths.CreateIterator(); It; ++It)
	{
		if (!It->ToString().StartsWith(TEXT("/Game/")))
		{
			It.RemoveCurrent();
		}
	}

	if (PackagePaths.Num() == 0)
	{
		return;
	}

	const TSharedRef<SWindow> Window =
		SNew(SWindow)
		.Type(EWindowType::Normal)
		.bDragAnywhere(true)
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(1280, 720))
		.SupportsTransparency(EWindowTransparency::PerPixel);

	Window->SetTitle(INVTEXT("Report Voxel Plugin bug"));
	Window->SetContent(
		SNew(SVoxelBugReport, AsShared())
		.OnCloseWindow_Lambda(MakeWeakPtrLambda(Window, [&Window = *Window]
		{
			Window.RequestDestroyWindow();
		})));

	const TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	if (!ensure(RootWindow))
	{
		return;
	}

	FSlateApplication::Get().AddWindowAsNativeChild(Window, RootWindow.ToSharedRef());
	Window->BringToFront();
}

void FVoxelBugReport::AddPackage(const FName PackageName)
{
	if (PackagePaths.Contains(PackageName))
	{
		return;
	}
	PackagePaths.Add_CheckNew(PackageName);

	static IAssetRegistry& AssetRegistry = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	TArray<FName> Dependencies;
	AssetRegistry.GetDependencies(PackageName, Dependencies);

	for (const FName Dependency : Dependencies)
	{
		if (Dependency.ToString().StartsWith(TEXT("/Script")))
		{
			continue;
		}

		// The asset registry can give some reference to some deleted assets. We don't want to migrate these.
		if (!AssetRegistry.GetAssetPackageDataCopy(Dependency).IsSet())
		{
			continue;
		}

		AddPackage(Dependency);
	}

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByPackageName(PackageName, Assets, true);

	for (const FAssetData& AssetData : Assets)
	{
		if (!AssetData.GetClass() ||
			!AssetData.GetClass()->IsChildOf<UWorld>())
		{
			continue;
		}

		for (const FString& ExternalObjectsPath : ULevel::GetExternalObjectsPaths(PackageName.ToString()))
		{
			if (ExternalObjectsPath.IsEmpty() ||
				VisitedExternalObjectsPaths.Contains(ExternalObjectsPath))
			{
				continue;
			}
			VisitedExternalObjectsPaths.Add_CheckNew(ExternalObjectsPath);

			AssetRegistry.ScanPathsSynchronous({ ExternalObjectsPath }, true, true);

			TArray<FAssetData> ExternalObjectAssets;
			AssetRegistry.GetAssetsByPath(FName(ExternalObjectsPath), ExternalObjectAssets, true, true);

			for (const FAssetData& ExternalObjectAsset : ExternalObjectAssets)
			{
				AddPackage(ExternalObjectAsset.PackageName);
			}
		}
	}
}