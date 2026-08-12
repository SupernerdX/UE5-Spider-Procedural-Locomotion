// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class AVoxelWorld;
class FVoxelState;

class VOXEL_API FVoxelRuntime
	: public TSharedFromThis<FVoxelRuntime>
	, public FVoxelTicker
{
public:
	const TVoxelObjectPtr<AVoxelWorld> WeakVoxelWorld;
	FSimpleMulticastDelegate OnInvalidated;
	TSharedPtr<FSimpleMulticastDelegate> OnNextStateRendered;

	explicit FVoxelRuntime(AVoxelWorld& VoxelWorld);
	UE_NONCOPYABLE(FVoxelRuntime);

	void Initialize();

public:
	bool IsProcessingNewState() const
	{
		return NewState.IsValid();
	}
	TSharedPtr<FVoxelState> GetState() const
	{
		return OldState;
	}
	TSharedPtr<FVoxelState> GetNewState() const
	{
		return NewState;
	}
	TSharedPtr<FVoxelState> GetNewestState() const
	{
		return NewState ? NewState : OldState;
	}

	//~ Begin FVoxelTicker Interface
	virtual void Tick() override;
	//~ End FVoxelTicker Interface

	void AddReferencedObjects(FReferenceCollector& Collector);

public:
	template<typename T>
	T* NewComponent()
	{
		return CastChecked<T>(this->NewComponent(StaticClassFast<T>()), ECastCheckedType::NullAllowed);
	}

	USceneComponent* NewComponent(const UClass* Class);
	void RemoveComponent(USceneComponent* Component);

public:
	void RemoveComponents(
		UClass* Class,
		TConstVoxelArrayView<USceneComponent*> ComponentsToRemove);

	template<typename T>
	requires std::derived_from<T, USceneComponent>
	void RemoveComponents(TConstVoxelArrayView<T*> Components)
	{
		this->RemoveComponents(
			StaticClassFast<T>(),
			Components.template ReinterpretAs<USceneComponent*>());
	}


private:
	TSharedPtr<FVoxelState> NewState;
	TSharedPtr<FVoxelState> OldState;

	TVoxelSet<TVoxelObjectPtr<USceneComponent>> AllComponents;
	TVoxelMap<const UClass*, TVoxelChunkedArray<TVoxelObjectPtr<USceneComponent>>> ClassToPooledComponents;

	void DrawDebug() const;

	friend AVoxelWorld;
};