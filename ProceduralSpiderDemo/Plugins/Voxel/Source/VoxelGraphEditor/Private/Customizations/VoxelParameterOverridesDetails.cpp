// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelParameterOverridesDetails.h"
#include "Customizations/VoxelParameterDetails.h"
#include "VoxelGraph.h"
#include "VoxelGraphQuery.h"
#include "VoxelGraphTracker.h"
#include "VoxelParameterView.h"
#include "VoxelCategoryBuilder.h"
#include "VoxelGraphEnvironment.h"
#include "VoxelExternalParameter.h"
#include "VoxelGraphParametersView.h"
#include "VoxelParameterOverridesOwner.h"
#include "VoxelGraphParametersViewContext.h"
#include "Editor/PropertyEditor/Private/DetailCategoryBuilderImpl.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelParameterOverridesDetails);

TSharedPtr<FVoxelParameterOverridesDetails> FVoxelParameterOverridesDetails::CreateImpl(
	const FVoxelParameterDetailsBuilder& DetailLayout,
	TFunctionRef<void(TVoxelArray<FWeakOwner>&)> GatherOwners,
	const FSimpleDelegate& RefreshDelegate,
	const FString& BaseCategory,
	const TOptional<FGuid> ParameterDefaultValueGuid)
{
	ensure(RefreshDelegate.IsBound());

	TVoxelArray<FWeakOwner> Owners;
	GatherOwners(Owners);

	const TSharedRef<FVoxelParameterOverridesDetails> Result = MakeShareable(new FVoxelParameterOverridesDetails(
		Owners,
		RefreshDelegate,
		ParameterDefaultValueGuid));

	Result->Initialize(DetailLayout, BaseCategory);

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelParameterOverridesDetails::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	if (!IsExplicitlyNull(WeakDetailsView))
	{
		if (bWindowValid)
		{
			if (!IsWindowValid())
			{
				bWindowValid = false;
				return;
			}
		}
		else
		{
			if (IsWindowValid())
			{
				ForceRefresh();
				return;
			}

			return;
		}
	}

	const bool bIsInvalidated = INLINE_LAMBDA
	{
		VOXEL_SCOPE_COUNTER("Check invalidation");

		if (!DependencyTracker ||
			DependencyTracker->IsInvalidated())
		{
			return true;
		}

		for (const FCachedEnvironment& CachedEnvironment : CachedEnvironments)
		{
			if (CachedEnvironment.GetHash() != CachedEnvironment.Hash)
			{
				return true;
			}
		}

		return false;
	};

	if (bIsInvalidated)
	{
		UpdateEnvironments();
	}

	for (auto It = GuidToParameterDetails.CreateIterator(); It; ++It)
	{
		const TSharedPtr<FVoxelParameterDetails> ParameterDetails = It.Value();
		if (!ParameterDetails->IsValid())
		{
			// Details are refreshing
			return;
		}

		ParameterDetails->Tick();
	}

	// Check for graph changes
	TVoxelArray<TVoxelObjectPtr<const UVoxelGraph>> NewGraphs;
	for (const FOwner& Owner : GetOwners())
	{
		NewGraphs.Add(Owner.Parameters.GetGraph());
	}

	if (Graphs != NewGraphs)
	{
		ForceRefresh();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelArray<FVoxelParameterOverridesDetails::FOwner> FVoxelParameterOverridesDetails::GetOwners() const
{
	TVoxelArray<FOwner> Result;
	for (const FWeakOwner& WeakOwner : Owners)
	{
		if (IVoxelParameterOverridesOwner* Owner = WeakOwner.OwnerPtr.Get())
		{
			Result.Add(FOwner
			{
				*Owner,
				WeakOwner.PreEditChange,
				WeakOwner.PostEditChange,
				WeakOwner.PropertyChain,
			});
		}
	}
	return Result;
}

void FVoxelParameterOverridesDetails::GenerateView(
	const TVoxelArray<FVoxelParameterView*>& ParameterViews,
	const FVoxelDetailInterface& DetailInterface)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(ParameterViews.Num() > 0))
	{
		return;
	}

	const FGuid Guid = ParameterViews[0]->Guid;
	for (const FVoxelParameterView* ParameterView : ParameterViews)
	{
		ensure(Guid == ParameterView->Guid);
	}

	if (ParameterDefaultValueGuid.IsSet() &&
		ParameterDefaultValueGuid.GetValue() != Guid)
	{
		return;
	}

	if (!ParameterDefaultValueGuid.IsSet() &&
		ParameterViews[0]->GetType().CanBeCastedTo<FVoxelExternalParameter>())
	{
		// Only show spline defaults in graph parameter views
		return;
	}

	const TSharedRef<FVoxelParameterDetails> ParameterDetails = MakeShared<FVoxelParameterDetails>(
		*this,
		Guid,
		ParameterViews);

	GuidToParameterDetails.Add_EnsureNew(Guid, ParameterDetails);

	ParameterDetails->MakeRow(DetailInterface);
}

