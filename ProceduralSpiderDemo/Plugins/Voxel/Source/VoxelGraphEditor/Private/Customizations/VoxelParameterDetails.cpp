// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Customizations/VoxelParameterDetails.h"
#include "VoxelGraph.h"
#include "VoxelNodeEvaluator.h"
#include "VoxelParameterView.h"
#include "VoxelCompiledGraph.h"
#include "VoxelObjectPinType.h"
#include "VoxelCompiledTerminalGraph.h"
#include "VoxelInvalidationCallstack.h"
#include "VoxelParameterOverridesDetails.h"
#include "VoxelGraphParametersViewContext.h"
#include "VoxelPinValueCustomizationHelper.h"
#include "Nodes/VoxelNode_CustomizeParameter.h"

struct FVoxelParameterStructProvider : public IStructureDataProvider
{
public:
	FVoxelParameterStructProvider(const TSharedPtr<FVoxelParameterDetails>& ParameterDetails)
		: WeakParameterDetails(ParameterDetails)
	{}

public:
	virtual bool IsValid() const override
	{
		return WeakParameterDetails.IsValid();
	}
	virtual const UStruct* GetBaseStructure() const override
	{
		return FVoxelPinValue::StaticStruct();
	}
	virtual void GetInstances(TArray<TSharedPtr<FStructOnScope>>& OutInstances, const UStruct* ExpectedBaseStructure) const override
	{
		VOXEL_FUNCTION_COUNTER();
		ensure(OutInstances.Num() == 0);

		const TSharedPtr<FVoxelParameterDetails> ParameterDetails = WeakParameterDetails.Pin();
		if (!ParameterDetails)
		{
			return;
		}

		const UScriptStruct* BaseStructure = Cast<UScriptStruct>(ExpectedBaseStructure);

		for (int32 Index = 0; Index < ParameterDetails->ParameterViews.Num(); Index++)
		{
			const FVoxelParameterView* ParameterView = ParameterDetails->ParameterViews[Index];

			TSharedPtr<FStructOnScope> StructOnScope = ParameterDetails->GetEmptyValue(Index);
			if (FVoxelPinValue* ValuePtr = ParameterView->GetOverrideValue())
			{
				UPackage* Package = StructOnScope->GetPackage();
				StructOnScope = MakeShared<FStructOnScope>(BaseStructure, reinterpret_cast<uint8*>(ValuePtr));
				StructOnScope->SetPackage(Package);
			}

			OutInstances.Add(StructOnScope);
		}

		for (const TSharedPtr<TStructOnScope<FVoxelPinValue>>& StructOnScope : ParameterDetails->OrphanStructOnScopes)
		{
			OutInstances.Add(StructOnScope);
		}
	}

private:
	TWeakPtr<FVoxelParameterDetails> WeakParameterDetails;
};

FVoxelParameterDetails::FVoxelParameterDetails(
	FVoxelParameterOverridesDetails& ContainerDetail,
	const FGuid& Guid,
	const TVoxelArray<FVoxelParameterView*>& ParameterViews)
	: OverridesDetails(ContainerDetail)
	, Guid(Guid)
	, ParameterViews(ParameterViews)
{
	EmptyStructOnScopes.SetNum(ParameterViews.Num());
	for (const FVoxelParameterOverridesDetails::FOwner& Owner : OverridesDetails.GetOwners())
	{
		bForceEnableOverride = Owner.Parameters.ShouldForceEnableOverride(Guid);
	}
	for (const FVoxelParameterOverridesDetails::FOwner& Owner : OverridesDetails.GetOwners())
	{
		ensure(bForceEnableOverride == Owner.Parameters.ShouldForceEnableOverride(Guid));
	}
}

void FVoxelParameterDetails::InitializeOrphan(const TArray<FVoxelPinValue>& Values)
{
	ensure(IsOrphan());

	for (const FVoxelPinValue& Value : Values)
	{
		OrphanStructOnScopes.Add(MakeShared<TStructOnScope<FVoxelPinValue>>(Value));
	}
}

