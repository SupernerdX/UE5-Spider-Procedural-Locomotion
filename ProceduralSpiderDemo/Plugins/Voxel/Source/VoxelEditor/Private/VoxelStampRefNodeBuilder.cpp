// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampRefNodeBuilder.h"
#include "VoxelStampRef.h"
#include "VoxelStampRefDataProvider.h"

void FVoxelStampRefNodeBuilder::Initialize()
{
	VOXEL_FUNCTION_COUNTER();

	StructHandle->SetOnPropertyValueChanged(MakeWeakPtrDelegate(this, [this]
	{
		if (LastStructs != GetStructs())
		{
			OnRebuildChildren.ExecuteIfBound();
		}
	}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelArray<UScriptStruct*> FVoxelStampRefNodeBuilder::GetStructs() const
{
	TVoxelArray<UScriptStruct*> Result;
	FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(StructHandle, [&](const FVoxelStampRef& StampRef)
	{
		Result.Add(StampRef.GetStruct());
	});
	return Result;
}

void FVoxelStampRefNodeBuilder::SetOnRebuildChildren(const FSimpleDelegate NewOnRebuildChildren)
{
	OnRebuildChildren = NewOnRebuildChildren;
}

void FVoxelStampRefNodeBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
}

void FVoxelStampRefNodeBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	LastStructs = GetStructs();

	if (StructHandle->GetNumPerObjectValues() > 500 &&
		!bDisableObjectCountLimit)
	{
		ChildBuilder.AddCustomRow(INVTEXT("Expand"))
		.WholeRowContent()
		[
			SNew(SVoxelDetailButton)
			.Text(FText::FromString(FString::Printf(TEXT("Expand %d structs"), StructHandle->GetNumPerObjectValues())))
			.OnClicked_Lambda([this]
			{
				bDisableObjectCountLimit = true;
				OnRebuildChildren.ExecuteIfBound();
				return FReply::Handled();
			})
		];

		return;
	}

	const TSharedRef<FVoxelStampRefDataProvider> StructProvider = MakeShared<FVoxelStampRefDataProvider>(StructHandle);
	const UStruct* BaseStruct = StructProvider->GetBaseStructure();

	if (!BaseStruct)
	{
		return;
	}

	IDetailPropertyRow* PropertyRow = ChildBuilder.AddChildStructure(
		StructHandle,
		StructProvider,
		StructHandle->GetProperty()->GetFName(),
		BaseStruct->GetDisplayNameText());

	if (!ensureVoxelSlow(PropertyRow))
	{
		return;
	}

	// Expansion state is not properly persisted for these structures, so let's expand it by default for now
	PropertyRow->ShouldAutoExpand(true);

	const TSharedPtr<IPropertyHandle> Handle = PropertyRow->GetPropertyHandle();
	if (!ensureVoxelSlow(Handle))
	{
		return;
	}

	if (const FProperty* MetaDataProperty = StructHandle->GetMetaDataProperty())
	{
		if (const TMap<FName, FString>* MetaDataMap = MetaDataProperty->GetMetaDataMap())
		{
			for (const auto& It : *MetaDataMap)
			{
				Handle->SetInstanceMetaData(It.Key, It.Value);
			}
		}
	}

	if (const TMap<FName, FString>* MetaDataMap = StructHandle->GetInstanceMetaDataMap())
	{
		for (const auto& It : *MetaDataMap)
		{
			Handle->SetInstanceMetaData(It.Key, It.Value);
		}
	}
}

void FVoxelStampRefNodeBuilder::Tick(float DeltaTime)
{
	VOXEL_FUNCTION_COUNTER();

	if (LastStructs != GetStructs())
	{
		OnRebuildChildren.ExecuteIfBound();
	}
}

FName FVoxelStampRefNodeBuilder::GetName() const
{
	return "VoxelStampRefNodeBuilder";
}