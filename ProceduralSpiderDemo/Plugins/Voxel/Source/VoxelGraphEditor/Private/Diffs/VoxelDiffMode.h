// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelDiffEntry.h"

struct FVoxelDiffEntry;

struct FVoxelModeDiffEntry : public FVoxelDiffEntry
{
	GENERATED_VOXEL_DIFF_ENTRY_BODY(FVoxelModeDiffEntry)

	using FVoxelDiffEntry::FVoxelDiffEntry;

	virtual TSharedRef<SWidget> GenerateWidget() const override;

	bool bNoDifferences = false;
};

struct FVoxelNoDiffEntry : public FVoxelDiffEntry
{
	GENERATED_VOXEL_DIFF_ENTRY_BODY(FVoxelNoDiffEntry)

	using FVoxelDiffEntry::FVoxelDiffEntry;

	virtual TSharedRef<SWidget> GenerateWidget() const override;
};

struct FVoxelDiffMode : public TSharedFromThis<FVoxelDiffMode>
{
public:
	virtual FString GetName() = 0;
	virtual void Tick() const {}
	virtual void GenerateDifferencesList(TArray<TSharedPtr<FVoxelDiffEntry>>& OutEntries) {}
	virtual TSharedRef<SWidget> GetWidget() const { return SNullWidget::NullWidget; }
	virtual void OnEntrySelected(const TSharedPtr<FVoxelDiffEntry>& Entry) {}

public:
	TSharedPtr<FVoxelDiffEntry> GenerateTreeEntry();

public:
	virtual ~FVoxelDiffMode() = default;
};
