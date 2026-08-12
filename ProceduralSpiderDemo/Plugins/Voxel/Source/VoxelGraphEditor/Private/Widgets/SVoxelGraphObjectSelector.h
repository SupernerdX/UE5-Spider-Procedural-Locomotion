// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class SVoxelGraphObjectSelector : public SCompoundWidget
{
private:
	struct FObjectOrAssetData
	{
		UObject* Object = nullptr;
		FSoftObjectPath ObjectPath;
		FAssetData AssetData;

		FObjectOrAssetData() = default;
		explicit FObjectOrAssetData(UObject* Object)
			: Object(Object)
			, ObjectPath(Object)
		{
			if (Object && !Object->IsA<AActor>())
			{
				AssetData = FAssetData(Object);
			}
		}
		explicit FObjectOrAssetData(const FSoftObjectPath& ObjectPath)
			: ObjectPath(ObjectPath)
		{
		}
		explicit FObjectOrAssetData(const FAssetData& AssetData)
			: ObjectPath(AssetData.ToSoftObjectPath())
			, AssetData(AssetData)
		{
		}

		bool IsValid() const
		{
			return
				Object != nullptr ||
				ObjectPath.IsValid() ||
				AssetData.IsValid();
		}
	};

public:
	VOXEL_SLATE_ARGS()
	{
		FArguments()
			: _ThumbnailSize(FIntPoint(48, 48))
		{
		}
		SLATE_ATTRIBUTE(FString, ObjectPath)
		SLATE_ARGUMENT(TVoxelArray<UClass*>, AllowedClasses)
		SLATE_ARGUMENT(FIntPoint, ThumbnailSize)
		SLATE_ARGUMENT(TSharedPtr<FAssetThumbnailPool>, ThumbnailPool)
		SLATE_EVENT(FOnSetObject, OnObjectChanged)
	};

	void Construct(const FArguments& InArgs);

private:
	bool OnAssetDraggedOver(TArrayView<FAssetData> InAssets, FText& OutReason) const;
	void OnAssetDropped(const FDragDropEvent&, TArrayView<FAssetData> InAssets) const;

	TSharedRef<SWidget> OnGetMenuContent() const;
	void OnMenuOpenChanged(bool bOpen) const;

	bool IsFilteredActor(const AActor* Actor) const;
	void CloseComboButton() const;

	FText OnGetAssetName() const;

	FReply OnAssetThumbnailDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) const;

	void OnUseClicked() const;
	void OnBrowseClicked() const;

	void OnGetAllowedClasses(TArray<const UClass*>& OutAllowedClasses) const;
	void OnActorSelected(AActor* Actor) const;

private:
	FObjectOrAssetData GetValue() const;
	void SetValue(const FAssetData& AssetData) const;

	bool CanSetBasedOnCustomClasses(const FAssetData& AssetData) const;
	bool CanSetBasedOnAssetReferenceFilter( const FAssetData& InAssetData, FText& OutOptionalFailureReason) const;
	bool IsClassAllowed(const UClass* Class) const;

private:
	TAttribute<FString> ObjectPath;

	TSharedPtr<SComboButton> AssetComboButton;
	TSharedPtr<SBorder> ThumbnailBorder;

	TVoxelArray<const UClass*> AllowedClasses;
	TSharedPtr<FAssetThumbnail> AssetThumbnail;
	mutable FAssetData CachedAssetData;
	TArray<FAssetData> OwnerAssetDataArray;

	bool bIsActor = false;

	FOnSetObject OnSetObject;
	FOnShouldFilterAsset OnShouldFilterAsset;
};