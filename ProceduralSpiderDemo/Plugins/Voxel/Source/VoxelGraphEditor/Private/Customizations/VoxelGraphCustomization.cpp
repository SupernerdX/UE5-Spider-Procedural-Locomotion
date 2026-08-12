// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelGraph.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"
#include "VoxelGraphEnvironment.h"
#include "VoxelParameterOverridesDetails.h"

VOXEL_CUSTOMIZE_CLASS(UVoxelGraph)(IDetailLayoutBuilder& DetailLayout)
{
	FVoxelEditorUtilities::EnableRealtime();

	if (IDetailsView* DetailView = DetailLayout.UE_506_SWITCH(GetDetailsView, GetDetailsViewSharedPtr().Get)())
	{
		if (DetailView->GetGenericLayoutDetailsDelegate().IsBound())
		{
			// Manual customization, exit
			DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelGraph, Category))->MarkHiddenByCustomization();
			DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelGraph, GraphTags))->MarkHiddenByCustomization();
			DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelGraph, Description))->MarkHiddenByCustomization();
			DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelGraph, DisplayNameOverride))->MarkHiddenByCustomization();
			DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelGraph, bShowInContextMenu))->MarkHiddenByCustomization();
			DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelGraph, AssetIcon))->MarkHiddenByCustomization();
			return;
		}
	}

	const TVoxelArray<TVoxelObjectPtr<UVoxelGraph>> WeakGraphs = GetWeakObjectsBeingCustomized(DetailLayout);

	const auto GetBaseGraphs = [=]
	{
		TVoxelSet<UVoxelGraph*> BaseGraphs;
		for (const TVoxelObjectPtr<UVoxelGraph>& WeakGraph : WeakGraphs)
		{
			const UVoxelGraph* Graph = WeakGraph.Resolve();
			if (!ensureVoxelSlow(Graph))
			{
				continue;
			}

			if (!Graph->GetBaseGraph_Unsafe())
			{
				continue;
			}

			BaseGraphs.Add(Graph->GetBaseGraph_Unsafe());
		}
		return BaseGraphs;
	};

	// Force graph at the top
	IDetailCategoryBuilder& Category = DetailLayout.EditCategory(
		"Config",
		{},
		ECategoryPriority::Important);

	Category.AddCustomRow(INVTEXT("BaseGraph"))
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
		MakeAttributeLambda(MakeWeakPtrLambda(this, [=]
		{
			return GetBaseGraphs().Num() > 0;
		})),
		MakeWeakPtrDelegate(this, [=]
		{
			FScopedTransaction Transaction(INVTEXT("Set Base Graph"));
			bool bGraphAssigned = false;

			for (const TVoxelObjectPtr<UVoxelGraph>& WeakGraph : WeakGraphs)
			{
				UVoxelGraph* Graph = WeakGraph.Resolve();
				if (!ensure(Graph))
				{
					continue;
				}

				FVoxelTransaction InnerTransaction(Graph);
				Graph->PreEditChange(nullptr);
				Graph->SetBaseGraph(nullptr);
				Graph->PostEditChange();
				bGraphAssigned = true;
			}

			if (!bGraphAssigned)
			{
				Transaction.Cancel();
			}
		}),
		false))
	.NameContent()
	[
		SNew(SVoxelDetailText)
		.Text(INVTEXT("Base Graph"))
	]
	.ValueContent()
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVoxelDetailText)
			.Text(INVTEXT("Multiple Values"))
			.Visibility_Lambda([=]
			{
				return GetBaseGraphs().Num() > 1 ? EVisibility::Visible : EVisibility::Collapsed;
			})
		]
		+ SOverlay::Slot()
		[
			SNew(SObjectPropertyEntryBox)
			.Visibility_Lambda([=]
			{
				return GetBaseGraphs().Num() < 2 ? EVisibility::Visible : EVisibility::Collapsed;
			})
			.AllowedClass(UVoxelGraph::StaticClass())
			.AllowClear(true)
			.ThumbnailPool(FVoxelEditorUtilities::GetThumbnailPool())
			.ObjectPath_Lambda([=]() -> FString
			{
				const TVoxelSet<UVoxelGraph*> BaseGraphs = GetBaseGraphs();
				if (BaseGraphs.Num() != 1)
				{
					return {};
				}

				const UVoxelGraph* BaseGraph = BaseGraphs.GetUniqueValue();
				if (!BaseGraph)
				{
					return {};
				}

				return BaseGraph->GetPathName();
			})
			.OnShouldFilterAsset_Lambda([=](const FAssetData& AssetData)
			{
				for (TVoxelObjectPtr<UVoxelGraph> Graph : WeakGraphs)
				{
					if (AssetData.GetSoftObjectPath() == FSoftObjectPath(Graph.GetPathName()))
					{
						return true;
					}
				}
				return false;
			})
			.OnObjectChanged_Lambda([=](const FAssetData& NewAssetData)
			{
				UVoxelGraph* NewBaseGraph = CastEnsured<UVoxelGraph>(NewAssetData.GetAsset());

				TVoxelSet<UVoxelGraph*> BaseGraphs;
				if (NewBaseGraph)
				{
					for (UVoxelGraph* BaseGraph : NewBaseGraph->GetBaseGraphs())
					{
						if (BaseGraph == NewBaseGraph)
						{
							continue;
						}

						BaseGraphs.Add(BaseGraph);
					}
				}

				FScopedTransaction Transaction(INVTEXT("Set Base Graph"));
				bool bGraphAssigned = false;
				for (const TVoxelObjectPtr<UVoxelGraph>& WeakGraph : WeakGraphs)
				{
					UVoxelGraph* Graph = WeakGraph.Resolve();
					if (!ensure(Graph))
					{
						continue;
					}

					if (BaseGraphs.Contains(Graph))
					{
						VOXEL_MESSAGE(Error, "Failed to assign base graph, because {1} already inherits {0}", Graph, NewBaseGraph);
						continue;
					}

					FVoxelTransaction InnerTransaction(Graph);
					Graph->PreEditChange(nullptr);
					Graph->SetBaseGraph(NewBaseGraph);
					Graph->PostEditChange();
					bGraphAssigned = true;
				}

				if (!bGraphAssigned)
				{
					Transaction.Cancel();
				}
			})
		]
	];

	{
		FSimpleDelegate RefreshDelegate = FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout);

		TArray<UObject*> MainTerminalGraphs;
		for (const TVoxelObjectPtr<UVoxelGraph>& WeakGraph : WeakGraphs)
		{
			UVoxelGraph* Graph = WeakGraph.Resolve();
			if (!Graph ||
				!Graph->HasMainTerminalGraph())
			{
				continue;
			}

			MainTerminalGraphs.Add(&Graph->GetMainTerminalGraph());
			GVoxelGraphTracker->OnTerminalGraphChanged(*Graph).Add(FOnVoxelGraphChanged::Make(this, [RefreshDelegate]
			{
				RefreshDelegate.ExecuteIfBound();
			}));
		}

		if (MainTerminalGraphs.Num() > 0)
		{
			Category.AddExternalObjectProperty(
				MainTerminalGraphs,
				GET_MEMBER_NAME_CHECKED(UVoxelTerminalGraph, bDisableNodeMerging),
				EPropertyLocation::Advanced);
		}
	}

	KeepAlive(FVoxelParameterOverridesDetails::Create(
		DetailLayout,
		[&](TVoxelArray<FVoxelParameterOverridesDetails::FWeakOwner>& OutOwners)
		{
			for (UVoxelGraph* Graph : GetObjectsBeingCustomized(DetailLayout))
			{
				OutOwners.Add(FVoxelParameterOverridesDetails::FWeakOwner
				{
					Graph,
					MakeWeakObjectPtrLambda(Graph, [Graph]
					{
						return Graph->GetParameterOverrides().GetHash();
					}),
					MakeWeakObjectPtrLambda(Graph, [Graph](FVoxelDependencyCollector& DependencyCollector)
					{
						return FVoxelGraphEnvironment::CreatePreview(
							Graph,
							*Graph,
							FTransform::Identity,
							DependencyCollector);
					}),
					MakeWeakObjectPtrLambda(Graph, [=]
					{
						Graph->PreEditChange(Graph->GetParameterOverridesProperty());
					}),
					MakeWeakObjectPtrLambda(Graph, [=]
					{
						FPropertyChangedEvent Event(Graph->GetParameterOverridesProperty());
						Graph->PostEditChangeProperty(Event);
					})
				});
			}
		},
		FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout)));
}