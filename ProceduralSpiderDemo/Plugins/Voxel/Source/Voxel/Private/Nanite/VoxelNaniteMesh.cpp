// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nanite/VoxelNaniteMesh.h"
#include "Render/VoxelTexturePool.h"
#include "Render/VoxelTextureManager.h"
#include "Render/VoxelRenderSubsystem.h"
#include "VoxelMesh.h"
#include "VoxelNaniteBuilder.h"
#include "MegaMaterial/VoxelMegaMaterialRenderUtilities.h"

#include "Engine/StaticMesh.h"
#include "Rendering/NaniteResources.h"

#if WITH_EDITOR
#include "NaniteBuilder.h"
#endif

DEFINE_VOXEL_COUNTER(STAT_VoxelNumNaniteMeshes);
DEFINE_VOXEL_COUNTER(STAT_VoxelNumNanitePages);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelNaniteMemory);
DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelNaniteMesh);

TVoxelFuture<TSharedPtr<FVoxelNaniteMesh>> FVoxelNaniteMesh::Create(
	const FVoxelRenderSubsystem& Subsystem,
	const TSharedRef<FVoxelMesh>& Mesh,
	const TSharedRef<const FVoxelMegaMaterialRenderData>& MegaMaterialRenderData,
	const FVoxelChunkNeighborInfo& NeighborInfo)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelNaniteMesh> Result = MakeShareable(new FVoxelNaniteMesh(Mesh, MegaMaterialRenderData, NeighborInfo));
	Subsystem.AddGCObject(Result);
	return Result->Initialize(Subsystem);
}

