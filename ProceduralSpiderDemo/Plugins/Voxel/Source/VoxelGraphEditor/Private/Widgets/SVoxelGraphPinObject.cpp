// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphPinObject.h"
#include "VoxelPinType.h"
#include "VoxelObjectPinType.h"
#include "SVoxelGraphObjectSelector.h"

void SVoxelGraphPinObject::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	VOXEL_FUNCTION_COUNTER();

	SGraphPinObject::Construct(SGraphPinObject::FArguments(), InGraphPinObj);
	GetLabelAndValue()->SetWrapSize(300.f);

	SVoxelGraphPin::ConstructDebugHighlight(this);
}

TSharedRef<SWidget>	SVoxelGraphPinObject::GetDefaultValueWidget()
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(GraphPinObj))
	{
		return SNullWidget::NullWidget;
	}

	const FVoxelPinType PinType = FVoxelPinType(GraphPinObj->PinType).GetInnerType();

	if (!ensureVoxelSlow(PinType.IsValid()) ||
		!ensureVoxelSlow(PinType.IsStruct()))
	{
		// Will happen if we remove a FVoxelObjectPinType
		return
			SNew(SBox)
			.MaxDesiredWidth(200.f)
			[
				SNew(SVoxelGraphObjectSelector)
				.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
				.IsEnabled(false)
				.AllowedClasses({ UObject::StaticClass() })
				.ThumbnailPool(FVoxelEditorUtilities::GetThumbnailPool())
				.ObjectPath_Lambda([this]
				{
					return GetAssetData(true).GetSoftObjectPath().ToString();
				})
				.OnObjectChanged(this, &SVoxelGraphPinObject::OnAssetSelectedFromPicker)
			];
	}

	const TSharedPtr<const FVoxelObjectPinType> ObjectPinType = FVoxelObjectPinType::StructToPinType().FindRef(PinType.GetStruct());
	if (!ensure(ObjectPinType))
	{
		return SNullWidget::NullWidget;
	}

	return
		SNew(SBox)
		.MaxDesiredWidth(200.f)
		[
			SNew(SVoxelGraphObjectSelector)
			.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
			.IsEnabled(this, &SGraphPin::IsEditingEnabled)
			.AllowedClasses(ObjectPinType->GetAllowedClasses())
			.ThumbnailPool(FVoxelEditorUtilities::GetThumbnailPool())
			.ObjectPath_Lambda([this]
			{
				return GetAssetData(true).GetSoftObjectPath().ToString();
			})
			.OnObjectChanged(this, &SVoxelGraphPinObject::OnAssetSelectedFromPicker)
		];
}

void SVoxelGraphPinObject::OnAssetSelectedFromPicker(const FAssetData& AssetData)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(!GraphPinObj->IsPendingKill()) ||
		AssetData == GetAssetData(true))
	{
		return;
	}

	const FVoxelTransaction Transaction(GraphPinObj, "Change Object Pin Value");

	GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, AssetData.GetAsset());
}