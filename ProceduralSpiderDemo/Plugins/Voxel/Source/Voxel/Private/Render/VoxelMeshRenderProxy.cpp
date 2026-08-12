// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelMeshRenderProxy.h"
#include "Render/VoxelVertexFactory.h"
#include "Render/VoxelRenderSubsystem.h"
#include "MegaMaterial/VoxelMegaMaterialRenderUtilities.h"
#include "VoxelMesh.h"
#include "VoxelDistanceFieldWrapper.h"
#include "RayTracingGeometryManagerInterface.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, int32, GVoxelMegaMaterialDebugMode, 0,
	"voxel.MegaMaterial.DebugMode",
	"1: shows the number of layers queried per pixel\n"
	"2: shows the number of neighbors queried per pixel to enable DDX/DDY\n"
	"3: shows dither bias",
	Voxel::RefreshAll,
	[]
	{
		if (GVoxelMegaMaterialDebugMode == 1)
		{
			static const TVoxelArray<FLinearColor> Colors =
			{
				FLinearColor(1.0, 1.0, 1.0),
				FLinearColor(0.0, 1.0, 0.0),
				FLinearColor(0.0, 1.0, 1.0),
				FLinearColor(1.0, 1.0, 0.0),
				FLinearColor(1.0, 0.5, 0.0),
				FLinearColor(1.0, 0.0, 0.0),
				FLinearColor(1.0, 0.0, 1.0),
				FLinearColor(0.5, 0.0, 1.0),
				FLinearColor(0.0, 0.0, 1.0),
				FLinearColor(0.0, 0.0, 0.0)
			};

			for (int32 Index = 1; Index < 9; Index++)
			{
				GEngine->AddOnScreenDebugMessage(
					FVoxelUtilities::MurmurHash(&GVoxelMegaMaterialDebugMode) ^ FVoxelUtilities::MurmurHash(Index),
					2 * FApp::GetDeltaTime(),
					Colors[Index].ToFColor(true),
					FString::FromInt(Index) + " layers queried");
			}
		}

		if (GVoxelMegaMaterialDebugMode == 2)
		{
			static const TVoxelArray<FLinearColor> Colors =
			{
				FLinearColor(0, 1, 0),
				FLinearColor(1, 1, 0),
				FLinearColor(1, 0, 0),
				FLinearColor(1, 0, 1),
			};

			for (int32 Index = 0; Index < 4; Index++)
			{
				GEngine->AddOnScreenDebugMessage(
					FVoxelUtilities::MurmurHash(&GVoxelMegaMaterialDebugMode) ^ FVoxelUtilities::MurmurHash(Index),
					2 * FApp::GetDeltaTime(),
					Colors[Index].ToFColor(true),
					FString::FromInt(Index) + " neighbors queried");
			}
		}
	});

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelMeshGpuMemory);
DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelMeshRenderProxy);

FVoxelMeshRenderProxy::FVoxelMeshRenderProxy(
	const TSharedRef<const FVoxelMesh>& Mesh,
	const TSharedRef<const FVoxelMegaMaterialRenderData>& MegaMaterialRenderData,
	const bool bRenderInBasePass,
	const bool bEnableLumen,
	const bool bEnableRaytracing,
	const bool bEnableMeshDistanceField,
	const TVoxelArray<TVoxelObjectPtr<URuntimeVirtualTexture>>& RuntimeVirtualTextures,
	const FVoxelChunkNeighborInfo& NeighborInfo)
	: Mesh(Mesh)
	, MegaMaterialRenderData(MegaMaterialRenderData)
	, bRenderInBasePass(bRenderInBasePass)
	, bEnableLumen(bEnableLumen)
	, bEnableRaytracing(bEnableRaytracing)
	, bEnableMeshDistanceField(bEnableMeshDistanceField)
	, RuntimeVirtualTextures(RuntimeVirtualTextures)
{
	VOXEL_FUNCTION_COUNTER();

	Vertices = TVoxelArray<FVector4f>(Mesh->GetDisplacedVertices(NeighborInfo));
}