FVoxelNaniteMesh::~FVoxelNaniteMesh()
{
	Voxel::GameTask([WeakStaticMesh = WeakStaticMesh]
	{
		VOXEL_FUNCTION_COUNTER();

		UStaticMesh* StaticMeshToRelease = WeakStaticMesh.Resolve();
		if (!StaticMeshToRelease)
		{
			return;
		}

		StaticMeshToRelease->ReleaseResources();

		// Wait for render thread before doing SetRenderData(nullptr) otherwise FStreamingManager::Remove crashes
		Voxel::RenderTask([=]
		{
			Voxel::GameTask([=]
			{
				UStaticMesh* StaticMeshToDestroy = WeakStaticMesh.Resolve();
				if (!StaticMeshToDestroy)
				{
					return;
				}

				StaticMeshToDestroy->SetRenderData(nullptr);
				StaticMeshToDestroy->MarkAsGarbage();
			});
		});
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNaniteMesh::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(StaticMesh);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelNaniteMesh::FVoxelNaniteMesh(
	const TSharedRef<FVoxelMesh>& Mesh,
	const TSharedRef<const FVoxelMegaMaterialRenderData>& MegaMaterialRenderData,
	const FVoxelChunkNeighborInfo& NeighborInfo)
	: Mesh(Mesh)
	, MegaMaterialRenderData(MegaMaterialRenderData)
	, NeighborInfo(NeighborInfo)
{
}

TVoxelFuture<TSharedPtr<FVoxelNaniteMesh>> FVoxelNaniteMesh::Initialize(const FVoxelRenderSubsystem& Subsystem)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(Mesh->Indices.Num() > 0);

	const TSharedRef<TUniquePtr<FStaticMeshRenderData>> RenderDataRef = INLINE_LAMBDA
	{
		const TVoxelArray<FVector3f> Vertices = Mesh->GetDisplacedVertices(NeighborInfo);

		if (Subsystem.GetConfig().bUseNaniteBuilder)
		{
#if WITH_EDITOR
			FMeshNaniteSettings NaniteSettings = {};
			NaniteSettings.bEnabled = true;
			NaniteSettings.TargetMinimumResidencyInKB = 0;
			NaniteSettings.KeepPercentTriangles = 1.0f;
			NaniteSettings.TrimRelativeError = 0.0f;
			NaniteSettings.FallbackPercentTriangles = 1.0f;
			NaniteSettings.FallbackRelativeError = 0.0f;

			Nanite::IBuilderModule& NaniteBuilderModule = Nanite::IBuilderModule::Get();

			Nanite::IBuilderModule::FInputMeshData InputMeshData;
			InputMeshData.Vertices.Position = Vertices;

			InputMeshData.Vertices.TangentX.SetNum(Mesh->Normals.Num());
			InputMeshData.Vertices.TangentY.SetNum(Mesh->Normals.Num());
			InputMeshData.Vertices.TangentZ.SetNum(Mesh->Normals.Num());
			for (int32 Index = 0; Index < Mesh->Normals.Num(); Index++)
			{
				const FVector3f NewZ = Mesh->Normals[Index].GetUnitVector();

				const FVector3f UpVector = FMath::Abs(NewZ.Z) < 1.f - UE_KINDA_SMALL_NUMBER ? FVector3f::UpVector : FVector3f::ForwardVector;

				const FVector3f NewX = (UpVector ^ NewZ).GetSafeNormal();
				const FVector3f NewY = NewZ ^ NewX;

				InputMeshData.Vertices.TangentX[Index] = NewX;
				InputMeshData.Vertices.TangentY[Index] = NewY;
				InputMeshData.Vertices.TangentZ[Index] = NewZ;
			}
			InputMeshData.TriangleIndices = ReinterpretCastArray<uint32>(Mesh->Indices);
			InputMeshData.MaterialIndices.SetNumZeroed(Mesh->Indices.Num() / 3);
			InputMeshData.TriangleCounts = TArray<uint32>{ uint32(Mesh->Indices.Num() / 3) };
			InputMeshData.NumTexCoords = 0;

			const FVoxelBox Bounds = FVoxelBox::FromPositions(Vertices);
			InputMeshData.VertexBounds.Min = FVector3f(Bounds.Min);
			InputMeshData.VertexBounds.Max = FVector3f(Bounds.Max);

			Nanite::FResources Resources;
			if (!NaniteBuilderModule.Build(
				Resources,
				InputMeshData,
				nullptr, // OutFallbackMeshData
#if VOXEL_ENGINE_VERSION >= 506
				nullptr, // OutRayTracingFallbackMeshData
				nullptr, // RayTracingFallbackBuildSettings
#endif
				NaniteSettings
#if VOXEL_ENGINE_VERSION < 506
				, {}
#endif
				))
			{
				return MakeShared<TUniquePtr<FStaticMeshRenderData>>(nullptr);
			}

			TUniquePtr<FStaticMeshRenderData> RenderData = MakeUnique<FStaticMeshRenderData>();
			RenderData->Bounds = Bounds.ToFBox();
			RenderData->NumInlinedLODs = 1;
			RenderData->NaniteResourcesPtr = MakePimpl<Nanite::FResources>(MoveTemp(Resources));

			FStaticMeshLODResources* LODResource = new FStaticMeshLODResources();
			LODResource->bBuffersInlined = true;
			LODResource->Sections.Emplace();

			// Ensure UStaticMesh::HasValidRenderData returns true
			// Use MAX_flt to try to not have the vertex picked by vertex snapping
			const TVoxelArray<FVector3f> DummyPositions = { FVector3f(MAX_flt) };

			LODResource->VertexBuffers.StaticMeshVertexBuffer.Init(DummyPositions.Num(), 1);
			LODResource->VertexBuffers.PositionVertexBuffer.Init(DummyPositions);
			LODResource->VertexBuffers.ColorVertexBuffer.Init(DummyPositions.Num());

			// Ensure FStaticMeshRenderData::GetFirstValidLODIdx doesn't return -1
			LODResource->BuffersSize = 1;

			RenderData->LODResources.Add(LODResource);

			RenderData->LODVertexFactories.Add(FStaticMeshVertexFactories(GMaxRHIFeatureLevel));

			if (RenderData.IsValid())
			{
				FStaticMeshSectionArray& Sections = RenderData->LODResources[0].Sections;
				Sections.Reset();

				// Actual material we use for rendering
				Sections.Emplace_GetRef().MaterialIndex = 0;
				// World grid material
				Sections.Emplace_GetRef().MaterialIndex = 1;

				for (int32 Index = 0; Index < Mesh->UsedSurfaceTypes.Num(); Index++)
				{
					Sections.Emplace_GetRef().MaterialIndex = 2 + Index;
				}
			}

			return MakeSharedCopy(MoveTemp(RenderData));
#endif
		}

		FVoxelNaniteBuilder NaniteBuilder;
		NaniteBuilder.Mesh.Positions = Vertices;
		NaniteBuilder.Mesh.Normals = Mesh->Normals;
		NaniteBuilder.Mesh.Indices = Mesh->Indices;

		NaniteBuilder.PositionPrecision = Subsystem.GetConfig().NanitePositionPrecision;
		NaniteBuilder.bCompressVertices = Subsystem.GetConfig().bCompressNaniteVertices;
		NaniteBuilder.UniqueId = Subsystem.GetConfig().VoxelWorldId;
		NaniteBuilder.ChunkIndex = MegaMaterialRenderData->ChunkIndicesIndex;

		TUniquePtr<FStaticMeshRenderData> RenderData = NaniteBuilder.CreateRenderData();

		if (RenderData.IsValid())
		{
			FStaticMeshSectionArray& Sections = RenderData->LODResources[0].Sections;
			Sections.Reset();

			// Actual material we use for rendering
			Sections.Emplace_GetRef().MaterialIndex = 0;
			// World grid material
			Sections.Emplace_GetRef().MaterialIndex = 1;

			for (int32 Index = 0; Index < Mesh->UsedSurfaceTypes.Num(); Index++)
			{
				Sections.Emplace_GetRef().MaterialIndex = 2 + Index;
			}
		}
		return MakeSharedCopy(MoveTemp(RenderData));
	};

	if (!ensure(*RenderDataRef))
	{
		return nullptr;
	}

	NumNaniteMeshes = 1;
	NumNanitePages = (*RenderDataRef)->NaniteResourcesPtr->PageStreamingStates.Num();
	NaniteMemory = GetNaniteResourcesSize((*RenderDataRef)->NaniteResourcesPtr);

	return Voxel::GameTask(MakeStrongPtrLambda(this, [=, this]() -> TVoxelFuture<TSharedPtr<FVoxelNaniteMesh>>
	{
		StaticMesh = FVoxelNaniteBuilder::CreateStaticMesh(MoveTemp(*RenderDataRef));
		WeakStaticMesh = StaticMesh;

		if (!ensure(StaticMesh))
		{
			return nullptr;
		}

		// Add fake materials so that NumMaterials returns what we want in GetStreamingRenderAssetInfo
		{
			TArray<FStaticMaterial> StaticMaterials;
			// One for the actual material we use for rendering
			// One for the world grid material
			StaticMaterials.SetNum(2 + Mesh->UsedSurfaceTypes.Num());

			for (FStaticMaterial& StaticMaterial : StaticMaterials)
			{
				// Fix ensure in UStaticMesh::GetUVChannelData
				// Assume a default texel density of 100 for texture streaming
				StaticMaterial.UVChannelData = FMeshUVChannelInfo(100.f);
			}

			StaticMesh->SetStaticMaterials(StaticMaterials);
		}

		return Voxel::RenderTask(MakeStrongPtrLambda(this, [=, this]() -> TSharedPtr<FVoxelNaniteMesh>
		{
			const FStaticMeshRenderData* MeshRenderData = StaticMesh->GetRenderData();
			if (!ensure(MeshRenderData) ||
				!ensure(MeshRenderData->NaniteResourcesPtr))
			{
				return nullptr;
			}

			const int32 RootPageIndex = MeshRenderData->NaniteResourcesPtr->RootPageIndex;
			if (!ensure(RootPageIndex != -1))
			{
				return nullptr;
			}

			return AsShared();
		}));
	}));
}