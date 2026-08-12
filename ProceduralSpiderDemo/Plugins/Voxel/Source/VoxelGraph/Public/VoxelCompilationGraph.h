// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelGraphNodeRef.h"
#include "VoxelRuntimePinValue.h"

struct FVoxelNode;
struct FVoxelTemplateNodeUtilities;

DECLARE_VOXEL_MEMORY_STAT(VOXELGRAPH_API, STAT_VoxelCompilationGraph, "Compilation Graph");

namespace Voxel::Graph
{

class FPin;
class FNode;
class FGraph;

VOXELGRAPH_API void PushPinView(FPin* const * Pins);
VOXELGRAPH_API void PopPinView(FPin* const * Pins);

template<typename T>
class TPinsView
{
public:
	TPinsView(const TConstVoxelArrayView<FPin*> Pins)
		: Pins(Pins)
	{
		PushPinView(Pins.GetData());
	}
	template<typename AllocatorType>
	TPinsView(const TVoxelArray<FPin*, AllocatorType>& Pins)
		: TPinsView(MakeVoxelArrayView(Pins))
	{
	}
	~TPinsView()
	{
		PopPinView(Pins.GetData());
	}

	T& operator[](const int32 Index)
	{
		return *Pins[Index];
	}
	const T& operator[](const int32 Index) const
	{
		return *Pins[Index];
	}

	int32 Num() const
	{
		return Pins.Num();
	}

	TArray<FPin*> Array() const
	{
		return TArray<FPin*>(Pins);
	}

	auto begin() const { return MakeDereferencingIterator(Pins.begin()); }
	auto end() const { return MakeDereferencingIterator(Pins.end()); }

private:
	TConstVoxelArrayView<FPin*> Pins;
};

template<typename T>
using TNodesView = TVoxelDereferencingRangeRef<const TVoxelSet<T*>>;

template<typename T>
class TNodesCopy
{
public:
	TNodesCopy(const TVoxelSet<FNode*>& Nodes)
		: Nodes(Nodes)
	{
	}

	auto begin() const { return MakeDereferencingIterator(Nodes.begin()); }
	auto end() const { return Nodes.end(); }

private:
	TVoxelSet<FNode*> Nodes;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum class EPinDirection : uint8
{
	Input,
	Output
};

class VOXELGRAPH_API FPin
{
	enum class EPrivate {};

	friend class FGraph;

public:
	const FName Name;
	const FVoxelPinType Type;
	const EPinDirection Direction;
	FNode& Node;

	FPin(
		EPrivate,
		const FName Name,
		const FVoxelPinType& Type,
		const FVoxelPinValue& DefaultValue,
		const EPinDirection Direction,
		FNode& Node)
		: Name(Name)
		, Type(Type)
		, Direction(Direction)
		, Node(Node)
		, DefaultValue(DefaultValue)
	{
		ensure(Type.IsValid());
	}
	UE_NONCOPYABLE(FPin);

public:
	const FVoxelPinValue& GetDefaultValue() const;
	void SetDefaultValue(const FVoxelPinValue& NewDefaultValue);

	FName GetParentName() const
	{
		return ParentName;
	}
	void SetParentName(const FName NewParentName)
	{
		ParentName = NewParentName;
	}

private:
	FVoxelPinValue DefaultValue;
	FName ParentName;

public:
	TPinsView<FPin> GetLinkedTo() const
	{
		return LinkedTo;
	}
	TConstVoxelArrayView<FPin*> GetRawLinkedTo() const { return LinkedTo; }

	void MakeLinkTo(FPin& Other);
	bool IsLinkedTo(FPin& Other) const;
	void TryMakeLinkTo(FPin& Other);

	void BreakLinkTo(FPin& Other);
	void BreakAllLinks();

	void CopyInputPinTo(FPin& Target) const;
	void CopyOutputPinTo(FPin& Target) const;

	void Check(const FGraph& Graph) const;
	TSharedRef<FVoxelMessageToken> CreateMessageToken() const;

private:
	TVoxelInlineArray<FPin*, 4> LinkedTo;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum class ENodeType : uint8
{
	Struct,
	Passthrough
};

class VOXELGRAPH_API FNode
{
	enum class EPrivate {};

	friend class FGraph;

public:
	const ENodeType Type;
	const FVoxelGraphNodeRef NodeRef;

	FNode(
		EPrivate,
		const ENodeType Type,
		const FVoxelGraphNodeRef& NodeRef,
		FGraph& Graph)
		: Type(Type)
		, NodeRef(NodeRef)
		, Graph(Graph)
	{
		ensure(!NodeRef.NodeId.IsNone());
		NameToPin.Reserve(NumInlinePins);
	}
	UE_NONCOPYABLE(FNode);

public:
	template<typename T>
	const T& GetVoxelNode() const
	{
		ensure(Type == ENodeType::Struct);
		return CastStructChecked<T>(*VoxelNode);
	}
	const FVoxelNode& GetVoxelNode() const
	{
		ensure(Type == ENodeType::Struct);
		return *VoxelNode;
	}

	void SetVoxelNode(const TSharedRef<const FVoxelNode>& Node);
	void CopyFrom(const FNode& Src);

	// Queue errors so we don't fail compilation if the node is unused
	void AddError(const FString& Error);
	void ApplyErrors() const;
	void ApplyErrorsTo(FNode& Other) const;
	TSharedRef<FVoxelMessageToken> CreateMessageToken() const;

private:
	TArray<FString> Errors;
	TSharedPtr<const FVoxelNode> VoxelNode;

public:
	FPin& GetInputPin(const int32 Index) { return *InputPins[Index]; }
	FPin& GetOutputPin(const int32 Index) { return *OutputPins[Index]; }

