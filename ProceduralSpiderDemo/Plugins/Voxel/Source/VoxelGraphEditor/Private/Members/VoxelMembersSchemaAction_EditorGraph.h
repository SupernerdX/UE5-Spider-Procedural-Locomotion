// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Members/VoxelMembersSchemaAction_Base.h"

class UVoxelGraph;

struct FVoxelMembersSchemaAction_EditorGraph : public FVoxelMembersSchemaAction_Base
{
public:
	TVoxelObjectPtr<UVoxelGraph> WeakGraph;

	using FVoxelMembersSchemaAction_Base::FVoxelMembersSchemaAction_Base;

	//~ Begin FVoxelMembersSchemaAction_Base Interface
	virtual UObject* GetOuter() const override;
	virtual TSharedRef<SWidget> CreatePaletteWidget(const FCreateWidgetForActionData& CreateData) const override;
	virtual void BuildContextMenu(FMenuBuilder& MenuBuilder) override;
	virtual void ApplyNewGuids(const TArray<FGuid>& NewGuids) const override;
	virtual void OnActionDoubleClick() const override;
	virtual void OnSelected() const override;
	virtual FString GetName() const override;
	virtual bool CanBeRenamed() const override { return false; }
	//~ End FVoxelMembersSchemaAction_Base Interface
};

class SVoxelMembersPaletteItem_EditorGraph : public SVoxelMembersPaletteItem<SVoxelMembersPaletteItem_EditorGraph>
{
public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(const FArguments& InArgs, const FCreateWidgetForActionData& CreateData);
};