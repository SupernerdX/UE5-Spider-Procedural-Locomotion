// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelGraph.h"
#include "VoxelTerminalGraph.h"
#include "VoxelParameterOverridesDetails.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

class FUVoxelTerminalGraphCustomization : public FVoxelDetailCustomization
{
public:
	static TVoxelArray<UVoxelTerminalGraph*> GetObjectsBeingCustomized(IDetailLayoutBuilder& DetailLayout) { return FVoxelEditorUtilities::GetObjectsBeingCustomized<UVoxelTerminalGraph>(DetailLayout); }
	static UVoxelTerminalGraph* GetUniqueObjectBeingCustomized(IDetailLayoutBuilder& DetailLayout) { return FVoxelEditorUtilities::GetUniqueObjectBeingCustomized<UVoxelTerminalGraph>(DetailLayout); }
	static TVoxelArray<TVoxelObjectPtr<UVoxelTerminalGraph>> GetWeakObjectsBeingCustomized(IDetailLayoutBuilder& DetailLayout) { return TVoxelArray<TVoxelObjectPtr<UVoxelTerminalGraph>>(GetObjectsBeingCustomized(DetailLayout)); }
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
	{
		UVoxelTerminalGraph* TerminalGraph = GetUniqueObjectBeingCustomized(DetailLayout);
		if (!ensure(TerminalGraph))
		{
			return;
		}

		if (IDetailsView* DetailView = DetailLayout.UE_506_SWITCH(GetDetailsView, GetDetailsViewSharedPtr().Get)())
		{
			if (DetailView->GetGenericLayoutDetailsDelegate().IsBound())
			{
				// Manual customization, exit
				return;
			}
		}

		const TVoxelObjectPtr<UVoxelTerminalGraph> WeakTerminalGraph = TerminalGraph;

		// Hide ExposeToLibrary
		{
			const TSharedRef<IPropertyHandle> ExposeToLibraryHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelTerminalGraph, bExposeToLibrary));
			const TSharedRef<IPropertyHandle> KeywordsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelTerminalGraph, Keywords));
			const TSharedRef<IPropertyHandle> WhitelistedTypesHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelTerminalGraph, WhitelistedTypes));
			if (!TerminalGraph->GetGraph().IsFunctionLibrary())
			{
				ExposeToLibraryHandle->MarkHiddenByCustomization();
				KeywordsHandle->MarkHiddenByCustomization();
				WhitelistedTypesHandle->MarkHiddenByCustomization();
			}
		}

		if (TerminalGraph->IsMainTerminalGraph())
		{
			const TVoxelObjectPtr<UVoxelGraph> WeakGraph = &TerminalGraph->GetGraph();

			for (UVoxelGraph* Graph : TerminalGraph->GetGraph().GetBaseGraphs())
			{
				BaseGraphs.Add(Graph);
			}

			IDetailCategoryBuilder& Category = DetailLayout.EditCategory(
				"Config",
				{},
				ECategoryPriority::Important);

			Category.AddCustomRow(INVTEXT("BaseGraph"))
			.NameContent()
			[
				SNew(SVoxelDetailText)
				.Text(INVTEXT("Base Graph"))
			]
			.ValueContent()
			[
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(TerminalGraph->GetGraph().GetClass())
				.AllowClear(true)
				.ThumbnailPool(FVoxelEditorUtilities::GetThumbnailPool())
				.ObjectPath_Lambda([=]() -> FString
				{
					const UVoxelGraph* Graph = WeakGraph.Resolve();
					if (!ensureVoxelSlow(Graph))
					{
						return {};
					}

					const UVoxelGraph* BaseGraph = Graph->GetBaseGraph_Unsafe();
					if (!BaseGraph)
					{
						return {};
					}

					return BaseGraph->GetPathName();
				})
				.OnShouldFilterAsset_Lambda([this](const FAssetData& Asset)
				{
					return BaseGraphs.Contains(Asset.GetSoftObjectPath());
				})
				.OnObjectChanged_Lambda([=, this](const FAssetData& NewAssetData)
				{
					UVoxelGraph* ThisGraph = WeakGraph.Resolve();
					if (!ensureVoxelSlow(ThisGraph))
					{
						return;
					}

					ThisGraph->PreEditChange(nullptr);
					ThisGraph->SetBaseGraph(CastEnsured<UVoxelGraph>(NewAssetData.GetAsset()));
					ThisGraph->PostEditChange();

					BaseGraphs.Reset();
					for (UVoxelGraph* Graph : ThisGraph->GetBaseGraphs())
					{
						BaseGraphs.Add(Graph);
					}
				})
			];

			Category.AddProperty(DetailLayout.AddObjectPropertyData({ WeakGraph.Resolve() }, GET_MEMBER_NAME_STATIC(UVoxelGraph, Category)));
			Category.AddProperty(DetailLayout.AddObjectPropertyData({ WeakGraph.Resolve() }, GET_MEMBER_NAME_STATIC(UVoxelGraph, GraphTags)));
			Category.AddProperty(DetailLayout.AddObjectPropertyData({ WeakGraph.Resolve() }, GET_MEMBER_NAME_STATIC(UVoxelGraph, Description)));
			Category.AddProperty(DetailLayout.AddObjectPropertyData({ WeakGraph.Resolve() }, GET_MEMBER_NAME_STATIC(UVoxelGraph, DisplayNameOverride)));
			Category.AddProperty(DetailLayout.AddObjectPropertyData({ WeakGraph.Resolve() }, GET_MEMBER_NAME_STATIC(UVoxelGraph, bShowInContextMenu)));
			Category.AddProperty(DetailLayout.AddObjectPropertyData({ WeakGraph.Resolve() }, GET_MEMBER_NAME_STATIC(UVoxelGraph, AssetIcon)));

			UVoxelGraph* Graph = &TerminalGraph->GetGraph();

			KeepAlive(FVoxelParameterOverridesDetails::Create(
				DetailLayout,
				[&](TVoxelArray<FVoxelParameterOverridesDetails::FWeakOwner>& OutOwners)
				{
					OutOwners.Add(FVoxelParameterOverridesDetails::FWeakOwner
					{
						Graph,
						nullptr,
						nullptr,
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
				},
				FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout)));

			return;
		}

		if (TerminalGraph->IsEditorTerminalGraph())
		{
			// TODO?
			return;
		}

		IDetailCategoryBuilder& FunctionCategory = DetailLayout.EditCategory("Function");

		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////

		FunctionCategory.AddCustomRow(INVTEXT("Name"))
		.IsEnabled(TerminalGraph->IsTopmostTerminalGraph())
		.NameContent()
		[
			SNew(SVoxelDetailText)
			.Text(INVTEXT("Name"))
		]
		.ValueContent()
		[
			SNew(SBox)
			.MinDesiredWidth(125.f)
			[
				SNew(SEditableTextBox)
				.Font(FVoxelEditorUtilities::Font())
				.Text_Lambda([=]() -> FText
				{
					const UVoxelTerminalGraph* LocalTerminalGraph = WeakTerminalGraph.Resolve();
					if (!ensure(LocalTerminalGraph))
					{
						return {};
					}
					return FText::FromString(LocalTerminalGraph->GetDisplayName());
				})
				.OnTextCommitted_Lambda([=](const FText& NewText, ETextCommit::Type)
				{
					UVoxelTerminalGraph* LocalTerminalGraph = WeakTerminalGraph.Resolve();
					if (!ensure(LocalTerminalGraph))
					{
						return;
					}

					LocalTerminalGraph->PreEditChange(nullptr);
					LocalTerminalGraph->UpdateMetadata([&](FVoxelGraphMetadata& Metadata)
					{
						Metadata.DisplayName = NewText.ToString();
					});
					LocalTerminalGraph->PostEditChange();
				})
			]
		];

		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////

		FunctionCategory.AddCustomRow(INVTEXT("Category"))
		.IsEnabled(TerminalGraph->IsTopmostTerminalGraph())
		.NameContent()
		[
			SNew(SVoxelDetailText)
			.Text(INVTEXT("Category"))
		]
		.ValueContent()
		[
			SNew(SBox)
			.MinDesiredWidth(125.f)
			[
				SNew(SVoxelDetailComboBox<FString>)
				.RefreshDelegate(this, DetailLayout)
				.Options_Lambda([WeakGraph = MakeVoxelObjectPtr(TerminalGraph->GetGraph())]() -> TArray<FString>
				{
					const UVoxelGraph* Graph = WeakGraph.Resolve();
					if (!ensure(Graph))
					{
						return {};
					}

					TSet<FString> Categories;
					for (const FGuid& Guid : Graph->GetTerminalGraphs())
					{
						if (Guid == GVoxelMainTerminalGraphGuid ||
							Guid == GVoxelEditorTerminalGraphGuid)
						{
							continue;
						}

						Categories.Add(Graph->FindTerminalGraphChecked(Guid).GetMetadata().Category);
					}

					Categories.Remove("");
					Categories.Add("Default");
					return Categories.Array();
				})
				.CurrentOption_Lambda([=]() -> FString
				{
					const UVoxelTerminalGraph* LocalTerminalGraph = WeakTerminalGraph.Resolve();
					if (!ensure(LocalTerminalGraph))
					{
						return {};
					}

					const FString Category = LocalTerminalGraph->GetMetadata().Category;
					if (Category.IsEmpty())
					{
						return "Default";
					}
					return Category;
				})
				.CanEnterCustomOption(true)
				.OptionText(MakeLambdaDelegate([](const FString Option)
				{
					return Option;
				}))
				.OnSelection_Lambda([=](const FString NewValue)
				{
					UVoxelTerminalGraph* LocalTerminalGraph = WeakTerminalGraph.Resolve();
					if (!ensure(LocalTerminalGraph))
					{
						return;
					}

					LocalTerminalGraph->PreEditChange(nullptr);
					LocalTerminalGraph->UpdateMetadata([&](FVoxelGraphMetadata& Metadata)
					{
						if (NewValue == "Default")
						{
							Metadata.Category = {};
						}
						else
						{
							Metadata.Category = NewValue;
						}
					});
					LocalTerminalGraph->PostEditChange();
				})
			]
		];

		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////

		FunctionCategory.AddCustomRow(INVTEXT("Description"))
		.IsEnabled(TerminalGraph->IsTopmostTerminalGraph())
		.NameContent()
		[
			SNew(SVoxelDetailText)
			.Text(INVTEXT("Description"))
		]
		.ValueContent()
		[
			SNew(SBox)
			.MinDesiredWidth(125.f)
			[
				SNew(SMultiLineEditableTextBox)
				.Font(FVoxelEditorUtilities::Font())
				.Text_Lambda([=]() -> FText
				{
					const UVoxelTerminalGraph* LocalTerminalGraph = WeakTerminalGraph.Resolve();
					if (!ensure(LocalTerminalGraph))
					{
						return {};
					}

					return FText::FromString(LocalTerminalGraph->GetMetadata().Description);
				})
				.OnTextCommitted_Lambda([=](const FText& NewText, ETextCommit::Type)
				{
					UVoxelTerminalGraph* LocalTerminalGraph = WeakTerminalGraph.Resolve();
					if (!ensure(LocalTerminalGraph))
					{
						return;
					}

					LocalTerminalGraph->PreEditChange(nullptr);
					LocalTerminalGraph->UpdateMetadata([&](FVoxelGraphMetadata& Metadata)
					{
						Metadata.Description = NewText.ToString();
					});
					LocalTerminalGraph->PostEditChange();
				})
			]
		];
	}

private:
	TSet<FSoftObjectPath> BaseGraphs;
};

DEFINE_VOXEL_CLASS_LAYOUT(UVoxelTerminalGraph, FUVoxelTerminalGraphCustomization);