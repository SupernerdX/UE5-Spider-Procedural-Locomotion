// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelConfig.h"
#include "VoxelSubsystem.generated.h"

class FVoxelState;
class FVoxelLayers;
class FVoxelRuntime;
class FVoxelSurfaceTypeTable;
struct IVoxelSubsystemGCObject;

USTRUCT()
struct VOXEL_API FVoxelSubsystem : public FVoxelVirtualStruct
{
	GENERATED_BODY()
	DECLARE_VIRTUAL_STRUCT_PARENT_NO_COPY(FVoxelSubsystem, GENERATED_VOXEL_SUBSYSTEM_BODY)
	VOXEL_COUNT_INSTANCES();

public:
	FVoxelSubsystem() = default;
	virtual ~FVoxelSubsystem() override;
	UE_NONCOPYABLE(FVoxelSubsystem);

public:
	template<typename T>
	requires std::derived_from<T, FVoxelSubsystem>
	FORCEINLINE T& GetSubsystem() const
	{
		return CastStructChecked<T>(this->GetSubsystem(StaticStructFast<T>()));
	}

public:
	bool IsPreviousSubsystem() const;
	const FVoxelConfig& GetConfig() const;
	FVoxelLayers& GetLayers() const;
	FVoxelSurfaceTypeTable& GetSurfaceTypeTable() const;
	FVoxelTaskContext& GetTaskContext() const;
	TSharedRef<FVoxelDependencyTracker> Finalize(FVoxelDependencyCollector& DependencyCollector) const;
	FVoxelSubsystem& GetSubsystem(const UScriptStruct* Struct) const;
	void AddGCObject(const TSharedRef<IVoxelSubsystemGCObject>& Object) const;

public:
	virtual bool ShouldCreateOnServer() const { return true; }
	virtual void LoadFromPrevious(FVoxelSubsystem& InPreviousSubsystem) {}
	virtual void Initialize() {}
	virtual void Compute() {}
	virtual void Render(FVoxelRuntime& Runtime) {}
	virtual void AddReferencedObjects(FReferenceCollector& Collector);

private:
	FVoxelState* PrivateState = nullptr;
	bool bPrivateIsPreviousSubsystem = false;
	FVoxelCriticalSection_NoPadding GCObjects_CriticalSection;
	mutable TVoxelArray<TWeakPtr<IVoxelSubsystemGCObject>> GCObjects_RequiresLock;

	friend FVoxelState;
};

template<typename T>
requires std::derived_from<T, FVoxelSubsystem>
struct TStructOpsTypeTraits<T> : public TStructOpsTypeTraitsBase2<T>
{
	enum
	{
		WithCopy = false
	};
};

#define GENERATED_VOXEL_SUBSYSTEM_BODY() \
	GENERATED_VIRTUAL_STRUCT_BODY_NO_COPY(FVoxelSubsystem)