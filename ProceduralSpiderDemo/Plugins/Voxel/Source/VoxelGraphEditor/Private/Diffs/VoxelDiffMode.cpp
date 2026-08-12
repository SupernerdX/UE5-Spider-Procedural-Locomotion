// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelDiffMode.h"

TSharedRef<SWidget> FVoxelModeDiffEntry::GenerateWidget() const
{
	FString ModeName = "INVALID";
	if (const TSharedPtr<FVoxelDiffMode> Mode = WeakMode.Pin())
	{
		ModeName = Mode->GetName();
	}

	return SNew(STextBlock)
		.Text(FText::FromString(ModeName))
		.ToolTipText(FText::FromString(ModeName))
		.ColorAndOpacity(bNoDifferences ? FSlateColor::UseForeground() : FLinearColor(0.85f, 0.71f, 0.25f));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> FVoxelNoDiffEntry::GenerateWidget() const
{
	return SNew(STextBlock)
		.Text(INVTEXT("No differences detected..."))
		.TextStyle(FAppStyle::Get(), TEXT("BlueprintDif.ItalicText"))
		.ColorAndOpacity(FSlateColor::UseSubduedForeground());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelDiffEntry> FVoxelDiffMode::GenerateTreeEntry()
{
	TSharedRef<FVoxelModeDiffEntry> Entry = MakeShared<FVoxelModeDiffEntry>(AsShared());
	GenerateDifferencesList(Entry->Children);
	Entry->bNoDifferences = Entry->Children.Num() == 0;
	return Entry;
}