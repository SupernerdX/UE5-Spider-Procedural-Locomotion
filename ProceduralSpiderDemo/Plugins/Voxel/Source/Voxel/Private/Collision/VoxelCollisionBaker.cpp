// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Collision/VoxelCollisionBaker.h"
#include "Collision/VoxelCollisionState.h"
#include "Collision/VoxelCollisionComponent.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "VoxelLayers.h"

#include "SceneView.h"
#include "SceneManagement.h"
#include "PrimitiveSceneProxy.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	Voxel::OnRefreshAll.AddLambda([]
	{
		ForEachObjectOfClass_Copy<AVoxelCollisionBaker>([&](AVoxelCollisionBaker& Baker)
		{
			Baker.DestroyState();
		});
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelCollisionBakerRootComponent::UVoxelCollisionBakerRootComponent()
{
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UVoxelCollisionBakerRootComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

FPrimitiveSceneProxy* UVoxelCollisionBakerRootComponent::CreateSceneProxy()
{
	if (UE_BUILD_SHIPPING)
	{
		return nullptr;
	}

	class FSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		const float Radius;

		explicit FSceneProxy(const UVoxelCollisionBakerRootComponent* Component)
			: FPrimitiveSceneProxy(Component)
			, Radius(CastChecked<AVoxelCollisionBaker>(Component->GetOwner())->Radius)
		{
		}

		virtual void GetDynamicMeshElements(
			const TArray<const FSceneView*>& Views,
			const FSceneViewFamily& ViewFamily,
			const uint32 VisibilityMap,
			FMeshElementCollector& Collector) const override
		{
			VOXEL_FUNCTION_COUNTER();

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (!(VisibilityMap & (1 << ViewIndex)))
				{
					continue;
				}

				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				const FSceneView* View = Views[ViewIndex];
				const FMatrix LocalToWorld = GetLocalToWorld();

				const FLinearColor Color = GetViewSelectionColor(
					FColor::White,
					*View,
					IsSelected(),
					IsHovered(),
					false,
					IsIndividuallySelected());

				DrawWireSphere(
					PDI,
					LocalToWorld.GetOrigin(),
					Color,
					Radius,
					64,
					SDPG_World);
			}
		}
		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = !View->Family->EngineShowFlags.Game && IsSelected();
			Result.bDynamicRelevance = true;
			return Result;
		}
		virtual SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}
		virtual uint32 GetMemoryFootprint() const override
		{
			return sizeof(*this) + GetAllocatedSize();
		}
	};
	return new FSceneProxy(this);
}

FBoxSphereBounds UVoxelCollisionBakerRootComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	const AVoxelCollisionBaker* Owner = Cast<AVoxelCollisionBaker>(GetOwner());
	if (!Owner)
	{
		return {};
	}

	return FBoxSphereBounds(FVector::ZeroVector, FVector(Owner->Radius), Owner->Radius).TransformBy(LocalToWorld);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelCollisionBakerData::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);

	if (Ar.IsTransacting())
	{
		return;
	}

	int32 Version = 0;
	Ar << Version;
	if (!ensure(Version == 0))
	{
		return;
	}

	int32 VoxelSize = 0;
	int32 ChunkSize = 0;
	if (State)
	{
		VoxelSize = State->VoxelSize;
		ChunkSize = State->ChunkSize;
	}
	Ar << VoxelSize;
	Ar << ChunkSize;

	if (Ar.IsLoading() ||
		!State)
	{
		State = MakeShared<FVoxelCollisionState>(
			VoxelSize,
			ChunkSize);
	}

	State->Serialize(Ar);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelCollisionBaker::AVoxelCollisionBaker()
{
	RootComponent = CreateDefaultSubobject<UVoxelCollisionBakerRootComponent>("RootComponent");
	Data = CreateDefaultSubobject<UVoxelCollisionBakerData>("Data");

	PrimaryActorTick.bCanEverTick = true;
}

void AVoxelCollisionBaker::DestroyState()
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(Data))
	{
		return;
	}

	TSharedPtr<FVoxelCollisionState>& State = Data->State;
	if (!State)
	{
		return;
	}

	State->ShouldExit.Set(true);
	State.Reset();
}

void AVoxelCollisionBaker::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

void AVoxelCollisionBaker::BeginPlay()
{
	VOXEL_FUNCTION_COUNTER();

	Super::BeginPlay();

	if (!ensure(Data))
	{
		return;
	}

	TSharedPtr<FVoxelCollisionState>& State = Data->State;
	if (!State)
	{
		return;
	}

	State->Layers = FVoxelLayers::Get(GetWorld());
	State->SurfaceTypeTable = FVoxelSurfaceTypeTable::Get();
	State->MegaMaterialProxy = FVoxelMegaMaterialProxy::Default();
	State->InvokerPosition = GetActorLocation();
	State->InvokerRadius = Radius;
	State->UpdateFuture = Voxel::AsyncTask([State = State, WeakLayer = FVoxelWeakStackLayer(Layer)]
	{
		return State->Update(WeakLayer, true);
	});

	while (!State->UpdateFuture.IsComplete())
	{
		LOG_VOXEL(Log, "Waiting for collision chunks: %d/%d",
			State->NumTasks.Get(),
			State->TotalNumTasks.Get());

		FPlatformProcess::Sleep(0.01f);
	}

	Render(State->ChunkKeyToChunk);
}

