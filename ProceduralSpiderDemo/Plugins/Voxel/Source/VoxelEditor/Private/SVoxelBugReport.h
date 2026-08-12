// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class FVoxelBugReport;

class SVoxelBugReport : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_EVENT(FSimpleDelegate, OnCloseWindow)
	};

	const TSharedRef<FVoxelBugReport> BugReport = SharedRef_Null;

	void Construct(
		const FArguments& Args,
		const TSharedRef<FVoxelBugReport>& BugReport);

private:
	struct FPath
	{
		FName PackageName;
		FName Name;
		bool bIsFile = false;
		int64 Size = 0;
		TVoxelMap<FName, TSharedPtr<FPath>> NameToChild;

		void Add(TConstVoxelArrayView<FString> Path);
		FString GetDiskPath() const;
	};

	FSimpleDelegate OnCloseWindow;
	FString Name;
	int64 TotalSize = 0;
	TVoxelSet<FName> PackagesToSkip;
	TArray<TSharedPtr<FPath>> RootPaths;
	TSharedPtr<STreeView<TSharedPtr<FPath>>> TreeView;

	void Submit();
	void UpdatePaths();
	void UpdateTotalSize();
};