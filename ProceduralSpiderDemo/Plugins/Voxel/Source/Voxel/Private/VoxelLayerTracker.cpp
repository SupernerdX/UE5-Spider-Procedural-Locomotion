// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelLayerTracker.h"
#include "VoxelHeightLayer.h"
#include "VoxelVolumeLayer.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"
#include "VoxelLayers.h"
#include "VoxelLayerStack.h"
#include "VoxelDependency.h"
#include "VoxelStampManager.h"
#include "VoxelStampChangeProcessor.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelStampShowTree, false,
	"voxel.stamp.ShowTree",
	"Show stamp trees");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelStampShowAllBounds, false,
	"voxel.ShowStampBounds",
	"Show all stamp bounds");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelStampShowSelectedBounds, false,
	"voxel.stamp.ShowSelectedBounds",
	"Show the bounds of the selected stamp");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelLayerTrackerStamps> FVoxelLayerTrackerStamps::Create(
	const FVoxelLayerTracker& Tracker,
	const float StackMaxDistance,
	UVoxelLayer& Layer)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelLayerTrackerStamps> Result = MakeShareable(new FVoxelLayerTrackerStamps(Tracker, StackMaxDistance, Layer));
	Result->Initialize();
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelLayerTrackerStamps::UpdateStampsIfNeeded(const FVoxelLayerTrackerStamps* PreviousStamps)
{
	VOXEL_FUNCTION_COUNTER();

	if (Timestamp == LayerTimestamp->Get())
	{
		return false;
	}
	Timestamp = LayerTimestamp->Get();

	const TVoxelChunkedSparseArray<TSharedRef<const FVoxelStampRuntime>>& Stamps = LayerManager->GetStamps();

	VOXEL_SCOPE_COUNTER_FORMAT("Stamps.Num = %d", Stamps.Num());

	IntersectBounds = FVoxelBox::Infinite;

	if (!bIs2D)
	{
		VOXEL_SCOPE_COUNTER("IntersectBounds");

		for (const TSharedRef<const FVoxelStampRuntime>& Stamp : Stamps)
		{
			const FVoxelVolumeStamp& VolumeStamp = CastStructChecked<FVoxelVolumeStamp>(Stamp->GetStamp());
			if (VolumeStamp.BlendMode != EVoxelVolumeBlendMode::Intersect)
			{
				continue;
			}

			const FVoxelVolumeTransform StampToQuery = FVoxelVolumeTransform::Create(
				Stamp->GetLocalToWorld(),
				WorldToQuery,
				Stamp->GetLocalBounds(),
				VolumeStamp.BoundsExtensionMultiplier,
				VolumeStamp.MaximumBoundsExtension);

			IntersectBounds = IntersectBounds.IntersectWith(StampToQuery.GetBounds(Stamp->GetLocalBounds()));
		}
	}

	ShouldSplit.Reset();
	ShouldSplit.SetNumZeroed(NumLODs + 1);

	for (const TSharedRef<const FVoxelStampRuntime>& Stamp : Stamps)
	{
		ShouldSplit[Stamp->GetLODRange().Min] = true;
		ShouldSplit[Stamp->GetLODRange().Max + 1] = true;
	}

	if (PreviousStamps)
	{
		for (int32 Index = 0; Index <= NumLODs; Index++)
		{
			ShouldSplit[Index] |= PreviousStamps->ShouldSplit[Index];
		}
	}

	LODToTree.Reset();

	TSharedPtr<FVoxelFutureStampTree> Tree;
	for (int32 LOD = 0; LOD < NumLODs; LOD++)
	{
		if (!Tree ||
			ShouldSplit[LOD])
		{
			Tree = MakeShared<FVoxelFutureStampTree>();

			TVoxelArray<TSharedRef<const FVoxelStampRuntime>> LocalStamps;
			LocalStamps.Reserve(Stamps.Num());

			for (const TSharedRef<const FVoxelStampRuntime>& Stamp : Stamps)
			{
				if (Stamp->ShouldComputeLOD(LOD))
				{
					LocalStamps.Add_EnsureNoGrow(Stamp);
				}
			}

			if (PreviousStamps)
			{
				PreviousStamps->LODToTree[LOD]->Then(MakeStrongPtrLambda(this, [
					this,
					LOD,
					Tree,
					LocalStampsRef = MakeSharedCopy(MoveTemp(LocalStamps))](const TSharedRef<FVoxelStampTree>& PreviousTree)
				{
					Tree->Set(CreateTree(
						LOD,
						MoveTemp(*LocalStampsRef),
						PreviousTree));
				}));
			}
			else
			{
				GVoxelStampTreeManager->AddTask(MakeStrongPtrLambda(this, [
					this,
					LOD,
					Tree,
					LocalStampsRef = MakeSharedCopy(MoveTemp(LocalStamps))]
				{
					Tree->Set(CreateTree(
						LOD,
						MoveTemp(*LocalStampsRef),
						nullptr));
				}));
			}
		}

		LODToTree.Add(Tree.ToSharedRef());
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelLayerTrackerStamps::FVoxelLayerTrackerStamps(
	const FVoxelLayerTracker& Tracker,
	const float StackMaxDistance,
	UVoxelLayer& Layer)
	: bIs2D(Layer.IsA<UVoxelHeightLayer>())
	, StackMaxDistance(StackMaxDistance)
	, Layer(Layer)
	, LayerManager(FVoxelStampManager::Get(Tracker.World)->FindOrAddLayer(Layer))
	, TrackerTimestamp(Tracker.Timestamp)
	, QueryToWorld(Tracker.QueryToWorld)
	, WorldToQuery(FVoxelUtilities::MakeTransformSafe(Tracker.QueryToWorld.ToInverseMatrixWithScale()))
	, LODToDependency(SharedRef_Null)
{
	VOXEL_FUNCTION_COUNTER();

	// Voxel World shouldn't be scaled
	ensure(QueryToWorld.GetScale3D().Equals(FVector::OneVector));

	for (int32 LOD = 0; LOD < NumLODs; LOD++)
	{
		if (bIs2D)
		{
			ConstCast(LODToDependency[LOD]) = FVoxelDependency3D::Create(FString::Printf(TEXT("HeightStamps %s LOD=%d"), *Layer.GetName(), LOD));
		}
		else
		{
			ConstCast(LODToDependency[LOD]) = FVoxelDependency3D::Create(FString::Printf(TEXT("VolumeStamps %s LOD=%d"), *Layer.GetName(), LOD));
		}
	}
}

void FVoxelLayerTrackerStamps::Initialize()
{
	LayerManager->OnStampChanged.Add(MakeWeakPtrDelegate(this, [this](const TVoxelChunkedArray<FVoxelStampChange>& StampChanges)
	{
		LayerTimestamp->Increment();
		TrackerTimestamp->Increment();

		FVoxelStampChangeProcessor Processor(
			bIs2D,
			StackMaxDistance,
			QueryToWorld,
			WorldToQuery,
			LODToDependency);

		Processor.Process(StampChanges);
	}));
}

TSharedRef<FVoxelStampTree> FVoxelLayerTrackerStamps::CreateTree(
	const int32 LOD,
	TVoxelArray<TSharedRef<const FVoxelStampRuntime>> Stamps,
	const TSharedPtr<FVoxelStampTree>& PreviousTree) const
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_COUNTER_FORMAT("CreateTree Layer=%s LOD=%d", *Layer.GetName(), LOD);

	// TODO MOVE OUT

	struct FSortableElement
	{
		const FVoxelStampRuntime* Stamp = nullptr;
		int32 Priority = 0;
		int32 SubPriority = 0;
		double LocationZ;
		double LocationX;
		double LocationY;
		uint64 PropertyHash = 0;

		FORCEINLINE bool operator<(const FSortableElement& Other) const
		{
			if (Priority != Other.Priority)
			{
				return Priority < Other.Priority;
			}
			if (SubPriority != Other.SubPriority)
			{
				return SubPriority < Other.SubPriority;
			}

			if (LocationZ != Other.LocationZ)
			{
				return LocationZ < Other.LocationZ;
			}
			if (LocationX != Other.LocationX)
			{
				return LocationX < Other.LocationX;
			}
			if (LocationY != Other.LocationY)
			{
				return LocationY < Other.LocationY;
			}

			if (PropertyHash != Other.PropertyHash)
			{
				return PropertyHash < Other.PropertyHash;
			}

			// Stamps are fully identical, we have a priority collision
			//ensureVoxelSlow(false);

			return false;
		}
	};

	TVoxelArray<FSortableElement> SortableElements;
	{
		VOXEL_SCOPE_COUNTER_NUM("Build SortableElements", Stamps.Num());

		FVoxelUtilities::SetNumFast(SortableElements, Stamps.Num());
		FVoxelCounter32 Index;

		Voxel::ParallelFor(Stamps, [&](const TSharedRef<const FVoxelStampRuntime>& Stamp)
		{
			checkVoxelSlow(Stamp->ShouldComputeLOD(LOD));

			const FVector WorldPosition = Stamp->GetLocalToWorld().GetLocation();
			const FVector LocalPosition = QueryToWorld.InverseTransformPosition(WorldPosition);

			FSortableElement& SortableElement = SortableElements[Index.Increment_ReturnOld()];
			SortableElement.Stamp = &Stamp.Get();
			SortableElement.Priority = Stamp->GetStamp().Priority;
			SortableElement.SubPriority = INLINE_LAMBDA
			{
				if (bIs2D)
				{
					switch (CastStructChecked<FVoxelHeightStamp>(Stamp->GetStamp()).BlendMode)
					{
					default: ensure(false);
					case EVoxelHeightBlendMode::Max: return 1;
					case EVoxelHeightBlendMode::Min: return 2;
					case EVoxelHeightBlendMode::Override: return 3;
					}
				}
				else
				{
					switch (CastStructChecked<FVoxelVolumeStamp>(Stamp->GetStamp()).BlendMode)
					{
					default: ensure(false);
					case EVoxelVolumeBlendMode::Intersect: return 1;
					case EVoxelVolumeBlendMode::Additive: return 2;
					case EVoxelVolumeBlendMode::Subtractive: return 3;
					case EVoxelVolumeBlendMode::Override: return 4;
					}
				}
			};
			SortableElement.LocationZ = LocalPosition.Z;
			SortableElement.LocationX = LocalPosition.X;
			SortableElement.LocationY = LocalPosition.Y;
			SortableElement.PropertyHash = Stamp->GetPropertyHash();
		});

		check(Index.Get() <= SortableElements.Num());
		SortableElements.SetNum(Index.Get(), EAllowShrinking::No);
	}

	{
		VOXEL_SCOPE_COUNTER_NUM("Sort", SortableElements.Num());
		SortableElements.Sort();
	}

	FVoxelCounter32 NumRelativeHeightRanges;
	TVoxelArray<FVoxelStampTreeElement> Elements;
	{
		VOXEL_SCOPE_COUNTER_NUM("Build Elements", SortableElements.Num());

		TVoxelArray<TVoxelInlineArray<FVoxelStampTreeElement, 1>> ElementArrays;
		FVoxelUtilities::SetNum(ElementArrays, SortableElements.Num());

		Voxel::ParallelFor(SortableElements, [&](const FSortableElement& SortableElement, const int32 Index)
		{
			const FVoxelStampRuntime& Stamp = *SortableElement.Stamp;
			const TVoxelInlineArray<FVoxelBox, 1> Children = Stamp.GetChildren();

			TVoxelInlineArray<FVoxelStampTreeElement, 1>& ElementArray = ElementArrays[Index];
			ElementArray.Reserve(Children.Num());

			for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++)
			{
				const FVoxelBox Child = Children[ChildIndex];

				FVoxelStampTreeElement& Element = ElementArray.Emplace_GetRef_EnsureNoGrow();
				Element.Stamp = &Stamp;
				Element.ChildIndex = ChildIndex;

				if (bIs2D)
				{
					const FVoxelHeightStamp& HeightStamp = Stamp.GetStamp().AsChecked<FVoxelHeightStamp>();
					Element.HeightStampToQuery = FVoxelHeightTransform::Create(
						Stamp.GetLocalToWorld(),
						WorldToQuery,
						Child,
						HeightStamp.HeightPaddingMultiplier);

					Element.Bounds = Element.HeightStampToQuery.GetBounds(Child);

					if (CastStructChecked<FVoxelHeightStampRuntime>(Stamp).HasRelativeHeightRange())
					{
						NumRelativeHeightRanges.Increment();
					}

					checkVoxelSlow(Element.Bounds.Min.Z <= Element.HeightStampToQuery.MinHeight);
					checkVoxelSlow(Element.Bounds.Max.Z >= Element.HeightStampToQuery.MaxHeight);
				}
				else
				{
					const FVoxelVolumeStamp& VolumeStamp = Stamp.GetStamp().AsChecked<FVoxelVolumeStamp>();
					Element.VolumeStampToQuery = FVoxelVolumeTransform::Create(
						Stamp.GetLocalToWorld(),
						WorldToQuery,
						Child,
						VolumeStamp.BoundsExtensionMultiplier,
						VolumeStamp.MaximumBoundsExtension);

					Element.Bounds = Element.VolumeStampToQuery.GetBounds(Child);

					if (CastStructChecked<FVoxelVolumeStampRuntime>(Stamp).GetBlendMode() == EVoxelVolumeBlendMode::Intersect)
					{
						// No MaxDistance for intersect stamps, otherwise we get an incorrect border
						Element.VolumeStampToQuery.MaxDistance = FVoxelUtilities::FloatInf();
					}
				}
			}
		});

		VOXEL_SCOPE_COUNTER("Flatten arrays");

		int32 NumElements = 0;
		for (const TVoxelInlineArray<FVoxelStampTreeElement, 1>& ElementArray : ElementArrays)
		{
			NumElements += ElementArray.Num();
		}

		Elements.Reserve(NumElements);

		for (const TVoxelInlineArray<FVoxelStampTreeElement, 1>& ElementArray : ElementArrays)
		{
			for (const FVoxelStampTreeElement& Element : ElementArray)
			{
				Elements.Add_EnsureNoGrow(Element);
			}
		}
	}

	if (NumRelativeHeightRanges.Get() > 0)
	{
		VOXEL_SCOPE_COUNTER("Relative height ranges");

		FVoxelAABBTree::FElementArray TreeElements;
		TreeElements.SetNum(Elements.Num());

		TVoxelArray<int32> RelativeElements;
		RelativeElements.Reserve(NumRelativeHeightRanges.Get());

		for (int32 Index = 0; Index < Elements.Num(); Index++)
		{
			const FVoxelStampTreeElement& Element = Elements[Index];

			TreeElements.Set(
				Index,
				Element.Bounds,
				Index);

			if (CastStructChecked<FVoxelHeightStampRuntime>(*Element.Stamp).HasRelativeHeightRange())
			{
				RelativeElements.Add_EnsureNoGrow(Index);
			}
		}

		checkVoxelSlow(RelativeElements.Num() == NumRelativeHeightRanges.Get());

		FVoxelAABBTree Tree;
		Tree.Initialize(MoveTemp(TreeElements));

		// Process high priority stamps last

		for (const int32 Index : RelativeElements)
		{
			FVoxelStampTreeElement& Element = Elements[Index];

			FVoxelInterval Range = FVoxelInterval::InvertedInfinite;

			Tree.TraverseBounds(
				FVoxelFastBox(FVoxelBox2D(Element.Bounds).ToBox3D_Infinite()),
				[&](const int32 OtherIndex)
				{
					if (OtherIndex >= Index)
					{
						// After us, do not include
						return;
					}

					const FVoxelStampTreeElement& OtherElement = Elements[OtherIndex];
					if (OtherElement.Stamp == Element.Stamp)
					{
						// Same stamp, skip
						return;
					}

					if (!EnumHasAllFlags(OtherElement.Stamp->GetStamp().Behavior, EVoxelStampBehavior::AffectShape))
					{
						// Doesn't affect shape, skip
						return;
					}

					Range += Elements[OtherIndex].Bounds.GetZ();
				});

			// Also check previous layers
			for (TSharedPtr<FVoxelStampTree> It = PreviousTree; It; It = It->PreviousTree)
			{
				It->ForeachElement_Unsorted(
					FVoxelDependencyCollector::Null,
					FVoxelBox2D(Element.Bounds).ToBox3D_Infinite(),
					EVoxelStampBehavior::AffectShape,
					[&](const FVoxelStampTreeElement& PreviousElement)
					{
						Range += PreviousElement.Bounds.GetZ();
					});
			}

			if (!Range.IsValid())
			{
				continue;
			}

			// No need to rebuild the tree after this as we do ToBox3D_Infinite

			FVoxelHeightTransform StampToQuery = Element.HeightStampToQuery;
			// Too messy, ignore when computing bounds
			StampToQuery.Rotation3D = FVector2f(ForceInit);

			FVoxelBox LocalBounds = Element.Stamp->GetChildren()[Element.ChildIndex];
			LocalBounds.Min.Z += StampToQuery.InverseTransformHeight(Range.Min, FVector2D(ForceInit));
			LocalBounds.Max.Z += StampToQuery.InverseTransformHeight(Range.Max, FVector2D(ForceInit));

			Element.Bounds = StampToQuery.GetBounds(LocalBounds);
			Element.HeightStampToQuery.MinHeight = FVoxelUtilities::DoubleToFloat_Higher(Element.Bounds.Min.Z - Element.HeightStampToQuery.HeightPadding);
			Element.HeightStampToQuery.MaxHeight = FVoxelUtilities::DoubleToFloat_Lower(Element.Bounds.Max.Z + Element.HeightStampToQuery.HeightPadding);
		}
	}

	FVoxelAABBTree::FElementArray TreeElements;
	if (bIs2D)
	{
		VOXEL_SCOPE_COUNTER_NUM("Build TreeElements", Elements.Num());

		TreeElements.SetNum(Elements.Num());

		for (int32 Index = 0; Index < Elements.Num(); Index++)
		{
			TreeElements.Set(
				Index,
				Elements[Index].Bounds,
				Index);
		}
	}
	else
	{
		VOXEL_SCOPE_COUNTER_NUM("Build TreeElements", Elements.Num());

		TreeElements.Reserve(Elements.Num());

		FVoxelBox CurrentIntersectBounds = FVoxelBox::Infinite;

		// Combine intersect stamps to support spline intersections
		const FVoxelStampRuntime* LastStamp = nullptr;
		FVoxelBox LastStampBounds;

		for (int32 Index = Elements.Num() - 1; Index >= 0; Index--)
		{
			FVoxelStampTreeElement& Element = Elements[Index];

			if (Element.Stamp == LastStamp)
			{
				LastStampBounds += Element.Bounds;
			}
			else
			{
				if (LastStamp &&
					CastStructChecked<FVoxelVolumeStampRuntime>(*LastStamp).GetBlendMode() == EVoxelVolumeBlendMode::Intersect)
				{
					ensureVoxelSlow(LastStampBounds.IsValidAndNotEmpty());
					CurrentIntersectBounds = CurrentIntersectBounds.IntersectWith(LastStampBounds);
				}

				LastStamp = Element.Stamp;
				LastStampBounds = Element.Bounds;
			}

			Element.Bounds = Element.Bounds.IntersectWith(CurrentIntersectBounds);

			if (!Element.Bounds.IsValidAndNotEmpty())
			{
				continue;
			}

			TreeElements.Add(Element.Bounds, Index);
		}
	}

	TVoxelArray<int32> CollectDependenciesIndices;
	{
		VOXEL_SCOPE_COUNTER("CollectDependenciesIndices");

		for (int32 Index = 0; Index < Elements.Num(); Index++)
		{
			FVoxelStampTreeElement& Element = Elements[Index];
			if (Element.Stamp->HasCollectDependencies())
			{
				CollectDependenciesIndices.Add(Index);
			}
		}
	}

	FVoxelAABBTree AABBTree;
	AABBTree.Initialize(MoveTemp(TreeElements));

	return MakeShared<FVoxelStampTree>(
		bIs2D,
		LODToDependency[LOD],
		MoveTemp(AABBTree),
		MoveTemp(CollectDependenciesIndices),
		MoveTemp(Elements),
		MoveTemp(Stamps),
		PreviousTree);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelLayerTracker> FVoxelLayerTracker::Create(
	const TVoxelObjectPtr<UWorld> World,
	const AActor* Actor)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedRef<FVoxelLayerTracker> LayerTracker = MakeShareable_Stats(new FVoxelLayerTracker(World, Actor));
	if (Actor)
	{
		LayerTracker->QueryToWorld = Actor->ActorToWorld();
	}

	const FSimpleDelegate Delegate = MakeWeakPtrDelegate(LayerTracker->Timestamp, [&Timestamp = *LayerTracker->Timestamp]
	{
		Timestamp.Increment();
	});

	Voxel::OnRefreshAll.Add(Delegate);
	UVoxelLayerStack::OnChanged.Add(Delegate);
	FVoxelStampManager::Get(World)->OnChanged.Add(Delegate);

	LayerTracker->UpdateLayers();
	return LayerTracker;
}