void FVoxelParameterDetails::ComputeEditorGraphs(
	FVoxelDependencyCollector& DependencyCollector,
	const TVoxelArray<TSharedRef<FVoxelGraphEnvironment>>& Environments)
{
	VOXEL_FUNCTION_COUNTER();

	if (IsOrphan())
	{
		return;
	}

	bIsVisible = true;
	bIsReadOnly = false;
	DisplayName = "";
	bool bMultipleDisplayNames = false;

	for (const TSharedRef<FVoxelGraphEnvironment>& Environment : Environments)
	{
		const UVoxelGraph* Graph = Environment->RootCompiledGraph->Graph.Resolve();
		if (!ensureVoxelSlow(Graph))
		{
			continue;
		}

		const TSharedRef<const FVoxelCompiledGraph> CompiledGraph = Graph->GetCompiledGraph(DependencyCollector);
		const FVoxelCompiledTerminalGraph* CompiledTerminalGraph = CompiledGraph->FindTerminalGraph(GVoxelEditorTerminalGraphGuid);
		if (!CompiledTerminalGraph)
		{
			continue;
		}

		const FVoxelNode_CustomizeParameter* Node = nullptr;
		for (const FVoxelNode_CustomizeParameter* OtherNode : CompiledTerminalGraph->GetNodes<FVoxelNode_CustomizeParameter>())
		{
			if (OtherNode->ParameterGuid == Guid)
			{
				ensure(!Node);
				Node = OtherNode;
			}
		}

		if (!Node)
		{
			continue;
		}

		const TVoxelNodeEvaluator<FVoxelNode_CustomizeParameter> Evaluator = FVoxelNodeEvaluator::Create<FVoxelNode_CustomizeParameter>(
			Environment,
			Graph->GetEditorTerminalGraph(),
			*Node);

		if (!Evaluator)
		{
			continue;
		}

		FVoxelGraphContext Context = Evaluator.MakeContext(DependencyCollector);
		FVoxelGraphQueryImpl& Query = Context.MakeQuery();

		bIsVisible &= Evaluator->IsVisiblePin.GetSynchronous(Query);
		bIsReadOnly |= Evaluator->IsReadOnlyPin.GetSynchronous(Query);

		if (bMultipleDisplayNames)
		{
			continue;
		}

		FName Name = Evaluator->DisplayNamePin.GetSynchronous(Query);
		if (Name.IsNone())
		{
			continue;
		}

		FString NameString = Name.ToString();
		if (!DisplayName.IsEmpty() &&
			DisplayName != NameString)
		{
			DisplayName = {};
			bMultipleDisplayNames = true;
			continue;
		}

		DisplayName = NameString;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelParameterDetails::Tick()
{
	const double Time = FPlatformTime::Seconds();
	if (IsOrphan() ||
		ParameterViews.Num() == 0 ||
		Time < LastSyncTime + 0.1)
	{
		return;
	}

	LastSyncTime = Time;

	// Keep default value up to date to graph
	for (const TSharedPtr<TStructOnScope<FVoxelPinValue>>& StructOnScope : EmptyStructOnScopes)
	{
		if (!StructOnScope)
		{
			continue;
		}

		*StructOnScope->Get() = ParameterViews[0]->GetDefaultValue();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelParameterDetails::MakeRow(const FVoxelDetailInterface& DetailInterface)
{
	VOXEL_FUNCTION_COUNTER();

	if (!IsOrphan())
	{
		ensure(!RowExposedType.IsValid());
		RowType = ParameterViews[0]->GetType();
	}
	else
	{
		if (OrphanStructOnScopes.Num() > 0)
		{
			RowType = OrphanStructOnScopes[0]->Get()->GetType();
		}
	}

	RowExposedType = RowType.GetExposedType();

	FVoxelPinTypeSet::OnUserTypeChanged.Add(MakeWeakPtrDelegate(this, [this](const FVoxelPinType& Type)
	{
		if (RowType.GetInnerType() == Type)
		{
			OverridesDetails.ForceRefresh();
		}
	}));

	ParameterDetailsProvider = MakeShared<FVoxelParameterStructProvider>(SharedThis(this));
	IDetailPropertyRow* DummyRow = DetailInterface.AddExternalStructure(ParameterDetailsProvider.ToSharedRef());
	if (!ensure(DummyRow))
	{
		return;
	}

	DummyRow->CustomWidget(false);
	DummyRow->Visibility(EVisibility::Collapsed);

	ensure(!PropertyHandle);
	PropertyHandle = DummyRow->GetPropertyHandle();

	FString PropertyChain;
	for (const FVoxelParameterOverridesDetails::FOwner& WeakOwner : OverridesDetails.GetOwners())
	{
		if (WeakOwner.PropertyChain.IsEmpty())
		{
			continue;
		}

		if (!PropertyChain.IsEmpty() &&
			PropertyChain != WeakOwner.PropertyChain)
		{
			PropertyChain = {};
			break;
		}

		PropertyChain = WeakOwner.PropertyChain;
	}

	PropertyHandle->SetInstanceMetaData("PropertyGuid", Guid.ToString());
	if (!PropertyChain.IsEmpty())
	{
		PropertyHandle->SetInstanceMetaData("VoxelPropertyChain", PropertyChain);
	}

	if (!ensure(PropertyHandle))
	{
		return;
	}

	FVoxelEditorUtilities::TrackHandle(PropertyHandle);

	const FSimpleDelegate PreChangeDelegate = MakeWeakPtrDelegate(this, [this]
	{
		PreEditChange();
	});
	const FSimpleDelegate PostChangeDelegate = MakeWeakPtrDelegate(this, [this]
	{
		PostEditChange();
	});

	PropertyHandle->SetOnPropertyValuePreChange(PreChangeDelegate);
	PropertyHandle->SetOnPropertyValueChanged(PostChangeDelegate);

	PropertyHandle->SetOnChildPropertyValuePreChange(PreChangeDelegate);
	PropertyHandle->SetOnChildPropertyValueChanged(PostChangeDelegate);

	bool bMetadataSet = false;
	TMap<FName, FString> MetaData;
	for (const FVoxelParameterView* ParameterView : ParameterViews)
	{
		TMap<FName, FString> NewMetaData = ParameterView->GetParameter().GetMetaData();
		if (bMetadataSet)
		{
			ensure(MetaData.OrderIndependentCompareEqual(NewMetaData));
		}
		else
		{
			bMetadataSet = true;
			MetaData = MoveTemp(NewMetaData);
		}
	}

	if (!IsOrphan() &&
		// GetInnerType as we also want this to apply to arrays
		RowExposedType.GetInnerType().IsObject())
	{
		const FVoxelPinType PinType = ParameterViews[0]->GetType().GetInnerType();
		const TSharedPtr<const FVoxelObjectPinType> ObjectPinType = FVoxelObjectPinType::StructToPinType().FindRef(PinType.GetStruct());
		if (ensure(ObjectPinType))
		{
			FString& AllowedClasses = MetaData.FindOrAdd("AllowedClasses");
			AllowedClasses.Reset();

			for (const UClass* Class : ObjectPinType->GetAllowedClasses())
			{
				if (!AllowedClasses.IsEmpty())
				{
					AllowedClasses += ",";
				}

				AllowedClasses += Class->GetPathName();
			}
		}
	}

	ensure(!StructWrapper);
	StructWrapper = FVoxelPinValueCustomizationHelper::CreatePinValueCustomization(
		PropertyHandle.ToSharedRef(),
		DetailInterface,
		MakeWeakPtrDelegate(&OverridesDetails, [&OverridesDetails = OverridesDetails]
		{
			OverridesDetails.ForceRefresh();
		}),
		MetaData,
		[&](FDetailWidgetRow& Row, const TSharedRef<SWidget>& ValueWidget)
		{
			BuildRow(DetailInterface, Row, ValueWidget);
		},
		// Used to load/save expansion state
		FAddPropertyParams().UniqueId(FName(Guid.ToString())),
		MakeAttributeLambda(MakeWeakPtrLambda(this, [this]
		{
			return
				bForceEnableOverride ||
				IsOrphan() ||
				IsEnabled() == ECheckBoxState::Checked;
		})));
}

void FVoxelParameterDetails::BuildRow(
	const FVoxelDetailInterface& DetailInterface,
	FDetailWidgetRow& Row,
	const TSharedRef<SWidget>& ValueWidget)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelPinType ExposedType;
	if (ParameterViews.Num() > 0)
	{
		ExposedType = ParameterViews[0]->GetType().GetExposedType();
		for (const FVoxelParameterView* ParameterView : ParameterViews)
		{
			ensure(ExposedType == ParameterView->GetType().GetExposedType());
		}
	}
	else
	{
		if (OrphanStructOnScopes.Num() > 0)
		{
			ExposedType = OrphanStructOnScopes[0]->Get()->GetType();
		}
	}

	const float Width = FVoxelPinValueCustomizationHelper::GetValueWidgetWidthByType(PropertyHandle, ExposedType);

	const auto GetRowToolTip = MakeWeakPtrLambda(this, [this]() -> FText
	{
		if (ParameterViews.Num() == 0)
		{
			return {};
		}

		const FString Description = ParameterViews[0]->GetDescription();
		for (const FVoxelParameterView* ParameterView : ParameterViews)
		{
			ensure(Description == ParameterView->GetDescription());
		}
		return FText::FromString(Description);
	});

	TSharedRef<SWidget> NameWidget =
		SNew(SVoxelDetailText)
		.ColorAndOpacity(IsOrphan() ? FStyleColors::Warning : FSlateColor::UseForeground())
		.Text_Lambda(MakeWeakPtrLambda(this, [this]
		{
			if (DisplayName.IsEmpty())
			{
				return FText::FromString(GetRowName());
			}

			return FText::FromString(DisplayName);
		}))
		.ToolTipText_Lambda(GetRowToolTip);

	if (IsOrphan())
	{
		NameWidget =
			SNew(SHorizontalBox)
			.ToolTipText(INVTEXT("This parameter is orphaned, to remove it, reset it to default"))
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				NameWidget
			]
			+ SHorizontalBox::Slot()
			.Padding(4.f, 2.f)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.Error"))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			];
	}

	if (!bForceEnableOverride &&
		!IsOrphan())
	{
		const TAttribute<bool> EnabledAttribute = MakeAttributeLambda(MakeWeakPtrLambda(this, [this]
		{
			return IsEnabled() == ECheckBoxState::Checked;
		}));

		Row.IsEnabled(EnabledAttribute);
		NameWidget->SetEnabled(EnabledAttribute);
		ValueWidget->SetEnabled(EnabledAttribute);

		NameWidget =
			SNew(SVoxelAlwaysEnabledWidget)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked_Lambda(MakeWeakPtrLambda(this, [this]
					{
						return IsEnabled();
					}))
					.OnCheckStateChanged_Lambda(MakeWeakPtrLambda(this, [this](const ECheckBoxState NewState)
					{
						ensure(NewState != ECheckBoxState::Undetermined);
						SetEnabled(NewState == ECheckBoxState::Checked);
					}))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.f)
				[
					NameWidget
				]
			];
	}

	Row
	.FilterString(FText::FromString(GetRowName()))
	.NameContent()
	[
		NameWidget
	]
	.Visibility(MakeAttributeLambda(MakeWeakPtrLambda(this, [this]
	{
		return bIsVisible ? EVisibility::Visible : EVisibility::Hidden;
	})))
	.EditCondition(MakeAttributeLambda(MakeWeakPtrLambda(this, [this]
	{
		return !bIsReadOnly;
	})), {})
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
		MakeAttributeLambda(MakeWeakPtrLambda(this, [this]
		{
			return CanResetToDefault();
		})),
		MakeWeakPtrDelegate(this, [this]
		{
			ResetToDefault();
		}),
		false));

	const FVoxelDetailsViewCustomData* CustomData = DetailInterface.GetCustomData();
	if (!CustomData ||
		!CustomData->HasMetadata("ShowTooltipColumn"))
	{
		Row
		.ValueContent()
		.MinDesiredWidth(Width)
		.MaxDesiredWidth(Width)
		[
			ValueWidget
		];
		return;
	}

	Row
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SSplitter)
		.Style(FAppStyle::Get(), "DetailsView.Splitter")
		.PhysicalSplitterHandleSize(1.0f)
		.HitDetectionSplitterHandleSize(5.0f)
		.Orientation(Orient_Horizontal)
		+ SSplitter::Slot()
		.Value(CustomData->GetValueColumnWidth())
		.OnSlotResized(CustomData->GetOnValueColumnResized())
		[
			SNew(SBox)
			.Padding(0.f, 0.f, 12.f, 0.f)
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.MinDesiredWidth(Width)
				.MaxDesiredWidth(Width)
				[
					ValueWidget
				]
			]
		]
		+ SSplitter::Slot()
		.Value(CustomData->GetToolTipColumnWidth())
		.OnSlotResized_Lambda([](float){})
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			.Padding(12.f, 0.f, 2.f, 0.f) // DetailWidgetConstants::RightRowPadding
			[
				SNew(STextBlock)
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				.Clipping(EWidgetClipping::ClipToBounds)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Text_Lambda(GetRowToolTip)
			]
		]
	];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ECheckBoxState FVoxelParameterDetails::IsEnabled() const
{
	ensure(!bForceEnableOverride);
	ensure(!IsOrphan());

	bool bAnyEnabled = false;
	bool bAnyDisabled = false;
	for (const FVoxelParameterOverridesDetails::FOwner& Owner : OverridesDetails.GetOwners())
	{
		if (const FVoxelParameterValueOverride* ValueOverride = Owner.Parameters.GetGuidToValueOverride().Find(Guid))
		{
			if (ValueOverride->bEnable)
			{
				bAnyEnabled = true;
			}
			else
			{
				bAnyDisabled = true;
			}
		}
		else
		{
			bAnyDisabled = true;
		}
	}

	if (bAnyEnabled && !bAnyDisabled)
	{
		return ECheckBoxState::Checked;
	}
	if (!bAnyEnabled && bAnyDisabled)
	{
		return ECheckBoxState::Unchecked;
	}
	return ECheckBoxState::Undetermined;
}

