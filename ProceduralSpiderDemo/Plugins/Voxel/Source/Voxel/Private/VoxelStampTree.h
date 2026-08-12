// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelQuery.h"
#include "VoxelAABBTree.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"
#include "VoxelStampTransform.h"

struct FVoxelStampTreeElement
{
	const FVoxelStampRuntime* Stamp = nullptr;
	int32 ChildIndex = -1;

	union
	{
		FVoxelHeightTransform HeightStampToQuery;
		FVoxelVolumeTransform VolumeStampToQuery;
	};

	// in Query space
	FVoxelBox Bounds;

	FVoxelStampTreeElement()
	{
		FMemory::Memzero(HeightStampToQuery);
		FMemory::Memzero(VolumeStampToQuery);
	}
};
checkStatic(sizeof(FVoxelStampTreeElement) == 128);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelStampTree
{
public:
	const bool bIs2D;
	const TSharedRef<FVoxelDependency3D> Dependency;
	const FVoxelAABBTree AABBTree;
	const TVoxelArray<int32> CollectDependenciesIndices;
	// Deterministically sorted
	const TVoxelArray<FVoxelStampTreeElement> Elements;
	const TVoxelArray<TSharedRef<const FVoxelStampRuntime>> Stamps;
	const TSharedPtr<FVoxelStampTree> PreviousTree;

	FVoxelStampTree(
		const bool bIs2D,
		const TSharedRef<FVoxelDependency3D>& Dependency,
		FVoxelAABBTree&& AABBTree,
		TVoxelArray<int32>&& CollectDependenciesIndices,
		TVoxelArray<FVoxelStampTreeElement>&& Elements,
		TVoxelArray<TSharedRef<const FVoxelStampRuntime>>&& Stamps,
		const TSharedPtr<FVoxelStampTree>& PreviousTree)
		: bIs2D(bIs2D)
		, Dependency(Dependency)
		, AABBTree(MoveTemp(AABBTree))
		, CollectDependenciesIndices(MoveTemp(CollectDependenciesIndices))
		, Elements(MoveTemp(Elements))
		, Stamps(MoveTemp(Stamps))
		, PreviousTree(PreviousTree)
	{
	}

public:
	struct FStamp
	{
	public:
		template<typename T>
		FORCEINLINE const T& GetStamp() const
		{
			checkVoxelSlow(Stamp);
			return CastStructChecked<T>(*Stamp);
		}
		FORCEINLINE const FVoxelStampTreeElement* GetUniqueElement() const
		{
			if (Elements.Num() > 1)
			{
				return nullptr;
			}

			return Elements[0];
		}
		FORCEINLINE TConstVoxelArrayView<const FVoxelStampTreeElement*> GetElements() const
		{
			return Elements;
		}

	private:
		const FVoxelStampRuntime* Stamp = nullptr;
		TConstVoxelArrayView<const FVoxelStampTreeElement*> Elements;

		FStamp(
			const FVoxelStampRuntime* Stamp,
			const TConstVoxelArrayView<const FVoxelStampTreeElement*> Elements)
			: Stamp(Stamp)
			, Elements(Elements)
		{
			checkVoxelSlow(Elements.Num() > 0);

			for (const FVoxelStampTreeElement* Element : Elements)
			{
				checkVoxelSlow(Element->Stamp == Stamp);
			}
		}

		friend FVoxelStampTree;
	};
	struct FIterator
	{
	public:
		TVoxelInlineArray<FStamp, 128> Stamps;

	private:
		TVoxelInlineArray<const FVoxelStampTreeElement*, 128> ElementStorage;

		friend FVoxelStampTree;
	};
	TUniquePtr<FIterator> CreateIterator(
		const FVoxelQuery& Query,
		const FVoxelBox& Bounds) const;

public:
	template<typename LambdaType>
	requires LambdaHasSignature_V<LambdaType, EVoxelIterate(int32)>
	FORCENOINLINE void ForeachElementIndex_Unsorted(
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelBox& Bounds,
		LambdaType Lambda) const
	{
		VOXEL_FUNCTION_COUNTER();

		DependencyCollector.AddDependency(*Dependency, Bounds);

		for (const int32 Index : CollectDependenciesIndices)
		{
			const FVoxelStampTreeElement& Element = Elements[Index];

			if (bIs2D)
			{
				CastStructChecked<FVoxelHeightStampRuntime>(*Element.Stamp).CollectDependencies(
					DependencyCollector,
					Element.HeightStampToQuery,
					FVoxelBox2D(Bounds));
			}
			else
			{
				CastStructChecked<FVoxelVolumeStampRuntime>(*Element.Stamp).CollectDependencies(
					DependencyCollector,
					Element.VolumeStampToQuery,
					Bounds);
			}
		}

		AABBTree.TraverseBounds(FVoxelFastBox(Bounds), [&](const int32 Index)
		{
			return Lambda(Index);
		});
	}

	template<typename LambdaType>
	requires LambdaHasSignature_V<LambdaType, EVoxelIterate(const FVoxelStampTreeElement&)>
	FORCEINLINE void ForeachElement_Unsorted(
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelBox& Bounds,
		const EVoxelStampBehavior BehaviorMask,
		LambdaType Lambda) const
	{
		this->ForeachElementIndex_Unsorted(DependencyCollector, Bounds, [&](const int32 Index)
		{
			const FVoxelStampTreeElement& Element = Elements[Index];

			if (!EnumHasAnyFlags(Element.Stamp->GetStamp().Behavior, BehaviorMask))
			{
				return EVoxelIterate::Continue;
			}

			return Lambda(Element);
		});
	}
	template<typename LambdaType>
	requires LambdaHasSignature_V<LambdaType, void(const FVoxelStampTreeElement&)>
	FORCEINLINE void ForeachElement_Unsorted(
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelBox& Bounds,
		const EVoxelStampBehavior BehaviorMask,
		LambdaType Lambda) const
	{
		this->ForeachElement_Unsorted(DependencyCollector, Bounds, BehaviorMask, [&](const FVoxelStampTreeElement& Element)
		{
			Lambda(Element);
			return EVoxelIterate::Continue;
		});
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelStampTreeManager : public FVoxelSingleton
{
public:
	void AddTask(TVoxelUniqueFunction<void()> Lambda);
	void ProcessTask();

private:
	FVoxelCriticalSection CriticalSection;
	TQueue<TVoxelUniqueFunction<void()>> Tasks_RequiresLock;
};
extern FVoxelStampTreeManager* GVoxelStampTreeManager;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelFutureStampTree
{
public:
	FVoxelFutureStampTree() = default;
	UE_NONCOPYABLE(FVoxelFutureStampTree);

	FORCEINLINE FVoxelStampTree& GetTree() const
	{
		if (PrivateIsComplete.Get())
		{
			return *Tree;
		}

		return GetTree_Impl();
	}
	FORCEINLINE bool IsComplete() const
	{
		return PrivateIsComplete.Get();
	}

	void Set(const TSharedRef<FVoxelStampTree>& NewTree);
	void Then(TVoxelUniqueFunction<void(const TSharedRef<FVoxelStampTree>&)> Lambda);

private:
	FVoxelCriticalSection_NoPadding CriticalSection;
	TSharedPtr<FVoxelStampTree> Tree;
	TVoxelAtomic<bool> PrivateIsComplete;
	TVoxelArray<TVoxelUniqueFunction<void(const TSharedRef<FVoxelStampTree>&)>> Continuations_RequiresLock;

	FVoxelStampTree& GetTree_Impl() const;
};