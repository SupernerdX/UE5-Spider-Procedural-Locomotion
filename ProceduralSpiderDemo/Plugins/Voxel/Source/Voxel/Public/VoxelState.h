// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelConfig.h"

struct FVoxelSubsystem;
class FVoxelSurfaceTypeTable;
class FVoxelLayers;
class FVoxelRuntime;
class FVoxelInvalidationQueue;

class VOXEL_API FVoxelState
{
public:
	const TSharedRef<const FVoxelConfig> Config;
	const TSharedRef<FVoxelLayers> Layers;
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	const TSharedRef<FVoxelInvalidationQueue> InvalidationQueue;
	FSimpleMulticastDelegate OnReadyToRender;
	FSimpleMulticastDelegate OnRendered;

	VOXEL_COUNT_INSTANCES();

	FVoxelState(
		const TSharedRef<const FVoxelConfig>& Config,
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedPtr<const FVoxelState>& PreviousState);
	~FVoxelState();
	UE_NONCOPYABLE(FVoxelState);

public:
	void Tick();
	void Render(FVoxelRuntime& Runtime);
	void AddReferencedObjects(FReferenceCollector& Collector);
	bool IsReadyToRender() const;

public:
	float GetProgress() const
	{
		return ProgressInfo.Progress;
	}
	bool IsRendered() const
	{
		return bIsRendered;
	}
	bool IsInvalidated() const
	{
		return Timestamp < TimestampRef->Get();
	}
	int32 GetStateIndex() const
	{
		return StateIndex;
	}
	FVoxelTaskContext* GetTaskContext() const
	{
		return TaskContext.Get();
	}

public:
	template<typename T>
	requires std::derived_from<T, FVoxelSubsystem>
	T& GetSubsystem() const
	{
		return CastStructChecked<T>(this->GetSubsystem(StaticStructFast<T>()));
	}
	template<typename T>
	requires std::derived_from<T, FVoxelSubsystem>
	T* GetSubsystemPtr() const
	{
		FVoxelSubsystem* Subsystem = this->GetSubsystemPtr(StaticStructFast<T>());
		if (!Subsystem)
		{
			return nullptr;
		}

		return &CastStructChecked<T>(*Subsystem);
	}

	FVoxelSubsystem& GetSubsystem(const UScriptStruct* Struct) const;
	FVoxelSubsystem* GetSubsystemPtr(const UScriptStruct* Struct) const;

private:
	const TSharedRef<FVoxelCounter64> TimestampRef;
	const int64 Timestamp;
	const bool bHasPreviousState;

	const int32 StateIndex;
	const FName StatName;
	const double StartTime = FPlatformTime::Seconds();
	const uint64 StartFrame = GFrameCounter;

	TSharedPtr<const uint64> RegionIdRef;

	struct FProgressInfo
	{
		int32 MaxNumTasks = 0;
		float Progress = 0;
	};
	FProgressInfo ProgressInfo;

	bool bIsRendered = false;

	TSharedPtr<FVoxelTaskContext> TaskContext;
	bool bAllSubsystemInitialized = false;
	TVoxelMap<const UScriptStruct*, TUniquePtr<FVoxelSubsystem>> StructToSubsystem;

	TSharedPtr<FVoxelDebugDrawGroup> DrawGroup;
	TVoxelArray<TSharedPtr<FVoxelDebugDrawGroup>> ActiveDrawGroups;

	friend FVoxelSubsystem;
};