FVoxelMeshRenderProxy::~FVoxelMeshRenderProxy()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInRenderingThread());

	if (IndicesBuffer)
	{
		IndicesBuffer->ReleaseResource();
		IndicesBuffer.Reset();
	}
	if (IndicesSRV)
	{
		IndicesSRV->SafeRelease();
		IndicesSRV.Reset();
	}
	if (VerticesBuffer)
	{
		VerticesBuffer->ReleaseResource();
		VerticesBuffer.Reset();
	}
	if (NormalsBuffer)
	{
		NormalsBuffer->ReleaseResource();
		NormalsBuffer.Reset();
	}
	if (VertexFactory)
	{
		VertexFactory->ReleaseResource();
		VertexFactory.Reset();
	}

#if RHI_RAYTRACING
	if (RayTracingGeometryGroupHandle != -1)
	{
		GRayTracingGeometryManager->ReleaseRayTracingGeometryGroup(RayTracingGeometryGroupHandle);
		RayTracingGeometryGroupHandle = -1;
	}

	if (RayTracingGeometry)
	{
		RayTracingGeometry->ReleaseResource();
		RayTracingGeometry.Reset();
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int64 FVoxelMeshRenderProxy::GetAllocatedSize() const
{
	int64 AllocatedSize = 0;

	if (IndicesBuffer &&
		IndicesBuffer->IndexBufferRHI)
	{
		AllocatedSize += IndicesBuffer->IndexBufferRHI->GetSize();
	}

	if (VerticesBuffer &&
		VerticesBuffer->VertexBufferRHI)
	{
		AllocatedSize += VerticesBuffer->VertexBufferRHI->GetSize();
	}

	if (NormalsBuffer &&
		NormalsBuffer->VertexBufferRHI)
	{
		AllocatedSize += NormalsBuffer->VertexBufferRHI->GetSize();
	}

	if (CardRepresentationData)
	{
		AllocatedSize += CardRepresentationData->GetResourceSizeBytes();
	}

	if (DistanceFieldVolumeData)
	{
		AllocatedSize += DistanceFieldVolumeData->GetResourceSizeBytes();
	}


	return AllocatedSize;
}

FVoxelFuture FVoxelMeshRenderProxy::Initialize_AsyncThread(const FVoxelRenderSubsystem& Subsystem)
{
	VOXEL_FUNCTION_COUNTER();

	if (bEnableMeshDistanceField)
	{
		if (ensure(Mesh->DistanceFieldsData))
		{
			DistanceFieldVolumeData = Mesh->DistanceFieldsData;
		}
	}
	else
	{
		ensure(!Mesh->DistanceFieldsData);
	}

	// Always generate - bit of a waste but shouldn't be a bottleneck
	{
		VOXEL_SCOPE_COUNTER("Cards");

		// TODO IMPROVE
		// Only create additional cards if needed (in most cases we just need one above the chunk, not 6 all around!)
		// Generate additional cards to handle self-shadowing

		CardRepresentationData = MakeUnique<FCardRepresentationData>();
		CardRepresentationData->MeshCardsBuildData.Bounds = Mesh->Bounds.ToFBox();

		for (int32 Index = 0; Index < 6; Index++)
		{
			FLumenCardBuildData& CardBuildData = CardRepresentationData->MeshCardsBuildData.CardBuildData.Emplace_GetRef();

			const uint32 AxisIndex = Index / 2;
			FVector3f Direction(0.0f, 0.0f, 0.0f);
			Direction[AxisIndex] = Index & 1 ? 1.0f : -1.0f;

			CardBuildData.OBB.AxisZ = Direction;
			CardBuildData.OBB.AxisZ.FindBestAxisVectors(CardBuildData.OBB.AxisX, CardBuildData.OBB.AxisY);
			CardBuildData.OBB.AxisX = FVector3f::CrossProduct(CardBuildData.OBB.AxisZ, CardBuildData.OBB.AxisY);
			CardBuildData.OBB.AxisX.Normalize();

			CardBuildData.OBB.Origin = FVector3f(Mesh->Bounds.GetCenter());
			CardBuildData.OBB.Extent = CardBuildData.OBB.RotateLocalToCard(FVector3f(Mesh->Bounds.GetExtent()) + FVector3f(1.0f)).GetAbs();

			CardBuildData.AxisAlignedDirectionIndex = Index;
		}
	}

	return Voxel::RenderTask(MakeStrongPtrLambda(this, [this, &Subsystem](FRHICommandListBase& RHICmdList)
	{
		Initialize_RenderThread(RHICmdList, Subsystem);
	}));
}

void FVoxelMeshRenderProxy::InitializeVertexFactory_RenderThread(FRHICommandListBase& RHICmdList)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(!bIsFinalized);
	bIsFinalized = true;

	VOXEL_SCOPE_COUNTER("VertexFactory InitResource");
	VertexFactory->InitResource(RHICmdList);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMeshRenderProxy::Initialize_RenderThread(
	FRHICommandListBase& RHICmdList,
	const FVoxelRenderSubsystem& Subsystem)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInRenderingThread());
	ON_SCOPE_EXIT
	{
		UpdateStats();
	};
	ensure(!IndicesBuffer);

	IndicesBuffer = MakeUnique<FIndexBuffer>();
	{
		VOXEL_SCOPE_COUNTER("Indices");

		FVoxelResourceArrayRef ResourceArray(Mesh->Indices);

#if VOXEL_ENGINE_VERSION >= 506
		IndicesBuffer->IndexBufferRHI = RHICmdList.CreateBuffer(
			FRHIBufferCreateDesc::CreateIndex<uint32>(TEXT("Indices"), Mesh->Indices.Num())
			.AddUsage(BUF_Static | BUF_ShaderResource)
			.SetInitActionResourceArray(&ResourceArray)
			.DetermineInitialState());
#else
		FRHIResourceCreateInfo CreateInfo(TEXT("Indices"), &ResourceArray);
		IndicesBuffer->IndexBufferRHI = RHICmdList.CreateIndexBuffer(
			sizeof(uint32),
			ResourceArray.GetResourceDataSize(),
			BUF_Static | BUF_ShaderResource,
			CreateInfo);
#endif

		IndicesBuffer->InitResource(RHICmdList);
	}

