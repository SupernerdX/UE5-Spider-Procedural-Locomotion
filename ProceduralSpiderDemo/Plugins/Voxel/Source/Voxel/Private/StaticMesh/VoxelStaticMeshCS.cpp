// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStaticMeshCS.h"
#include "VoxelMaterialUsage.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelNormalBuffer.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialRenderProxy.h"
#include "SceneView.h"
#include "MeshPassProcessor.h"
#include "MeshDrawShaderBindings.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "PrimitiveUniformShaderParameters.h"
#include "PrimitiveUniformShaderParametersBuilder.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FVoxelStaticMeshCSParameters, "StaticMeshParameters");

FVoxelStaticMeshCS::FVoxelStaticMeshCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FMaterialShader(Initializer)
{
	Parameter.Bind(Initializer.ParameterMap, TEXT("StaticMeshParameters"));
}

bool FVoxelStaticMeshCS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
	if (!IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6) ||
		!EnumHasAllFlags(Parameters.Flags, EShaderPermutationFlags::HasEditorOnlyData))
	{
		return false;
	}

	if (Parameters.MaterialParameters.ShadingModels.HasShadingModel(MSM_SingleLayerWater))
	{
		// Will fail to compile
		return false;
	}

	return FVoxelMaterialUsage::ShouldCompilePermutation(Parameters.MaterialParameters);
}

void FVoxelStaticMeshCS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	// Not working because we need to set WorldPosition_DDX
	//OutEnvironment.SetDefine(TEXT("USE_ANALYTIC_DERIVATIVES"), 1);
}

void FVoxelStaticMeshCS::GetShaderBindings(
	const FSceneInterface* Scene,
	const ERHIFeatureLevel::Type FeatureLevel,
	const FMaterialRenderProxy& MaterialRenderProxy,
	const FMaterial& Material,
	FMeshDrawSingleShaderBindings& ShaderBindings,
	const FRHIUniformBuffer& ViewUniformBuffer,
	const FRHIUniformBuffer& PrimitiveParameters,
	const TUniformBufferRef<FVoxelStaticMeshCSParameters>& Parameters) const
{
	FMaterialShader::GetShaderBindings(
		Scene,
		FeatureLevel,
		MaterialRenderProxy,
		Material,
		ShaderBindings);

	ShaderBindings.Add(GetUniformBufferParameter<FViewUniformShaderParameters>(), &ViewUniformBuffer);
	ShaderBindings.Add(GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), &PrimitiveParameters);
	ShaderBindings.Add(Parameter, Parameters);
}

