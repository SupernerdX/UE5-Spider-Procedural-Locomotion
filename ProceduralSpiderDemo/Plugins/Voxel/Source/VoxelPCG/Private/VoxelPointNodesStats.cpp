// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPointNodesStats.h"
#include "VoxelNodeStats.h"
#include "VoxelGraphNodeRef.h"
#include "EdGraph/EdGraphNode.h"

#if WITH_EDITOR
class FVoxelNodePointFilterStatManager
	: public FVoxelSingleton
	, public IVoxelNodeStatProvider
{
public:
	struct FQueuedStat
	{
		FVoxelGraphMergedNodeRef NodeRef;
		int64 Count = 0;
		int64 FilteredCount = 0;
	};
	TQueue<FQueuedStat, EQueueMode::Mpsc> Queue;

	struct FStats
	{
		int64 NumPoints = 0;
		int64 NumFilteredPoints = 0;
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

		const auto AddNode = [this](const FQueuedStat& QueuedStat, UEdGraphNode* GraphNode)
		{
			if (!ensure(GraphNode))
			{
				return;
			}

			FStats& Stats = NodeToStats.FindOrAdd(GraphNode);
			Stats.NumPoints += QueuedStat.Count;
			Stats.NumFilteredPoints += QueuedStat.FilteredCount;
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
		return "FoliageEditMode.Filter";
	}
	virtual FText GetNodeToolTip(const UEdGraphNode& Node) override
	{
		static FNumberFormattingOptions FormattingOptions = FNumberFormattingOptions().SetMinimumFractionalDigits(2).SetMaximumFractionalDigits(2);

		const FStats* Stats = NodeToStats.Find(&Node);
		if (!Stats)
		{
			return {};
		}

		return FText::Format(INVTEXT("Filtered {0} of {1} elements, {2}% filtered"),
			FVoxelUtilities::NumberToText(Stats->NumFilteredPoints),
			FVoxelUtilities::NumberToText(Stats->NumPoints),
			FText::AsNumber((double(Stats->NumFilteredPoints) / Stats->NumPoints) * 100.f, &FormattingOptions));
	}
	virtual FText GetNodeText(const UEdGraphNode& Node) override
	{
		static FNumberFormattingOptions FormattingOptions = FNumberFormattingOptions().SetMinimumFractionalDigits(2).SetMaximumFractionalDigits(2);

		const FStats* Stats = NodeToStats.Find(&Node);
		if (!Stats)
		{
			return {};
		}

		return FText::Format(INVTEXT("{0} - {1} = {2} | {3}%"),
			FVoxelUtilities::NumberToText(Stats->NumPoints),
			FVoxelUtilities::NumberToText(Stats->NumFilteredPoints),
			FVoxelUtilities::NumberToText(Stats->NumPoints - Stats->NumFilteredPoints),
			FText::AsNumber((double(Stats->NumPoints - Stats->NumFilteredPoints) / Stats->NumPoints) * 100.f, &FormattingOptions));
	}
	//~ End IVoxelNodeStatProvider Interface
};
FVoxelNodePointFilterStatManager* GVoxelNodePointFilterStatManager = new FVoxelNodePointFilterStatManager();
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void FVoxelPointFilterStats::RecordNodeStats(const IVoxelNodeInterface& Node, const int64 Count, const int64 RemainingCount)
{
	if (!GVoxelEnableNodeStats)
	{
		return;
	}

	GVoxelNodePointFilterStatManager->Queue.Enqueue(
	{
		Node.GetNodeRef(),
		Count,
		Count - RemainingCount
	});
}
#endif