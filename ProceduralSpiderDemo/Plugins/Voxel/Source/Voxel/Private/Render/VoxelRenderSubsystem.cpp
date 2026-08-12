// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelRenderSubsystem.h"
#include "Render/VoxelRenderChunk.h"
#include "Render/VoxelMeshRenderProxy.h"
#include "Render/VoxelRenderInvokersManager.h"
#include "Render/VoxelRenderComponentCreator.h"
#include "Nanite/VoxelNaniteMesh.h"
#include "Nanite/VoxelNaniteMaterialRenderer.h"
#include "Collision/VoxelCollider.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "MegaMaterial/VoxelMegaMaterialRenderUtilities.h"
#include "VoxelMesher.h"
#include "VoxelCubicMesher.h"
#include "VoxelLayers.h"
#include "VoxelRenderTree.h"
#include "VoxelTaskContext.h"
#include "VoxelCellGenerator.h"
#include "VoxelTextureManager.h"
#include "VoxelQueryDebugDrawer.h"
#include "Materials/MaterialInstanceDynamic.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelShowInvalidatedChunks, false,
	"voxel.render.ShowInvalidatedChunks",
	"If true will show the bounds of invalidated chunks");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelMaterialInstanceRef> FVoxelRenderSubsystem::GetMaterialInstanceRef(const EVoxelMegaMaterialTarget Target) const
{
	return MaterialInstanceRefs[int32(Target)].ToSharedRef();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderSubsystem::LoadFromPrevious(FVoxelSubsystem& InPreviousSubsystem)
{
	FVoxelRenderSubsystem& PreviousSubsystem = CastStructChecked<FVoxelRenderSubsystem>(InPreviousSubsystem);

	TextureManager = PreviousSubsystem.TextureManager;
	NaniteMaterialRenderer = PreviousSubsystem.NaniteMaterialRenderer;
	InvokerView = PreviousSubsystem.InvokerView;

	ensure(!PreviousSubsystem.PreviousTextureManager);
	PreviousTextureManager = PreviousSubsystem.TextureManager;
	PreviousSubsystem.TextureManager = nullptr;

	ensure(!PreviousSubsystem.PreviousNaniteMaterialRenderer);
	PreviousNaniteMaterialRenderer = PreviousSubsystem.NaniteMaterialRenderer;
	PreviousSubsystem.NaniteMaterialRenderer = nullptr;

	RenderTree = PreviousSubsystem.RenderTree;
	PreviousSubsystem.RenderTree = nullptr;

	if (PreviousSubsystem.GetConfig().FeatureLevel != GetConfig().FeatureLevel ||

		PreviousSubsystem.GetConfig().VoxelSize != GetConfig().VoxelSize ||
		PreviousSubsystem.GetConfig().RenderChunkSize != GetConfig().RenderChunkSize ||
		PreviousSubsystem.GetConfig().MegaMaterialProxy != GetConfig().MegaMaterialProxy ||
		PreviousSubsystem.GetConfig().LayerToRender != GetConfig().LayerToRender ||
		PreviousSubsystem.GetConfig().bEnableNanite != GetConfig().bEnableNanite ||
		PreviousSubsystem.GetConfig().bEnableCubicMesher != GetConfig().bEnableCubicMesher ||
		PreviousSubsystem.GetConfig().MaxLOD != GetConfig().MaxLOD ||

		!FVoxelUtilities::BodyInstanceEqual(PreviousSubsystem.GetConfig().VisibilityCollision, GetConfig().VisibilityCollision) ||

		PreviousSubsystem.GetConfig().NaniteMaxTessellationLOD != GetConfig().NaniteMaxTessellationLOD ||
		PreviousSubsystem.GetConfig().DisplacementFade != GetConfig().DisplacementFade ||
		PreviousSubsystem.GetConfig().NanitePositionPrecision != GetConfig().NanitePositionPrecision ||
		PreviousSubsystem.GetConfig().bCompressNaniteVertices != GetConfig().bCompressNaniteVertices ||
		PreviousSubsystem.GetConfig().bUseNaniteBuilder != GetConfig().bUseNaniteBuilder ||

		PreviousSubsystem.GetConfig().bEnableLumen != GetConfig().bEnableLumen ||
		PreviousSubsystem.GetConfig().bEnableRaytracing != GetConfig().bEnableRaytracing ||
		PreviousSubsystem.GetConfig().bGenerateMeshDistanceFields != GetConfig().bGenerateMeshDistanceFields ||
		PreviousSubsystem.GetConfig().RuntimeVirtualTextures != GetConfig().RuntimeVirtualTextures ||
		PreviousSubsystem.GetConfig().BlockinessMetadata != GetConfig().BlockinessMetadata ||
		PreviousSubsystem.GetConfig().RaytracingMaxLOD != GetConfig().RaytracingMaxLOD ||
		PreviousSubsystem.GetConfig().MeshDistanceFieldMaxLOD != GetConfig().MeshDistanceFieldMaxLOD ||
		PreviousSubsystem.GetConfig().MeshDistanceFieldBias != GetConfig().MeshDistanceFieldBias ||
		PreviousSubsystem.GetConfig().MeshDistanceFieldBricksPerChunk != GetConfig().MeshDistanceFieldBricksPerChunk ||
		PreviousSubsystem.GetConfig().ComponentSettings != GetConfig().ComponentSettings)
	{
		if (RenderTree)
		{
			RenderTree->DestroyAllRenderDatas(*this);
			RenderTree = nullptr;
		}

		// No need to reset texture manager
		NaniteMaterialRenderer = nullptr;
	}

	if (InvokerView &&
		!InvokerView->Equal(*this, PreviousSubsystem))
	{
		InvokerView = nullptr;
	}

	if (PreviousSubsystem.GetConfig().MegaMaterialProxy == GetConfig().MegaMaterialProxy)
	{
		MaterialInstanceRefs = PreviousSubsystem.MaterialInstanceRefs;
	}

	if (TextureManager &&
		TextureManager->MetadataIndexToMetadata != GetConfig().MegaMaterialProxy->GetMetadataIndexToMetadata())
	{
		TextureManager.Reset();
	}
}
void FVoxelRenderSubsystem::Initialize()
{
	VOXEL_FUNCTION_COUNTER();

	for (const EVoxelMegaMaterialTarget Target : TEnumRange<EVoxelMegaMaterialTarget>())
	{
		UMaterialInterface* Material = GetConfig().MegaMaterialProxy->GetTargetMaterial(Target).Resolve();

		TSharedPtr<FVoxelMaterialInstanceRef>& MaterialInstanceRef = MaterialInstanceRefs[int32(Target)];
		if (MaterialInstanceRef)
		{
			if (const UMaterialInstanceDynamic* Instance = MaterialInstanceRef->GetInstance())
			{
				if (Instance->Parent != Material)
				{
					MaterialInstanceRef.Reset();
				}
			}
		}

		if (MaterialInstanceRef)
		{
			continue;
		}

		UMaterialInstanceDynamic* Instance = UMaterialInstanceDynamic::Create(Material, GetTransientPackage());
		check(Instance);
		MaterialInstanceRef = FVoxelMaterialInstanceRef::Make(*Instance);
	}

	if (!TextureManager)
	{
		TextureManager = MakeShared<FVoxelTextureManager>(*GetConfig().MegaMaterialProxy);
	}

	if (GetConfig().bEnableNanite &&
		!NaniteMaterialRenderer)
	{
		NaniteMaterialRenderer = MakeShared_Stats<FVoxelNaniteMaterialRenderer>(GetConfig().MegaMaterialProxy);
	}

	if (!InvokerView)
	{
		InvokerView = FVoxelRenderInvokersManager::Get(GetConfig().World)->MakeView(*this);
	}
}

void FVoxelRenderSubsystem::AddReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_FUNCTION_COUNTER();

	Super::AddReferencedObjects(Collector);

	if (TextureManager)
	{
		TextureManager->AddReferencedObjects(Collector);
	}

	if (PreviousTextureManager)
	{
		PreviousTextureManager->AddReferencedObjects(Collector);
	}
}

