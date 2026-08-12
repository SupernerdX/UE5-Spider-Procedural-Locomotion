// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSubsystem.h"
#include "VoxelState.h"
#include "VoxelSubsystemGCObject.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelSubsystem);

FVoxelSubsystem::~FVoxelSubsystem()
{
	ensure(!PrivateState);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelSubsystem::IsPreviousSubsystem() const
{
	return bPrivateIsPreviousSubsystem;
}

const FVoxelConfig& FVoxelSubsystem::GetConfig() const
{
	checkVoxelSlow(PrivateState);
	return *PrivateState->Config;
}

FVoxelLayers& FVoxelSubsystem::GetLayers() const
{
	checkVoxelSlow(PrivateState);
	return *PrivateState->Layers;
}

FVoxelSurfaceTypeTable& FVoxelSubsystem::GetSurfaceTypeTable() const
{
	checkVoxelSlow(PrivateState);
	return *PrivateState->SurfaceTypeTable;
}

FVoxelTaskContext& FVoxelSubsystem::GetTaskContext() const
{
	checkVoxelSlow(PrivateState);
	return *PrivateState->TaskContext;
}

TSharedRef<FVoxelDependencyTracker> FVoxelSubsystem::Finalize(FVoxelDependencyCollector& DependencyCollector) const
{
	checkVoxelSlow(PrivateState);

	return DependencyCollector.Finalize(
		&PrivateState->InvalidationQueue.Get(),
		MakeWeakPtrLambda(PrivateState->TimestampRef, [&Timestamp = *PrivateState->TimestampRef](const FVoxelInvalidationCallstack& Callstack)
		{
			Timestamp.Increment();
		}));
}

FVoxelSubsystem& FVoxelSubsystem::GetSubsystem(const UScriptStruct* Struct) const
{
	checkVoxelSlow(PrivateState);
	return PrivateState->GetSubsystem(Struct);
}

void FVoxelSubsystem::AddGCObject(const TSharedRef<IVoxelSubsystemGCObject>& Object) const
{
	VOXEL_SCOPE_LOCK(GCObjects_CriticalSection);

	GCObjects_RequiresLock.Add(Object);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSubsystem::AddReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelUtilities::AddStructReferencedObjects(Collector, *this);

	VOXEL_SCOPE_LOCK(GCObjects_CriticalSection);

	for (auto It = GCObjects_RequiresLock.CreateIterator(); It; ++It)
	{
		const TSharedPtr<IVoxelSubsystemGCObject> Object = It->Pin();
		if (!Object)
		{
			It.RemoveCurrentSwap();
			continue;
		}

		Object->AddReferencedObjects(Collector);
	}
}