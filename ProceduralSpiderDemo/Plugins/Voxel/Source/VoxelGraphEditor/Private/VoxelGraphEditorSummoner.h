// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"

class UVoxelGraph;
struct FVoxelGraphToolkit;

struct FVoxelGraphEditorSummoner : public FDocumentTabFactoryForObjects<UEdGraph>
{
public:
	const TWeakPtr<FVoxelGraphToolkit> WeakToolkit;

	explicit FVoxelGraphEditorSummoner(const TSharedRef<FVoxelGraphToolkit>& Toolkit);

	//~ Begin FDocumentTabFactoryForObjects Interface
	virtual void OnTabActivated(TSharedPtr<SDockTab> Tab) const override;
	virtual void SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const override;

	virtual TAttribute<FText> ConstructTabNameForObject(UEdGraph* EdGraph) const override;
	virtual TSharedRef<SWidget> CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* EdGraph) const override;
	virtual const FSlateBrush* GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* EdGraph) const override;
	virtual TSharedRef<FGenericTabHistory> CreateTabHistoryNode(TSharedPtr<FTabPayload> Payload) override;
	//~ End FDocumentTabFactoryForObjects Interface

	static TSharedPtr<SGraphEditor> GetGraphEditor(const TSharedPtr<SDockTab>& Tab);

private:
	void FocusGraphEditor(const TSharedPtr<SGraphEditor>& GraphEditor) const;
};

struct FVoxelGraphReadOnlySummoner : public FDocumentTabFactoryForObjects<UVoxelGraph>
{
public:
	const TWeakPtr<FVoxelGraphToolkit> WeakToolkit;

	explicit FVoxelGraphReadOnlySummoner(const TSharedRef<FVoxelGraphToolkit>& Toolkit);

	//~ Begin FDocumentTabFactoryForObjects Interface
	virtual void OnTabActivated(TSharedPtr<SDockTab> Tab) const override;
	virtual void SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const override;

	virtual TAttribute<FText> ConstructTabNameForObject(UVoxelGraph* VoxelGraph) const override;
	virtual TSharedRef<SWidget> CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UVoxelGraph* VoxelGraph) const override;
	virtual const FSlateBrush* GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UVoxelGraph* VoxelGraph) const override;
	virtual TSharedRef<FGenericTabHistory> CreateTabHistoryNode(TSharedPtr<FTabPayload> Payload) override;
	//~ End FDocumentTabFactoryForObjects Interface

private:
	void FocusReadOnlyPreview() const;
};

struct FVoxelGraphTabHistory : public FGenericTabHistory
{
public:
	using FGenericTabHistory::FGenericTabHistory;

	//~ Begin FGenericTabHistory Interface
	virtual void EvokeHistory(TSharedPtr<FTabInfo> InTabInfo, bool bPrevTabMatches) override;
	virtual void SaveHistory() override;
	virtual void RestoreHistory() override;
	//~ End FGenericTabHistory Interface

private:
	TWeakPtr<SGraphEditor> WeakGraphEditor;
	UE_506_SWITCH(FVector2D, FVector2f) SavedLocation = UE_506_SWITCH(FVector2D, FVector2f)::ZeroVector;
	float SavedZoomAmount = -1.f;
	FGuid SavedBookmarkId;
};