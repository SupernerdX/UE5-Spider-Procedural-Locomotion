// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelDetailsSplitter.h"

#include "DiffUtils.h"

#include "AsyncTreeDifferences.h"
#include "Editor/PropertyEditor/Private/IDetailsViewPrivate.h"

namespace DetailsSplitterHelpers
{
	const TArray<TWeakObjectPtr<UObject>>* GetObjects(const TSharedPtr<FDetailTreeNode>& TreeNode)
	{
		if (const IDetailsViewPrivate* DetailsView = TreeNode->UE_506_SWITCH(GetDetailsView, GetDetailsViewSharedPtr().Get)())
		{
			return &DetailsView->GetSelectedObjects();
		}
		return nullptr;
	}

	// converts from a container helper like FScriptArrayHelper to a property like FArrayProperty
	template<typename HelperType>
	using TContainerPropertyType =
		std::conditional_t<std::is_same_v<HelperType, FScriptArrayHelper>, FArrayProperty,
		std::conditional_t<std::is_same_v<HelperType, FScriptMapHelper>, FMapProperty,
		std::conditional_t<std::is_same_v<HelperType, FScriptSetHelper>, FSetProperty,
		void>>>;

	template<typename ContainerPropertyType>
	bool TryGetSourceContainer(TSharedPtr<FDetailTreeNode> DetailsNode, FPropertyNode*& OutPropNode)
	{
		if ((OutPropNode = DetailsNode->GetPropertyNode().Get()) == nullptr)
		{
			return false;
		}
		if ((OutPropNode = OutPropNode->GetParentNode()) == nullptr)
		{
			return false;
		}
		if (CastField<ContainerPropertyType>(OutPropNode->GetProperty()) != nullptr)
		{
			return true;
		}
		return false;
	}

	template<typename ContainerPropertyType>
	bool TryGetDestinationContainer(TSharedPtr<FDetailTreeNode> DetailsNode, FPropertyNode*& OutPropNode, int32& OutInsertIndex)
	{
		if ((OutPropNode = DetailsNode->GetPropertyNode().Get()) == nullptr)
		{
			return false;
		}
		if (CastField<ContainerPropertyType>(OutPropNode->GetProperty()) != nullptr)
		{
			OutInsertIndex = 0;
			return true;
		}
		OutInsertIndex = OutPropNode->GetArrayIndex() + 1;
		while((OutPropNode = OutPropNode->GetParentNode()) != nullptr)
		{
			if (CastField<ContainerPropertyType>(OutPropNode->GetProperty()) != nullptr)
			{
				return true;
			}
			OutInsertIndex = OutPropNode->GetArrayIndex() + 1;
		}
		return false;
	}

	template<typename HelperType>
	bool TryGetSourceContainer(TSharedPtr<FDetailTreeNode> DetailsNode, FPropertyNode*& OutPropNode, TArray<TUniquePtr<HelperType>>& OutContainerHelper)
	{
		using ContainerPropertyType = TContainerPropertyType<HelperType>;
		if (TryGetSourceContainer<ContainerPropertyType>(DetailsNode, OutPropNode))
		{
			const FPropertySoftPath SoftPropertyPath(FPropertyNode::CreatePropertyPath(OutPropNode->AsShared()).Get());
			if (const TArray<TWeakObjectPtr<UObject>>* Objects = DetailsSplitterHelpers::GetObjects(DetailsNode))
			{
				for (const TWeakObjectPtr<UObject> WeakObject : *Objects)
				{
					if (const UObject* Object = WeakObject.Get())
					{
						const FResolvedProperty Resolved = SoftPropertyPath.Resolve(Object);
						const ContainerPropertyType* ContainerProperty = CastFieldChecked<ContainerPropertyType>(Resolved.Property);
						OutContainerHelper.Add(MakeUnique<HelperType>(ContainerProperty, ContainerProperty->template ContainerPtrToValuePtr<UObject*>(Resolved.Object)));
					}
				}
			}
			return true;
		}
		return false;
	}

	template<typename HelperType>
	bool TryGetDestinationContainer(TSharedPtr<FDetailTreeNode> DetailsNode, FPropertyNode*& OutPropNode, TArray<TUniquePtr<HelperType>>& OutContainerHelper, int32& OutInsertIndex)
	{
		using ContainerPropertyType = TContainerPropertyType<HelperType>;
		if (TryGetDestinationContainer<ContainerPropertyType>(DetailsNode, OutPropNode, OutInsertIndex))
		{
			const FPropertySoftPath SoftPropertyPath(FPropertyNode::CreatePropertyPath(OutPropNode->AsShared()).Get());
			if (const TArray<TWeakObjectPtr<UObject>>* Objects = DetailsSplitterHelpers::GetObjects(DetailsNode))
			{
				for (const TWeakObjectPtr<UObject> WeakObject : *Objects)
				{
					if (const UObject* Object = WeakObject.Get())
					{
						const FResolvedProperty Resolved = SoftPropertyPath.Resolve(Object);
						const ContainerPropertyType* ContainerProperty = CastFieldChecked<ContainerPropertyType>(Resolved.Property);
						OutContainerHelper.Add(MakeUnique<HelperType>(ContainerProperty, ContainerProperty->template ContainerPtrToValuePtr<UObject*>(Resolved.Object)));
					}
				}
			}
			return true;
		}
		return false;
	}