void FVoxelParameterDetails::SetEnabled(const bool bNewEnabled) const
{
	ensure(!bForceEnableOverride);
	ensure(!IsOrphan());

	const FScopedTransaction Transaction(FText::FromString((bNewEnabled ? "Enable " : "Disable ") + GetRowName()));

	const TVoxelArray<FVoxelParameterOverridesDetails::FOwner> Owners = OverridesDetails.GetOwners();
	if (!ensure(Owners.Num() == ParameterViews.Num()))
	{
		return;
	}

	for (int32 Index = 0; Index < Owners.Num(); Index++)
	{
		const FVoxelParameterOverridesDetails::FOwner& Owner = Owners[Index];
		Owner.PreEditChange();

		if (FVoxelParameterValueOverride* ExistingValueOverride = Owner.Parameters.GetGuidToValueOverride().Find(Guid))
		{
			ExistingValueOverride->bEnable = bNewEnabled;
			ensure(ExistingValueOverride->Value.IsValid());
		}
		else
		{
			const FVoxelParameterView* ParameterView = ParameterViews[Index];
			if (!ensure(ParameterView))
			{
				continue;
			}

			FVoxelParameterValueOverride ValueOverride;
			ValueOverride.bEnable = true;
			ValueOverride.Value = ParameterView->GetValue();

			// Add AFTER doing GetValue so we don't query ourselves
			Owner.Parameters.GetGuidToValueOverride().Add(Guid, ValueOverride);
		}

		Owner.Parameters.FixupParameterOverrides();
		Owner.PostEditChange();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelParameterDetails::CanResetToDefault() const
{
	if (IsOrphan())
	{
		return true;
	}

	const TVoxelArray<FVoxelParameterOverridesDetails::FOwner> Owners = OverridesDetails.GetOwners();
	if (!ensure(Owners.Num() == ParameterViews.Num()))
	{
		return false;
	}

	for (int32 Index = 0; Index < Owners.Num(); Index++)
	{
		const FVoxelParameterOverridesDetails::FOwner& Owner = Owners[Index];
		const FVoxelParameterView* ParameterView = ParameterViews[Index];
		if (!ensure(ParameterView))
		{
			continue;
		}

		const FVoxelParameterValueOverride* ValueOverride = Owner.Parameters.GetGuidToValueOverride().Find(Guid);
		if (!ValueOverride)
		{
			continue;
		}

		ParameterView->Context.DisableMainOverridesOwner();
		const FVoxelPinValue DefaultValue = ParameterView->GetValue();
		ParameterView->Context.EnableMainOverridesOwner();

		if (ValueOverride->Value != DefaultValue)
		{
			return true;
		}
	}
	return false;
}

void FVoxelParameterDetails::ResetToDefault()
{
	const FScopedTransaction Transaction(FText::FromString("Reset " + GetRowName() + " to default"));

	if (IsOrphan())
	{
		for (const FVoxelParameterOverridesDetails::FOwner& Owner : OverridesDetails.GetOwners())
		{
			Owner.PreEditChange();
			Owner.Parameters.GetGuidToValueOverride().Remove(Guid);
			Owner.Parameters.FixupParameterOverrides();
			Owner.PostEditChange();

			// No need to broadcast OnChanged for orphans
		}

		// Force refresh to remove orphans rows that were removed
		OverridesDetails.ForceRefresh();
		return;
	}

	const TVoxelArray<FVoxelParameterOverridesDetails::FOwner> Owners = OverridesDetails.GetOwners();
	if (!ensure(Owners.Num() == ParameterViews.Num()))
	{
		return;
	}

	for (int32 Index = 0; Index < Owners.Num(); Index++)
	{
		const FVoxelParameterOverridesDetails::FOwner& Owner = Owners[Index];
		const FVoxelParameterView* ParameterView = ParameterViews[Index];
		if (!ensure(ParameterView))
		{
			continue;
		}

		FVoxelParameterValueOverride* ValueOverride = Owner.Parameters.GetGuidToValueOverride().Find(Guid);
		if (!ValueOverride)
		{
			// We might be able to only reset to default one of the multi-selected objects
			ensure(OverridesDetails.GetOwners().Num() > 1);
			continue;
		}

		ParameterView->Context.DisableMainOverridesOwner();
		const FVoxelPinValue DefaultValue = ParameterView->GetValue();
		ParameterView->Context.EnableMainOverridesOwner();

		Owner.PreEditChange();
		ValueOverride->Value = DefaultValue;
		Owner.Parameters.FixupParameterOverrides();
		Owner.PostEditChange();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelParameterDetails::PreEditChange() const
{
	for (const FVoxelParameterOverridesDetails::FOwner& Owner : OverridesDetails.GetOwners())
	{
		Owner.PreEditChange();
	}
}

void FVoxelParameterDetails::PostEditChange() const
{
	FVoxelInvalidationScope Scope(GetRowName() + " changed");

	for (int32 Index = 0; Index < ParameterViews.Num(); Index++)
	{
		const FVoxelParameterView* ParameterView = ParameterViews[Index];
		if (!ParameterView)
		{
			continue;
		}

		IVoxelParameterOverridesOwner* Parameters = ParameterView->Context.GetOverridesOwner().Get();
		if (!Parameters)
		{
			continue;
		}

		if (!Parameters->GetGuidToValueOverride().Contains(Guid) &&
			EmptyStructOnScopes[Index])
		{
			FVoxelParameterValueOverride& ValueOverride = Parameters->GetGuidToValueOverride().FindOrAdd(Guid);
			if (bForceEnableOverride)
			{
				ValueOverride.bEnable = true;
			}
			else
			{
				ensure(ValueOverride.bEnable);
			}

			ValueOverride.Value = *EmptyStructOnScopes[Index]->Get();
		}

		Parameters->FixupParameterOverrides();
	}

	for (const FVoxelParameterOverridesDetails::FOwner& Owner : OverridesDetails.GetOwners())
	{
		Owner.PostEditChange();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FString FVoxelParameterDetails::GetRowName() const
{
	if (ParameterViews.Num() == 0)
	{
		return OrphanName.ToString();
	}

	const FName Name = ParameterViews[0]->GetName();
	for (const FVoxelParameterView* ParameterView : ParameterViews)
	{
		ensure(Name == ParameterView->GetName());
	}
	return Name.ToString();
}

TSharedPtr<TStructOnScope<FVoxelPinValue>> FVoxelParameterDetails::GetEmptyValue(const int32 Index) const
{
	if (!ensure(EmptyStructOnScopes.IsValidIndex(Index)))
	{
		return nullptr;
	}

	TSharedPtr<TStructOnScope<FVoxelPinValue>>& EmptyStructOnScope = EmptyStructOnScopes[Index];
	if (!EmptyStructOnScope)
	{
		EmptyStructOnScope = MakeShared<TStructOnScope<FVoxelPinValue>>();
		EmptyStructOnScope->InitializeAs<FVoxelPinValue>();
		if (const UObject* Object = ParameterViews[Index]->Context.GetOverridesOwner().GetObject())
		{
			StaticCastSharedPtr<FStructOnScope>(EmptyStructOnScope)->SetPackage(Object->GetPackage());
		}
		*EmptyStructOnScope->Get() = ParameterViews[Index]->GetDefaultValue();
	}

	return EmptyStructOnScope;
}