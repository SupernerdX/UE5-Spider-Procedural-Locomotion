// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterCacheNode.h"
#include "Scatter/VoxelScatterMeshNode.h"
#include "Scatter/VoxelScatterCacheManager.h"
#include "Scatter/VoxelScatterFunctionLibrary.h"
#include "VoxelQuery.h"
#include "VoxelGraphPositionParameter.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Graphs/VoxelStampGraphParameters.h"

#if WITH_EDITOR
#include "EdGraph/EdGraphNode.h"
#endif

#if WITH_EDITOR
class FVoxelNodeCacheStatManager
	: public FVoxelSingleton
	, public IVoxelNodeStatProvider
{
public:
	struct FQueuedStat
	{
		FVoxelGraphMergedNodeRef NodeRef;
		FGuid NodeGuid;
		TWeakPtr<FVoxelScatterCacheManager> WeakCacheManager;
		int32 NumFound = 0;
		int32 NumRequested = 0;
	};
	TQueue<FQueuedStat, EQueueMode::Mpsc> Queue;

	struct FStats
	{
		int64 AllocatedSize = 0;
		TVoxelSet<TWeakPtr<FVoxelScatterCacheManager>> WeakCacheManagers;
		int64 TotalRequests = 0;
		int64 CacheHits = 0;
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
			Stats.WeakCacheManagers.Add(QueuedStat.WeakCacheManager);
			Stats.TotalRequests += QueuedStat.NumRequested + QueuedStat.NumFound;
			Stats.CacheHits += QueuedStat.NumFound;

			Stats.AllocatedSize = 0;
			for (auto It = Stats.WeakCacheManagers.CreateIterator(); It; ++It)
			{
				const TSharedPtr<FVoxelScatterCacheManager> CacheManager = It->Pin();
				if (!CacheManager)
				{
					It.RemoveCurrent();
					continue;
				}

				Stats.AllocatedSize += CacheManager->GetAllocatedSize(QueuedStat.NodeGuid);
			}
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

		return FText::Format(INVTEXT("{0} data cached for this node.\n{1}% cache hits {2} of {3}"),
			FVoxelUtilities::BytesToText(Stats->AllocatedSize),
			FText::AsNumber((double(Stats->CacheHits) / Stats->TotalRequests) * 100.f, &FormattingOptions),
			FVoxelUtilities::NumberToText(Stats->CacheHits),
			FVoxelUtilities::NumberToText(Stats->TotalRequests));
	}
	virtual FText GetNodeText(const UEdGraphNode& Node) override
	{
		static FNumberFormattingOptions FormattingOptions = FNumberFormattingOptions().SetMinimumFractionalDigits(2).SetMaximumFractionalDigits(2);

		const FStats* Stats = NodeToStats.Find(&Node);
		if (!Stats)
		{
			return {};
		}

		return FText::Format(INVTEXT("{0} cached | {1}% hits"),
			FVoxelUtilities::BytesToText(Stats->AllocatedSize),
			FText::AsNumber((double(Stats->CacheHits) / Stats->TotalRequests) * 100.f, &FormattingOptions));
	}
	//~ End IVoxelNodeStatProvider Interface

	void Enqueue(
		const IVoxelNodeInterface& Node,
		const FGuid NodeGuid,
		const TWeakPtr<FVoxelScatterCacheManager>& CacheManager,
		const int32 NumFound,
		const int32 NumRequested)
	{
		if (!GVoxelEnableNodeStats)
		{
			return;
		}

		Queue.Enqueue(
		{
			Node.GetNodeRef(),
			NodeGuid,
			CacheManager,
			NumFound,
			NumRequested
		});
	}
};
FVoxelNodeCacheStatManager* GVoxelNodeCacheStatManager = new FVoxelNodeCacheStatManager();
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_CachePoints::Compute(FVoxelGraphQuery Query) const
{
	const TValue<float> CacheChunkSize = ChunkSizePin.Get(Query);
	const TValue<float> CacheSize = CacheSizePin.Get(Query);
	const TValue<float> BoundsExtension = BoundsExtensionPin.Get(Query);
	VOXEL_GRAPH_WAIT(CacheChunkSize, CacheSize, BoundsExtension)
	{
		const FVoxelGraphParameters::FScatterCacheManager* CacheManagerParameter = Query->FindParameter<FVoxelGraphParameters::FScatterCacheManager>();
		const FVoxelGraphParameters::FPointSetChunkBounds* BoundsParameter = Query->FindParameter<FVoxelGraphParameters::FPointSetChunkBounds>();
		const FVoxelGraphParameters::FQuery* QueryParameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
		if (!CacheManagerParameter ||
			!BoundsParameter ||
			!QueryParameter)
		{
			VOXEL_MESSAGE(Error, "{0}: Cannot use cache here", this);
			return;
		}

		const FVoxelBox Bounds = BoundsParameter->GetBounds();

		const TVoxelArray<FIntVector> ChunkKeys = INLINE_LAMBDA
		{
			const FVoxelBox ExtendedBounds = Bounds.Extend(BoundsExtension);

			const float HalfChunkSize = CacheChunkSize * 100.f * 0.5f;

			const FIntVector MinChunkKey = FVoxelUtilities::FloorToInt((ExtendedBounds.Min - HalfChunkSize) / (CacheChunkSize * 100.f)) + 1;
			const FIntVector MaxChunkKey = FVoxelUtilities::CeilToInt((ExtendedBounds.Max + HalfChunkSize) / (CacheChunkSize * 100.f)) - 1;
			TVoxelArray<FIntVector> Result;
			for (int32 X = MinChunkKey.X; X <= MaxChunkKey.X; ++X)
			{
				for (int32 Y = MinChunkKey.Y; Y <= MaxChunkKey.Y; ++Y)
				{
					for (int32 Z = MinChunkKey.Z; Z <= MaxChunkKey.Z; ++Z)
					{
						Result.Add(FIntVector(X, Y, Z));
					}
				}
			}
			return MoveTemp(Result);
		};

		TVoxelSet<FName> AttributesToKeep = TVoxelSet<FName>(AttributesToCache);
		AttributesToKeep.Add(FVoxelPointAttributes::Id);
		AttributesToKeep.Add(FVoxelPointAttributes::Position);

		TVoxelArray<TSharedRef<const FVoxelPointSet>> Chunks;
		Chunks.Reserve(ChunkKeys.Num());

#if VOXEL_ENGINE_VERSION >= 506
		const uint32 NodeKey = HashCombine(
			GetTypeHash(CacheChunkSize),
			GetTypeHash(CacheSize),
			GetTypeHash(BoundsExtension));
#else
		const uint32 NodeKey = HashCombine(
			GetTypeHash(CacheChunkSize),
			HashCombine(
				GetTypeHash(CacheSize),
				GetTypeHash(BoundsExtension)));
#endif
		int32 CachedChunks = 0;
		for (const FIntVector& ChunkKey : ChunkKeys)
		{
			const FVoxelBox ChunkBounds = 
				FVoxelBox(FVector(ChunkKey) * CacheChunkSize * 100.f)
				.Extend(CacheChunkSize * 100.f * 0.5f);

			const uint32 Key = HashCombine(NodeKey, GetTypeHash(ChunkKey));

			bool bWasCached = false;
			Chunks.Add(CacheManagerParameter->CacheManager->GetCachedChunk(
				Query->Context.Environment,
				Query->CompiledTerminalGraph,
				Query->Context.DependencyCollector,
				QueryParameter->Query.Layers,
				QueryParameter->Query.SurfaceTypeTable,
				CacheManagerParameter->WeakActor,
				BoundsExtension,
				AttributesToKeep,
				ChunkBounds,
				NodeGuid,
				Key,
				InPin,
				bWasCached));

			if (bWasCached)
			{
				CachedChunks++;
			}
		}

#if WITH_EDITOR
		GVoxelNodeCacheStatManager->Enqueue(
			*this,
			NodeGuid,
			CacheManagerParameter->CacheManager,
			CachedChunks,
			ChunkKeys.Num() - CachedChunks);		
#endif

		const TSharedRef<const FVoxelPointSet> Result = FVoxelPointSet::Merge(Chunks);
		if (Result->Num() == 0)
		{
			OutPin.Set(Query, MakeShared<FVoxelPointSet>());
			return;
		}

		const FVoxelDoubleVectorBuffer* PositionsBuffer = Result->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
		if (!ensure(PositionsBuffer))
		{
			OutPin.Set(Query, MakeShared<FVoxelPointSet>());
			return;
		}

		FVoxelBoolBuffer IsNeighborBuffer;
		IsNeighborBuffer.Allocate(Result->Num());

		TVoxelArray<int32> IndicesToKeep;
		IndicesToKeep.Reserve(Result->Num());

		int32 WriteIndex = 0;
		for (int32 Index = 0; Index < Result->Num(); Index++)
		{
			if (!Bounds.Contains((*PositionsBuffer)[Index]))
			{
				continue;
			}

			IndicesToKeep.Add(Index);
			IsNeighborBuffer.Set(WriteIndex++, !BoundsParameter->GetInitialBounds().Contains((*PositionsBuffer)[Index]));
		}

		const TSharedRef<FVoxelPointSet> NewPointSet = Result->Gather(IndicesToKeep);
		if (BoundsParameter->HasExtendedBounds())
		{
			IsNeighborBuffer.ShrinkTo(IndicesToKeep.Num());
			NewPointSet->Add(FVoxelPointAttributes::IsNeighbor, MoveTemp(IsNeighborBuffer));
		}

		OutPin.Set(Query, NewPointSet);
	};
}