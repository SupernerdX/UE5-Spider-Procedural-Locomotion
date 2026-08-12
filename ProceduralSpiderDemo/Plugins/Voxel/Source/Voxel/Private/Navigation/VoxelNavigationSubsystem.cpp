// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Navigation/VoxelNavigationSubsystem.h"
#include "Navigation/VoxelNavigationMesh.h"
#include "Navigation/VoxelNavigationMesher.h"
#include "Navigation/VoxelNavigationInvoker.h"
#include "Navigation/VoxelNavigationMeshInterface.h"
#include "VoxelRuntime.h"
#include "VoxelTaskContext.h"
#include "NavMesh/NavMeshBoundsVolume.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, int32, GVoxelNavigationMaxChunks, 100000,
	"voxel.navigation.MaxChunks",
	"Max navigation chunks before erroring out");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNavigationSubsystem::LoadFromPrevious(FVoxelSubsystem& InPreviousSubsystem)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelNavigationSubsystem& PreviousSubsystem = CastStructChecked<FVoxelNavigationSubsystem>(InPreviousSubsystem);

	{
		PreviousChunkKeyToNavigationMesh = MoveTemp(PreviousSubsystem.ChunkKeyToNavigationMesh_RequiresLock);
		PreviousChunkKeyToNavigationComponent = MoveTemp(PreviousSubsystem.ChunkKeyToNavigationComponent);

		ChunkKeyToLastRenderTime = MoveTemp(PreviousSubsystem.ChunkKeyToLastRenderTime);

		ensure(PreviousSubsystem.PreviousChunkKeyToNavigationMesh.Num() == 0);
		ensure(PreviousSubsystem.PreviousChunkKeyToNavigationComponent.Num() == 0);
		ensure(PreviousSubsystem.ChunkKeyToLastRenderTime.Num() == 0);

		InvokerView = PreviousSubsystem.InvokerView;
		PreviousSubsystem.InvokerView.Reset();
	}

	for (auto It = PreviousChunkKeyToNavigationMesh.CreateIterator(); It; ++It)
	{
		if (It.Value()->DependencyTracker->IsInvalidated())
		{
			It.RemoveCurrent();
		}
	}

	if (PreviousSubsystem.GetConfig().LayerToRender != GetConfig().LayerToRender ||
		PreviousSubsystem.GetConfig().NavigationVoxelSize != GetConfig().NavigationVoxelSize ||
		PreviousSubsystem.GetConfig().NavigationChunkSize != GetConfig().NavigationChunkSize ||
		PreviousSubsystem.GetConfig().NavigationMeshComponentClass != GetConfig().NavigationMeshComponentClass)
	{
		PreviousChunkKeyToNavigationMesh.Empty();
	}

	if (PreviousSubsystem.GetConfig().NavigationVoxelSize != GetConfig().NavigationVoxelSize ||
		PreviousSubsystem.GetConfig().NavigationChunkSize != GetConfig().NavigationChunkSize ||
		!PreviousSubsystem.GetConfig().LocalToWorld.Equals(GetConfig().LocalToWorld))
	{
		InvokerView = nullptr;
	}
}

void FVoxelNavigationSubsystem::Initialize()
{
	VOXEL_FUNCTION_COUNTER();

	if (!InvokerView)
	{
		InvokerView = FVoxelNavigationInvokerManager::Get(GetConfig().World)->MakeView(
			GetConfig().NavigationChunkSize * GetConfig().NavigationVoxelSize,
			GetConfig().LocalToWorld);
	}

	if (GetConfig().bGenerateInsideNavMeshBounds)
	{
		UWorld* World = GetConfig().World.Resolve();
		if (!ensure(World))
		{
			return;
		}

		for (const ANavMeshBoundsVolume* NavMeshVolume : TActorRange<ANavMeshBoundsVolume>(World))
		{
			const FBox Box = NavMeshVolume->GetComponentsBoundingBox(true);
			NavMeshBounds.Add(FVoxelBox(Box).InverseTransformBy(GetConfig().LocalToWorld));
		}
	}
}

