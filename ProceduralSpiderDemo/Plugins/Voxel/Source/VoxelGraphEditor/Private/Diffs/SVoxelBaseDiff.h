// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "AssetDefinition.h"
#include "Widgets/SCompoundWidget.h"

struct FVoxelDiffMode;
struct FVoxelDiffEntry;

class VOXELGRAPHEDITOR_API SVoxelBaseDiff_Base : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ARGUMENT(UObject*, OldAsset)
		SLATE_ARGUMENT(UObject*, NewAsset)
		SLATE_ARGUMENT(FRevisionInfo, OldRevision)
		SLATE_ARGUMENT(FRevisionInfo, NewRevision)
	};

	virtual void Construct(const FArguments& InArgs);

	//~ Begin SCompoundWidget Interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	//~ End SCompoundWidget Interface

	void AssignWindow(const TSharedPtr<SWindow>& Window);

	virtual FText GetWindowName() = 0;
	virtual void InitializeModes(TArray<TSharedPtr<FVoxelDiffMode>>& OutModes) {}
	void OnEntrySelected(const TSharedPtr<FVoxelDiffEntry>& Entry);

	void RequestRefresh() { bRefresh = true; }

protected:
	void GenerateDifferencesList();
	bool NewAssetIsLocal() const;
	void SetMode(const TSharedPtr<FVoxelDiffMode>& NewMode);

private:
	bool HasPrev() const;
	void JumpToPrev();
	bool HasNext() const;
	void JumpToNext();

	int32 GetSelectedEntryIndex() const;

public:
	template<typename T>
	static TSharedRef<T> CreateDiffWindow(
		const UObject* OldObject,
		const UObject* NewObject,
		const FRevisionInfo& OldRevision,
		const FRevisionInfo& NewRevision)
	{
		TSharedRef<T> Widget =
			SNew(T)
			.OldAsset(ConstCast(OldObject))
			.NewAsset(ConstCast(NewObject))
			.OldRevision(OldRevision)
			.NewRevision(NewRevision);

		CreateDiffWindow(Widget);

		return Widget;
	}

private:
	static void CreateDiffWindow(const TSharedRef<SVoxelBaseDiff_Base>& Widget);

private:
	TObjectPtr<UObject> OldAsset;
	TObjectPtr<UObject> NewAsset;

	FRevisionInfo OldRevision;
	FRevisionInfo NewRevision;

	TWeakPtr<SWindow> WeakWindow;
	FDelegateHandle AssetEditorCloseDelegate;
	TMulticastDelegate<void(const TSharedPtr<SVoxelBaseDiff_Base>&)> OnWindowClosed;

	TArray<TSharedPtr<FVoxelDiffMode>> Modes;
	TSharedPtr<FVoxelDiffMode> CurrentMode;

	bool bRefresh = false;

protected:
	TSharedPtr<SBox> ModeContents;
	TSharedPtr<STreeView<TSharedPtr<FVoxelDiffEntry>>> DiffTreeView;

	TArray<TSharedPtr<FVoxelDiffEntry>> Entries;
	TArray<TSharedPtr<FVoxelDiffEntry>> LinearEntries;

	template<typename>
	friend class SVoxelBaseDiff;
};

template<typename>
class SVoxelBaseDiff;

template<typename T>
requires std::derived_from<T, UObject>
class SVoxelBaseDiff<T> : public SVoxelBaseDiff_Base
{
protected:
	T* GetOldAsset()
	{
		return Cast<T>(OldAsset);
	}
	const T* GetOldAsset() const
	{
		return Cast<T>(OldAsset);
	}

	T* GetNewAsset()
	{
		return Cast<T>(NewAsset);
	}
	const T* GetNewAsset() const
	{
		return Cast<T>(NewAsset);
	}

public:
	virtual FText GetWindowName() override
	{
		const FString ObjectName = GetNewAsset() ? GetNewAsset()->GetName() : GetOldAsset()->GetName();
		return FText::FromString(ObjectName + " - " + T::StaticClass()->GetDisplayNameText().ToString() + " Diff");
	}
};