// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_ValueDebug.h"

#include "VoxelNode.h"
#include "VoxelGraph.h"
#include "Buffer/VoxelBaseBuffers.h"

#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

#if WITH_EDITOR
struct FVoxelNodeValueStats
{
	FString Text;
	FString ToolTip;

	FVoxelNodeValueStats() = default;
	explicit FVoxelNodeValueStats(const FVoxelRuntimePinValue& Value, const bool bFullValue)
	{
		VOXEL_FUNCTION_COUNTER();
		Build(Value, bFullValue);
	}

private:
	void Build(const FVoxelRuntimePinValue& Value, const bool bFullValue)
	{
		FVoxelRuntimePinValue ConstantValue;
		if (Value.IsBuffer())
		{
			const FVoxelBuffer& Buffer = Value.Get<FVoxelBuffer>();
			if (Buffer.Num_Slow() == 0 ||
				!ensure(Buffer.IsConstant_Slow()))
			{
				return;
			}

			ConstantValue = Buffer.GetGenericConstant();
		}
		else
		{
			ConstantValue = Value;
		}

		Text = ConstantValue.ToDebugString(bFullValue, true);
		ToolTip = ConstantValue.ToDebugString(true, true);
	}
};

class FVoxelNodeValueStatManager
	: public FVoxelSingleton
	, public IVoxelNodeStatProvider
{
public:
	struct FQueuedStat
	{
		FVoxelGraphMergedNodeRef NodeRef;
		FName PinRef;
		TSharedPtr<FVoxelNodeValueStats> Stats;
	};
	TQueue<FQueuedStat, EQueueMode::Mpsc> Queue;

	TVoxelMap<TVoxelObjectPtr<const UEdGraphNode>, TVoxelMap<FEdGraphPinReference, TSharedPtr<FVoxelNodeValueStats>>> NodeToPinToStats;

	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override
	{
		GVoxelNodeStatProviders.Add(this);
	}
	virtual void Tick() override
	{
		VOXEL_FUNCTION_COUNTER();

		const auto& AddNode = [this](const FQueuedStat& QueuedStat, UEdGraphNode* GraphNode)
		{
			if (!ensure(GraphNode))
			{
				return;
			}

			UEdGraphPin* Pin = GraphNode->FindPin(QueuedStat.PinRef);
			if (!ensure(Pin))
			{
				return;
			}

			TVoxelMap<FEdGraphPinReference, TSharedPtr<FVoxelNodeValueStats>>& PinToStats = NodeToPinToStats.FindOrAdd(GraphNode);

			PinToStats.FindOrAdd(Pin) = QueuedStat.Stats;
		};

		FQueuedStat QueuedStat;
		while (Queue.Dequeue(QueuedStat))
		{
			AddNode(QueuedStat, QueuedStat.NodeRef.Node.GetGraphNode_EditorOnly());
			for (const FVoxelGraphNodeRef& Node : QueuedStat.NodeRef.MergedNodes)
			{
				AddNode(QueuedStat, Node.GetGraphNode_EditorOnly());
			}
		}
	}
	//~ End FVoxelSingleton Interface

	//~ Begin IVoxelNodeStatProvider Interface
	virtual void ClearStats() override
	{
		NodeToPinToStats.Empty();
	}
	virtual bool IsEnabled(const UVoxelGraph& Graph) override
	{
		return Graph.bEnableNodeValueStats;
	}
	virtual FName GetIconStyleSetName() override
	{
		return STATIC_FNAME("EditorStyle");
	}
	virtual FName GetIconName() override
	{
		return "Kismet.WatchIcon";
	}
	virtual FText GetPinText(const UEdGraphPin& Pin) override
	{
		const TVoxelMap<FEdGraphPinReference, TSharedPtr<FVoxelNodeValueStats>>* PinToStats = NodeToPinToStats.Find(Pin.GetOwningNode());
		if (!PinToStats)
		{
			return {};
		}

		const TSharedPtr<FVoxelNodeValueStats> Stats = PinToStats->FindRef(&Pin);
		if (!Stats ||
			Stats->Text.IsEmpty())
		{
			return {};
		}

		const FString PinName = Pin.GetDisplayName().ToString().TrimStartAndEnd();
		return FText::FromString(PinName + ":\n" + Stats->Text);
	}
	virtual FText GetPinToolTip(const UEdGraphPin& Pin) override
	{
		const TVoxelMap<FEdGraphPinReference, TSharedPtr<FVoxelNodeValueStats>>* PinToStats = NodeToPinToStats.Find(Pin.GetOwningNode());
		if (!PinToStats)
		{
			return {};
		}

		const TSharedPtr<FVoxelNodeValueStats> Stats = PinToStats->FindRef(&Pin);
		if (!Stats ||
			Stats->ToolTip.IsEmpty())
		{
			return {};
		}

		return FText::FromString(Stats->ToolTip);
	}
	//~ End IVoxelNodeStatProvider Interface

	void EnqueueStats(const IVoxelNodeInterface& InNode, const FName PinName, const TSharedRef<FVoxelNodeValueStats>& Stats)
	{
		Queue.Enqueue(
		{
			InNode.GetNodeRef(),
			PinName,
			Stats
		});
	}
};
FVoxelNodeValueStatManager* GVoxelNodeValueStatManager = new FVoxelNodeValueStatManager();
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_ValueDebug::Compute(const FVoxelGraphQuery Query) const
{
	ensure(WITH_EDITOR);

	const FValue Value = InPin.Get(Query);

	VOXEL_GRAPH_WAIT(Value)
	{
#if WITH_EDITOR
		if (const FVoxelGraphParameters::FDebugValuePosition* Parameter = Query->FindParameter<FVoxelGraphParameters::FDebugValuePosition>())
		{
			GVoxelNodeValueStatManager->EnqueueStats(*this, RefPin, MakeShared<FVoxelNodeValueStats>(Value, Parameter->bFullValue));
		}
#endif

		OutPin.Set(Query, Value);
	};
}