FVoxelLayerTracker::FVoxelLayerTracker(
	const TVoxelObjectPtr<UWorld> World,
	const TVoxelObjectPtr<const AActor> WeakActor)
	: World(World)
	, WeakActor(WeakActor)
	, Dependency(FVoxelDependency::Create("FVoxelLayerTracker " + World.GetName() + " " + WeakActor.GetName()))
{
	ensure(World.IsValid_Slow());
}

void FVoxelLayerTracker::UpdateLayers()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	// Flush changes, this might increase timestamp
	FVoxelStampManager::Get(World)->FlushUpdates();

	if (Layers &&
		Layers->Timestamp == Timestamp->Get())
	{
		return;
	}

	bool bHasNewDependencies = false;
	FString Changes = "FVoxelLayerTracker";
	TVoxelMap<FVoxelWeakStackLayer, TSharedPtr<const FVoxelHeightLayer>> WeakLayerToHeightLayer;
	TVoxelMap<FVoxelWeakStackLayer, TSharedPtr<const FVoxelVolumeLayer>> WeakLayerToVolumeLayer;

	ForEachObjectOfClass_Copy<UVoxelLayerStack>([&](UVoxelLayerStack& Stack)
	{
		TVoxelSet<UVoxelLayer*> VisitedLayers;
		TSharedPtr<const FVoxelHeightLayer> PreviousHeightLayer;
		TSharedPtr<const FVoxelVolumeLayer> PreviousVolumeLayer;

		TSharedPtr<const FVoxelLayerTrackerStamps> PreviousStamps;
		bool bShouldForceUpdateStamps = false;

		const auto GetStamps = [&](UVoxelLayer& Layer)
		{
			TSharedPtr<FVoxelLayerTrackerStamps>& Stamps = LayerToStamps.FindOrAdd(Layer);
			if (!Stamps ||
				Stamps->StackMaxDistance != Stack.MaxDistance ||
				!Stamps->QueryToWorld.Equals(QueryToWorld, 0))
			{
#if VOXEL_INVALIDATION_TRACKING
				if (Stamps)
				{
					Changes += "\n\t" + Layer.GetName() + ": " + Stamps->QueryToWorld.ToString() + " -> " + QueryToWorld.ToString();
				}
#endif

				Stamps = FVoxelLayerTrackerStamps::Create(
					*this,
					Stack.MaxDistance,
					Layer);

				bHasNewDependencies = true;
			}

			if (bShouldForceUpdateStamps)
			{
				// Previous layer changed, we need to force a rebuild
				Stamps->LayerTimestamp->Increment();
			}

			if (Stamps->UpdateStampsIfNeeded(PreviousStamps.Get()))
			{
				bShouldForceUpdateStamps = true;
			}

			PreviousStamps = Stamps;

			return Stamps.ToSharedRef();
		};

		for (UVoxelHeightLayer* HeightLayer : Stack.HeightLayers)
		{
			if (!HeightLayer)
			{
				continue;
			}

			if (VisitedLayers.Contains(HeightLayer))
			{
				VOXEL_MESSAGE(Error, "Layer {0} is used twice in stack {1}", HeightLayer->GetName(), Stack);
				continue;
			}
			VisitedLayers.Add_CheckNew(HeightLayer);

			const FVoxelWeakStackLayer WeakLayer(FVoxelStackLayer
			{
				&Stack,
				HeightLayer
			});

			const TSharedRef<FVoxelLayerTrackerStamps> Stamps = GetStamps(*HeightLayer);

			const TSharedRef<FVoxelHeightLayer> Layer = MakeShared<FVoxelHeightLayer>(
				WeakLayer,
				Stack.MaxDistance,
				Stamps->LODToTree,
				PreviousHeightLayer);

			WeakLayerToHeightLayer.Add_EnsureNew(WeakLayer, Layer);

			PreviousHeightLayer = Layer;
		}

		for (UVoxelVolumeLayer* VolumeLayer : Stack.VolumeLayers)
		{
			if (!VolumeLayer)
			{
				continue;
			}

			if (VisitedLayers.Contains(VolumeLayer))
			{
				VOXEL_MESSAGE(Error, "Layer {0} is used twice in stack {1}", VolumeLayer->GetName(), Stack);
				continue;
			}
			VisitedLayers.Add_CheckNew(VolumeLayer);

			const FVoxelWeakStackLayer WeakLayer(FVoxelStackLayer
			{
				&Stack,
				VolumeLayer
			});

			const TSharedRef<FVoxelLayerTrackerStamps> Stamps = GetStamps(*VolumeLayer);

			const TSharedRef<FVoxelVolumeLayer> Layer = MakeShared<FVoxelVolumeLayer>(
				WeakLayer,
				Stamps->LODToTree,
				Stamps->IntersectBounds,
				PreviousHeightLayer,
				PreviousVolumeLayer);

			WeakLayerToVolumeLayer.Add_EnsureNew(WeakLayer, Layer);

			PreviousHeightLayer = nullptr;
			PreviousVolumeLayer = Layer;
		}
	});

	const TSharedPtr<FVoxelLayers> OldLayers = Layers;
	const TSharedRef<FVoxelLayers> NewLayers = MakeShareable_Stats(new FVoxelLayers(
		World,
		Dependency,
		Timestamp->Get(),
		MoveTemp(WeakLayerToHeightLayer),
		MoveTemp(WeakLayerToVolumeLayer)));

	Layers = NewLayers;

	bool bInvalidate = false;
	if (bHasNewDependencies)
	{
		bInvalidate = true;
	}

	if (!OldLayers)
	{
#if VOXEL_INVALIDATION_TRACKING
		Changes += "\n\tNo previous layers";
#endif
		bInvalidate = true;
	}
	else if (
		!OldLayers->WeakLayerToHeightLayer.HasSameKeys(NewLayers->WeakLayerToHeightLayer) ||
		!OldLayers->WeakLayerToVolumeLayer.HasSameKeys(NewLayers->WeakLayerToVolumeLayer))
	{
#if VOXEL_INVALIDATION_TRACKING
		Changes += "\n\tLayer stack changed";
#endif
		bInvalidate = true;
	}

	if (bInvalidate)
	{
		FVoxelInvalidationScope Scope(Changes);
		Dependency->Invalidate();
	}
}