	void CopyPropertyValueForInsert(const TSharedPtr<FDetailTreeNode>& SourceDetailsNode, const TSharedPtr<FDetailTreeNode>& DestinationDetailsNode)
	{
		const TSharedPtr<IPropertyHandle> DestinationHandle = DestinationDetailsNode->CreatePropertyHandle();
		const TSharedPtr<IPropertyHandle> SourceHandle = SourceDetailsNode->CreatePropertyHandle();

		// Array
		TArray<TUniquePtr<FScriptArrayHelper>> SourceArrays;
		FPropertyNode* SourceArrayPropertyNode;
		if (TryGetSourceContainer(SourceDetailsNode, SourceArrayPropertyNode, SourceArrays))
		{
			int32 InsertIndex;
			TArray<TUniquePtr<FScriptArrayHelper>> DestinationArrays;
			FPropertyNode* DestinationArrayPropertyNode;
			if (ensure(TryGetDestinationContainer(DestinationDetailsNode, DestinationArrayPropertyNode, DestinationArrays, InsertIndex)))
			{
				ensure(SourceArrays.Num() == DestinationArrays.Num());

				GEditor->BeginTransaction(TEXT("DetailsSplitter"), FText::Format(INVTEXT("Insert {0}"), SourceHandle->GetPropertyDisplayName()), nullptr);
				DestinationHandle->NotifyPreChange();
				for (int32 ArrayNum = 0; ArrayNum < SourceArrays.Num(); ++ ArrayNum)
				{
					DestinationArrays[ArrayNum]->InsertValues(InsertIndex, 1);
					const void* SourceData = SourceArrays[ArrayNum]->GetElementPtr(SourceDetailsNode->GetPropertyNode()->GetArrayIndex());
					void* DestinationData = DestinationArrays[ArrayNum]->GetElementPtr(InsertIndex);
					const FArrayProperty* ArrayProperty = CastFieldChecked<FArrayProperty>(DestinationArrayPropertyNode->GetProperty());
					const FProperty* ElementProperty = ArrayProperty->Inner;
					ElementProperty->CopySingleValue(DestinationData, SourceData);
				}

				DestinationHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
				DestinationHandle->NotifyFinishedChangingProperties();
				GEditor->EndTransaction();
			}
			return;
		}

		// Set
		TArray<TUniquePtr<FScriptSetHelper>> SourceSets;
		FPropertyNode* SourceSetPropertyNode;
		if (TryGetSourceContainer(SourceDetailsNode, SourceSetPropertyNode, SourceSets))
		{
			int32 InsertIndex;
			TArray<TUniquePtr<FScriptSetHelper>> DestinationSets;
			FPropertyNode* DestinationPropertyNode;
			if (ensure(TryGetDestinationContainer(DestinationDetailsNode, DestinationPropertyNode, DestinationSets, InsertIndex)))
			{
				ensure(SourceSets.Num() == DestinationSets.Num());
				GEditor->BeginTransaction(TEXT("DetailsSplitter"), FText::Format(INVTEXT("Insert {0}"), SourceHandle->GetPropertyDisplayName()), nullptr);
				DestinationHandle->NotifyPreChange();
				for (int32 SetNum = 0; SetNum < SourceSets.Num(); ++ SetNum)
				{
					const void* SourceData = SourceSets[SetNum]->FindNthElementPtr(SourceDetailsNode->GetPropertyNode()->GetArrayIndex());
					DestinationSets[SetNum]->AddElement(SourceData);
				}

				DestinationHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
				DestinationHandle->NotifyFinishedChangingProperties();
				GEditor->EndTransaction();
			}
			return;
		}

		// Map
		TArray<TUniquePtr<FScriptMapHelper>> SourceMaps;
		FPropertyNode* SourceMapPropertyNode;
		if (TryGetSourceContainer(SourceDetailsNode, SourceMapPropertyNode, SourceMaps))
		{
			int32 InsertIndex;
			TArray<TUniquePtr<FScriptMapHelper>> DestinationMaps;
			FPropertyNode* DestinationPropertyNode;
			if (ensure(TryGetDestinationContainer(DestinationDetailsNode, DestinationPropertyNode, DestinationMaps, InsertIndex)))
			{
				ensure(SourceMaps.Num() == DestinationMaps.Num());
				GEditor->BeginTransaction(TEXT("DetailsSplitter"), FText::Format(INVTEXT("Insert {0}"), SourceHandle->GetPropertyDisplayName()), nullptr);
				DestinationHandle->NotifyPreChange();
				for (int32 MapNum = 0; MapNum < SourceMaps.Num(); ++ MapNum)
				{
					const int32 Index = SourceMaps[MapNum]->FindInternalIndex(SourceDetailsNode->GetPropertyNode()->GetArrayIndex());
					const void* SourceKey = SourceMaps[MapNum]->GetKeyPtr(Index);
					const void* SourceVal = SourceMaps[MapNum]->GetValuePtr(Index);
					DestinationMaps[MapNum]->AddPair(SourceKey, SourceVal);
				}

				DestinationHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
				DestinationHandle->NotifyFinishedChangingProperties();
				GEditor->EndTransaction();
			}
			return;
		}
	}