void FVoxelParameterOverridesDetails::AddOrphans(
	FVoxelCategoryBuilder& CategoryBuilder,
	const FString& BaseCategory)
{
	VOXEL_FUNCTION_COUNTER();

	if (ParameterDefaultValueGuid.IsSet())
	{
		// Never add orphans if we have a whitelist
		return;
	}

	bool bCommonGuidsSet = false;
	TVoxelSet<FGuid> CommonGuids;

	for (const FOwner& Owner : GetOwners())
	{
		TVoxelSet<FGuid> Guids;
		for (const auto& It : Owner.Parameters.GetGuidToValueOverride())
		{
			Guids.Add(It.Key);
		}

		if (bCommonGuidsSet)
		{
			CommonGuids = CommonGuids.Intersect(Guids);
		}
		else
		{
			bCommonGuidsSet = true;
			CommonGuids = MoveTemp(Guids);
		}
	}

	for (const FGuid& Guid : CommonGuids)
	{
		bool bIsAnyValid = false;
		for (const TSharedPtr<FVoxelGraphParametersView>& GraphParametersView : GraphParametersViews)
		{
			if (GraphParametersView->FindByGuid(Guid))
			{
				bIsAnyValid = true;
				break;
			}
		}

		if (bIsAnyValid)
		{
			// Not orphan for at least one of the views, skip
			continue;
		}

		FName Name;
		FString Category;
		TArray<FVoxelPinValue> Values;
		{
			TVoxelArray<FVoxelParameterValueOverride> ValueOverrides;
			for (const FOwner& Owner : GetOwners())
			{
				ValueOverrides.Add(Owner.Parameters.GetGuidToValueOverride().FindChecked(Guid));
			}

			if (!ensure(ValueOverrides.Num() > 0))
			{
				continue;
			}

			Name = ValueOverrides[0].CachedName;
			Category = ValueOverrides[0].CachedCategory;
			for (const FVoxelParameterValueOverride& ValueOverride : ValueOverrides)
			{
				if (Name != ValueOverride.CachedName)
				{
					Name = "Multiple Values";
				}
				if (Category != ValueOverride.CachedCategory)
				{
					Category.Reset();
				}
				Values.Add(ValueOverride.Value);
			}
		}

		const TSharedRef<FVoxelParameterDetails> ParameterDetails = MakeShared<FVoxelParameterDetails>(
			*this,
			Guid,
			TVoxelArray<FVoxelParameterView*>());

		ParameterDetails->OrphanName = Name;
		ParameterDetails->InitializeOrphan(Values);
		GuidToParameterDetails.Add_EnsureNew(Guid, ParameterDetails);

		if (!BaseCategory.IsEmpty())
		{
			Category = BaseCategory + "|" + Category;
		}

		CategoryBuilder.AddProperty(
			Category,
			MakeWeakPtrLambda(this, [ParameterDetails](const FVoxelDetailInterface& DetailInterface)
			{
				ParameterDetails->MakeRow(DetailInterface);
			}));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelParameterOverridesDetails::Initialize(
	const FVoxelParameterDetailsBuilder& DetailLayout,
	const FString& BaseCategory)
{
	VOXEL_FUNCTION_COUNTER();

	if (IDetailsView* DetailsView = DetailLayout.GetDetailsView())
	{
		WeakDetailsView = DetailsView->AsShared();
		bWindowValid = IsWindowValid();
	}

	for (const FOwner& Owner : GetOwners())
	{
		Owner.Parameters.FixupParameterOverrides();

		const UVoxelGraph* Graph = Owner.Parameters.GetGraph();
		Graphs.Add(Graph);

		if (!Graph)
		{
			continue;
		}

		for (const UVoxelGraph* BaseGraph : Graph->GetBaseGraphs())
		{
			GVoxelGraphTracker->OnParameterChanged(*BaseGraph).Add(FOnVoxelGraphChanged::Make(this, [this]
			{
				ForceRefresh();
			}));
		}

		const TSharedRef<FVoxelGraphParametersView> ParametersView = Owner.Parameters.GetParametersView_ValidGraph();

		// We want GetValue to show the override value even if it's disabled
		ParametersView->GetContext().ForceEnableOverrides();

		GraphParametersViews.Add(ParametersView);
	}

	FVoxelCategoryBuilder CategoryBuilder({});

	for (const TVoxelArray<FVoxelParameterView*>& ChildParameterViews : FVoxelGraphParametersView::GetCommonChildren(GraphParametersViews))
	{
		FString Category = ChildParameterViews[0]->GetParameter().Category;
		for (const FVoxelParameterView* ChildParameterView : ChildParameterViews)
		{
			ensure(ChildParameterView->GetParameter().Category == Category);
		}

		if (!BaseCategory.IsEmpty())
		{
			Category = BaseCategory + "|" + Category;
		}

		CategoryBuilder.AddProperty(
			Category,
			MakeWeakPtrLambda(this, [=, this](const FVoxelDetailInterface& DetailInterface)
			{
				GenerateView(ChildParameterViews, DetailInterface);
			}));
	}

	AddOrphans(CategoryBuilder, BaseCategory);

	if (DetailLayout.IsLayoutBuilder())
	{
		if (ParameterDefaultValueGuid.IsSet())
		{
			// Skip categories
			CategoryBuilder.ApplyFlat(DetailLayout.GetLayoutBuilder().EditCategory("Default Value"));
		}
		else
		{
			CategoryBuilder.Apply(DetailLayout.GetLayoutBuilder());
		}
	}
	else
	{
		IDetailsViewPrivate* DetailsView = reinterpret_cast<IDetailsViewPrivate*>(DetailLayout.GetDetailsView());
		const FDetailCategoryImpl* ParentCategory = reinterpret_cast<FDetailCategoryImpl*>(&DetailLayout.GetChildrenBuilder().GetParentCategory());

		const FString& PathName = ParentCategory->GetCategoryPathName();
		for (const auto& It : CategoryBuilder.RootCategory->NameToChild)
		{
			const FString CategoryPath =
				(CategoryBuilder.BaseNameForExpansionState.IsNone() ? "FVoxelCategoryBuilder" : CategoryBuilder.BaseNameForExpansionState.ToString()) +
				"." +
				It.Key.ToString();

			DetailsView->SaveCustomExpansionState(PathName + "." + CategoryPath, true);
		}

		CategoryBuilder.Apply(DetailLayout.GetChildrenBuilder());
	}

	for (const FWeakOwner& WeakOwner : Owners)
	{
		if (!WeakOwner.GetHash ||
			!WeakOwner.MakeEnvironment)
		{
			ensure(!WeakOwner.GetHash);
			ensure(!WeakOwner.MakeEnvironment);
			continue;
		}

		CachedEnvironments.Add(FCachedEnvironment
		{
			nullptr,
			0,
			WeakOwner.GetHash,
			WeakOwner.MakeEnvironment
		});
	}
}

bool FVoxelParameterOverridesDetails::IsWindowValid() const
{
	ensure(!IsExplicitlyNull(WeakDetailsView));

	const TSharedPtr<SWidget> Widget = WeakDetailsView.Pin();
	if (!Widget)
	{
		return false;
	}

	return FSlateApplication::Get().FindWidgetWindow(Widget.ToSharedRef()) != nullptr;
}

void FVoxelParameterOverridesDetails::UpdateEnvironments()
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelDependencyCollector DependencyCollector("FVoxelParameterOverridesDetails");

	TVoxelArray<TSharedRef<FVoxelGraphEnvironment>> Environments;
	for (FCachedEnvironment& CachedEnvironment : CachedEnvironments)
	{
		CachedEnvironment.Environment = CachedEnvironment.MakeEnvironment(DependencyCollector);
		CachedEnvironment.Hash = CachedEnvironment.GetHash();

		if (CachedEnvironment.Environment)
		{
			Environments.Add(CachedEnvironment.Environment.ToSharedRef());
		}
	}

	for (const auto& It : GuidToParameterDetails)
	{
		It.Value->ComputeEditorGraphs(DependencyCollector, Environments);
	}

	DependencyTracker = DependencyCollector.Finalize(nullptr, {});
}