// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelDebugActor.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "Surface/VoxelSurfaceTypeTable.h"

#include "TextureResource.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	Voxel::OnRefreshAll.AddLambda([]
	{
		ForEachObjectOfClass_Copy<AVoxelDebugActor>([&](AVoxelDebugActor& DebugActor)
		{
			DebugActor.Refresh();
		});
	});
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void FVoxelDebugActorState::Generate()
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelDoubleVectorBuffer Positions;
	Positions.Allocate(6 * Resolution * Resolution);

	for (int32 Face = 0; Face < 6; Face++)
	{
		for (int32 Y = 0; Y < Resolution; Y++)
		{
			for (int32 X = 0; X < Resolution; X++)
			{
				const int32 Index = FVoxelUtilities::Get2DIndex<int32>(
					FIntPoint(6 * Resolution, Resolution),
					Face * Resolution + X,
					Y);

				constexpr int32 Min = 0;
				constexpr int32 Max = Resolution - 1;

				const FVector IndexPosition = INLINE_LAMBDA
				{
					switch (Face)
					{
					default: VOXEL_ASSUME(false);
					case 0: return FVector(Min, X, Y);
					case 1: return FVector(Max, X, Y);
					case 2: return FVector(X, Min, Y);
					case 3: return FVector(X, Max, Y);
					case 4: return FVector(X, Y, Min);
					case 5: return FVector(X, Y, Max);
					}
				};

				const FVector Position = IndexToWorld.TransformPosition(IndexPosition);

				Positions.Set(Index, Position);
			}
		}
	}

	FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelDebugActorState"));

	const FVoxelQuery Query(
		LOD,
		*Layers,
		*SurfaceTypeTable,
		DependencyCollector);

	const FVoxelFloatBuffer Distances = Query.SampleVolumeLayer(WeakLayer, Positions);
	const float Scale = IndexToWorld.GetScaleVector().GetAbsMax();

	FVoxelUtilities::SetNumFast(Colors, 6 * Resolution * Resolution);

	for (int32 Index = 0; Index < 6 * Resolution * Resolution; Index++)
	{
		const float Distance = Distances[Index];

		if (FVoxelUtilities::IsNaN(Distance))
		{
			Colors[Index] = FColor::Purple;
			continue;
		}

		Colors[Index] = FVoxelUtilities::GetDistanceFieldColor(Distance / Scale / 100.f).ToFColor(false);
	}

	DependencyTracker = DependencyCollector.Finalize(nullptr, {});
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

AVoxelDebugActor::AVoxelDebugActor()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMesh(TEXT("/Engine/BasicShapes/Cube"));

	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	StaticMeshComponent->SetStaticMesh(StaticMesh.Object);

	RootComponent = StaticMeshComponent;

	SetActorScale3D(FVector(100.f));
}

void AVoxelDebugActor::Refresh()
{
	FutureState.Reset();
}

void AVoxelDebugActor::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

void AVoxelDebugActor::Tick(const float DeltaSeconds)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick(DeltaSeconds);

	LOD = FMath::Clamp(LOD, 0, 25);

	if (FutureState && !FutureState->IsComplete())
	{
		return;
	}

	const FMatrix IndexToWorld =
		FScaleMatrix(100.f / (FVoxelDebugActorState::Resolution - 1)) *
		FTranslationMatrix(FVector(-50.f)) *
		GetActorTransform().ToMatrixWithScale();

	const FVoxelWeakStackLayer WeakLayer = FVoxelWeakStackLayer(Layer);

	if (FutureState)
	{
		const FVoxelDebugActorState& State = FutureState->GetValueChecked();

		if (State.LOD == LOD &&
			State.IndexToWorld.Equals(IndexToWorld) &&
			State.WeakLayer == WeakLayer &&
			!State.IsInvalidated())
		{
			return;
		}
	}

	const TSharedRef<FVoxelDebugActorState> State = MakeShared<FVoxelDebugActorState>(
		FVoxelLayers::Get(GetWorld()),
		FVoxelSurfaceTypeTable::Get(),
		LOD,
		IndexToWorld,
		WeakLayer);

	FutureState =
		Voxel::AsyncTask([=]
		{
			State->Generate();
		})
		.Then_GameThread(MakeWeakObjectPtrLambda(this, [=, this]
		{
			return ApplyState(State);
		}))
		.Then_AnyThread([=]
		{
			return State;
		});
}

FVoxelFuture AVoxelDebugActor::ApplyState(const TSharedRef<FVoxelDebugActorState>& State)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!Material)
	{
		UMaterialInterface* ParentMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Debug/VoxelDebugActorMaterial"));
		if (!ensure(ParentMaterial))
		{
			return {};
		}

		Material = UMaterialInstanceDynamic::Create(ParentMaterial, nullptr);
		StaticMeshComponent->SetMaterial(0, Material);
	}

	if (!Texture)
	{
		Texture = FVoxelTextureUtilities::CreateTexture2D(
			"AVoxelDebugActor_Texture",
			FVoxelDebugActorState::Resolution * 6,
			FVoxelDebugActorState::Resolution,
			false,
			TF_Default,
			PF_B8G8R8A8);

		if (!ensure(Texture))
		{
			return {};
		}

		FVoxelTextureUtilities::RemoveBulkData(Texture);

		Material->SetTextureParameterValue("Texture", Texture);
	}

	FTextureResource* Resource = Texture->GetResource();
	if (!ensure(Resource))
	{
		return {};
	}

	return Voxel::RenderTask([State, Resource]
	{
		VOXEL_FUNCTION_COUNTER();

		FRHITexture* TextureRHI = Resource->GetTexture2DRHI();
		if (!ensure(TextureRHI))
		{
			return;
		}

		const FUpdateTextureRegion2D UpdateRegion(
			0,
			0,
			0,
			0,
			FVoxelDebugActorState::Resolution * 6,
			FVoxelDebugActorState::Resolution);

		RHIUpdateTexture2D_Safe(
			TextureRHI,
			0,
			UpdateRegion,
			FVoxelDebugActorState::Resolution * 6 * sizeof(FColor),
			State->GetColors().ReinterpretAs<uint8>());
	});
}