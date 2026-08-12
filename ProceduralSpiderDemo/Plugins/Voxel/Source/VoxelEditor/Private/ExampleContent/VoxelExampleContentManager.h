// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

struct FVoxelExampleContent
{
	FString Name;
	FString Version;
	int32 Size = 0;
	FString Hash;
	FString DisplayName;
	FString Description;
	TSharedPtr<TSharedPtr<ISlateBrushSource>> Thumbnail;
	TSharedPtr<TSharedPtr<ISlateBrushSource>> Image;
	FName Category;
	TOptional<int32> CategorySortValue;
	TSet<FName> Tags;
};

class FVoxelExampleContentManager
{
public:
	FVoxelExampleContentManager() = default;

	void OnExamplesReady(const FSimpleDelegate& OnReady);
	TArray<TSharedPtr<FVoxelExampleContent>> GetExamples() const;

	static FLinearColor GetContentTagColor(FName ContentTag);

private:
	TArray<TSharedPtr<FVoxelExampleContent>> ExampleContents;

	static TSharedRef<TSharedPtr<ISlateBrushSource>> MakeImage(const FString& Hash);

public:
	static FVoxelExampleContentManager& Get();
};