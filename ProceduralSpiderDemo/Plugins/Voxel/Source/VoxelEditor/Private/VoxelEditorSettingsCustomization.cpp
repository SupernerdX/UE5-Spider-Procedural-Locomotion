// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelEditorSettings.h"

VOXEL_CUSTOMIZE_CLASS(UVoxelEditorSettings)(IDetailLayoutBuilder& DetailLayout)
{
	IDetailCategoryBuilder& Category = DetailLayout.EditCategory("Examples");
	Category.AddCustomRow(INVTEXT("Clear Cache"))
	.NameContent()
	[
		SNew(SVoxelDetailText)
		.Text(INVTEXT("Clear Cache"))
	]
	.ValueContent()
	[
		SNew(SButton)
		.Text(INVTEXT("Clear"))
		.HAlign(HAlign_Center)
		.OnClicked_Lambda([]
		{
			const FString Path = FVoxelUtilities::GetAppDataCache() / "Examples";

			TArray<FString> Files;
			IFileManager::Get().FindFilesRecursive(Files, *Path, TEXT("*"), true, false);
			for (const FString& File : Files)
			{
				ensure(IFileManager::Get().Delete(*File));
			}

			return FReply::Handled();
		})
	];
}