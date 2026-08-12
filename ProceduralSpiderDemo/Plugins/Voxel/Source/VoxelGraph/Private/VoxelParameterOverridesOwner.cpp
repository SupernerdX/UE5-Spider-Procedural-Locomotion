// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelParameterOverridesOwner.h"
#include "VoxelGraph.h"
#include "VoxelExposedSeed.h"
#include "VoxelGraphTracker.h"
#include "VoxelParameterView.h"
#include "VoxelGraphParametersView.h"
#include "VoxelGraphParametersViewContext.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "FunctionLibrary/VoxelCurveFunctionLibrary.h"

uint64 FVoxelParameterOverrides::GetHash() const
{
	VOXEL_FUNCTION_COUNTER();

	return FVoxelUtilities::HashProperty(
		FindFPropertyChecked(FVoxelParameterOverrides, GuidToValueOverride),
		&GuidToValueOverride);
}

bool FVoxelParameterOverrides::operator==(const FVoxelParameterOverrides& Other) const
{
	VOXEL_FUNCTION_COUNTER();

	return GuidToValueOverride.OrderIndependentCompareEqual(Other.GuidToValueOverride);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void IVoxelParameterOverridesOwner::FixupParameterOverrides()
{
	VOXEL_FUNCTION_COUNTER();

	UVoxelGraph* Graph = GetGraph();
	TMap<FGuid, FVoxelParameterValueOverride>& GuidToValueOverride = GetGuidToValueOverride();

	for (auto& It : GuidToValueOverride)
	{
		It.Value.Value.Fixup();
	}

	if (!Graph)
	{
#if WITH_EDITOR
		OnParameterChangedPtr.Reset();
#endif
		return;
	}

#if WITH_EDITOR
	{
		OnParameterChangedPtr = MakeSharedVoid();

		FOnVoxelGraphChanged OnParameterChanged = FOnVoxelGraphChanged::Null();
		if (UObject* Object = _getUObject())
		{
			OnParameterChanged = FOnVoxelGraphChanged::Make(OnParameterChangedPtr, Object, [this]
			{
				FixupParameterOverrides();
			});
		}
		else if (const TSharedPtr<IVoxelParameterOverridesOwner> SharedThis = GetShared())
		{
			OnParameterChanged = FOnVoxelGraphChanged::Make(OnParameterChangedPtr, SharedThis, [this]
			{
				FixupParameterOverrides();
			});
		}
		else
		{
			ensure(false);
		}

		for (const UVoxelGraph* BaseGraph : Graph->GetBaseGraphs())
		{
			GVoxelGraphTracker->OnParameterChanged(*BaseGraph).Add(OnParameterChanged);
		}
	}
#endif

	const TSharedRef<FVoxelGraphParametersView> ParametersView = GetParametersView_ValidGraph();

	// Try to migrate orphans by finding a new parameter with the same name
	{
		TMap<FGuid, FVoxelParameterValueOverride> MigratedParameters;
		for (auto It = GuidToValueOverride.CreateIterator(); It; ++It)
		{
			if (ParametersView->FindByGuid(It.Key()))
			{
				continue;
			}

			const FVoxelParameterView* ParameterView = ParametersView->FindByName(It.Value().CachedName);
			if (!ParameterView)
			{
				continue;
			}

			if (GuidToValueOverride.Contains(ParameterView->Guid))
			{
				// Old orphan exists
				continue;
			}

			MigratedParameters.Add(ParameterView->Guid, It.Value());
			It.RemoveCurrent();
		}
		GuidToValueOverride.Append(MigratedParameters);
	}

	TVoxelArray<FVoxelParameterValueOverride> OrphanedParameters;
	for (auto It = GuidToValueOverride.CreateIterator(); It; ++It)
	{
		const FGuid Guid = It.Key();
		FVoxelParameterValueOverride& ParameterOverrideValue = It.Value();

		if (!ensureVoxelSlow(ParameterOverrideValue.Value.IsValid()))
		{
			It.RemoveCurrent();
			continue;
		}

		if (ShouldForceEnableOverride(Guid))
		{
			ParameterOverrideValue.bEnable = true;
		}

		ParametersView->GetContext().DisableMainOverridesOwner();
		ON_SCOPE_EXIT
		{
			ParametersView->GetContext().EnableMainOverridesOwner();
		};

		ParameterOverrideValue.Value.Fixup();

		const FVoxelParameterView* ParameterView = ParametersView->FindByGuid(Guid);
		if (!ParameterView)
		{
			continue;
		}

		ParameterOverrideValue.CachedName = ParameterView->GetName();
#if WITH_EDITOR
		ParameterOverrideValue.CachedCategory = ParameterView->GetParameter().Category;
#endif

		if (!ParameterOverrideValue.Value.GetType().CanBeCastedTo(ParameterView->GetType().GetExposedType()))
		{
			if (const TSharedPtr<FVoxelPinValueOps> Ops = FVoxelPinValueOps::Find(ParameterView->GetType(), EVoxelPinValueOpsUsage::MigrateParameter))
			{
				Ops->MigrateParameter(ParameterOverrideValue.Value);
			}

			if (!ParameterOverrideValue.Value.GetType().CanBeCastedTo(ParameterView->GetType().GetExposedType()))
			{
				FVoxelPinValue NewValue(ParameterView->GetType().GetExposedType());

				if (!NewValue.ImportFromUnrelated(ParameterOverrideValue.Value))
				{
					// Cannot migrate value, create new orphan and reset value to default
					OrphanedParameters.Add(ParameterOverrideValue);
					It.RemoveCurrent();
					continue;
				}

				FVoxelPinValue OldTypeNewValue(ParameterOverrideValue.Value.GetType());
				OldTypeNewValue.ImportFromUnrelated(NewValue);

				if (ParameterOverrideValue.Value.ExportToString() != OldTypeNewValue.ExportToString())
				{
					// Imperfect migration create orphan and still use new value
					OrphanedParameters.Add(ParameterOverrideValue);
				}

				ParameterOverrideValue.Value = NewValue;
			}
		}

		// Ensure the override type is the new type
		// If the old type can be casted to the new one the branch above will be skipped
		ParameterOverrideValue.Value.Fixup(ParameterView->GetType().GetExposedType());

		const FVoxelPinValue Value = ParameterView->GetValue();
		if (!ensure(Value.IsValid()))
		{
			continue;
		}

		if (!ShouldForceEnableOverride(Guid) &&
			ParameterOverrideValue.bEnable)
		{
			// Explicitly enabled: never reset to default
			continue;
		}

		if (ParameterOverrideValue.Value == Value)
		{
			// Back to default value, no need to store an override
			It.RemoveCurrent();
		}
	}

	// Make sure to do this after to avoid reallocating the map,
	// which would corrupt the iterator
	for (const FVoxelParameterValueOverride& Parameter : OrphanedParameters)
	{
		GuidToValueOverride.Add(FGuid::NewGuid(), Parameter);
	}

	// Fixup seeds
	for (const FVoxelParameterView* Child : ParametersView->GetChildren())
	{
		if (!Child->GetType().Is<FVoxelSeed>() ||
			!Child->GetValue().Get<FVoxelExposedSeed>().Seed.IsEmpty())
		{
			continue;
		}

		FVoxelExposedSeed NewSeed;
		NewSeed.Randomize();

		FVoxelParameterValueOverride ValueOverride;
		ValueOverride.bEnable = true;
		ValueOverride.Value = FVoxelPinValue::Make(NewSeed);
		GuidToValueOverride.Add(Child->Guid, ValueOverride);
	}

	for (const auto& It : GuidToValueOverride)
	{
		if (ParametersView->FindByGuid(It.Key))
		{
			continue;
		}

		LOG_VOXEL(Warning, "Orphaned parameter in %s: %s not in graph %s",
			*MakeVoxelObjectPtr(_getUObject()).GetPathName(),
			*It.Value.CachedName.ToString(),
			*Graph->GetPathName());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelGraphParametersView> IVoxelParameterOverridesOwner::GetParametersView() const
{
	const UVoxelGraph* Graph = GetGraph();
	if (!Graph)
	{
		return nullptr;
	}

	return GetParametersView_ValidGraph();
}

TSharedRef<FVoxelGraphParametersView> IVoxelParameterOverridesOwner::GetParametersView_ValidGraph() const
{
	VOXEL_FUNCTION_COUNTER();

	const UVoxelGraph* Graph = GetGraph();
	check(Graph);

	const TSharedRef<FVoxelGraphParametersView> ParametersView = MakeShareable(new FVoxelGraphParametersView());

	FVoxelGraphParametersViewContext& Context = *ParametersView->Context;
	Context.MainOverridesOwner = ConstCast(this);

	for (const UVoxelGraph* BaseGraph : Graph->GetBaseGraphs())
	{
		if (BaseGraph == this)
		{
			// Don't add ourselves twice
			check(Context.Graphs.Num() == 0);
			continue;
		}

		Context.Graphs.Add(BaseGraph);
	}

	const int32 NumParameters = Graph->NumParameters();

	ParametersView->ChildrenRefs.Reserve(NumParameters);
	ParametersView->Children.Reserve(NumParameters);
	ParametersView->GuidToParameterView.Reserve(NumParameters);
	ParametersView->NameToParameterView.Reserve(NumParameters);

	Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
	{
		if (!ensure(!ParametersView->NameToParameterView.Contains(Parameter.Name)))
		{
			return;
		}

		FVoxelParameterView& ParameterView = ParametersView->ChildrenRefs.Emplace_GetRef_EnsureNoGrow(Context, Guid, Parameter);

		ParametersView->Children.Add_EnsureNoGrow(&ParameterView);
		ParametersView->GuidToParameterView.Add_CheckNew_EnsureNoGrow(Guid, &ParameterView);
		ParametersView->NameToParameterView.Add_CheckNew_EnsureNoGrow(Parameter.Name, &ParameterView);
	});

	return ParametersView;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool IVoxelParameterOverridesOwner::HasParameter(const FName Name) const
{
	const TSharedPtr<FVoxelGraphParametersView> ParametersView = GetParametersView();
	return
		ParametersView &&
		ParametersView->FindByName(Name) != nullptr;
}

FVoxelPinValue IVoxelParameterOverridesOwner::GetParameter(
	const FName Name,
	FString* OutError) const
{
	VOXEL_FUNCTION_COUNTER();

	const UVoxelGraph* Graph = GetGraph();
	if (!Graph)
	{
		if (OutError)
		{
			*OutError = "Graph is null";
		}
		return {};
	}

	const TSharedRef<FVoxelGraphParametersView> ParametersView = GetParametersView_ValidGraph();
	const FVoxelParameterView* ParameterView = ParametersView->FindByName(Name);
	if (!ParameterView)
	{
		if (OutError)
		{
			TVoxelArray<FString> ValidParameters;
			for (const FGuid& Guid : Graph->GetParameters())
			{
				const FVoxelParameter& Parameter = Graph->FindParameterChecked(Guid);

				ValidParameters.Add(Parameter.Name.ToString() + " (" + Parameter.Type.GetExposedType().ToString() + ")");
			}
			*OutError = "Failed to find " + Name.ToString() + ". Valid parameters: " + FString::Join(ValidParameters, TEXT(", "));
		}
		return {};
	}

	return ParameterView->GetValue();
}

FVoxelPinValue IVoxelParameterOverridesOwner::GetParameterTyped(
	const FName Name,
	const FVoxelPinType& Type,
	FString* OutError) const
{
	const FVoxelPinValue Value = GetParameter(Name, OutError);
	if (!Value.IsValid())
	{
		return {};
	}

	if (!Value.CanBeCastedTo(Type))
	{
		if (OutError)
		{
			*OutError =
				"Parameter " + Name.ToString() + " has type " + Value.GetType().ToString() +
				", but type " + Type.ToString() + " was expected";
		}
		return {};
	}

	return Value;
}

bool IVoxelParameterOverridesOwner::SetParameter(
	const FName Name,
	const FVoxelPinValue& InValue,
	FString* OutError)
{
	VOXEL_FUNCTION_COUNTER();

	const UVoxelGraph* Graph = GetGraph();
	if (!Graph)
	{
		if (OutError)
		{
			*OutError = "Graph is null";
		}
		return false;
	}

	const TSharedRef<FVoxelGraphParametersView> ParametersView = GetParametersView_ValidGraph();
	const FVoxelParameterView* ParameterView = ParametersView->FindByName(Name);
	if (!ParameterView)
	{
		if (OutError)
		{
			TVoxelArray<FString> ValidParameters;
			for (const FGuid& Guid : Graph->GetParameters())
			{
				const FVoxelParameter& Parameter = Graph->FindParameterChecked(Guid);

				ValidParameters.Add(Parameter.Name.ToString() + " (" + Parameter.Type.GetExposedType().ToString() + ")");
			}
			*OutError = "Failed to find " + Name.ToString() + ". Valid parameters: " + FString::Join(ValidParameters, TEXT(", "));
		}
		return false;
	}

	FVoxelPinValue Value = InValue;

	const FVoxelPinType ExposedType = ParameterView->GetType().GetExposedType();
	Value.Fixup(ExposedType);

	if (!Value.CanBeCastedTo(ExposedType))
	{
		if (OutError)
		{
			*OutError =
				"Invalid parameter type for " + Name.ToString() + ". Parameter has type " + ExposedType.ToString() +
				", but value of type " + Value.GetType().ToString() + " was passed";
		}
		return false;
	}

	FVoxelParameterValueOverride ValueOverride;
	ValueOverride.bEnable = true;
	ValueOverride.Value = Value;
	GetGuidToValueOverride().Add(ParameterView->Guid, ValueOverride);

	FixupParameterOverrides();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelParameterOverridesOwnerPtr::FVoxelParameterOverridesOwnerPtr(IVoxelParameterOverridesOwner* Owner)
{
	if (!Owner)
	{
		return;
	}

	OwnerPtr = Owner;

	if (const UObject* Object = Owner->_getUObject())
	{
		ObjectPtr = Object;
	}
	else
	{
		WeakPtr = MakeSharedVoidPtr(Owner->GetShared());
	}

	ensure(IsValid());
}