	void AssignPropertyValue(const TSharedPtr<IPropertyHandle>& SourceHandle, const TSharedPtr<IPropertyHandle>& DestinationHandle)
	{
		TArray<FString> SourceValues;
		SourceHandle->GetPerObjectValues(SourceValues);
		DestinationHandle->SetPerObjectValues(SourceValues);
	}

	void CopyPropertyValue(const TSharedPtr<FDetailTreeNode>& SourceDetailsNode, const TSharedPtr<FDetailTreeNode>& DestinationDetailsNode, ETreeDiffResult Diff)
	{
		switch(Diff)
		{
			// traditional copy
			case ETreeDiffResult::DifferentValues:
				break;

			// insert
			case ETreeDiffResult::MissingFromTree1:
			case ETreeDiffResult::MissingFromTree2:
				CopyPropertyValueForInsert(SourceDetailsNode, DestinationDetailsNode);
				return;

			// no difference
			case ETreeDiffResult::Invalid:
			case ETreeDiffResult::Identical:
			default:
				return;
		}


		const TSharedPtr<IPropertyHandle> SourceHandle = SourceDetailsNode->CreatePropertyHandle();
		const TSharedPtr<IPropertyHandle> DestinationHandle = DestinationDetailsNode->CreatePropertyHandle();
		if (!ensure(SourceHandle && DestinationHandle))
		{
			return;
		}

		if (!SourceHandle->GetProperty()->SameType(DestinationHandle->GetProperty()))
		{
			// convert types by assigning via text serialization
			AssignPropertyValue(SourceHandle, DestinationHandle);
			return;
		}

		TArray<void*> SourceData;
		TArray<void*> DestinationData;
		SourceHandle->AccessRawData(SourceData);
		DestinationHandle->AccessRawData(DestinationData);
		if (!ensure(SourceData.Num() == DestinationData.Num()))
		{
			return;
		}

		GEditor->BeginTransaction(TEXT("DetailsSplitter"), FText::Format(INVTEXT("Copy {0}"), SourceHandle->GetPropertyDisplayName()), nullptr);
		DestinationHandle->NotifyPreChange();
		for (int32 I = 0; I < SourceData.Num(); ++I)
		{
			if (DestinationHandle->GetArrayIndex() != INDEX_NONE)
			{
				DestinationHandle->GetProperty()->CopySingleValue(DestinationData[I], SourceData[I]);
			}
			else
			{
				DestinationHandle->GetProperty()->CopyCompleteValue(DestinationData[I], SourceData[I]);
			}
		}
		DestinationHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
		DestinationHandle->NotifyFinishedChangingProperties();
		GEditor->EndTransaction();
	}

	// note: DestinationDetailsNode is the node before the position in the tree you wish to insert
	bool CanCopyPropertyValueForInsert(const TSharedPtr<FDetailTreeNode>& SourceDetailsNode, const TSharedPtr<FDetailTreeNode>& DestinationDetailsNode)
	{
		FPropertyNode* SourceArrayPropertyNode;
		if (TryGetSourceContainer<FArrayProperty>(SourceDetailsNode, SourceArrayPropertyNode))
		{
			FPropertyNode* DestinationArrayPropertyNode;
			int32 InsertIndex;
			if (TryGetDestinationContainer<FArrayProperty>(DestinationDetailsNode, DestinationArrayPropertyNode, InsertIndex))
			{
				return true;
			}
			return false;
		}

		FPropertyNode* SourceSetPropertyNode;
		if (TryGetSourceContainer<FSetProperty>(SourceDetailsNode, SourceSetPropertyNode))
		{
			FPropertyNode* DestinationSetPropertyNode;
			int32 InsertIndex;
			if (TryGetDestinationContainer<FSetProperty>(DestinationDetailsNode, DestinationSetPropertyNode, InsertIndex))
			{
				return true;
			}
			return false;
		}

		FPropertyNode* SourceMapPropertyNode;
		if (TryGetSourceContainer<FMapProperty>(SourceDetailsNode, SourceMapPropertyNode))
		{
			FPropertyNode* DestinationMapPropertyNode;
			int32 InsertIndex;
			if (TryGetDestinationContainer<FMapProperty>(DestinationDetailsNode, DestinationMapPropertyNode, InsertIndex))
			{
				return true;
			}
			return false;
		}

		// you can only insert into containers
		return false;
	}

