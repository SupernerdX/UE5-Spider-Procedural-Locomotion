// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelPinTypeSet.h"

struct FVoxelNode;
struct FVoxelGraphToolkit;
class UVoxelTerminalGraph;

class FVoxelGraphContextActionsBuilder
{
public:
	static TSharedPtr<FVoxelGraphContextActionsBuilder> Build(const TSharedPtr<FGraphContextMenuBuilder>& MenuBuilder);
	static TSharedPtr<FVoxelGraphContextActionsBuilder> BuildNoGraph(const TSharedPtr<FGraphContextMenuBuilder>& MenuBuilder);

private:
	static constexpr int32 GroupId_Functions = 0;
	static constexpr int32 GroupId_Graphs = 0;
	static constexpr int32 GroupId_Operators = 0;
	static constexpr int32 GroupId_StructNodes = 0;
	static constexpr int32 GroupId_ShortList = 1;
	static constexpr int32 GroupId_InlineFunctions = 2;
	static constexpr int32 GroupId_Parameters = 2;

	TWeakPtr<FGraphContextMenuBuilder> WeakMenuBuilder;
	TSharedPtr<FVoxelGraphToolkit> Toolkit;
	UVoxelTerminalGraph* ActiveTerminalGraph;

	TArray<TSharedPtr<FEdGraphSchemaAction>> ShortListActions_Forced;
	TArray<TSharedPtr<FEdGraphSchemaAction>> ShortListActions_ExactMatches;
	TArray<TSharedPtr<FEdGraphSchemaAction>> ShortListActions_Parameters;

	enum class EState
	{
		Initial,
		MemberFunctions,
		Parameters,
		Inputs,
		Outputs,
		LocalVariables,
		Promotions,
		CustomizeParameters,
		Nodes,
		FunctionLibraries,
		ShortList,
		Completed
	};
	EState State = EState::Initial;
	double EndTime = 0.f;
	double Progress = 0.f;

	int32 NumStepCompletedActions = 0;
	int32 NumStepActions = 0;

	TVoxelArray<FAssetData> AssetsToLoad;
	int32 NodeIndexToBuild = 0;

	FVoxelGraphContextActionsBuilder(
		const TSharedPtr<FGraphContextMenuBuilder>& MenuBuilder,
		const TSharedPtr<FVoxelGraphToolkit>& Toolkit,
		UVoxelTerminalGraph* ActiveTerminalGraph)
		: WeakMenuBuilder(MenuBuilder)
		, Toolkit(Toolkit)
		, ActiveTerminalGraph(ActiveTerminalGraph)
	{
	}

public:
	bool Build();
	float GetProgress() const;
	bool IsCompleted() const;

private:
	EVoxelIterate BuildInitialState(FGraphContextMenuBuilder& MenuBuilder) const;
	EVoxelIterate BuildMemberFunctions(FGraphContextMenuBuilder& MenuBuilder) const;
	EVoxelIterate BuildParameters(FGraphContextMenuBuilder& MenuBuilder);
	EVoxelIterate BuildInputs(FGraphContextMenuBuilder& MenuBuilder);
	EVoxelIterate BuildOutputs(FGraphContextMenuBuilder& MenuBuilder);
	EVoxelIterate BuildLocalVariables(FGraphContextMenuBuilder& MenuBuilder);
	EVoxelIterate BuildPromotions(FGraphContextMenuBuilder& MenuBuilder) const;
	EVoxelIterate BuildCustomizeParameters(FGraphContextMenuBuilder& MenuBuilder);
	EVoxelIterate BuildNodes(FGraphContextMenuBuilder& MenuBuilder);
	EVoxelIterate BuildFunctionLibraries(FGraphContextMenuBuilder& MenuBuilder);
	EVoxelIterate BuildShortList(FGraphContextMenuBuilder& MenuBuilder);

private:
	static bool CanCallTerminalGraph(const FGraphContextMenuBuilder& MenuBuilder, const UVoxelTerminalGraph* TerminalGraph);

private:
	bool ShouldStop() const;

private:
	template<typename T>
	static TSharedRef<T> MakeShortListAction(const TSharedRef<T>& Action)
	{
		const TSharedRef<T> ShortListAction = MakeSharedCopy(*Action);
		ShortListAction->Grouping = GroupId_ShortList;
		ShortListAction->CosmeticUpdateCategory({});
		return ShortListAction;
	}

	static TMap<FVoxelPinType, TSet<FVoxelPinType>> CollectOperatorPermutations(
		const FVoxelNode& Node,
		const UEdGraphPin& FromPin,
		const FVoxelPinTypeSet& PromotionTypes);
};