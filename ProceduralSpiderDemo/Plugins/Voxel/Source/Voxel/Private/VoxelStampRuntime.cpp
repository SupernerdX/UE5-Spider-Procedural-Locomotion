// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampRuntime.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"
#include "VoxelLayerTracker.h"
#include "VoxelInvalidationCallstack.h"
#include "VoxelInstancedStampComponent.h"
#include "VoxelParameterOverridesOwner.h"
#if WITH_EDITOR
#include "Selection.h"
#include "Editor/EditorEngine.h"
#endif

void FVoxelStampRuntime::BulkCreate(
	const TVoxelObjectPtr<UWorld> World,
	const TVoxelArrayView<FStampInfo> StampInfos)
{
	VOXEL_FUNCTION_COUNTER_NUM(StampInfos.Num());
	check(IsInGameThread());

	const TVoxelMap<UScriptStruct*, UScriptStruct*>& StampToStampRuntime = GetStampToStampRuntime();

	if (WITH_EDITOR)
	{
		VOXEL_SCOPE_COUNTER("Fixup properties (editor only)");

		for (const FStampInfo& StampInfo : StampInfos)
		{
			StampInfo.Stamp->FixupProperties();

			if (const IVoxelStampComponentInterface* Interface = Cast<IVoxelStampComponentInterface>(StampInfo.Component.Resolve()))
			{
				StampInfo.Stamp->FixupComponents(*Interface);
			}
		}
	}

	{
		VOXEL_SCOPE_COUNTER("PreInitialize");

		Voxel::ParallelFor(StampInfos, [&](FStampInfo& StampInfo)
		{
			const TSharedRef<FVoxelStampRuntime> Runtime = MakeSharedStruct<FVoxelStampRuntime>(StampToStampRuntime[StampInfo.Stamp->GetStruct()]);

			Runtime->PreInitialize(
				StampInfo.Stamp,
				World,
				StampInfo.Component);

			Runtime->StampIndex = StampInfo.StampIndex;
			Runtime->WeakStampRef = StampInfo.WeakStampRef;

			StampInfo.OutRuntime = Runtime;
			StampInfo.DependencyCollector = MakeUnique<FVoxelDependencyCollector>(STATIC_FNAME("FVoxelStamp::Initialize"));
		});
	}

	{
		VOXEL_SCOPE_COUNTER("Initialize");

		for (const FStampInfo& StampInfo : StampInfos)
		{
			FVoxelStampRuntime& Runtime = *StampInfo.OutRuntime;
			FInitializationInfo& InitializationInfo = Runtime.InitializationInfo;
			FVoxelDependencyCollector& DependencyCollector = *StampInfo.DependencyCollector;

			checkVoxelSlow(!InitializationInfo.bIsInitialized);
			checkVoxelSlow(!InitializationInfo.bInitializationFailed);

			if (!Runtime.Initialize(DependencyCollector))
			{
				InitializationInfo.bInitializationFailed = true;
			}
		}
	}

	VOXEL_SCOPE_COUNTER("FinalizeInitialization");

	Voxel::ParallelFor(StampInfos, [&](FStampInfo& StampInfo)
	{
		FVoxelStampRuntime& Runtime = *StampInfo.OutRuntime;
		FInitializationInfo& Info = Runtime.InitializationInfo;
		FVoxelDependencyCollector& DependencyCollector = *StampInfo.DependencyCollector;

		if (!Info.bInitializationFailed)
		{
			if (!Runtime.Initialize_Parallel(DependencyCollector))
			{
				Info.bInitializationFailed = true;
			}
		}

		checkVoxelSlow(!Info.bIsInitialized);
		Info.bIsInitialized = true;

		if (DependencyCollector.HasDependencies())
		{
			Info.DependencyTracker = DependencyCollector.Finalize(
				nullptr,
				MakeWeakPtrLambda(Runtime, [&Runtime](const FVoxelInvalidationCallstack& Callstack)
				{
					Runtime.RequestUpdate();
				}));
		}

		StampInfo.DependencyCollector.Reset();

		Runtime.FinalizeInitialization();
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelStampRuntime> FVoxelStampRuntime::Create(
	const TVoxelObjectPtr<UWorld> World,
	const FVoxelStampRef& StampRef,
	const TVoxelObjectPtr<USceneComponent> Component)
{
	VOXEL_FUNCTION_COUNTER();

	if (!StampRef)
	{
		return {};
	}

	FStampInfo StampInfo
	{
		{},
		StampRef,
		StampRef->MakeSharedCopy(),
		Component
	};
	BulkCreate(World, MakeVoxelArrayView(StampInfo));

	const TSharedRef<FVoxelStampRuntime> Runtime = StampInfo.GetRuntime();

	if (Runtime->FailedToInitialize())
	{
		return {};
	}

	return Runtime;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AActor* FVoxelStampRuntime::GetActor() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const USceneComponent* Component = GetComponent().Resolve();
	if (!ensureVoxelSlow(Component))
	{
		return nullptr;
	}

	return Component->GetOwner();
}

int32 FVoxelStampRuntime::GetInstanceIndex() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const FVoxelStampRef StampRef = GetWeakStampRef().Pin();
	if (!ensureVoxelSlow(StampRef))
	{
		return -1;
	}

	const USceneComponent* Component = GetComponent().Resolve();
	if (!ensureVoxelSlow(Component))
	{
		return -1;
	}

	const UVoxelInstancedStampComponent* InstancedComponent = Cast<UVoxelInstancedStampComponent>(Component);
	if (!InstancedComponent)
	{
		return -1;
	}

	return InstancedComponent->FindStampIndex(StampRef);
}

#if WITH_EDITOR
void FVoxelStampRuntime::SelectComponent_EditorOnly() const
{
	VOXEL_FUNCTION_COUNTER();

	USceneComponent* Component = GetComponent().Resolve();
	if (!Component)
	{
		return;
	}

	if (UVoxelInstancedStampComponent* InstancedStampComponent = Cast<UVoxelInstancedStampComponent>(Component))
	{
		InstancedStampComponent->Modify();
		InstancedStampComponent->SetActiveInstance(GetWeakStampRef().Pin());
	}

	GEditor->GetSelectedComponents()->Modify();

	GEditor->SelectNone(true, true);

	if (AActor* Owner = Component->GetOwner())
	{
		if (Owner->GetRootComponent() == Component)
		{
			GEditor->SelectActor(Owner, true, true, true);
			return;
		}
	}

	GEditor->SelectComponent(Component, true, true, true);
}
#endif

void FVoxelStampRuntime::RequestUpdate() const
{
	Voxel::GameTask(MakeWeakPtrLambda(this, [this, Callstack = FVoxelInvalidationCallstack::Create("FVoxelStampRuntime::RequestUpdate")]
	{
		const FVoxelStampRef StampRef = GetWeakStampRef().Pin();
		if (!StampRef)
		{
			return;
		}

		FVoxelInvalidationScope Scope(Callstack);
		StampRef.Update();
	}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStamp& FVoxelStampRuntime::GetMutableStamp()
{
	check(!InitializationInfo.bIsInitialized);
	InitializationInfo.bStampMutated = true;
	return ConstCast(GetStamp());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampRuntime::PreInitialize(
	const TSharedRef<FVoxelStamp>& Stamp,
	const TVoxelObjectPtr<UWorld> World,
	const TVoxelObjectPtr<USceneComponent> Component)
{
	VOXEL_FUNCTION_COUNTER();
	ensureVoxelSlowNoSideEffects(Component.IsExplicitlyNull() || Component.IsValid_Slow());

	Initializer.Stamp = Stamp;
	Initializer.World = World;
	Initializer.Component = Component;
#if VOXEL_STATS
	Initializer.StatName = BuildStatName(*Stamp->GetStruct(), Component);
#endif
	Initializer.PropertyHash = ComputeHash(*Stamp);

	Initializer.MinLOD = FMath::Clamp(Stamp->LODRange.Min, 0, GVoxelMaxStampLOD);
	Initializer.MaxLOD = FMath::Clamp(Stamp->LODRange.Max, 0, GVoxelMaxStampLOD);

	if (const FVoxelVolumeStamp* VolumeStamp = CastStruct<FVoxelVolumeStamp>(*Stamp))
	{
		if (VolumeStamp->BlendMode == EVoxelVolumeBlendMode::Intersect)
		{
			// Intersect stamps can't be clamped to specific LODs
			Initializer.MinLOD = 0;
			Initializer.MaxLOD = GVoxelMaxStampLOD;
		}
	}

	const int64 StampOffset = Internal_GetStampOffset();
	const FVoxelStamp*& StampPtr = *reinterpret_cast<const FVoxelStamp**>(reinterpret_cast<uint8*>(this) + StampOffset);

	check(uint64(StampPtr) == 0x1F1802EBB6147584_u64);
	StampPtr = &Stamp.Get();
}

void FVoxelStampRuntime::FinalizeInitialization()
{
	VOXEL_FUNCTION_COUNTER();

	checkStatic(offsetof(FVoxelHeightStamp, Layer) == offsetof(FVoxelVolumeStamp, Layer));
	checkStatic(offsetof(FVoxelHeightStamp, AdditionalLayers) == offsetof(FVoxelVolumeStamp, AdditionalLayers));

	InitializationInfo.Layers.Add(reinterpret_cast<const FVoxelHeightStamp*>(&GetStamp())->Layer);
	InitializationInfo.Layers.Append(reinterpret_cast<const FVoxelHeightStamp*>(&GetStamp())->AdditionalLayers);

#if WITH_EDITOR
	INLINE_LAMBDA
	{
		if (!GIsEditor ||
			!InitializationInfo.bStampMutated)
		{
			return;
		}

		const UWorld* World = GetWorld().Resolve_Ensured();
		if (!World ||
			World->IsGameWorld())
		{
			return;
		}

		const FVoxelStampRef StampRef = WeakStampRef.Pin();
		if (!ensure(StampRef) ||
			// Stamp changed type since registration
			!ensure(StampRef.GetStruct() == GetStamp().GetStruct()))
		{
			return;
		}

		VOXEL_SCOPE_COUNTER("Copy stamp");

		FVoxelStamp& DestStamp = *StampRef;
		const FVoxelStamp& SourceStamp = GetStamp();

		bool bChanged = false;
		for (FProperty& Property : GetStructProperties(SourceStamp.GetStruct()))
		{
			if (Property.HasAllPropertyFlags(CPF_Transient) ||
				Property.Identical_InContainer(&SourceStamp, &DestStamp))
			{
				continue;
			}

			Property.CopyCompleteValue_InContainer(&DestStamp, &SourceStamp);
			bChanged = true;
		}

		if (bChanged)
		{
			Voxel::GameTask_Async([StampRef]
			{
				StampRef.OnRefreshDetails_Editor().Broadcast();
			});
		}
	};
#endif

	// No need to store the parameters further (reduces memory load)
	// Make sure to do this after Copy stamp above

	for (FStructProperty& Property : GetStructProperties<FStructProperty>(GetStamp().GetStruct()))
	{
		if (Property.Struct == StaticStructFast<FVoxelParameterOverrides>())
		{
			*Property.ContainerPtrToValuePtr<FVoxelParameterOverrides>(&ConstCast(GetStamp())) = {};
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if VOXEL_STATS
FName FVoxelStampRuntime::BuildStatName(
	const UScriptStruct& Struct,
	const TVoxelObjectPtr<USceneComponent> WeakComponent)
{
	VOXEL_FUNCTION_COUNTER();

	TStringBuilder<NAME_SIZE> Result;

	if (const USceneComponent* Component = WeakComponent.Resolve())
	{
		if (const AActor* Owner = Component->GetOwner())
		{
			if (Owner->GetActorLabelView().IsEmpty())
			{
				Owner->GetFName().AppendString(Result);
			}
			else
			{
				Result += Owner->GetActorLabelView();
			}
		}
		else
		{
			Result += TEXT("<null>");
		}

		Result += TEXT(".");
		Component->GetFName().AppendString(Result);
	}

	Result += TEXT(" ");
	Struct.GetFName().AppendString(Result);

	if (!ensure(Result.Len() < NAME_SIZE))
	{
		return {};
	}

	return FName(Result);
}
#endif

uint64 FVoxelStampRuntime::ComputeHash(const FVoxelStamp& Stamp)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelInlineArray<uint64, 64> Hashes;
	Hashes.Add(FVoxelUtilities::HashString(Stamp.GetStruct()->GetName()));

	for (const FProperty& Property : GetStructProperties(Stamp.GetStruct()))
	{
		Hashes.Add(FVoxelUtilities::HashProperty(Property, Property.ContainerPtrToValuePtr<void>(&Stamp)));
	}

	return FVoxelUtilities::MurmurHashView(Hashes);
}

const TVoxelMap<UScriptStruct*, UScriptStruct*>& FVoxelStampRuntime::GetStampToStampRuntime()
{
	static const TVoxelMap<UScriptStruct*, UScriptStruct*> StampToStampRuntime = INLINE_LAMBDA
	{
		VOXEL_FUNCTION_COUNTER();

		TVoxelMap<UScriptStruct*, UScriptStruct*> Result;
		Result.Reserve(16);

		for (UScriptStruct* Struct : GetDerivedStructs<FVoxelStamp>())
		{
			UScriptStruct* RuntimeStruct = FindObjectChecked<UScriptStruct>(Struct->GetOuter(), *(Struct->GetName() + "Runtime"));
			check(RuntimeStruct);
			Result.Add_CheckNew(Struct, RuntimeStruct);;
		}

		return Result;
	};

	return StampToStampRuntime;
}