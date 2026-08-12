// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class SVoxelMenuAssetPicker : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		FArguments()
			: _InitialObject(nullptr)
			, _AllowClear(true)
			, _AllowCopyPaste(true)
			, _ForceShowEngineContent(false)
			, _ForceShowPluginContent(false)
		{}
		SLATE_ARGUMENT(FAssetData, InitialObject)
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle)
		SLATE_ARGUMENT(TArray<FAssetData>, OwnerAssetArray)
		SLATE_ARGUMENT(bool, AllowClear)
		SLATE_ARGUMENT(bool, AllowCopyPaste)
		SLATE_ARGUMENT(TArray<const UClass*>, AllowedClasses)
		SLATE_ARGUMENT(TArray<const UClass*>, DisallowedClasses)
		SLATE_ARGUMENT(TArray<UFactory*>, NewAssetFactories)
		SLATE_ARGUMENT(bool, ForceShowEngineContent)
		SLATE_ARGUMENT(bool, ForceShowPluginContent)
		SLATE_EVENT(FOnShouldFilterAsset, OnShouldFilterAsset)
		SLATE_EVENT(FOnAssetSelected, OnSet)
		SLATE_EVENT(FSimpleDelegate, OnClose)
	};

	void Construct( const FArguments& InArgs );

private:
	void OnEdit();
	void OnCopy();
	void OnPaste();
	bool CanPasteFromText(const FString& InTag, const FString& InText) const;
	void OnPasteFromText(const FString& InTag, const FString& InText, const TOptional<FGuid>& InOperationId);
	void PasteFromText(const FString& InTag, const FString& InText);
	bool CanPaste();
	void OnClear();
	void OnAssetSelected( const FAssetData& AssetData );
	void OnAssetEnterPressed( const TArray<FAssetData>& AssetData );
	void OnCreateNewAssetSelected(TWeakObjectPtr<UFactory> Factory);
	void SetValue( const FAssetData& AssetData );

	virtual FReply OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	FAssetData CurrentObject;
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<SWidget> AssetPickerWidget;
	bool bAllowClear = true;
	bool bAllowCopyPaste = true;
	TArray<const UClass*> AllowedClasses;
	TArray<const UClass*> DisallowedClasses;
	TArray<UFactory*> NewAssetFactories;
	FOnShouldFilterAsset OnShouldFilterAsset;
	FOnAssetSelected OnSet;
	FSimpleDelegate OnClose;
};