#if VOXEL_ENGINE_VERSION >= 506
	IndicesSRV = MakeUniqueCopy(RHICmdList.CreateShaderResourceView(
		IndicesBuffer->IndexBufferRHI,
		FRHIViewDesc::CreateBufferSRV()
		.SetType(FRHIViewDesc::EBufferType::Typed)
		.SetFormat(PF_R32_UINT)));
#else
	IndicesSRV = MakeUniqueCopy(RHICmdList.CreateShaderResourceView(
		IndicesBuffer->IndexBufferRHI,
		sizeof(uint32),
		PF_R32_UINT));
#endif

	VerticesBuffer = MakeUnique<FVertexBufferWithSRV>();
	{
		VOXEL_SCOPE_COUNTER("Vertices");

		FVoxelResourceArrayRef ResourceArray(Vertices);

#if VOXEL_ENGINE_VERSION >= 506
		VerticesBuffer->VertexBufferRHI = RHICmdList.CreateBuffer(
			FRHIBufferCreateDesc::CreateVertex<FVector4f>(TEXT("Vertices"), Mesh->Vertices.Num())
			.AddUsage(BUF_Static | BUF_ShaderResource)
			.SetInitActionResourceArray(&ResourceArray)
			.DetermineInitialState());

		VerticesBuffer->ShaderResourceViewRHI = RHICmdList.CreateShaderResourceView(
			VerticesBuffer->VertexBufferRHI,
			FRHIViewDesc::CreateBufferSRV()
			.SetType(FRHIViewDesc::EBufferType::Typed)
			.SetFormat(PF_A32B32G32R32F));
#else
		FRHIResourceCreateInfo CreateInfo(TEXT("Vertices"), &ResourceArray);
		VerticesBuffer->VertexBufferRHI = RHICmdList.CreateVertexBuffer(
			ResourceArray.GetResourceDataSize(),
			BUF_Static | BUF_ShaderResource,
			CreateInfo);

		VerticesBuffer->ShaderResourceViewRHI = RHICmdList.CreateShaderResourceView(
			VerticesBuffer->VertexBufferRHI,
			sizeof(FVector4f),
			PF_A32B32G32R32F);
#endif

		VerticesBuffer->InitResource(RHICmdList);
	}

	NormalsBuffer = MakeUnique<FVertexBufferWithSRV>();
	{
		VOXEL_SCOPE_COUNTER("Normals");

		FVoxelResourceArrayRef ResourceArray(Mesh->Normals);

#if VOXEL_ENGINE_VERSION >= 506
		NormalsBuffer->VertexBufferRHI = RHICmdList.CreateBuffer(
			FRHIBufferCreateDesc::CreateVertex<FVoxelOctahedron>(TEXT("Normals"), Mesh->Normals.Num())
			.AddUsage(BUF_Static | BUF_ShaderResource)
			.SetInitActionResourceArray(&ResourceArray)
			.DetermineInitialState());

		NormalsBuffer->ShaderResourceViewRHI = RHICmdList.CreateShaderResourceView(
			NormalsBuffer->VertexBufferRHI,
			FRHIViewDesc::CreateBufferSRV()
			.SetType(FRHIViewDesc::EBufferType::Typed)
			.SetFormat(PF_R8G8));
#else
		FRHIResourceCreateInfo CreateInfo(TEXT("Normals"), &ResourceArray);
		NormalsBuffer->VertexBufferRHI = RHICmdList.CreateVertexBuffer(
			ResourceArray.GetResourceDataSize(),
			BUF_Static | BUF_ShaderResource,
			CreateInfo);

		NormalsBuffer->ShaderResourceViewRHI = RHICmdList.CreateShaderResourceView(
			NormalsBuffer->VertexBufferRHI,
			sizeof(FVoxelOctahedron),
			PF_R8G8);
#endif

		NormalsBuffer->InitResource(RHICmdList);
	}

	const ERHIFeatureLevel::Type FeatureLevel = Subsystem.GetConfig().FeatureLevel;

	if (FVoxelUtilities::CanUseBarycentricsSemantic(GShaderPlatformForFeatureLevel[FeatureLevel]))
	{
		if (GVoxelMegaMaterialDebugMode != 0)
		{
			VertexFactory = MakeUnique<TVoxelVertexFactory<EVoxelVertexFactoryType::WithBarycentrics_BasePass_Debug>>(FeatureLevel);
			VertexFactory->VoxelDebugMode = GVoxelMegaMaterialDebugMode;
		}
		else
		{
			VertexFactory = MakeUnique<TVoxelVertexFactory<EVoxelVertexFactoryType::WithBarycentrics_BasePass>>(FeatureLevel);
		}
	}
	else
	{
		VertexFactory = MakeUnique<TVoxelVertexFactory<EVoxelVertexFactoryType::NoBarycentrics_BasePass>>(FeatureLevel);
	}

	VertexFactory->Vertices = VerticesBuffer->ShaderResourceViewRHI;
	VertexFactory->Normals = NormalsBuffer->ShaderResourceViewRHI;
	VertexFactory->Indices = *IndicesSRV;
	VertexFactory->RenderData = MegaMaterialRenderData;

	Vertices.Empty();

