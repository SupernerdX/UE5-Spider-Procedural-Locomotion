// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStamp.h"
#include "VoxelStampIndex.h"
#include "VoxelStampComponentInterface.h"
#include "VoxelStampRuntime.generated.h"

class UVoxelLayer;
class FVoxelStampManager;
struct FVoxelStamp;

USTRUCT()
struct VOXEL_API FVoxelStampRuntime
	: public FVoxelVirtualStruct
#if CPP
	, public TSharedFromThis<FVoxelStampRuntime>
#endif
{
	GENERATED_BODY()
	DECLARE_VIRTUAL_STRUCT_PARENT_NO_COPY(FVoxelStampRuntime, GENERATED_VOXEL_SUBSYSTEM_BODY)

public:
	struct FStampInfo
	{
	public:
		const FVoxelStampIndex StampIndex;
		const FVoxelWeakStampRef WeakStampRef;
		const TSharedRef<FVoxelStamp> Stamp;
		const TVoxelObjectPtr<USceneComponent> Component;

		FORCEINLINE FStampInfo(
			const FVoxelStampIndex& StampIndex,
			const FVoxelWeakStampRef& WeakStampRef,
			const TSharedRef<FVoxelStamp>& Stamp,
			const TVoxelObjectPtr<USceneComponent>& Component)
			: StampIndex(StampIndex)
			, WeakStampRef(WeakStampRef)
			, Stamp(Stamp)
			, Component(Component)
		{
		}

		FORCEINLINE TSharedRef<FVoxelStampRuntime> GetRuntime() const
		{
			return OutRuntime.ToSharedRef();
		}

	private:
		TSharedPtr<FVoxelStampRuntime> OutRuntime;
		TUniquePtr<FVoxelDependencyCollector> DependencyCollector;

		friend FVoxelStampRuntime;
	};
	static void BulkCreate(
		TVoxelObjectPtr<UWorld> World,
		TVoxelArrayView<FStampInfo> StampInfos);

public:
	static TSharedPtr<FVoxelStampRuntime> Create(
		TVoxelObjectPtr<UWorld> World,
		const FVoxelStampRef& StampRef,
		TVoxelObjectPtr<USceneComponent> Component);

public:
	FVoxelStampRuntime() = default;
	UE_NONCOPYABLE(FVoxelStampRuntime);

	FORCEINLINE const FVoxelStamp& GetStamp() const
	{
		return *Initializer.Stamp;
	}

public:
	virtual bool Initialize(FVoxelDependencyCollector& DependencyCollector)
	{
		return true;
	}
	virtual bool Initialize_Parallel(FVoxelDependencyCollector& DependencyCollector)
	{
		return true;
	}

	virtual FVoxelBox GetLocalBounds() const
	{
		return {};
	}
	virtual TVoxelInlineArray<FVoxelBox, 1> GetChildren() const
	{
		return { GetLocalBounds() };
	}

	// Called globally, used by sculpt actors to not invalidate everything when expanding their bounds
	virtual bool HasCollectDependencies() const
	{
		return false;
	}
	virtual bool CanPartiallyInvalidate() const
	{
		return false;
	}

	virtual bool TryToPartiallyInvalidate(
		const FVoxelStampRuntime& PreviousRuntime,
		TVoxelArray<FVoxelBox>& OutLocalBoundsToInvalidate) const
	{
		ensure(false);
		return false;
	}
	virtual bool ShouldUseQueryPrevious() const
	{
		return false;
	}

public:
	FORCEINLINE bool FailedToInitialize() const
	{
		checkVoxelSlow(InitializationInfo.bIsInitialized);
		return InitializationInfo.bInitializationFailed;
	}
	FORCEINLINE TVoxelObjectPtr<UWorld> GetWorld() const
	{
		return Initializer.World;
	}
	FORCEINLINE TVoxelObjectPtr<USceneComponent> GetComponent() const
	{
		return Initializer.Component;
	}
	FORCEINLINE TConstVoxelArrayView<TVoxelObjectPtr<UVoxelLayer>> GetLayers() const
	{
		checkVoxelSlow(InitializationInfo.bIsInitialized);
		return InitializationInfo.Layers;
	}
#if VOXEL_STATS
	FORCEINLINE FName GetStatName() const
	{
		return Initializer.StatName;
	}
#endif
	FORCEINLINE uint64 GetPropertyHash() const
	{
		return Initializer.PropertyHash;
	}
	FORCEINLINE FInt32Interval GetLODRange() const
	{
		return FInt32Interval(Initializer.MinLOD, Initializer.MaxLOD);
	}
	FORCEINLINE bool ShouldComputeLOD(const int32 LOD) const
	{
		return Initializer.MinLOD <= LOD && LOD <= Initializer.MaxLOD;
	}
	FORCEINLINE const FVoxelWeakStampRef& GetWeakStampRef() const
	{
		return WeakStampRef;
	}
	FORCEINLINE const FTransform& GetLocalToWorld() const
	{
		return GetStamp().Transform;
	}

	AActor* GetActor() const;
	// Will return -1 if this stamp isn't stored in an instanced stamp component
	int32 GetInstanceIndex() const;
#if WITH_EDITOR
	void SelectComponent_EditorOnly() const;
#endif

	void RequestUpdate() const;

public:
	FORCEINLINE bool AffectShape() const
	{
		return EnumHasAllFlags(GetStamp().Behavior, EVoxelStampBehavior::AffectShape);
	}
	FORCEINLINE bool AffectSurfaceType() const
	{
		return EnumHasAllFlags(GetStamp().Behavior, EVoxelStampBehavior::AffectSurfaceType);
	}
	FORCEINLINE bool AffectMetadata() const
	{
		return EnumHasAllFlags(GetStamp().Behavior, EVoxelStampBehavior::AffectMetadata);
	}

protected:
	template<typename T>
	T* FindComponent() const
	{
		check(IsInGameThread());
		check(!InitializationInfo.bIsInitialized);

		const IVoxelStampComponentInterface* Interface = Cast<IVoxelStampComponentInterface>(Initializer.Component.Resolve());
		if (!Interface)
		{
			return nullptr;
		}

		return Interface->FindComponent<T>();
	}

	// Only callable in Initialize, use this to fixup a stamp
	FVoxelStamp& GetMutableStamp();

	virtual int64 Internal_GetStampOffset() const VOXEL_PURE_VIRTUAL({});

private:
	struct FInitializer
	{
		TSharedRef<FVoxelStamp> Stamp = SharedRef_Null;
		TVoxelObjectPtr<UWorld> World;
		TVoxelObjectPtr<USceneComponent> Component;
#if VOXEL_STATS
		FName StatName;
#endif
		uint64 PropertyHash = 0;
		int32 MinLOD = 0;
		int32 MaxLOD = 0;
	};
	FInitializer Initializer;

	FVoxelStampIndex StampIndex;
	FVoxelWeakStampRef WeakStampRef;

	struct FInitializationInfo
	{
		bool bIsInitialized = false;
		bool bInitializationFailed = false;
		bool bStampMutated = false;
		TSharedPtr<FVoxelDependencyTracker> DependencyTracker;
		TVoxelArray<TVoxelObjectPtr<UVoxelLayer>> Layers;
	};
	FInitializationInfo InitializationInfo;

	void PreInitialize(
		const TSharedRef<FVoxelStamp>& Stamp,
		TVoxelObjectPtr<UWorld> World,
		TVoxelObjectPtr<USceneComponent> Component);

	void FinalizeInitialization();

private:
#if VOXEL_STATS
	static FName BuildStatName(
		const UScriptStruct& Struct,
		TVoxelObjectPtr<USceneComponent> WeakComponent);
#endif

	static uint64 ComputeHash(const FVoxelStamp& Stamp);
	static const TVoxelMap<UScriptStruct*, UScriptStruct*>& GetStampToStampRuntime();

	friend struct FVoxelStampRef;
	friend class FVoxelStampManager;
};