void FVoxelRenderSubsystem::Compute()
{
	VOXEL_FUNCTION_COUNTER();

	// This is needed as we could have a dependency tracker invalidated mid-update,
	// which would break subdivision logic
	if (RenderTree)
	{
		VOXEL_SCOPE_COUNTER("Copy invalidation");

		RenderTree->ForeachChunk([&](FVoxelRenderChunk& Chunk)
		{
			Chunk.bMeshInvalidated =
				Chunk.MeshDependencyTracker &&
				Chunk.MeshDependencyTracker->IsInvalidated();

			if (GVoxelShowInvalidatedChunks &&
				Chunk.bMeshInvalidated)
			{
				const FVoxelBox Bounds = Chunk.ChunkKey.GetBounds(
					GetConfig().RenderChunkSize,
					GetConfig().VoxelSize);

				FVoxelDebugDrawer()
				.LifeTime(0.5f)
				.Color(FLinearColor::Red)
				.DrawBox(Bounds, GetConfig().LocalToWorld);
			}
		});
	}

	if (!TryInitializeRootChunks())
	{
		if (RenderTree)
		{
			RenderTree->DestroyAllRenderDatas(*this);
			RenderTree = nullptr;
		}

		if (GetConfig().bEnableNanite)
		{
			NaniteMaterialRenderer->PrepareRender({});
		}

		return;
	}

	const TVoxelFuture<const FVoxelRenderInvokersContainer> FutureLODInvokers = INLINE_LAMBDA
	{
		FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelRenderSubsystem Invokers"));

		const TVoxelFuture<const FVoxelRenderInvokersContainer> Result = GetTaskContext().Wrap(InvokerView->GetInvokers(DependencyCollector));
		InvokersDependencyTracker = Finalize(DependencyCollector);
		return Result;
	};

	FutureLODInvokers.Then_AsyncThread([this](const FVoxelRenderInvokersContainer& InvokersContainer)
	{
		RenderTree->Update(*this, InvokersContainer);

		StartMeshingTasks().Then_AsyncThread([this]
		{
			StartRenderTasks().Then_AnyThread([this]
			{
				if (GetConfig().bEnableNanite)
				{
					Voxel::AsyncTask([this]
					{
						FinalizeRender_Nanite();
					});
				}
			});
		});
	});
}