void FVoxelNavigationSubsystem::Compute()
{
	VOXEL_FUNCTION_COUNTER();

	if (!GetConfig().bEnableNavigation ||
		(GetConfig().bOnlyGenerateNavigationInEditor && !GetConfig().bIsEditorWorld))
	{
		return;
	}

	const TVoxelFuture<const TVoxelSet<FIntVector>> FutureChunkKeys = INLINE_LAMBDA
	{
		FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelNavigationSubsystem"));

		const TVoxelFuture<const TVoxelSet<FIntVector>> Result = GetTaskContext().Wrap(InvokerView->GetChunks(DependencyCollector));
		InvokerViewDependencyTracker = Finalize(DependencyCollector);
		return Result;
	};

	FutureChunkKeys.Then_AsyncThread([this](TSharedRef<const TVoxelSet<FIntVector>> ChunkKeys)
	{
		VOXEL_FUNCTION_COUNTER();

		if (NavMeshBounds.Num() > 0)
		{
			TVoxelSet<FIntVector> NewChunkKeys = *ChunkKeys;

			for (const FVoxelBox& Bounds : NavMeshBounds)
			{
				VOXEL_SCOPE_COUNTER("NavMeshBounds");

				const FVoxelIntBox IntBounds = FVoxelIntBox::FromFloatBox_WithPadding(Bounds.Scale(1. / GetConfig().NavigationVoxelSize));
				const int32 ChunkSize = GetConfig().NavigationChunkSize;

				const FIntVector Min = FVoxelUtilities::DivideFloor(IntBounds.Min, ChunkSize);
				const FIntVector Max = FVoxelUtilities::DivideCeil(IntBounds.Max, ChunkSize);

				const int64 MaxChildren =
					int64(Max.X - Min.X) *
					int64(Max.Y - Min.Y) *
					int64(Max.Z - Min.Z);

				NewChunkKeys.Reserve(NewChunkKeys.Num() + FMath::Min(MaxChildren, GVoxelNavigationMaxChunks));

				INLINE_LAMBDA
				{
					for (int32 X = Min.X; X < Max.X; X++)
					{
						for (int32 Y = Min.Y; Y < Max.Y; Y++)
						{
							for (int32 Z = Min.Z; Z < Max.Z; Z++)
							{
								if (NewChunkKeys.Num() > GVoxelNavigationMaxChunks)
								{
									VOXEL_MESSAGE(Warning, "{0}: More than {1} navigation chunks processed - throttling",
										GetConfig().VoxelWorldObject,
										GVoxelNavigationMaxChunks);

									return;
								}

								const FIntVector ChunkKey = FIntVector(X, Y, Z);
								NewChunkKeys.Add(ChunkKey);
							}
						}
					}
				};
			}

			ChunkKeys = MakeSharedCopy(MoveTemp(NewChunkKeys));
		}

		if (ChunkKeys->Num() == 0)
		{
			return;
		}

		ChunkKeyToNavigationMesh_RequiresLock.Reserve(ChunkKeys->Num());

		for (const FIntVector& ChunkKey : *ChunkKeys)
		{
			if (const TSharedPtr<FVoxelNavigationMesh> PreviousNavigationMesh = PreviousChunkKeyToNavigationMesh.FindRef(ChunkKey))
			{
				VOXEL_SCOPE_LOCK(CriticalSection);
				ChunkKeyToNavigationMesh_RequiresLock.Add_EnsureNew(ChunkKey, PreviousNavigationMesh);
				continue;
			}

			Voxel::AsyncTask([this, ChunkKey]
			{
				const TSharedRef<FVoxelNavigationMesher> Mesher = MakeShared<FVoxelNavigationMesher>(
					*this,
					GetConfig().LayerToRender,
					FVector(ChunkKey) * GetConfig().NavigationChunkSize * GetConfig().NavigationVoxelSize,
					GetConfig().NavigationChunkSize,
					GetConfig().NavigationVoxelSize);

				const TSharedRef<FVoxelNavigationMesh> Mesh = Mesher->CreateMesh();

				VOXEL_SCOPE_LOCK(CriticalSection);
				ChunkKeyToNavigationMesh_RequiresLock.Add_EnsureNew(ChunkKey, Mesh);
			});
		}
	});
}