	const FPin& GetInputPin(const int32 Index) const { return *InputPins[Index]; }
	const FPin& GetOutputPin(const int32 Index) const { return *OutputPins[Index]; }

public:
	TPinsView<FPin> GetPins() { return Pins; }
	TPinsView<const FPin> GetPins() const { return Pins; }

	TPinsView<FPin> GetInputPins() { return InputPins; }
	TPinsView<FPin> GetOutputPins() { return OutputPins; }

	TPinsView<const FPin> GetInputPins() const { return InputPins; }
	TPinsView<const FPin> GetOutputPins() const { return OutputPins; }

	TConstVoxelArrayView<FPin*> GetRawPins() const { return Pins; }

public:
	FPin* FindPin(FName Name);

	FPin* FindInput(FName Name);
	FPin* FindOutput(FName Name);

	FPin& FindInputChecked(FName Name);
	FPin& FindOutputChecked(FName Name);

	const FPin* FindPin(const FName Name) const
	{
		return ConstCast(this)->FindPin(Name);
	}

	const FPin* FindInput(const FName Name) const
	{
		return ConstCast(this)->FindInput(Name);
	}
	const FPin* FindOutput(const FName Name) const
	{
		return ConstCast(this)->FindOutput(Name);
	}

	const FPin& FindInputChecked(const FName Name) const
	{
		return ConstCast(this)->FindInputChecked(Name);
	}
	const FPin& FindOutputChecked(const FName Name) const
	{
		return ConstCast(this)->FindOutputChecked(Name);
	}

public:
	FPin& NewInputPin(FName Name, const FVoxelPinType& PinType, const FVoxelPinValue& DefaultValue = {});
	FPin& NewOutputPin(FName Name, const FVoxelPinType& PinType);

	void Check();
	void BreakAllLinks();
	void RemovePin(FPin& Pin);

	void AddMergedNodeRef(const FNode& Other);
	const TVoxelArray<FVoxelGraphNodeRef>& GetMergedNodeRefs() const
	{
		return MergedNodeRefs;
	}

private:
	FGraph& Graph;

	static constexpr int32 NumInlinePins = 8;

	TVoxelInlineArray<FPin*, NumInlinePins> Pins;
	TVoxelInlineMap<FName, FPin*, NumInlinePins> NameToPin;

	TVoxelInlineArray<FPin*, NumInlinePins> InputPins;
	TVoxelInlineArray<FPin*, NumInlinePins> OutputPins;

	TVoxelArray<FVoxelGraphNodeRef> MergedNodeRefs;

	friend FVoxelTemplateNodeUtilities;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXELGRAPH_API FGraph
{
public:
	FGraph();
	UE_NONCOPYABLE(FGraph);

	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelCompilationGraph);

	int64 GetAllocatedSize() const;

public:
	TFunction<void(FNode&)> OnNewNode;

public:
	TNodesView<FNode> GetNodes()
	{
		return Nodes;
	}
	TNodesView<const FNode> GetNodes() const
	{
		return ReinterpretCastRef<TNodesView<const FNode>>(ConstCast(this)->GetNodes());
	}

	TNodesCopy<FNode> GetNodesCopy()
	{
		return Nodes;
	}
	TNodesCopy<const FNode> GetNodesCopy() const
	{
		return Nodes;
	}

	TVoxelArray<FNode*> GetNodesArray()
	{
		return Nodes.Array();
	}
	TVoxelArray<const FNode*> GetNodesArray() const
	{
		return ReinterpretCastVoxelArray<const FNode*>(Nodes.Array());
	}

	FNode& NewNode(ENodeType Type, const FVoxelGraphNodeRef& NodeRef);
	FNode& NewNode(const FVoxelGraphNodeRef& NodeRef)
	{
		return NewNode(ENodeType::Struct, NodeRef);
	}

	FNode& NewGeneratedNode(const ENodeType Type, const FVoxelGraphNodeRef& NodeRef)
	{
		FVoxelGraphNodeRef NewRef = NodeRef;
		NewRef.bGeneratedNode = true;
		return NewNode(Type, NewRef);
	}
	FNode& NewGeneratedNode(const FVoxelGraphNodeRef& NodeRef)
	{
		return NewGeneratedNode(ENodeType::Struct, NodeRef);
	}

	template<typename PredicateType>
	void RemoveNodes(PredicateType Predicate)
	{
		for (auto It = Nodes.CreateIterator(); It; ++It)
		{
			FNode& Node = **It;
			if (!Predicate(Node))
			{
				continue;
			}

			Node.BreakAllLinks();
			It.RemoveCurrent();
		}
	}
	void RemoveNode(FNode& Node)
	{
		Node.BreakAllLinks();
		Nodes.Remove_Ensure(&Node);
	}

	void Check();
	void CloneInto(FGraph& Graph) const;
	TSharedRef<FGraph> Clone() const;

	bool Identical(const FGraph& Other, FString* OutDiff = nullptr) const;

private:
	TVoxelSet<FNode*> Nodes;

	TVoxelChunkedArray<FPin, sizeof(FPin) * 64> PinAllocations;
	TVoxelChunkedArray<FNode, sizeof(FNode) * 16> NodeAllocations;

	FPin& NewPin(
		FName Name,
		const FVoxelPinType& Type,
		const FVoxelPinValue& DefaultValue,
		EPinDirection Direction,
		FNode& Node);

	friend class FPin;
	friend class FNode;
};

}