#if RHI_RAYTRACING
	if (!bEnableRaytracing)
	{
		return;
	}

	RayTracingGeometryGroupHandle = GRayTracingGeometryManager->RegisterRayTracingGeometryGroup(1, 0);

	RayTracingGeometry = MakeUnique<FRayTracingGeometry>();
	RayTracingGeometry->GroupHandle = RayTracingGeometryGroupHandle;
	RayTracingGeometry->LODIndex = 0;

	{
		static FVoxelCounter32 DebugNumber;
		checkVoxelSlow(Mesh->Indices.Num() % 3 == 0);

		FRayTracingGeometrySegment Segment;
		Segment.VertexBuffer = VerticesBuffer->VertexBufferRHI;
		Segment.NumPrimitives = Mesh->Indices.Num() / 3;
		Segment.VertexBufferElementType = VET_Float4;
		Segment.VertexBufferStride = sizeof(FVector4f);
		Segment.MaxVertices = Mesh->Vertices.Num();

		FRayTracingGeometryInitializer Initializer;
		Initializer.DebugName = FDebugName(STATIC_FNAME("FVoxelMeshSceneProxy"), DebugNumber.Increment_ReturnNew());
		Initializer.IndexBuffer = IndicesBuffer->IndexBufferRHI;
		Initializer.TotalPrimitiveCount = Mesh->Indices.Num() / 3;
		Initializer.GeometryType = RTGT_Triangles;
		Initializer.bFastBuild = true;
		Initializer.bAllowUpdate = false;
		Initializer.Segments.Add(Segment);

		RayTracingGeometry->SetInitializer(MoveTemp(Initializer));
	}

	RayTracingGeometry->InitResource(RHICmdList);
#endif
}