TSharedRef<FVoxelLayers> FVoxelLayerTracker::GetLayers()
{
	UpdateLayers();

	ensure(Layers->Timestamp == Timestamp->Get());
	return Layers.ToSharedRef();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelLayerTracker> FVoxelLayerTrackerSubsystem::GetLayerTracker()
{
	check(IsInGameThread());

	if (!RootLayerTracker)
	{
		RootLayerTracker = FVoxelLayerTracker::Create(GetWorld(), nullptr);
	}
	return RootLayerTracker.ToSharedRef();
}

TSharedRef<FVoxelLayerTracker> FVoxelLayerTrackerSubsystem::GetLayerTracker(const AActor& Actor)
{
	check(IsInGameThread());

	TSharedPtr<FVoxelLayerTracker>& LayerTracker = ActorToLayerTracker.FindOrAdd(Actor);
	if (!LayerTracker)
	{
		LayerTracker = FVoxelLayerTracker::Create(GetWorld(), &Actor);
	}
	return LayerTracker.ToSharedRef();
}

void FVoxelLayerTrackerSubsystem::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	if (RootLayerTracker)
	{
		RootLayerTracker->UpdateLayers();
	}

	for (auto It = ActorToLayerTracker.CreateIterator(); It; ++It)
	{
		const AActor* Actor = It.Key().Resolve();
		if (!Actor)
		{
			It.RemoveCurrent();
			continue;
		}
		FVoxelLayerTracker& LayerTracker = *It.Value();

		const FTransform NewLocalToWorld = Actor->ActorToWorld();
		if (!LayerTracker.QueryToWorld.Equals(NewLocalToWorld))
		{
			LayerTracker.QueryToWorld = NewLocalToWorld;
			LayerTracker.Timestamp->Increment();
		}

		LayerTracker.UpdateLayers();
	}

	if (GVoxelStampShowTree)
	{
		int32 LayerIndex = 0;

		for (const auto& It : GetLayerTracker()->GetLayers()->WeakLayerToVolumeLayer)
		{
			const FLinearColor Color = GEngine->LODColorationColors[LayerIndex % GEngine->LODColorationColors.Num()];
			LayerIndex++;

			const UVoxelLayer* Layer = It.Key.Layer.Resolve();
			if (!ensureVoxelSlow(Layer))
			{
				continue;
			}

			GEngine->AddOnScreenDebugMessage(
				FVoxelUtilities::MurmurHash(this) ^ FVoxelUtilities::MurmurHash(It.Key),
				2 * FApp::GetDeltaTime(),
				Color.ToFColor(true),
				"Layer " + Layer->GetPathName());

			const int32 Index = FMath::RoundToInt(FPlatformTime::Seconds() / 2);

			FVoxelDebugDrawer Drawer(GetWorld());

			It.Value->GetTree(0).AABBTree.DrawTree(
				Drawer.Color(Color).OneFrame(),
				FMatrix::Identity,
				Index);
		}
	}

	if (GVoxelStampShowAllBounds ||
		GVoxelStampShowSelectedBounds)
	{
		int32 LayerIndex = 0;

		TVoxelArray<TSharedPtr<const FVoxelLayerBase>> Layers;
		Layers.Append(GetLayerTracker()->GetLayers()->WeakLayerToHeightLayer.ValueArray());
		Layers.Append(GetLayerTracker()->GetLayers()->WeakLayerToVolumeLayer.ValueArray());

		{
			TVoxelSet<FVoxelWeakStackLayer> UniqueLayers;

			Layers.RemoveAll([&](const TSharedPtr<const FVoxelLayerBase>& Layer)
			{
				if (!UniqueLayers.TryAdd(Layer->WeakLayer))
				{
					// Duplicate
					return true;
				}

				for (int32 LOD = 0; LOD < Layer->LODToTree.Num(); LOD++)
				{
					if (Layer->GetTree(LOD).Elements.Num() > 0)
					{
						return false;
					}
				}

				// Empty
				return true;
			});
		}

		for (const TSharedPtr<const FVoxelLayerBase>& Layer : Layers)
		{
			const FLinearColor Color = GEngine->LODColorationColors[LayerIndex % GEngine->LODColorationColors.Num()];
			LayerIndex++;

			TVoxelSet<TPair<const FVoxelStampRuntime*, FVoxelBox>> VisitedRuntimes;
			VisitedRuntimes.Reserve(1000);

			const UVoxelLayer* LayerObject = Layer->WeakLayer.Layer.Resolve();
			if (!ensureVoxelSlow(LayerObject))
			{
				continue;
			}

			GEngine->AddOnScreenDebugMessage(
				FVoxelUtilities::MurmurHash(this) ^ FVoxelUtilities::MurmurHash(Layer.Get()),
				2 * FApp::GetDeltaTime(),
				Color.ToFColor(true),
				"Layer " + LayerObject->GetPathName());

			for (int32 LOD = 0; LOD < Layer->LODToTree.Num(); LOD++)
			{
				const TSharedRef<FVoxelFutureStampTree> FutureTree = Layer->LODToTree[LOD];
				if (!FutureTree->IsComplete())
				{
					continue;
				}
				FVoxelStampTree& Tree = FutureTree->GetTree();

				for (const FVoxelAABBTree::FLeaf& Leaf : Tree.AABBTree.GetLeaves())
				{
					for (int32 Index = Leaf.StartIndex; Index < Leaf.EndIndex; Index++)
					{
						const FVoxelStampRuntime* Stamp = Tree.Elements[Tree.AABBTree.GetPayload(Index)].Stamp;
						const FVoxelBox Bounds = Tree.AABBTree.GetBounds(Index).GetBox();

						if (!VisitedRuntimes.TryAdd({ Stamp, Bounds }))
						{
							continue;
						}

#if WITH_EDITOR
						if (!GVoxelStampShowAllBounds)
						{
							const USceneComponent* Component = Stamp->GetComponent().Resolve();
							if (!ensureVoxelSlow(Component))
							{
								continue;
							}

							const bool bIsSelected = INLINE_LAMBDA
							{
								if (Component->IsSelectedInEditor())
								{
									return true;
								}

								const AActor* Owner = Component->GetOwner();
								if (!Owner)
								{
									return false;
								}

								return Owner->IsActorOrSelectionParentSelected();
							};

							if (!bIsSelected)
							{
								continue;
							}
						}
#endif

						FVoxelDebugDrawer(GetWorld())
						.OneFrame()
						.Color(Color)
						.DrawBox(Bounds, FTransform::Identity);
					}
				}
			}
		}
	}
}