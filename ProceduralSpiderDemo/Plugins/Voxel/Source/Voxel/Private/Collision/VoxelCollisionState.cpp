// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Collision/VoxelCollisionState.h"
#include "Collision/VoxelCollider.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "VoxelQuery.h"
#include "VoxelMesher.h"
#include "VoxelChaosTriangleMeshCooker.h"

#include "Chaos/TriangleMeshImplicitObject.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelCollisionChunk);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelCollisionChunkMemory);

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, float, GVoxelCollisionBakerSkipTolerance, 2,
	"voxel.CollisionBaker.SkipTolerance",
	"Increase this if some areas do not generate with the collision baker");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelCollisionBakerShowProcessedChunks, false,
	"voxel.CollisionBaker.ShowProcessedChunks",
	"Show chunks processed by the collision baker");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelCollisionState::~FVoxelCollisionState()
{
	VOXEL_FUNCTION_COUNTER();

	ChunkKeyToChunk.Empty();
}

FVoxelFuture FVoxelCollisionState::Update(
	const FVoxelWeakStackLayer& WeakLayer,
	const bool bCanLoad)
{
	VOXEL_FUNCTION_COUNTER();

	const double StartTime = FPlatformTime::Seconds();

	if (ShouldExit.Get())
	{
		return {};
	}

	const FVector Center = InvokerPosition / ChunkSize / VoxelSize;
	const double RadiusInChunks = InvokerRadius / ChunkSize / VoxelSize;

	// Offset due to chunk position being the chunk lower corner
	constexpr double ChunkOffset = 0.5;
	// We want to check the chunk against invoker, not the chunk center
	// To avoid a somewhat expensive box-to-point distance, we offset the invoker radius by the chunk diagonal
	// (from chunk center to any chunk corner)
	constexpr double ChunkHalfDiagonal = UE_SQRT_3 / 2.;

	const FIntVector Min = FVoxelUtilities::FloorToInt(Center - RadiusInChunks - ChunkOffset);
	const FIntVector Max = FVoxelUtilities::CeilToInt(Center + RadiusInChunks - ChunkOffset);
	const double RadiusSquared = FMath::Square(RadiusInChunks + ChunkHalfDiagonal);

	TVoxelChunkedArray<FIntVector> NewChunkKeys;
	{
		VOXEL_SCOPE_COUNTER("NewChunkKeys");

		for (int32 X = Min.X; X <= Max.X; X++)
		{
			for (int32 Y = Min.Y; Y <= Max.Y; Y++)
			{
				for (int32 Z = Min.Z; Z <= Max.Z; Z++)
				{
					if (Z % 1024 == 0 &&
						ShouldExit.Get(std::memory_order_relaxed))
					{
						return {};
					}

					const double DistanceSquared = (FVector(X, Y, Z) + ChunkOffset - Center).SizeSquared();
					if (DistanceSquared > RadiusSquared)
					{
						continue;
					}

					NewChunkKeys.Add(FIntVector(X, Y, Z));
				}
			}
		}
	}

	if (ShouldExit.Get())
	{
		return {};
	}

	if (LastWeakLayer == WeakLayer)
	{
		VOXEL_SCOPE_COUNTER("Remove chunks");

		TVoxelSet<FIntVector> ValidChunks;
		ValidChunks.Reserve(NewChunkKeys.Num());

		for (const FIntVector& Chunk : NewChunkKeys)
		{
			ValidChunks.Add(Chunk);
		}

		for (auto It = ChunkKeyToChunk.CreateIterator(); It; ++It)
		{
			if (!ValidChunks.Contains(It.Key()))
			{
				It.RemoveCurrent();
			}
		}
	}
	else
	{
		LastWeakLayer = WeakLayer;
		ChunkKeyToChunk.Empty();
	}

	ChunkKeyToChunk.Reserve(NewChunkKeys.Num());

	TVoxelArray<FVoxelFuture> Futures;
	Futures.Reserve(NewChunkKeys.Num());

	TVoxelChunkedArray<FIntVector> ChunkKeysToCompute;
	for (const FIntVector& ChunkKey : NewChunkKeys)
	{
		const TSharedPtr<FVoxelCollisionChunk> Chunk = ChunkKeyToChunk.FindOrAdd(ChunkKey);
		if (!Chunk ||
			!Chunk->DependencyTracker ||
			Chunk->DependencyTracker->IsInvalidated())
		{
			ChunkKeysToCompute.Add(ChunkKey);
			continue;
		}

		if (!Chunk->bNeedsLoad)
		{
			continue;
		}

		if (!bCanLoad)
		{
			// Never load from state in editor
			ChunkKeysToCompute.Add(ChunkKey);
			continue;
		}

		Chunk->bNeedsLoad = false;

		if (Chunk->Indices.Num() == 0)
		{
			// Nothing to do
			continue;
		}

		Futures.Add(Voxel::AsyncTask([Chunk]
		{
			const TRefCountPtr<Chaos::FTriangleMeshImplicitObject> TriangleMesh = FVoxelChaosTriangleMeshCooker::Create(
				Chunk->Indices,
				Chunk->Vertices,
				Chunk->FaceMaterials);

			if (ensureVoxelSlow(TriangleMesh))
			{
				ensure(!Chunk->Collider);
				Chunk->Collider = MakeShared<FVoxelCollider>(
					TriangleMesh,
					Chunk->SurfaceTypes);
			}

			Chunk->Indices.Empty();
			Chunk->Vertices.Empty();
			Chunk->FaceMaterials.Empty();
			Chunk->SurfaceTypes.Empty();
			Chunk->UpdateStats();
		}));
	}

	TotalNumTasks.Set(ChunkKeysToCompute.Num());

	for (const FIntVector& ChunkKey : ChunkKeysToCompute)
	{
		Futures.Add_EnsureNoGrow(Voxel::AsyncTask(MakeStrongPtrLambda(this, [this, WeakLayer, ChunkKey]
		{
			ChunkKeyToChunk[ChunkKey] = CreateChunk(WeakLayer, ChunkKey);
			NumTasks.Increment();
		})));
	}

	return
		FVoxelFuture(Futures)
		.Then_AnyThread(MakeStrongPtrLambda(this, [this, StartTime]
		{
			NumTasks.Set(0);
			TotalNumTasks.Set(0);

			LOG_VOXEL(Log, "Computing collision chunks took %s", *FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - StartTime))
		}));
}

