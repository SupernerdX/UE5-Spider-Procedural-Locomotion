// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSplineMetadataDetails.h"
#include "Spline/VoxelSplineMetadata.h"
#include "Spline/VoxelSplineComponent.h"
#include "DetailCategoryBuilderImpl.h"
#include "DetailItemNode.h"
#include "CustomChildBuilder.h"

UClass* UVoxelSplineMetadataDetailsFactory::GetMetadataClass() const
{
	return UVoxelSplineMetadata::StaticClass();
}

TSharedPtr<ISplineMetadataDetails> UVoxelSplineMetadataDetailsFactory::Create()
{
	return MakeShared<FVoxelSplineMetadataDetails>();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FName FVoxelSplineMetadataDetails::GetName() const
{
	return "FVoxelSplineMetadataDetails";
}

FText FVoxelSplineMetadataDetails::GetDisplayName() const
{
	return INVTEXT("Voxel");
}

void FVoxelSplineMetadataDetails::Update(USplineComponent* SplineComponent, const TSet<int32>& SelectedKeys)
{
	VOXEL_FUNCTION_COUNTER();

	for (const TSharedPtr<FVoxelSplineMetadataStructureDataProvider>& StructProvider : StructProviders)
	{
		StructProvider->SelectedKeys = SelectedKeys;
	}
}

//template<class T>
//void SetValues(FVoxelSplineMetadataDetails& Details, TArray<FInterpCurvePoint<T>>& Points, const T& NewValue, ETextCommit::Type CommitInfo, const FText& TransactionText)
//{
//	// Scope the transaction to only include the value change and none of the derived data changes that might arise from NotifyPropertyModified
//	{
//		const FScopedTransaction Transaction(TransactionText);
//		Details.SplineComp->GetSplinePointsMetadata()->Modify();
//		for (int32 Index : Details.SelectedKeys)
//		{
//			Points[Index].OutVal = NewValue;
//		}
//	}
//
//	Details.SplineComp->UpdateSpline();
//	Details.SplineComp->bSplineHasBeenEdited = true;
//	static FProperty* SplineCurvesProperty = FindFProperty<FProperty>(USplineComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(USplineComponent, SplineCurves));
//	EPropertyChangeType::Type PropertyChangeType = CommitInfo == ETextCommit::OnEnter ? EPropertyChangeType::ValueSet : EPropertyChangeType::Interactive;
//	FComponentVisualizer::NotifyPropertyModified(Details.SplineComp, SplineCurvesProperty, PropertyChangeType);
//	Details.Update(Details.SplineComp, Details.SelectedKeys);
//
//	GEditor->RedrawLevelEditingViewports(true);
//}

DEFINE_PRIVATE_ACCESS(FDetailCategoryImpl, SimpleChildNodes);
DEFINE_PRIVATE_ACCESS(FDetailItemNode, Customization);
DEFINE_PRIVATE_ACCESS(FDetailCustomBuilderRow, ChildrenBuilder);

void FVoxelSplineMetadataDetails::GenerateChildContent(IDetailGroup& DetailGroup)
{
	VOXEL_FUNCTION_COUNTER();

	IDetailCategoryBuilder* Category = FVoxelEditorUtilities::GetParentCategory(DetailGroup);
	if (!ensureVoxelSlow(Category))
	{
		return;
	}

	TArray<TSharedRef<FDetailTreeNode>>& ChildNodes = PrivateAccess::SimpleChildNodes(static_cast<FDetailCategoryImpl&>(*Category));
	if (ChildNodes.Num() == 0)
	{
		return;
	}

	if (!ensureVoxelSlow(ChildNodes.Num() == 1))
	{
		return;
	}

	const TSharedPtr<IDetailChildrenBuilder> ChildrenBuilder = PrivateAccess::ChildrenBuilder(*PrivateAccess::Customization(static_cast<FDetailItemNode&>(*ChildNodes[0])).CustomBuilderRow);
	if (!ensureVoxelSlow(ChildrenBuilder))
	{
		return;
	}

	const TArray<TWeakObjectPtr<UVoxelSplineComponent>> SplineComponents = Category->GetParentLayout().GetObjectsOfTypeBeingCustomized<UVoxelSplineComponent>();
	if (!ensureVoxelSlow(SplineComponents.Num() == 1))
	{
		return;
	}

	const UVoxelSplineComponent* SplineComponent = SplineComponents[0].Get();
	if (!ensureVoxelSlow(SplineComponent))
	{
		return;
	}

	UVoxelSplineMetadata* Metadata = SplineComponent->Metadata;
	if (!ensureVoxelSlow(Metadata))
	{
		return;
	}

	DetailGroup.HeaderRow().Visibility(EVisibility::Collapsed);

	for (const auto& It : Metadata->GuidToValues)
	{
		const TSharedRef<FVoxelSplineMetadataStructureDataProvider> StructProvider = MakeShared<FVoxelSplineMetadataStructureDataProvider>(
			Metadata,
			It.Key);

		StructProviders.Add(StructProvider);

		IDetailPropertyRow* Row = ChildrenBuilder->AddExternalStructure(StructProvider);
		if (!ensureVoxelSlow(Row))
		{
			continue;
		}

		Row->DisplayName(FText::FromName(It.Value.Parameter.Name));
		Row->ToolTip(FText::FromString(It.Value.Parameter.Description));

		const TSharedPtr<IPropertyHandle> PropertyHandle = Row->GetPropertyHandle();
		if (!ensureVoxelSlow(PropertyHandle))
		{
			continue;
		}

		PropertyHandle->SetOnChildPropertyValuePreChange(MakeWeakObjectPtrDelegate(Metadata, [Metadata]
		{
			Metadata->PreEditChange(nullptr);
		}));
		PropertyHandle->SetOnChildPropertyValueChanged(MakeWeakObjectPtrDelegate(Metadata, [Metadata]
		{
			Metadata->PostEditChange();
		}));

		for (auto& MetaDataIt : It.Value.Parameter.GetMetaData())
		{
			PropertyHandle->SetInstanceMetaData(MetaDataIt.Key, MetaDataIt.Value);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelSplineMetadataStructureDataProvider::IsValid() const
{
	return WeakMetadata.IsValid_Slow();
}

const UStruct* FVoxelSplineMetadataStructureDataProvider::GetBaseStructure() const
{
	return StaticStructFast<FVoxelPinValue>();
}

void FVoxelSplineMetadataStructureDataProvider::GetInstances(TArray<TSharedPtr<FStructOnScope>>& OutInstances, const UStruct* ExpectedBaseStructure) const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(OutInstances.Num() == 0);

	UVoxelSplineMetadata* Metadata = WeakMetadata.Resolve();
	if (!ensureVoxelSlow(Metadata))
	{
		return;
	}

	FVoxelSplineMetadataValues* Values = Metadata->GuidToValues.Find(Guid);
	if (!ensureVoxelSlow(Values))
	{
		return;
	}

	if (SelectedKeys.Num() == 0)
	{
		TVoxelInstancedStruct<FVoxelPinValue> Value = FVoxelPinValue(Values->Parameter.Type);
		OutInstances.Add(Value.MakeStructOnScope());
		return;
	}

	for (const int32 Key : SelectedKeys)
	{
		if (!ensureVoxelSlow(Values->Values.IsValidIndex(Key)))
		{
			continue;
		}

		const TVoxelInstancedStruct<FVoxelPinValue> Value = Values->Values[Key];
		if (!ensureVoxelSlow(Value))
		{
			continue;
		}

		OutInstances.Add(Values->Values[Key].MakeStructOnScope());
	}
}