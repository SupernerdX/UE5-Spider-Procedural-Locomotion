// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class UVoxelGraph;
class UVoxelTerminalGraph;

class SVoxelGraphMessages : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ARGUMENT(UVoxelGraph*, Graph);
	};

	void Construct(const FArguments& Args);
	void UpdateNodes();

private:
	class FGraphNode;
	class FMessageNode;

	class INode
	{
	public:
		INode() = default;
		virtual ~INode() = default;

		virtual TSharedRef<SWidget> GetWidget() const = 0;
		virtual TArray<TSharedPtr<INode>> GetChildren() const = 0;
	};
	class FRootNode : public INode
	{
	public:
		const FString Text;
		TMap<TVoxelObjectPtr<UVoxelTerminalGraph>, TSharedPtr<FGraphNode>> GraphToNode;

		explicit FRootNode(const FString& Text)
			: Text(Text)
		{
		}

		//~ Begin INode Interface
		virtual TSharedRef<SWidget> GetWidget() const override;
		virtual TArray<TSharedPtr<INode>> GetChildren() const override;
		//~ End INode Interface
	};
	class FGraphNode : public INode
	{
	public:
		const TVoxelObjectPtr<UVoxelTerminalGraph> WeakGraph;
		TMap<TSharedPtr<FVoxelMessage>, TSharedPtr<FMessageNode>> MessageToNode;

		explicit FGraphNode(const TVoxelObjectPtr<UVoxelTerminalGraph>& WeakGraph)
			: WeakGraph(WeakGraph)
		{
		}

		//~ Begin INode Interface
		virtual TSharedRef<SWidget> GetWidget() const override;
		virtual TArray<TSharedPtr<INode>> GetChildren() const override;
		//~ End INode Interface
	};
	class FMessageNode : public INode
	{
	public:
		const TSharedRef<FVoxelMessage> Message;

		explicit FMessageNode(const TSharedRef<FVoxelMessage>& Message)
			: Message(Message)
		{
		}

		//~ Begin INode Interface
		virtual TSharedRef<SWidget> GetWidget() const override;
		virtual TArray<TSharedPtr<INode>> GetChildren() const override;
		//~ End INode Interface
	};

private:
	using STree = STreeView<TSharedPtr<INode>>;

	TVoxelObjectPtr<UVoxelGraph> WeakGraph;
	TSharedPtr<STree> Tree;
	TArray<TSharedPtr<INode>> Nodes;

	TVoxelMap<TWeakPtr<FVoxelMessage>, TWeakPtr<FVoxelMessage>> MessageToCanonMessage;
	TMap<uint64, TWeakPtr<FVoxelMessage>> HashToCanonMessage;

	const TSharedRef<FRootNode> CompileNode = MakeShared<FRootNode>("Compile Errors");
	const TSharedRef<FRootNode> RuntimeNode = MakeShared<FRootNode>("Runtime Errors");

	TSharedRef<FVoxelMessage> GetCanonMessage(const TSharedRef<FVoxelMessage>& Message);
};