void FVoxelCollisionState::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	while (!UpdateFuture.IsComplete())
	{
		LOG_VOXEL(Log, "Waiting for collision chunks");
		FPlatformProcess::Sleep(0.1f);
	}

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;

	if (!ensure(Version == FVersion::LatestVersion))
	{
		return;
	}

	TVoxelArray<FIntVector> ChunkKeys;
	if (Ar.IsSaving())
	{
		ChunkKeys = ChunkKeyToChunk.KeyArray();
	}

	Ar << ChunkKeys;

	if (Ar.IsLoading())
	{
		ChunkKeyToChunk.Reset();
		ChunkKeyToChunk.Reserve(ChunkKeys.Num());

		for (const FIntVector& ChunkKey : ChunkKeys)
		{
			ChunkKeyToChunk.Add_EnsureNew(ChunkKey);
		}
	}

	TVoxelArray<TObjectPtr<UVoxelSurfaceTypeAsset>> IndexToSurfaceType;
	TVoxelMap<TVoxelObjectPtr<UVoxelSurfaceTypeAsset>, int32> SurfaceTypeToIndex_Saving;
	if (Ar.IsSaving())
	{
		TVoxelSet<TVoxelObjectPtr<UVoxelSurfaceTypeAsset>> SurfaceTypeSet;
		for (const auto& It : ChunkKeyToChunk)
		{
			SurfaceTypeSet.Append(It.Value->SurfaceTypes);
		}

		for (const TVoxelObjectPtr<UVoxelSurfaceTypeAsset>& WeakSurfaceType : SurfaceTypeSet)
		{
			UVoxelSurfaceTypeAsset* SurfaceType = WeakSurfaceType.Resolve();
			if (!SurfaceType)
			{
				continue;
			}

			const int32 Index = IndexToSurfaceType.Add(SurfaceType);
			SurfaceTypeToIndex_Saving.Add_EnsureNew(WeakSurfaceType, Index);
		}
	}

	Ar << IndexToSurfaceType;

	for (auto& It : ChunkKeyToChunk)
	{
		if (Ar.IsLoading())
		{
			It.Value = MakeShared<FVoxelCollisionChunk>();
			It.Value->bNeedsLoad = true;
		}
		FVoxelCollisionChunk& Chunk = *It.Value;

		Ar << Chunk.Indices;
		Ar << Chunk.Vertices;
		Ar << Chunk.FaceMaterials;

		TVoxelInlineArray<int32, 32> SurfaceTypeIndices;
		if (Ar.IsSaving())
		{
			SurfaceTypeIndices.Reserve(Chunk.SurfaceTypes.Num());

			for (const TVoxelObjectPtr<UVoxelSurfaceTypeAsset>& SurfaceType : Chunk.SurfaceTypes)
			{
				if (const int32* IndexPtr = SurfaceTypeToIndex_Saving.Find(SurfaceType))
				{
					SurfaceTypeIndices.Add_EnsureNoGrow(*IndexPtr);
				}
				else
				{
					SurfaceTypeIndices.Add_EnsureNoGrow(-1);
				}
			}
		}

		Ar << SurfaceTypeIndices;

		if (Ar.IsLoading())
		{
			Chunk.SurfaceTypes.Reserve(SurfaceTypeIndices.Num());

			for (const int32 Index : SurfaceTypeIndices)
			{
				if (Index == -1)
				{
					Chunk.SurfaceTypes.Add_EnsureNoGrow(nullptr);
				}
				else
				{
					Chunk.SurfaceTypes.Add_EnsureNoGrow(IndexToSurfaceType[Index]);
				}
			}
		}

		Chunk.UpdateStats();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelCollisionChunk> FVoxelCollisionState::CreateChunk(
	const FVoxelWeakStackLayer& WeakLayer,
	const FIntVector& ChunkKey)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelCollisionChunk> Chunk = MakeShared<FVoxelCollisionChunk>();

	if (ShouldExit.Get())
	{
		return Chunk;
	}

	FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelCollisionChunk"));

	const bool bCanSkip = INLINE_LAMBDA
	{
		const FVoxelQuery Query(
			0,
			*Layers,
			*SurfaceTypeTable,
			DependencyCollector);

		const FVoxelFloatBuffer Distances = Query.SampleVolumeLayer(
			WeakLayer,
			(FVector(FInt64Vector3(ChunkKey) * ChunkSize) + ChunkSize / 4) * VoxelSize,
			FIntVector(2, 2, 2),
			VoxelSize * ChunkSize / 2);

		const FFloatInterval MinMax = FVoxelUtilities::GetMinMaxSafe(Distances.View());
		if (!MinMax.IsValid())
		{
			// Full of NaN
			return true;
		}

		if (MinMax.Contains(0))
		{
			return false;
		}

		if (FMath::Min(FMath::Abs(MinMax.Min), FMath::Abs(MinMax.Max)) > (DOUBLE_UE_SQRT_3 + GVoxelCollisionBakerSkipTolerance) * ChunkSize * VoxelSize)
		{
			return true;
		}

		return false;
	};

	if (GVoxelCollisionBakerShowProcessedChunks)
	{
		const FVoxelBox Bounds = FVoxelIntBox(ChunkKey).ToVoxelBox().Scale(ChunkSize * VoxelSize);

		FVoxelDebugDrawer()
		.Color(bCanSkip ? FColor::Green : FColor::Red)
		.DrawBox(Bounds, FTransform::Identity);
	}

	if (bCanSkip)
	{
		return Chunk;
	}

	FVoxelMesher Mesher(
		*Layers,
		*SurfaceTypeTable,
		DependencyCollector,
		WeakLayer,
		0,
		FInt64Vector3(ChunkKey) * ChunkSize,
		VoxelSize,
		ChunkSize,
		FTransform(),
		*MegaMaterialProxy,
		{},
		false,
		0.f,
		0);

	Mesher.bQueryMetadata = false;

	const TSharedPtr<FVoxelMesh> Mesh = Mesher.CreateMesh(nullptr);

	Chunk->DependencyTracker = DependencyCollector.Finalize(nullptr, MakeWeakPtrLambda(this, [this](const FVoxelInvalidationCallstack& Callstack)
	{
		Invalidated.Set(true);
	}));

	if (!Mesh)
	{
		return Chunk;
	}

	TVoxelArray<uint16> FaceMaterials;
	TVoxelArray<TVoxelObjectPtr<UVoxelSurfaceTypeAsset>> SurfaceTypes;
	FVoxelCollider::BuildSurfaceTypes(
		*Mesh,
		FaceMaterials,
		SurfaceTypes);

	const TRefCountPtr<Chaos::FTriangleMeshImplicitObject> TriangleMesh = FVoxelChaosTriangleMeshCooker::Create(
		Mesh->Indices,
		Mesh->Vertices,
		FaceMaterials);

	if (!ensureVoxelSlow(TriangleMesh))
	{
		return Chunk;
	}

	Chunk->Indices = Mesh->Indices;
	Chunk->Vertices = Mesh->Vertices;
	Chunk->FaceMaterials = FaceMaterials;
	Chunk->SurfaceTypes = SurfaceTypes;

	Chunk->Collider = MakeShared<FVoxelCollider>(
		TriangleMesh,
		SurfaceTypes);

	Chunk->UpdateStats();

	return Chunk;
}