template<typename T>
requires std::derived_from<T, FVoxelStampRuntime>
struct TStructOpsTypeTraits<T> : public TStructOpsTypeTraitsBase2<T>
{
	enum
	{
		WithCopy = false
	};
};

#define GENERATED_VOXEL_RUNTIME_STAMP_BODY_IMPL(Name) \
	GENERATED_VIRTUAL_STRUCT_BODY(FVoxelStampRuntime) \
	void operator=(const Name ## Runtime&) \
	{ \
		checkStatic(std::is_same_v<Name ## Runtime, VOXEL_THIS_TYPE>); \
		ensure(false); \
	}

#define GENERATED_VOXEL_RUNTIME_STAMP_ABSTRACT_BODY(Name) \
	GENERATED_VOXEL_RUNTIME_STAMP_BODY_IMPL(Name) \
	FORCEINLINE const Name& GetStamp() const \
	{ \
		return CastStructChecked<Name>(Super::GetStamp()); \
	}

#define GENERATED_VOXEL_RUNTIME_STAMP_BODY(Name) \
		GENERATED_VOXEL_RUNTIME_STAMP_BODY_IMPL(Name) \
		const void* __InternalDummy = nullptr; \
		const Name& Stamp = *reinterpret_cast<Name*>(0x1F1802EBB6147584_u64); \
		FORCEINLINE const Name& GetStamp() const \
		{ \
			return Stamp; \
		} \
	private: \
		FORCEINLINE Name& GetMutableStamp() \
		{ \
			return CastStructChecked<Name>(Super::GetMutableStamp()); \
		} \
	public: \
		virtual int64 Internal_GetStampOffset() const override final \
		{ \
			return offsetof(VOXEL_THIS_TYPE, __InternalDummy) + sizeof(__InternalDummy); \
		}