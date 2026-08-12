// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelPropertyEditorArray.h"
#include "SDropTarget.h"
#include "VoxelPinValue.h"
#include "DragAndDrop/AssetDragDropOp.h"

void SVoxelPropertyEditorArray::Construct(const FArguments& InArgs, const TSharedRef<IPropertyHandle>& Handle)
{
	PropertyHandle = Handle;

	const TSharedRef<IPropertyHandle> ArrayHandle = PropertyHandle->GetChildHandleStatic(FVoxelPinValue, Array);
	ChildSlot
	.Padding(0.f, 0.f, 2.0f, 0.f)
	.VAlign(VAlign_Center)
	[
		SNew(SDropTarget)
		.OnDropped(this, &SVoxelPropertyEditorArray::OnDragDropTarget)
		.OnAllowDrop(this, &SVoxelPropertyEditorArray::WillAddValidElements)
		.OnIsRecognized(this, &SVoxelPropertyEditorArray::IsValidAssetDropOp)
		.Content()
		[
			SNew(SVoxelDetailText)
			.Text_Lambda([ArrayHandle]
			{
				FString ArrayString;
				if (ArrayHandle->GetValueAsDisplayString(ArrayString) == FPropertyAccess::MultipleValues)
				{
					return NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values");
				}

				uint32 NumChildren = 0;
				ArrayHandle->GetNumChildren(NumChildren);
				if (NumChildren < 2)
				{
					return FText::Format(NSLOCTEXT("PropertyEditor", "SingleArrayItemFmt", "{0} Array element"), FText::AsNumber(NumChildren));
				}

				return FText::Format(NSLOCTEXT("PropertyEditor", "NumArrayItemsFmt", "{0} Array elements"), FText::AsNumber(NumChildren));
			})
		]
	];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FReply SVoxelPropertyEditorArray::OnDragDropTarget(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent) const
{
	TSharedPtr<FDragDropOperation> DragDropOperation = InDragDropEvent.GetOperation();
	if (!DragDropOperation ||
		!DragDropOperation->IsOfType<FAssetDragDropOp>())
	{
		return FReply::Handled();
	}

	TSharedPtr<FAssetDragDropOp> AssetDragDrop = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOperation);
	if (!AssetDragDrop)
	{
		return FReply::Handled();
	}

	const TSharedRef<IPropertyHandle> TypeHandle = PropertyHandle->GetChildHandleStatic(FVoxelPinValue, Type);
	const FVoxelPinType Type = FVoxelEditorUtilities::GetStructPropertyValue<FVoxelPinType>(TypeHandle);
	if (!Type.IsValid())
	{
		return FReply::Handled();
	}

	const FVoxelPinType ExposedType = Type.GetInnerExposedType();
	if (!ExposedType.IsObject())
	{
		return FReply::Handled();
	}

	const FVoxelPinType InnerType = Type.GetInnerType();

	const TSharedRef<IPropertyHandle> ArrayHandle = PropertyHandle->GetChildHandleStatic(FVoxelPinValue, Array);
	TArray<FVoxelPinValue> NewValues;
	for (const FAssetData& AssetData : AssetDragDrop->GetAssets())
	{
		if (AssetData.IsInstanceOf(ExposedType.GetObjectClass()))
		{
			FVoxelPinValue NewValue(InnerType);
			NewValue.Get<UObject>() = AssetData.GetAsset();
			NewValues.Add(NewValue);
		}
	}

	if (NewValues.Num() == 0)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(FText::FromString("Add " + LexToString(NewValues.Num()) + " elements to array."));

	ArrayHandle->NotifyPreChange();

	FVoxelEditorUtilities::ForeachData<TArray<FVoxelTerminalPinValue>>(ArrayHandle, [&](TArray<FVoxelTerminalPinValue>& Values)
	{
		for (const FVoxelPinValue& Value : NewValues)
		{
			Values.Add(Value.AsTerminalValue());
		}
	});

	ArrayHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
	ArrayHandle->NotifyFinishedChangingProperties();

	return FReply::Handled();
}

bool SVoxelPropertyEditorArray::IsValidAssetDropOp(TSharedPtr<FDragDropOperation> InOperation) const
{
	if (!InOperation ||
		!InOperation->IsOfType<FAssetDragDropOp>())
	{
		return false;
	}

	const TSharedRef<IPropertyHandle> TypeHandle = PropertyHandle->GetChildHandleStatic(FVoxelPinValue, Type);
	const FVoxelPinType Type = FVoxelEditorUtilities::GetStructPropertyValue<FVoxelPinType>(TypeHandle);
	if (!Type.IsValid())
	{
		return false;
	}

	return Type.GetInnerExposedType().IsObject();
}

bool SVoxelPropertyEditorArray::WillAddValidElements(TSharedPtr<FDragDropOperation> InOperation) const
{
	if (!InOperation ||
		!InOperation->IsOfType<FAssetDragDropOp>())
	{
		return false;
	}

	const TSharedPtr<FAssetDragDropOp> AssetDragDrop = StaticCastSharedPtr<FAssetDragDropOp>(InOperation);
	if (!AssetDragDrop)
	{
		return false;
	}

	const TSharedRef<IPropertyHandle> TypeHandle = PropertyHandle->GetChildHandleStatic(FVoxelPinValue, Type);
	const FVoxelPinType Type = FVoxelEditorUtilities::GetStructPropertyValue<FVoxelPinType>(TypeHandle);
	if (!Type.IsValid())
	{
		return false;
	}

	const FVoxelPinType ExposedType = Type.GetInnerExposedType();
	if (!ExposedType.IsObject())
	{
		return false;
	}

	for (const FAssetData& AssetData : AssetDragDrop->GetAssets())
	{
		if (!AssetData.IsInstanceOf(ExposedType.GetObjectClass()))
		{
			return false;
		}
	}

	return true;
}