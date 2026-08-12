// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class FVoxelBugReport : public TSharedFromThis<FVoxelBugReport>
{
public:
	TVoxelSet<FName> PackagePaths;

	FVoxelBugReport() = default;

	void Open(const TArray<FAssetData>& SelectedAssets);
	void AddPackage(FName PackageName);

private:
	TVoxelSet<FString> VisitedExternalObjectsPaths;
};