void FVoxelRenderSubsystem::Render(FVoxelRuntime& Runtime)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelConfig& Config = GetConfig();

	// Do this before UpdateInstance in case we need to re-allocate the texture
	TextureManager->ProcessUploads();

	for (const TSharedPtr<FVoxelMaterialInstanceRef>& MaterialInstanceRef : MaterialInstanceRefs)
	{
		UMaterialInstanceDynamic* Instance = MaterialInstanceRef->GetInstance();
		if (!ensure(Instance))
		{
			continue;
		}

		TextureManager->UpdateInstance(*Instance);
	}

	if (NaniteMaterialRenderer)
	{
		NaniteMaterialRenderer->UpdateRender(*this, Config.LocalToWorld);
	}

	FVoxelRenderComponentCreator Creator(Runtime, *this);
	Creator.ProcessRenderDatasToDestroy(RenderDatasToDestroy);
	Creator.ProcessRenderDatasToRender(RenderDatasToRender);
	Creator.DestroyUnusedComponents();

	RenderDatasToRender.Empty();

	Voxel::AsyncTask([RenderDatasToDestroyRef = MakeSharedCopy(MoveTemp(RenderDatasToDestroy))]
	{
		VOXEL_SCOPE_COUNTER("Delete RenderDatasToDestroy");

		RenderDatasToDestroyRef->Empty();
	});

	// We're rendered, we can destroy the previous renderer
	PreviousTextureManager.Reset();
	PreviousNaniteMaterialRenderer.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelRenderSubsystem::TryInitializeRootChunks()
{
	VOXEL_FUNCTION_COUNTER();

	if (!InvokerView ||
		!InvokerView->HasInvokers())
	{
		return false;
	}

	int32 Depth;
	{
		FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelRenderSubsystem BoundsToGenerate"));

		BoundsToGenerate = GetLayers().GetBoundsToGenerate(
			GetConfig().LayerToRender,
			DependencyCollector);

		BoundsToGenerateDependencyTracker = Finalize(DependencyCollector);

		if (!BoundsToGenerate.IsValid())
		{
			return false;
		}

		const FVoxelBox ChunkBounds = BoundsToGenerate.GetBox() / GetConfig().VoxelSize / GetConfig().RenderChunkSize;

		double MaxSize = FMath::Max(ChunkBounds.Min.GetAbsMax(), ChunkBounds.Max.GetAbsMax());

		// We need to be able to do FVoxelChunkKey::GetParent on a root chunk for chunk subdivision logic to not be too messy
		// This requires max depth to be 29
		MaxSize = FMath::Min(MaxSize, 1 << 29);

		const FVoxelBox SafeChunkBounds = ChunkBounds.IntersectWith(FVoxelBox(-MaxSize, MaxSize));

		Depth = FMath::CeilLogTwo(FMath::CeilToInt(MaxSize));

		const int32 RootChunkSize = 1 << Depth;
		ensure(FVoxelBox(-RootChunkSize, RootChunkSize).Contains(SafeChunkBounds));
		ensure(!FVoxelBox(-RootChunkSize / 2, RootChunkSize / 2).Contains(SafeChunkBounds));
	}

	if (RenderTree &&
		RenderTree->Depth != Depth)
	{
		RenderTree->DestroyAllRenderDatas(*this);
		RenderTree = nullptr;
	}

	if (!RenderTree)
	{
		RenderTree = MakeShared<FVoxelRenderTree>(*this, Depth);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFuture FVoxelRenderSubsystem::StartMeshingTasks()
{
	const TConstVoxelArrayView<FVoxelChunkKey> ChunkKeysToRender = RenderTree->GetChunkKeysToRender();
	VOXEL_FUNCTION_COUNTER_NUM(ChunkKeysToRender.Num());

	TVoxelMap<FVoxelChunkKey, TVoxelInlineArray<int32, 16>> ChunkKeyXYToChunkKeyZ;
	ChunkKeyXYToChunkKeyZ.Reserve(ChunkKeysToRender.Num());

	for (const FVoxelChunkKey& ChunkKey : ChunkKeysToRender)
	{
		const TSharedRef<FVoxelRenderChunk> Chunk = RenderTree->GetChunk(ChunkKey);
		check(Chunk->Children.Num() == 0);

		if (Chunk->Mesh.IsSet() &&
			!Chunk->bMeshInvalidated)
		{
			continue;
		}

		if (Chunk->ChunkKey.LOD > GetConfig().MaxLOD)
		{
			Chunk->Mesh = nullptr;
			Chunk->MeshDependencyTracker = {};
			continue;
		}

		DestroyRenderData(Chunk->RenderData);

		FVoxelChunkKey ChunkKeyXY = ChunkKey;
		ChunkKeyXY.ChunkKey.Z = 0;
		ChunkKeyXYToChunkKeyZ.FindOrAdd(ChunkKeyXY).Add(ChunkKey.ChunkKey.Z);
	}

	TVoxelArray<FVoxelFuture> Futures;
	Futures.Reserve(ChunkKeyXYToChunkKeyZ.Num());

	for (auto& It : ChunkKeyXYToChunkKeyZ)
	{
		TVoxelArray<TSharedRef<FVoxelRenderChunk>> Chunks;
		Chunks.Reserve(It.Value.Num());

		// Not needed but makes ShowProcessedChunks nicer
		It.Value.Sort();

		for (const int32 ChunkKeyZ : It.Value)
		{
			FVoxelChunkKey ChunkKey = It.Key;
			ChunkKey.ChunkKey.Z = ChunkKeyZ;

			Chunks.Add_EnsureNoGrow(RenderTree->GetChunk(ChunkKey));
		}

		Futures.Add(Voxel::AsyncTask([this, ChunkKeyXY = It.Key, Chunks = MoveTemp(Chunks)]
		{
			const FVoxelConfig& Config = GetConfig();
			const int32 ChunkSize = Config.RenderChunkSize;

			const auto GetOffset = [&](const TSharedRef<FVoxelRenderChunk>& Chunk)
			{
				const FIntVector Delta = Chunk->ChunkKey.ChunkKey - ChunkKeyXY.ChunkKey;
				checkVoxelSlow(Delta.X == 0);
				checkVoxelSlow(Delta.Y == 0);
				checkVoxelSlow(Delta.Z % (1 << ChunkKeyXY.LOD) == 0);

				return FIntVector(0, 0, Delta.Z >> ChunkKeyXY.LOD) * ChunkSize;
			};

			FVoxelIntBox MaxBounds = FVoxelIntBox::InvertedInfinite;
			for (const TSharedRef<FVoxelRenderChunk>& Chunk : Chunks)
			{
				MaxBounds += FVoxelIntBox(-1, ChunkSize + 3).ShiftBy(GetOffset(Chunk));
			}

			TSharedPtr<const FVoxelCellGeneratorHeights> CachedHeights;

			for (const TSharedRef<FVoxelRenderChunk>& Chunk : Chunks)
			{
				FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelRenderChunk"));

				if (Config.bEnableCubicMesher)
				{
					FVoxelCubicMesher CubicMesher(
						GetLayers(),
						GetSurfaceTypeTable(),
						DependencyCollector,
						Config.LayerToRender,
						FInt64Vector3(Chunk->ChunkKey.ChunkKey) * ChunkSize,
						Config.VoxelSize,
						ChunkSize,
						Config.LocalToWorld,
						*Config.MegaMaterialProxy);

					// CreateMeshes returns masked mesh first (contains opaque+masked surfaces),
					// then translucent mesh if any surfaces use translucent blend mode
					TVoxelArray<TSharedPtr<FVoxelMesh>> Meshes = CubicMesher.CreateMeshes();
					Chunk->Mesh = Meshes.Num() > 0 ? Meshes[0] : nullptr;
					Chunk->TranslucentMesh = Meshes.Num() > 1 ? Meshes[1] : nullptr;
				}
				else
				{
					FVoxelMesher Mesher(
						GetLayers(),
						GetSurfaceTypeTable(),
						DependencyCollector,
						Config.LayerToRender,
						Chunk->ChunkKey.LOD,
						FInt64Vector3(Chunk->ChunkKey.ChunkKey) * ChunkSize,
						Config.VoxelSize,
						ChunkSize,
						Config.LocalToWorld,
						*Config.MegaMaterialProxy,
						Config.BlockinessMetadata,
						Config.bGenerateMeshDistanceFields && Chunk->ChunkKey.LOD <= Config.MeshDistanceFieldMaxLOD,
						Config.MeshDistanceFieldBias,
						Config.MeshDistanceFieldBricksPerChunk);

					Mesher.bQueryMetadata = true;

					Chunk->Mesh = Mesher.CreateMesh(CachedHeights);

					if (Mesher.CellGenerator)
					{
						CachedHeights = Mesher.CellGenerator->Heights;
					}
				}

				Chunk->MeshDependencyTracker = Finalize(DependencyCollector);
			}
		}));
	}

	// If we computed any mesh, schedule another pass to make sure we have no meshes left to subdivide
	// Can't directly use ChunkKeysToSubdivide as we might be waiting on parent chunks to compute for the first time
	// Never subdivide again if we reached max chunks (otherwise we get in a loop)
	bHasChunksToSubdivide = Futures.Num() > 0 && !RenderTree->bHasReachedMaxChunks;

	return FVoxelFuture(Futures);
}

FVoxelFuture FVoxelRenderSubsystem::StartRenderTasks()
{
	VOXEL_FUNCTION_COUNTER();

	const int32 MaxLOD = GetConfig().MaxLOD;
	const TConstVoxelArrayView<FVoxelChunkKey> ChunkKeysToRender = RenderTree->GetChunkKeysToRender();

	TVoxelChunkedArray<FVoxelFuture> Futures;

	const TSharedRef<FVoxelMaterialInstanceRef> NewMeshComponentMaterial = GetMaterialInstanceRef(EVoxelMegaMaterialTarget::NonNanite);

	for (const FVoxelChunkKey& ChunkKey : ChunkKeysToRender)
	{
		const TSharedRef<FVoxelRenderChunk> Chunk = RenderTree->GetChunk(ChunkKey);
		check(Chunk->Children.Num() == 0);
		check(Chunk->Mesh.IsSet());

		if (!Chunk->Mesh.GetValue())
		{
			DestroyRenderData(Chunk->RenderData);
			continue;
		}

		FVoxelChunkNeighborInfo NeighborInfo;
		for (int32 X = -1; X <= 1; X++)
		{
			for (int32 Y = -1; Y <= 1; Y++)
			{
				for (int32 Z = -1; Z <= 1; Z++)
				{
					const FVoxelRenderChunk* NeighborChunk = RenderTree->FindNeighbor(ChunkKey, X, Y, Z);
					if (!NeighborChunk ||
						NeighborChunk->ChunkKey.LOD > MaxLOD)
					{
						continue;
					}

					ensureVoxelSlow(NeighborChunk->ChunkKey.LOD - ChunkKey.LOD <= FVoxelMesher::MaxRelativeLOD);
					NeighborInfo.SetLOD(X, Y, Z, NeighborChunk->ChunkKey.LOD);
				}
			}
		}

		TSharedPtr<FVoxelRenderChunkData>& RenderData = Chunk->RenderData;

		if (RenderData &&
			RenderData->NeighborInfo == NeighborInfo)
		{
			if (RenderData->RenderProxy &&
				RenderData->MeshComponentMaterial != NewMeshComponentMaterial)
			{
				// Don't invalidate, but still re-render the chunk to update its material
				RenderDatasToRender.Add(RenderData);
			}

			continue;
		}

		DestroyRenderData(RenderData);

		RenderData = MakeShared<FVoxelRenderChunkData>(Chunk->ChunkKey, NeighborInfo);
		RenderDatasToRender.Add(RenderData);

		Futures.Add(Voxel::AsyncTask([this, Chunk, NeighborInfo]
		{
			return ProcessChunk(Chunk, NeighborInfo);
		}));
	}

	return FVoxelFuture(Futures);
}

void FVoxelRenderSubsystem::FinalizeRender_Nanite() const
{
	VOXEL_FUNCTION_COUNTER();
	check(GetConfig().bEnableNanite);

	const TConstVoxelArrayView<FVoxelChunkKey> ChunkKeysToRender = RenderTree->GetChunkKeysToRender();

	TVoxelSet<TSharedPtr<const FVoxelNaniteMesh>> NaniteMeshes;
	NaniteMeshes.Reserve(ChunkKeysToRender.Num());

	for (const FVoxelChunkKey& ChunkKey : ChunkKeysToRender)
	{
		const TSharedRef<FVoxelRenderChunk> Chunk = RenderTree->GetChunk(ChunkKey);
		check(Chunk->Children.Num() == 0);
		check(Chunk->Mesh.IsSet());

		if (!Chunk->RenderData ||
			!Chunk->RenderData->NaniteMesh)
		{
			continue;
		}

		NaniteMeshes.Add_EnsureNew(Chunk->RenderData->NaniteMesh);
	}

	NaniteMaterialRenderer->PrepareRender(MoveTemp(NaniteMeshes));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFuture FVoxelRenderSubsystem::ProcessChunk(
	const TSharedRef<FVoxelRenderChunk>& Chunk,
	const FVoxelChunkNeighborInfo& NeighborInfo) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelConfig& Config = GetConfig();

	const bool bRenderInBasePass = !Config.bEnableNanite;
	const bool bEnableLumen = Config.bEnableLumen;
	const bool bEnableRaytracing = Config.bEnableRaytracing && Chunk->ChunkKey.LOD <= Config.RaytracingMaxLOD;
	const bool bGenerateMeshDistanceField = Config.bGenerateMeshDistanceFields && Chunk->ChunkKey.LOD <= Config.MeshDistanceFieldMaxLOD;

	const bool bRenderNaniteMesh = Config.bEnableNanite;

	const bool bRenderVoxelMesh =
		bRenderInBasePass ||
		bEnableRaytracing ||
		bGenerateMeshDistanceField ||
		Config.RuntimeVirtualTextures.Num() > 0;

	const TSharedRef<const FVoxelMegaMaterialRenderData> MegaMaterialRenderData = FVoxelMegaMaterialRenderUtilities::BuildRenderData(
		*GetConfig().MegaMaterialProxy,
		TextureManager.ToSharedRef(),
		*Chunk->Mesh.GetValue());

	TVoxelInlineArray<FVoxelFuture, 3> Futures;

	if (Config.VisibilityCollision.GetCollisionEnabled() != ECollisionEnabled::NoCollision)
	{
		Futures.Add(Voxel::AsyncTask([=]
		{
			Chunk->RenderData->Collider = FVoxelCollider::Create(*Chunk->Mesh.GetValue());
		}));
	}

	if (bRenderNaniteMesh)
	{
		Futures.Add(Voxel::AsyncTask([=, this]
		{
			const TVoxelFuture<TSharedPtr<FVoxelNaniteMesh>> FutureNaniteMesh = FVoxelNaniteMesh::Create(
				*this,
				Chunk->Mesh->ToSharedRef(),
				MegaMaterialRenderData,
				NeighborInfo);

			return FutureNaniteMesh.Then_AnyThread([=, this](const TSharedPtr<FVoxelNaniteMesh>& NewNaniteMesh)
			{
				ensure(NewNaniteMesh);
				Chunk->RenderData->NaniteMesh = NewNaniteMesh;
			});
		}));
	}

	if (bRenderVoxelMesh)
	{
		Futures.Add(Voxel::AsyncTask([=, this]
		{
			const TSharedRef<FVoxelMeshRenderProxy> RenderProxy = Voxel::MakeShareable_RenderThread(new FVoxelMeshRenderProxy(
				Chunk->Mesh->ToSharedRef(),
				MegaMaterialRenderData,
				bRenderInBasePass,
				bEnableLumen,
				bEnableRaytracing,
				bGenerateMeshDistanceField,
				GetConfig().RuntimeVirtualTextures,
				NeighborInfo));

			return RenderProxy->Initialize_AsyncThread(*this).Then_AnyThread([=, this]
			{
				Chunk->RenderData->RenderProxy = RenderProxy;
			});
		}));
	}

	// Create render proxy for translucent mesh (cubic mesher only)
	if (bRenderVoxelMesh &&
		Chunk->TranslucentMesh)
	{
		Futures.Add(Voxel::AsyncTask([=, this]
		{
			const TSharedRef<const FVoxelMegaMaterialRenderData> TranslucentMaterialRenderData = FVoxelMegaMaterialRenderUtilities::BuildRenderData(
				*GetConfig().MegaMaterialProxy,
				TextureManager.ToSharedRef(),
				*Chunk->TranslucentMesh);

			const TSharedRef<FVoxelMeshRenderProxy> TranslucentRenderProxy = Voxel::MakeShareable_RenderThread(new FVoxelMeshRenderProxy(
				Chunk->TranslucentMesh.ToSharedRef(),
				TranslucentMaterialRenderData,
				bRenderInBasePass,
				bEnableLumen,
				false, // No raytracing for translucent
				false, // No distance fields for translucent
				{}, // No RVT for translucent
				NeighborInfo));

			return TranslucentRenderProxy->Initialize_AsyncThread(*this).Then_AnyThread([=, this]
			{
				Chunk->RenderData->TranslucentRenderProxy = TranslucentRenderProxy;
			});
		}));
	}

	return FVoxelFuture(Futures);
}