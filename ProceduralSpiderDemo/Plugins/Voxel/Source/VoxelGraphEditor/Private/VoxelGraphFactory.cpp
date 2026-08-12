// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphFactory.h"
#include "VoxelGraph.h"
#include "Interfaces/IMainFrameModule.h"
#include "Widgets/SVoxelNewGraphAssetDialog.h"

UVoxelGraphFactory::UVoxelGraphFactory()
{
	SupportedClass = UVoxelGraph::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

bool UVoxelGraphFactory::ConfigureProperties()
{
	const IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
	const TSharedPtr<SWindow> ParentWindow = MainFrame.GetParentWindow();

	const TSharedRef<SWindow> Window =
		SNew(SWindow)
		.Title(INVTEXT("Create New Voxel Graph"))
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(450.f, 500.f))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	const TSharedRef<SVoxelNewGraphAssetDialog> NewGraphAssetWidget =
		SNew(SVoxelNewGraphAssetDialog)
		.ParentWindow(Window);

	Window->SetContent(
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		.Padding(0.f)
		[
			NewGraphAssetWidget
		]);

	FSlateApplication::Get().AddModalWindow(Window, ParentWindow);

	if (!NewGraphAssetWidget->bCreateAsset)
	{
		return false;
	}

	BaseGraph = NewGraphAssetWidget->SelectedGraph.Get();
	return BaseGraph != nullptr;
}

UObject* UVoxelGraphFactory::FactoryCreateNew(UClass* Class, UObject* InParent, const FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (!ensure(BaseGraph))
	{
		return nullptr;
	}

	UVoxelGraph* Result = DuplicateObject(BaseGraph, InParent, Name);
	if (!ensure(Result))
	{
		return nullptr;
	}

	Result->SetFlags(Flags);
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelGraphBaseFactory::UVoxelGraphBaseFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UVoxelGraphBaseFactory::FactoryCreateNew(UClass* Class, UObject* InParent, const FName Name, const EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	const UVoxelGraph* AssetToCopy = nullptr;
	if (ensure(Class))
	{
		AssetToCopy = Class->GetDefaultObject<UVoxelGraph>()->GetFactoryInfo().Template;
	}

	if (!ensure(AssetToCopy))
	{
		return NewObject<UVoxelGraph>(InParent, Class, Name, Flags);
	}

	UVoxelGraph* NewGraph = DuplicateObject(AssetToCopy, InParent, Name);
	if (!ensure(NewGraph))
	{
		return nullptr;
	}

	return NewGraph;
}