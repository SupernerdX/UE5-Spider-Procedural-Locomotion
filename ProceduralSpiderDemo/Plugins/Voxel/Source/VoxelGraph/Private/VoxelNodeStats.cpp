// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNodeStats.h"
#include "VoxelNode.h"
#include "VoxelCompilationGraph.h"
#include "EdGraph/EdGraphNode.h"

#if WITH_EDITOR
bool GVoxelEnableNodeStats = false;
TArray<IVoxelNodeStatProvider*> GVoxelNodeStatProviders;
#endif

#if WITH_EDITOR
class FVoxelNodeStatManager
	: public FVoxelSingleton
	, public IVoxelNodeStatProvider
{
public:
	struct FQueuedStat
	{
		FVoxelGraphMergedNodeRef NodeRef;
		double Duration = 0.;
		int64 Count = 0;
	};
	TQueue<FQueuedStat, EQueueMode::Mpsc> Queue;

	struct FStats
	{
		double Time = 0.;
		int64 NumElements = 0;
		bool bMerged = false;
	};
	TVoxelMap<TVoxelObjectPtr<const UEdGraphNode>, FStats> NodeToStats;

	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override
	{
		GVoxelNodeStatProviders.Add(this);
	}
	virtual void Tick() override
	{
		VOXEL_FUNCTION_COUNTER();

		const auto AddNode = [this](const FQueuedStat& QueuedStat, UEdGraphNode* GraphNode, const bool bMerged) -> FStats*
		{
			if (!ensure(GraphNode))
			{
				return nullptr;
			}

			FStats& Stats = NodeToStats.FindOrAdd(GraphNode);
			Stats.Time += QueuedStat.Duration;
			Stats.NumElements += QueuedStat.Count;
			Stats.bMerged = bMerged;
			return &Stats;
		};

		FQueuedStat QueuedStat;
		while (Queue.Dequeue(QueuedStat))
		{
			AddNode(QueuedStat, QueuedStat.NodeRef.Node.GetGraphNode_EditorOnly(), false);
			for (const FVoxelGraphNodeRef& Node : QueuedStat.NodeRef.MergedNodes)
			{
				AddNode(QueuedStat, Node.GetGraphNode_EditorOnly(), true);
			}
		}
	}
	//~ End FVoxelSingleton Interface

	//~ Begin IVoxelNodeStatProvider Interface
	virtual void ClearStats() override
	{
		NodeToStats.Empty();
	}
	virtual bool IsEnabled(const UVoxelGraph& Graph) override
	{
		return GVoxelEnableNodeStats;
	}
	virtual FName GetIconStyleSetName() override
	{
		return STATIC_FNAME("EditorStyle");
	}
	virtual FName GetIconName() override
	{
		return STATIC_FNAME("GraphEditor.Timeline_16x");
	}
	virtual FText GetNodeToolTip(const UEdGraphNode& Node) override
	{
		const FStats* Stats = NodeToStats.Find(&Node);
		if (!Stats)
		{
			return {};
		}

		FText MergedString;
		if (Stats->bMerged)
		{
			MergedString = INVTEXT("\nWas merged with another node so performance cost is shared");
		}

		return FText::Format(INVTEXT("Computed {0} elements in {1}, for an average of {2} per element{3}"),
			FVoxelUtilities::NumberToText(Stats->NumElements),
			FVoxelUtilities::SecondsToText(Stats->Time),
			FVoxelUtilities::SecondsToText(Stats->Time / Stats->NumElements),
			MergedString);
	}
	virtual FText GetNodeText(const UEdGraphNode& Node) override
	{
		const FStats* Stats = NodeToStats.Find(&Node);
		if (!Stats)
		{
			return {};
		}

		FText MergedString;
		if (Stats->bMerged)
		{
			MergedString = INVTEXT(" [Merged]");
		}

		return FText::Format(INVTEXT("{0} x {1} = {2}{3}"),
			FVoxelUtilities::SecondsToText(Stats->Time / Stats->NumElements),
			FVoxelUtilities::NumberToText(Stats->NumElements),
			FVoxelUtilities::SecondsToText(Stats->Time),
			MergedString);
	}
	//~ End IVoxelNodeStatProvider Interface
};
FVoxelNodeStatManager* GVoxelNodeStatManager = new FVoxelNodeStatManager();
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void FVoxelNodeStatScope::RecordStats(const double Duration) const
{
	GVoxelNodeStatManager->Queue.Enqueue(
	{
		Node->GetNodeRef(),
		Duration,
		Count
	});
}
#endif