// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class UVoxelTerminalGraph;
class SVoxelGraphMembers;
struct FVoxelGraphToolkit;
enum class EVoxelGraphMemberSection;

class FVoxelGraphSelection
{
public:
	FVoxelGraphToolkit& Toolkit;
	const TSharedRef<IDetailsView> DetailsView;
	const TSharedRef<SVoxelGraphMembers> GraphMembers;

	FVoxelGraphSelection(
		FVoxelGraphToolkit& Toolkit,
		const TSharedRef<IDetailsView>& DetailsView,
		const TSharedRef<SVoxelGraphMembers>& GraphMembers)
		: Toolkit(Toolkit)
		, DetailsView(DetailsView)
		, GraphMembers(GraphMembers)
	{
	}

public:
	void SelectNone();
	void SelectCategory(
		int32 SectionId,
		FName DisplayName);

	void SelectMainGraph();
	void SelectEditorGraph();
	void SelectFunction(const FGuid& Guid);
	void SelectParameter(const FGuid& Guid);

	void SelectFunctionInput(
		UVoxelTerminalGraph& TerminalGraph,
		const FGuid& Guid);

	void SelectOutput(
		UVoxelTerminalGraph& TerminalGraph,
		const FGuid& Guid);

	void SelectLocalVariable(
		UVoxelTerminalGraph& TerminalGraph,
		const FGuid& Guid);

	void SelectNode(UEdGraphNode* Node);
	void SelectTerminalGraph(const UVoxelTerminalGraph& TerminalGraph);

public:
	void RequestRename() const;
	void Update();

private:
	struct FNone
	{
	};
	struct FCategory
	{
		int32 SectionID = 0;
		FName DisplayName;
	};
	struct FMainGraph
	{
	};
	struct FEditorGraph
	{
	};
	struct FFunction
	{
		FGuid Guid;
	};
	struct FParameter
	{
		FGuid Guid;
	};
	struct FFunctionInput
	{
		TVoxelObjectPtr<UVoxelTerminalGraph> TerminalGraph;
		FGuid Guid;
	};
	struct FOutput
	{
		TVoxelObjectPtr<UVoxelTerminalGraph> TerminalGraph;
		FGuid Guid;
	};
	struct FLocalVariable
	{
		TVoxelObjectPtr<UVoxelTerminalGraph> TerminalGraph;
		FGuid Guid;
	};
	struct FNode
	{
		TVoxelObjectPtr<UEdGraphNode> Node;
	};
	using FSelection = TVariant<
		FNone,
		FCategory,
		FMainGraph,
		FEditorGraph,
		FFunction,
		FParameter,
		FFunctionInput,
		FOutput,
		FLocalVariable,
		FNode>;

	bool bIsUpdating = false;
	bool bIsSelecting = false;
	FSelection Selection;
	FSelection LastSelection;

private:
	void Update(const FNone& Data);
	void Update(const FCategory& Data);
	void Update(const FMainGraph& Data);
	void Update(const FEditorGraph& Data);
	void Update(const FFunction& Data);
	void Update(const FParameter& Data);
	void Update(const FFunctionInput& Data);
	void Update(const FOutput& Data);
	void Update(const FLocalVariable& Data);
	void Update(const FNode& Data);

private:
	template<typename T>
	void Select(const T& Value);

	template<typename T = decltype(nullptr)>
	void SetDetailObject(
		UObject* Object,
		bool bIsReadOnly = false,
		T Customization = nullptr);
};