IMPLEMENT_MATERIAL_SHADER_TYPE(
	,
	FVoxelStaticMeshCS,
	TEXT("/Plugin/Voxel/VoxelStaticMeshCS.usf"),
	TEXT("Main"),
	SF_Compute);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelFuture<TSharedPtr<FVoxelBuffer>> FVoxelStaticMeshCS::Compute(
	UMaterialInterface& MaterialObject,
	const FVoxelPinType& InnerType,
	const TVoxelArray<FVector4f>& Vertices,
	const TVoxelArray<FVector4f>& TangentX,
	const TVoxelArray<FVector4f>& TangentY,
	const TVoxelArray<FVector4f>& TangentZ,
	const TVoxelArray<FVector4f>& Colors,
	const TVoxelArray<TVoxelArray<FVector2f>>& TextureCoordinates,
	const EVoxelStaticMeshMetadataAttribute Attribute,
	const int32 NumRetries)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(NumRetries < 30))
	{
		return {};
	}

	FVoxelMaterialUsage::CheckMaterial(&MaterialObject);

	{
		FMaterialUpdateContext UpdateContext(FMaterialUpdateContext::EOptions::SyncWithRenderingThread);
		UpdateContext.AddMaterialInterface(&MaterialObject);
		MaterialObject.CacheShaders();
	}

	return Voxel::RenderTask([=, &MaterialObject, WeakMaterialObject = MakeVoxelObjectPtr(MaterialObject)](FRHICommandListImmediate& RHICmdList) -> TVoxelFuture<TSharedPtr<FVoxelBuffer>>
	{
		VOXEL_FUNCTION_COUNTER();

		const FMaterialRenderProxy* MaterialRenderProxy = MaterialObject.GetRenderProxy();
		if (!ensure(MaterialRenderProxy))
		{
			return {};
		}

		const auto Retry = [&]
		{
			// Wait for material to compile
			return FVoxelUtilities::DelayedCall_Future([=]() -> TVoxelFuture<TSharedPtr<FVoxelBuffer>>
			{
				UMaterialInterface* LocalMaterialObject = WeakMaterialObject.Resolve();
				if (!ensure(LocalMaterialObject))
				{
					return {};
				}

				return Compute(
					*LocalMaterialObject,
					InnerType,
					Vertices,
					TangentX,
					TangentY,
					TangentZ,
					Colors,
					TextureCoordinates,
					Attribute,
					NumRetries + 1);
			}, 1.f);
		};

		const FMaterial* Material = MaterialRenderProxy->UpdateUniformExpressionCacheIfNeeded(RHICmdList, GMaxRHIFeatureLevel);
		if (!ensureVoxelSlow(Material))
		{
			return Retry();
		}

		const FMaterialShaderMap* MaterialShaderMap = Material->GetRenderingThreadShaderMap();
		if (!ensureVoxelSlow(MaterialShaderMap))
		{
			return Retry();
		}

		const TShaderRef<FVoxelStaticMeshCS> Shader = MaterialShaderMap->GetShader<FVoxelStaticMeshCS>();
		if (!ensureVoxelSlow(Shader.IsValid()))
		{
			return Retry();
		}

		FRHIComputeShader* ComputeShader = Shader.GetComputeShader();
		if (!ensureVoxelSlow(ComputeShader))
		{
			return Retry();
		}

		const auto CreateBuffer = [&]<typename T>(const TVoxelArrayView<T> Data, const EPixelFormat PixelFormat)
		{
			VOXEL_SCOPE_COUNTER("Create buffer");
			check(GPixelFormats[PixelFormat].BlockBytes == sizeof(T));

			const TSharedRef<FVertexBufferWithSRV> Result = MakeShared<FVertexBufferWithSRV>();
			FVertexBufferWithSRV& Buffer = *Result;

			FVoxelResourceArrayRef ResourceArray(Data);

#if VOXEL_ENGINE_VERSION >= 506
			Buffer.VertexBufferRHI = RHICmdList.CreateBuffer(
				FRHIBufferCreateDesc::CreateVertex<T>(TEXT("FVoxelStaticMeshCS"), Data.Num())
				.AddUsage(BUF_Static | BUF_ShaderResource)
				.SetInitialState(ERHIAccess::SRVGraphics)
				.SetInitActionResourceArray(&ResourceArray));

			Buffer.ShaderResourceViewRHI = RHICmdList.CreateShaderResourceView(
				Buffer.VertexBufferRHI,
				FRHIViewDesc::CreateBufferSRV()
				.SetType(FRHIViewDesc::EBufferType::Typed)
				.SetFormat(PixelFormat));
#else
			FRHIResourceCreateInfo CreateInfo(TEXT("FVoxelStaticMeshCS"), &ResourceArray);

			Buffer.VertexBufferRHI = RHICmdList.CreateVertexBuffer(
				ResourceArray.GetResourceDataSize(),
				BUF_Static | BUF_ShaderResource,
				CreateInfo);

			Buffer.ShaderResourceViewRHI = RHICmdList.CreateShaderResourceView(
				Buffer.VertexBufferRHI,
				sizeof(T),
				PixelFormat);
#endif

			Buffer.InitResource(RHICmdList);

			return Result;
		};

		const TSharedRef<FVertexBufferWithSRV> VerticesBuffer = CreateBuffer(Vertices.View(), PF_A32B32G32R32F);
		const TSharedRef<FVertexBufferWithSRV> TangentXBuffer = CreateBuffer(TangentX.View(), PF_A32B32G32R32F);
		const TSharedRef<FVertexBufferWithSRV> TangentYBuffer = CreateBuffer(TangentY.View(), PF_A32B32G32R32F);
		const TSharedRef<FVertexBufferWithSRV> TangentZBuffer = CreateBuffer(TangentZ.View(), PF_A32B32G32R32F);
		const TSharedRef<FVertexBufferWithSRV> ColorsBuffer = CreateBuffer(Colors.View(), PF_A32B32G32R32F);

		TVoxelArray<TVoxelArray<FVector2f>> LocalTextureCoordinates = TextureCoordinates;
		while (LocalTextureCoordinates.Num() < 4)
		{
			LocalTextureCoordinates.Emplace_GetRef().SetNumZeroed(Vertices.Num());
		}

		const TSharedRef<FVertexBufferWithSRV> TextureCoordinates0 = CreateBuffer(LocalTextureCoordinates[0].View(), PF_G32R32F);
		const TSharedRef<FVertexBufferWithSRV> TextureCoordinates1 = CreateBuffer(LocalTextureCoordinates[1].View(), PF_G32R32F);
		const TSharedRef<FVertexBufferWithSRV> TextureCoordinates2 = CreateBuffer(LocalTextureCoordinates[2].View(), PF_G32R32F);
		const TSharedRef<FVertexBufferWithSRV> TextureCoordinates3 = CreateBuffer(LocalTextureCoordinates[3].View(), PF_G32R32F);

		const TSharedRef<FVertexBufferWithSRV> ResultBuffer = MakeShared<FVertexBufferWithSRV>();
		{
			VOXEL_SCOPE_COUNTER("ResultBuffer");

			FVertexBufferWithSRV& Buffer = *ResultBuffer;

#if VOXEL_ENGINE_VERSION >= 506
			Buffer.VertexBufferRHI = RHICmdList.CreateBuffer(
				FRHIBufferCreateDesc::CreateVertex<FVector4f>(TEXT("FVoxelStaticMeshCS"), Vertices.Num())
				.AddUsage(BUF_Static | BUF_ShaderResource | BUF_UnorderedAccess)
				.SetInitialState(ERHIAccess::UAVCompute));

			Buffer.UnorderedAccessViewRHI = RHICmdList.CreateUnorderedAccessView(
				Buffer.VertexBufferRHI,
				FRHIViewDesc::CreateBufferUAV()
				.SetType(FRHIViewDesc::EBufferType::Typed)
				.SetFormat(PF_A32B32G32R32F));
#else
			FRHIResourceCreateInfo CreateInfo(TEXT("FVoxelStaticMeshCS"));

			Buffer.VertexBufferRHI = RHICmdList.CreateVertexBuffer(
				Vertices.Num() * sizeof(FVector4f),
				BUF_Static | BUF_ShaderResource | BUF_UnorderedAccess,
				CreateInfo);

			Buffer.UnorderedAccessViewRHI = RHICmdList.CreateUnorderedAccessView(
				Buffer.VertexBufferRHI,
				PF_A32B32G32R32F);
#endif

			Buffer.InitResource(RHICmdList);
		}

		const TSharedRef<TUniformBuffer<FViewUniformShaderParameters>> ViewUniformBuffer = MakeShared<TUniformBuffer<FViewUniformShaderParameters>>();
		{
			VOXEL_SCOPE_COUNTER("ViewUniformBuffer");

			FViewUniformShaderParameters ViewUniformShaderParameters;
			ViewUniformBuffer->SetContents(RHICmdList, ViewUniformShaderParameters);
			ViewUniformBuffer->InitResource(RHICmdList);
		}

		const TSharedRef<TUniformBuffer<FPrimitiveUniformShaderParameters>> PrimitiveUniformBuffer = MakeShared<TUniformBuffer<FPrimitiveUniformShaderParameters>>();
		{
			VOXEL_SCOPE_COUNTER("PrimitiveUniformBuffer");

			FPrimitiveUniformShaderParametersBuilder Builder = FPrimitiveUniformShaderParametersBuilder()
				.Defaults()
				.LocalToWorld(FMatrix::Identity)
				.PreviousLocalToWorld(FMatrix::Identity)
				.ActorWorldPosition(FVector::ZeroVector);

			PrimitiveUniformBuffer->SetContents(RHICmdList, Builder.Build());
			PrimitiveUniformBuffer->InitResource(RHICmdList);
		}

		SCOPED_DRAW_EVENTF(RHICmdList, VoxelStaticMeshCS, TEXT("VoxelStaticMeshCS %s"), MaterialObject.GetName());

#if VOXEL_ENGINE_VERSION < 506
		RHICmdList.Transition(FRHITransitionInfo(VerticesBuffer->VertexBufferRHI, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
		RHICmdList.Transition(FRHITransitionInfo(TangentXBuffer->VertexBufferRHI, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
		RHICmdList.Transition(FRHITransitionInfo(TangentYBuffer->VertexBufferRHI, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
		RHICmdList.Transition(FRHITransitionInfo(TangentZBuffer->VertexBufferRHI, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
		RHICmdList.Transition(FRHITransitionInfo(ColorsBuffer->VertexBufferRHI, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
		RHICmdList.Transition(FRHITransitionInfo(ResultBuffer->VertexBufferRHI, ERHIAccess::Unknown, ERHIAccess::UAVCompute));
#endif

		FVoxelStaticMeshCSParameters ParametersValue;
		ParametersValue.Vertices = VerticesBuffer->ShaderResourceViewRHI;
		ParametersValue.TangentX = TangentXBuffer->ShaderResourceViewRHI;
		ParametersValue.TangentY = TangentYBuffer->ShaderResourceViewRHI;
		ParametersValue.TangentZ = TangentZBuffer->ShaderResourceViewRHI;
		ParametersValue.Colors = ColorsBuffer->ShaderResourceViewRHI;
		ParametersValue.TextureCoordinates0 = TextureCoordinates0->ShaderResourceViewRHI;
		ParametersValue.TextureCoordinates1 = TextureCoordinates1->ShaderResourceViewRHI;
		ParametersValue.TextureCoordinates2 = TextureCoordinates2->ShaderResourceViewRHI;
		ParametersValue.TextureCoordinates3 = TextureCoordinates3->ShaderResourceViewRHI;
		ParametersValue.Attribute = int32(Attribute);
		ParametersValue.OutData = ResultBuffer->UnorderedAccessViewRHI;

		const TUniformBufferRef<FVoxelStaticMeshCSParameters> Parameters = CreateUniformBufferImmediate(ParametersValue, UniformBuffer_SingleFrame);

		FMeshDrawShaderBindings ShaderBindings;

		FMeshProcessorShaders MeshProcessorShaders;
		MeshProcessorShaders.ComputeShader = Shader;
		ShaderBindings.Initialize(MeshProcessorShaders);

		FMeshDrawSingleShaderBindings SingleShaderBindings = ShaderBindings.GetSingleShaderBindings(SF_Compute);

		Shader->GetShaderBindings(
			nullptr,
			GMaxRHIFeatureLevel,
			*MaterialRenderProxy,
			*Material,
			SingleShaderBindings,
			*ViewUniformBuffer->GetUniformBufferRHI(),
			*PrimitiveUniformBuffer->GetUniformBufferRHI(),
			Parameters);

		ShaderBindings.SetOnCommandList(RHICmdList, ComputeShader);

		SetComputePipelineState(RHICmdList, ComputeShader);

		RHICmdList.DispatchComputeShader(
			FMath::DivideAndRoundUp(Vertices.Num(), 64),
			1,
			1);

		RHICmdList.Transition(FRHITransitionInfo(ResultBuffer->VertexBufferRHI, ERHIAccess::UAVCompute, ERHIAccess::CopySrc));

		return FVoxelUtilities::Readback<FVector4f>(ResultBuffer->VertexBufferRHI).Then_RenderThread([=](const TVoxelArray64<FVector4f>& Data) -> TSharedPtr<FVoxelBuffer>
		{
			VOXEL_FUNCTION_COUNTER();

			VerticesBuffer->ReleaseResource();
			TangentXBuffer->ReleaseResource();
			TangentYBuffer->ReleaseResource();
			TangentZBuffer->ReleaseResource();
			ColorsBuffer->ReleaseResource();
			TextureCoordinates0->ReleaseResource();
			TextureCoordinates1->ReleaseResource();
			TextureCoordinates2->ReleaseResource();
			TextureCoordinates3->ReleaseResource();
			ResultBuffer->ReleaseResource();

			ViewUniformBuffer->ReleaseResource();
			PrimitiveUniformBuffer->ReleaseResource();

			if (!ensure(Data.Num() == Vertices.Num()))
			{
				return nullptr;
			}

			if (InnerType.Is<float>())
			{
				const TSharedRef<FVoxelFloatBuffer> Result = MakeShared<FVoxelFloatBuffer>();
				Result->AllocateZeroed(Data.Num());

				for (int32 Index = 0; Index < Data.Num(); Index++)
				{
					Result->Set(Index, Data[Index].X);
				}

				return Result;
			}

			if (InnerType.Is<FLinearColor>())
			{
				const TSharedRef<FVoxelLinearColorBuffer> Result = MakeShared<FVoxelLinearColorBuffer>();
				Result->AllocateZeroed(Data.Num());

				for (int32 Index = 0; Index < Data.Num(); Index++)
				{
					Result->Set(Index, FLinearColor(Data[Index]));
				}

				return Result;
			}

			if (InnerType.Is<FVoxelOctahedron>())
			{
				const TSharedRef<FVoxelNormalBuffer> Result = MakeShared<FVoxelNormalBuffer>();
				Result->AllocateZeroed(Data.Num());

				for (int32 Index = 0; Index < Data.Num(); Index++)
				{
					Result->Set(Index, FVoxelOctahedron(FVector3f(Data[Index]).GetSafeNormal()));
				}

				return Result;
			}

			ensure(false);
			return {};
		});
	});
}