void AVoxelCollisionBaker::BeginDestroy()
{
	VOXEL_FUNCTION_COUNTER();

	DestroyState();

	Super::BeginDestroy();
}

void AVoxelCollisionBaker::Destroyed()
{
	VOXEL_FUNCTION_COUNTER();

	DestroyState();

	Super::Destroyed();
}

void AVoxelCollisionBaker::PostLoad()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostLoad();

	Collision.LoadProfileData(false);
}

void AVoxelCollisionBaker::Tick(const float DeltaSeconds)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick(DeltaSeconds);

	if (!bGenerate)
	{
		return;
	}

	if (!ensure(Data))
	{
		return;
	}
	TSharedPtr<FVoxelCollisionState>& State = Data->State;

	const auto Compute = [&]
	{
		ensure(State->UpdateFuture.IsComplete());

		State->Invalidated.Set(false);
		State->bIsRendered = false;
		State->Layers = FVoxelLayers::Get(GetWorld());
		State->SurfaceTypeTable = FVoxelSurfaceTypeTable::Get();
		State->MegaMaterialProxy = FVoxelMegaMaterialProxy::Default();
		State->InvokerPosition = GetActorLocation();
		State->InvokerRadius = Radius;
		State->UpdateFuture = Voxel::AsyncTask([
			State = State,
			WeakLayer = FVoxelWeakStackLayer(Layer),
			bCanLoad = !GIsEditor || FVoxelUtilities::IsPlayInEditor()]
		{
			return State->Update(WeakLayer, bCanLoad);
		});
	};

	if (!State ||
		State->VoxelSize != VoxelSize ||
		State->ChunkSize != ChunkSize ||
		State->InvokerRadius > 10 * Radius)
	{
		DestroyState();

		State = MakeShared<FVoxelCollisionState>(
			VoxelSize,
			ChunkSize);

		Compute();
	}

	if (!State->UpdateFuture.IsComplete())
	{
		GEngine->AddOnScreenDebugMessage(
			uint64(this),
			0.1f,
			FColor::White,
			FString::Printf(TEXT("Updating %s: %d/%d chunks using %d worker threads"),
				*GetActorNameOrLabel(),
				State->NumTasks.Get(),
				State->TotalNumTasks.Get(),
				FVoxelUtilities::GetNumBackgroundWorkerThreads()));

		return;
	}

	if (!State->bIsRendered)
	{
		Render(State->ChunkKeyToChunk);
		State->bIsRendered = true;
	}

	if (State->InvokerPosition != GetActorLocation() ||
		State->InvokerRadius != Radius)
	{
		State->Invalidated.Set(true);
	}

	if (State->Invalidated.Get())
	{
		Compute();
	}
}

bool AVoxelCollisionBaker::ShouldTickIfViewportsOnly() const
{
	return true;
}

void AVoxelCollisionBaker::Render(const TVoxelMap<FIntVector, TSharedPtr<FVoxelCollisionChunk>>& ChunkKeyToChunk)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelChunkedArray<UVoxelCollisionComponent*> ComponentsToDestroy;

	for (auto It = ChunkKeyToComponent.CreateIterator(); It; ++It)
	{
		if (ChunkKeyToChunk.Contains(It.Key()))
		{
			continue;
		}

		if (UVoxelCollisionComponent* Component = It.Value().Resolve())
		{
			ComponentsToDestroy.Add(Component);
		}

		It.RemoveCurrent();
	}

	for (const auto& It : ChunkKeyToChunk)
	{
		if (!It.Value->Collider)
		{
			TVoxelObjectPtr<UVoxelCollisionComponent> WeakComponent;
			ChunkKeyToComponent.RemoveAndCopyValue(It.Key, WeakComponent);

			if (UVoxelCollisionComponent* Component = WeakComponent.Resolve())
			{
				Component->DestroyComponent();
			}

			continue;
		}

		UVoxelCollisionComponent* Component = ChunkKeyToComponent.FindRef(It.Key).Resolve();
		if (!Component)
		{
			if (ComponentsToDestroy.Num() > 0)
			{
				Component = ComponentsToDestroy.Pop();
			}

			if (!Component)
			{
				Component = NewObject<UVoxelCollisionComponent>(
					RootComponent,
					NAME_None,
					RF_Transient |
					RF_DuplicateTransient |
					RF_TextExportTransient);

				if (!ensure(Component))
				{
					continue;
				}

				Component->SetupAttachment(RootComponent);
				Component->RegisterComponent();
			}

			ChunkKeyToComponent.FindOrAdd(It.Key) = Component;
		}

		Component->SetCollider(
			It.Value->Collider.ToSharedRef(),
			Collision,
			bDoubleSidedCollision,
			bGenerateOverlapEvents,
			FTransform(
				FQuat::Identity,
				FVector(It.Key) * ChunkSize * VoxelSize,
				FVector(VoxelSize)));
	}

	for (UVoxelCollisionComponent* Component : ComponentsToDestroy)
	{
		Component->DestroyComponent();
	}
}