	bool CanAssignPropertyValue(const TSharedPtr<IPropertyHandle>& SourceHandle, const TSharedPtr<IPropertyHandle>& DestinationHandle)
	{
		TArray<FString> SourceValues;
		TArray<FString> OldDestinationValues;
		SourceHandle->GetPerObjectValues(SourceValues);
		DestinationHandle->GetPerObjectValues(OldDestinationValues);

		FPropertyAccess::Result Result = DestinationHandle->SetPerObjectValues(SourceValues, EPropertyValueSetFlags::NotTransactable | EPropertyValueSetFlags::InteractiveChange);
		TArray<FString> ChangedDestinationValues;
		DestinationHandle->GetPerObjectValues(ChangedDestinationValues);

		// revert changes and query whether anything changed
		DestinationHandle->SetPerObjectValues(OldDestinationValues, EPropertyValueSetFlags::NotTransactable | EPropertyValueSetFlags::InteractiveChange);

		return Result != FPropertyAccess::Fail && OldDestinationValues != ChangedDestinationValues;
	}

	bool CanCopyPropertyValue(const TSharedPtr<FDetailTreeNode>& SourceDetailsNode, const TSharedPtr<FDetailTreeNode>& DestinationDetailsNode, ETreeDiffResult Diff)
	{
		if (!SourceDetailsNode || !DestinationDetailsNode)
		{
			return false;
		}

		// in order to copy properties there needs to be the same number of objects in each panel
		const TArray<TWeakObjectPtr<UObject>>* SourceObjects = DetailsSplitterHelpers::GetObjects(SourceDetailsNode);
		const TArray<TWeakObjectPtr<UObject>>* DestinationObjects = DetailsSplitterHelpers::GetObjects(DestinationDetailsNode);
		if (!SourceObjects || !DestinationObjects || SourceObjects->Num() != DestinationObjects->Num())
		{
			return false;
		}

		switch (Diff)
		{
		// traditional copy
		case ETreeDiffResult::DifferentValues:
		{
			const TSharedPtr<IPropertyHandle> SourceHandle = SourceDetailsNode->CreatePropertyHandle();
			const TSharedPtr<IPropertyHandle> DestinationHandle = DestinationDetailsNode->CreatePropertyHandle();
			if (SourceHandle && DestinationHandle && SourceHandle->GetProperty() && DestinationHandle->GetProperty())
			{
				TArray<void*> SourceData;
				TArray<void*> DestinationData;
				SourceHandle->AccessRawData(SourceData);
				DestinationHandle->AccessRawData(DestinationData);
				if (SourceData == DestinationData && SourceHandle->GetProperty() == DestinationHandle->GetProperty())
                {
                	// disable copying a value to itself since it's a no-op
                	return false;
                }
				if (SourceHandle->GetProperty()->SameType(DestinationHandle->GetProperty()))
				{
					return true;
				}
				if (CanAssignPropertyValue(SourceHandle, DestinationHandle))
				{
					return true;
				}
			}
			return false;
		}

		// insert
		case ETreeDiffResult::MissingFromTree1:
		case ETreeDiffResult::MissingFromTree2:
			return CanCopyPropertyValueForInsert(SourceDetailsNode, DestinationDetailsNode);

		// no difference
		case ETreeDiffResult::Invalid:
		case ETreeDiffResult::Identical:
		default:
			return false;
		}
	}

}

bool FVoxelPropertiesSplitterHelper::CanCopyPropertyValue(const TSharedPtr<FDetailTreeNode>& SourceDetailsNode, const TSharedPtr<FDetailTreeNode>& DestinationDetailsNode, const ETreeDiffResult Diff)
{
	return DetailsSplitterHelpers::CanCopyPropertyValue(SourceDetailsNode, DestinationDetailsNode, Diff);
}

void FVoxelPropertiesSplitterHelper::CopyPropertyValue(const TSharedPtr<FDetailTreeNode>& SourceDetailsNode, const TSharedPtr<FDetailTreeNode>& DestinationDetailsNode, ETreeDiffResult Diff)
{
	return DetailsSplitterHelpers::CopyPropertyValue(SourceDetailsNode, DestinationDetailsNode, Diff);
}