void FVoxelNavigationSubsystem::Render(FVoxelRuntime& Runtime)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const double RenderTime = FPlatformTime::Seconds();

	ChunkKeyToNavigationComponent.Reserve(ChunkKeyToNavigationMesh_RequiresLock.Num());

	const UClass* NavigationMeshComponentClass = GetConfig().NavigationMeshComponentClass.Resolve();

	for (const auto& It : ChunkKeyToNavigationMesh_RequiresLock)
	{
		const TSharedRef<FVoxelNavigationMesh> Mesh = It.Value.ToSharedRef();
		if (Mesh->IsEmpty())
		{
			continue;
		}

		USceneComponent* Component = nullptr;
		{
			TVoxelObjectPtr<USceneComponent> WeakComponent;
			if (PreviousChunkKeyToNavigationComponent.RemoveAndCopyValue(It.Key, WeakComponent))
			{
				Component = WeakComponent.Resolve();
			}

			if (Component &&
				Component->GetClass() != NavigationMeshComponentClass)
			{
				if (IVoxelNavigationMeshInterface* Interface = Cast<IVoxelNavigationMeshInterface>(Component))
				{
					Interface->SetNavigationMesh(nullptr);
				}
				Runtime.RemoveComponent(Component);

				Component = nullptr;
			}
		}
		if (!Component)
		{
			if (!ensure(NavigationMeshComponentClass))
			{
				continue;
			}

			Component = Runtime.NewComponent(NavigationMeshComponentClass);
		}
		if (!ensure(Component))
		{
			continue;
		}

		IVoxelNavigationMeshInterface* NavigationMeshInterface = Cast<IVoxelNavigationMeshInterface>(Component);
		if (!ensure(NavigationMeshInterface))
		{
			continue;
		}

		ChunkKeyToNavigationComponent.Add_EnsureNew(It.Key, Component);
		ChunkKeyToLastRenderTime.FindOrAdd(It.Key) = RenderTime;

		if (NavigationMeshInterface->GetNavigationMesh() == Mesh)
		{
			// Already set
			continue;
		}

		Component->SetRelativeScale3D(FVector(GetConfig().NavigationVoxelSize));
		Component->SetRelativeLocation(FVector(It.Key) * GetConfig().NavigationChunkSize * GetConfig().NavigationVoxelSize);
		NavigationMeshInterface->SetNavigationMesh(Mesh);
	}

	struct FAdditionalChunk
	{
		double Time;
		FIntVector ChunkKey;
		USceneComponent* Component;
		TSharedPtr<FVoxelNavigationMesh> NavigationMesh;
	};
	TVoxelArray<FAdditionalChunk> AdditionalChunks;

	for (const auto& It : PreviousChunkKeyToNavigationComponent)
	{
		USceneComponent* Component = It.Value.Resolve();
		if (!ensureVoxelSlow(Component))
		{
			ensureVoxelSlow(ChunkKeyToLastRenderTime.Remove(It.Key));
			continue;
		}

		const bool bRemove = INLINE_LAMBDA
		{
			if (ChunkKeyToNavigationMesh_RequiresLock.Contains(It.Key))
			{
				// Already computed
				return true;
			}

			const TSharedPtr<FVoxelNavigationMesh> NavigationMesh = PreviousChunkKeyToNavigationMesh.FindRef(It.Key);
			if (!NavigationMesh ||
				NavigationMesh->DependencyTracker->IsInvalidated())
			{
				return true;
			}

			const double* Time = ChunkKeyToLastRenderTime.Find(It.Key);
			if (!ensureVoxelSlow(Time))
			{
				return true;
			}

			if (AdditionalChunks.Num() == 0)
			{
				AdditionalChunks.Reserve(PreviousChunkKeyToNavigationComponent.Num());
			}

			AdditionalChunks.Add(FAdditionalChunk
			{
				*Time,
				It.Key,
				Component,
				NavigationMesh
			});

			return false;
		};

		if (!bRemove)
		{
			continue;
		}

		if (IVoxelNavigationMeshInterface* Interface = Cast<IVoxelNavigationMeshInterface>(Component))
		{
			Interface->SetNavigationMesh(nullptr);
		}
		Runtime.RemoveComponent(Component);
		ensureVoxelSlow(ChunkKeyToLastRenderTime.Remove(It.Key));
	}
	PreviousChunkKeyToNavigationComponent.Empty();

	AdditionalChunks.Sort([](const FAdditionalChunk& A, const FAdditionalChunk& B)
	{
		// Most recent first
		return A.Time > B.Time;
	});

	int32 NumAdditionalChunks = 0;
	for (const FAdditionalChunk& AdditionalChunk : AdditionalChunks)
	{
		if (NumAdditionalChunks >= GetConfig().MaxAdditionalNavigationChunks)
		{
			if (IVoxelNavigationMeshInterface* Interface = Cast<IVoxelNavigationMeshInterface>(AdditionalChunk.Component))
			{
				Interface->SetNavigationMesh(nullptr);
			}
			Runtime.RemoveComponent(AdditionalChunk.Component);
			ensureVoxelSlow(ChunkKeyToLastRenderTime.Remove(AdditionalChunk.ChunkKey));
			continue;
		}
		NumAdditionalChunks++;

		ChunkKeyToNavigationMesh_RequiresLock.Add_EnsureNew(AdditionalChunk.ChunkKey, AdditionalChunk.NavigationMesh);
		ChunkKeyToNavigationComponent.Add_EnsureNew(AdditionalChunk.ChunkKey, AdditionalChunk.Component);
	}

	// Can't do this in compute as we use it above
	PreviousChunkKeyToNavigationMesh.Empty();
}