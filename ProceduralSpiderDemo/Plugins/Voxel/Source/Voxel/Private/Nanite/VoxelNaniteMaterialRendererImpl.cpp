// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nanite/VoxelNaniteMaterialRendererImpl.h"
#include "Nanite/VoxelMaterialSelectionCS.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"

#pragma warning(disable : 4305)
#include "ScenePrivate.h"
#pragma warning(default : 4305)

#include "BasePassRendering.h"
#include "Materials/Material.h"
#include "BasePassRendering.inl"
#include "Materials/MaterialRenderProxy.h"
#include "Rendering/NaniteStreamingManager.h"
#include "PrimitiveUniformShaderParametersBuilder.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelDisableMaterialSelection, false,
	"voxel.MegaMaterial.DisableMaterialSelection",
	"Disable the voxel nanite material selection pass");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if !IS_MONOLITHIC
const FShaderParametersMetadata* FNaniteShadingUniformParameters::GetStructMetadata()
{
	return FShaderParametersMetadata::GetNameStructMap().FindChecked(FHashedName(TEXT("NaniteShading")));
}

const FShaderParametersMetadata* FNaniteRasterUniformParameters::GetStructMetadata()
{
	return FShaderParametersMetadata::GetNameStructMap().FindChecked(FHashedName(TEXT("NaniteRaster")));
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelNaniteMaterialRendererSingleton : public FVoxelRenderSingleton
{
public:
	FVoxelCriticalSection CriticalSection;
	TVoxelArray<TWeakPtr<FVoxelNaniteMaterialRendererImpl>> Impls_RequiresLock;

	//~ Begin FVoxelRenderSingleton Interface
	virtual void OnBeginFrame_RenderThread() override
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInRenderingThread());
		ensure(Impls_RenderThread.Num() == 0);

		VOXEL_SCOPE_LOCK(CriticalSection);

		for (auto It = Impls_RequiresLock.CreateIterator(); It; ++It)
		{
			const TSharedPtr<FVoxelNaniteMaterialRendererImpl> Impl = It->Pin();
			if (!Impl)
			{
				It.RemoveCurrentSwap();
				continue;
			}

			Impls_RenderThread.Add(Impl);
		}
	}
	virtual void OnEndFrame_RenderThread() override
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInRenderingThread());

		Impls_RenderThread.Reset();
	}

	virtual void PreRenderBasePass_RenderThread(
		FRDGBuilder& GraphBuilder,
		FSceneView& View,
		const bool bDepthBufferIsPopulated) override
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInRenderingThread());

		for (const TSharedPtr<FVoxelNaniteMaterialRendererImpl>& Impl : Impls_RenderThread)
		{
			Impl->PreRenderBasePass_RenderThread(GraphBuilder, View);
		}
	}
	//~ End FVoxelRenderSingleton Interface

private:
	TVoxelArray<TSharedPtr<FVoxelNaniteMaterialRendererImpl>> Impls_RenderThread;
};
FVoxelNaniteMaterialRendererSingleton* GVoxelNaniteRendererSingleton = new FVoxelNaniteMaterialRendererSingleton();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelNaniteMaterialRendererImpl> FVoxelNaniteMaterialRendererImpl::Create(const TSharedRef<FVoxelMegaMaterialProxy>& MegaMaterialProxy)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedRef<FVoxelMaterialInstanceRef> DefaultMaterialInstance = INLINE_LAMBDA
	{
		// Word aligned default material
		UMaterialInterface* DefaultMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Default/VoxelWorldGridMaterial.VoxelWorldGridMaterial"));
		ensure(DefaultMaterial);

		UMaterialInstanceDynamic* Instance = UMaterialInstanceDynamic::Create(
			DefaultMaterial,
			GetTransientPackage());
		check(Instance);

		return FVoxelMaterialInstanceRef::Make(*Instance);
	};

	const TVoxelMap<FVoxelMaterialRenderIndex, TVoxelObjectPtr<UMaterialInterface>>& MaterialIndexToMaterial = MegaMaterialProxy->GetMaterialIndexToMaterial();

	TVoxelMap<FVoxelMaterialRenderIndex, TSharedRef<FVoxelMaterialInstanceRef>> MaterialIndexToMaterialInstance;
	MaterialIndexToMaterialInstance.Reserve(MaterialIndexToMaterial.Num());

	for (const auto& It : MaterialIndexToMaterial)
	{
		UMaterialInstanceDynamic* Instance = UMaterialInstanceDynamic::Create(
			It.Value.Resolve_Ensured(),
			GetTransientPackage());
		check(Instance);
		MaterialIndexToMaterialInstance.Add_EnsureNew(It.Key, FVoxelMaterialInstanceRef::Make(*Instance));
	}

	const TSharedRef<FVoxelNaniteMaterialRendererImpl> Impl = MakeShareable_Stats(new FVoxelNaniteMaterialRendererImpl(
		MegaMaterialProxy,
		DefaultMaterialInstance,
		MoveTemp(MaterialIndexToMaterialInstance)));

	{
		VOXEL_SCOPE_LOCK(GVoxelNaniteRendererSingleton->CriticalSection);
		GVoxelNaniteRendererSingleton->Impls_RequiresLock.Add(Impl);
	}

	return Impl;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelMaterialInstanceRef> FVoxelNaniteMaterialRendererImpl::GetMaterialInstance(const FVoxelMaterialRenderIndex RenderIndex) const
{
	if (RenderIndex.Index == 0)
	{
		checkVoxelSlow(!MaterialIndexToMaterialInstance.Contains(RenderIndex));
		return DefaultMaterialInstance;
	}

	if (!ensureVoxelSlow(MaterialIndexToMaterialInstance.Contains(RenderIndex)))
	{
		return {};
	}

	return MaterialIndexToMaterialInstance[RenderIndex];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BEGIN_SHADER_PARAMETER_STRUCT(FVoxelNaniteMaterialSelectionPassParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<UlongType>, VisBuffer64)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<uint>, ShadingMask)

	SHADER_PARAMETER_RDG_BUFFER_SRV(ByteAddressBuffer, ClusterPageData)
	SHADER_PARAMETER_RDG_BUFFER_SRV(ByteAddressBuffer, VisibleClustersSWHW)

#if VOXEL_ENGINE_VERSION >= 507
	SHADER_PARAMETER_RDG_BUFFER_SRV(ByteAddressBuffer, AssemblyTransforms)
#endif

	SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneUniformParameters, SceneUniformBuffer)
END_SHADER_PARAMETER_STRUCT()

DECLARE_GPU_STAT(VoxelNaniteMaterialSelection);

void FVoxelNaniteMaterialRendererImpl::PreRenderBasePass_RenderThread(
	FRDGBuilder& GraphBuilder,
	FSceneView& View)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInRenderingThread());

	if (QueuedData_RenderThread.IsValid())
	{
		PrimitiveUniformBuffer.Reset();

		MaterialRef = QueuedData_RenderThread->MaterialRef;
		LocalToWorld = QueuedData_RenderThread->LocalToWorld;
		UsedSurfaceTypes = MoveTemp(QueuedData_RenderThread->UsedSurfaceTypes);
		UniqueId = QueuedData_RenderThread->UniqueId;

		QueuedData_RenderThread.Reset();
	}

	if (UsedSurfaceTypes.Num() == 0 ||
		GVoxelDisableMaterialSelection)
	{
		return;
	}

	// Ensure index 0 is always set
	UsedSurfaceTypes.AddUnique(FVoxelSurfaceType());

	// Sort to fix material swap issues when rendering
	// There's probably a deeper issue at play here
	UsedSurfaceTypes.Sort();

	if (UsedSurfaceTypes.Num() == 0)
	{
		return;
	}

	const FRDGTextureRef VisBuffer64 = FVoxelUtilities::FindTexture(GraphBuilder, "Nanite.VisBuffer64");
	const FRDGTextureRef ShadingMask = FVoxelUtilities::FindTexture(GraphBuilder, "Nanite.ShadingMask");
	if (!VisBuffer64 ||
		!ShadingMask)
	{
		return;
	}

	RDG_EVENT_SCOPE(GraphBuilder, "VoxelNaniteRenderer");

	const FIntPoint Extent = FIntPoint(
		ShadingMask->Desc.Extent.X,
		ShadingMask->Desc.Extent.Y);

	const FRDGBufferRef VisibleClustersSWHW = FVoxelUtilities::FindBuffer(GraphBuilder, "Nanite.VisibleClustersSWHW");
	if (!ensureVoxelSlow(VisibleClustersSWHW))
	{
		return;
	}

#if VOXEL_ENGINE_VERSION >= 507
	FRDGBufferRef AssemblyTransforms;
	if (NaniteAssembliesSupported())
	{
		AssemblyTransforms = FVoxelUtilities::FindBuffer(GraphBuilder, "Nanite.AssemblyTransforms");
	}
	else
	{
		AssemblyTransforms = GSystemTextures.GetDefaultByteAddressBuffer(GraphBuilder, 48u);
	}

	if (!ensureVoxelSlow(AssemblyTransforms))
	{
		return;
	}
#endif

	RDG_GPU_STAT_SCOPE(GraphBuilder, VoxelNaniteMaterialSelection);

	if (!PrimitiveUniformBuffer)
	{
		FPrimitiveUniformShaderParametersBuilder Builder = FPrimitiveUniformShaderParametersBuilder()
			.Defaults()
			.LocalToWorld(LocalToWorld.ToMatrixWithScale())
			.PreviousLocalToWorld(LocalToWorld.ToMatrixWithScale())
			.ActorWorldPosition(LocalToWorld.GetTranslation());

		TUniformBuffer<FPrimitiveUniformShaderParameters>* Buffer = new TUniformBuffer<FPrimitiveUniformShaderParameters>();
		Buffer->SetContents(GraphBuilder.RHICmdList, Builder.Build());
		Buffer->InitResource(GraphBuilder.RHICmdList);

		PrimitiveUniformBuffer = MakeShareable_CustomDestructor(Buffer, [=]
		{
			// Make sure to keep the buffer alive long enough
			FVoxelUtilities::DelayedCall([=]
			{
				Voxel::RenderTask([=]
				{
					Buffer->ReleaseResource();
					delete Buffer;
				});
			});
		});
	}
	check(PrimitiveUniformBuffer);

	FVoxelNaniteMaterialSelectionPassParameters* PassParameters = GraphBuilder.AllocParameters<FVoxelNaniteMaterialSelectionPassParameters>();
	PassParameters->VisBuffer64 = VisBuffer64;
	PassParameters->ShadingMask = GraphBuilder.CreateUAV(ShadingMask);
	PassParameters->ClusterPageData = Nanite::GStreamingManager.GetClusterPageDataSRV(GraphBuilder);
	PassParameters->VisibleClustersSWHW = GraphBuilder.CreateSRV(VisibleClustersSWHW);
#if VOXEL_ENGINE_VERSION >= 507
	PassParameters->AssemblyTransforms = GraphBuilder.CreateSRV(AssemblyTransforms);
#endif
	PassParameters->SceneUniformBuffer = GetSceneUniformBufferRef(GraphBuilder, View);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("Voxel Nanite Material Selection"),
		PassParameters,
		ERDGPassFlags::Compute,
		MakeWeakPtrLambda(this, [this, &View, Extent, PassParameters](FRHICommandList& RHICmdList)
		{
			ComputeMaterialSelection(View, Extent, *PassParameters, RHICmdList);
		}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNaniteMaterialRendererImpl::ComputeMaterialSelection(
	const FSceneView& View,
	const FIntPoint& Extent,
	const FVoxelNaniteMaterialSelectionPassParameters& PassParameters,
	FRHICommandList& RHICmdList)
{
	VOXEL_FUNCTION_COUNTER();

	const FScene& Scene = static_cast<FScene&>(*View.Family->Scene);

	const UMaterialInterface* MaterialObject = MaterialRef->GetMaterial();
	if (!ensureVoxelSlow(MaterialObject))
	{
		return;
	}

	const FMaterialRenderProxy* MaterialRenderProxy = MaterialObject->GetRenderProxy();
	if (!ensureVoxelSlow(MaterialRenderProxy))
	{
		return;
	}
	const FMaterial& Material = MaterialRenderProxy->GetMaterialWithFallback(View.GetFeatureLevel(), MaterialRenderProxy);

	const FMaterialShaderMap* MaterialShaderMap = Material.GetRenderingThreadShaderMap();
	if (!ensureVoxelSlow(MaterialShaderMap))
	{
		return;
	}

	const TShaderRef<FMaterialShader> Shader = MaterialShaderMap->GetShader<FVoxelMaterialSelectionCS>();
	if (!ensureVoxelSlow(Shader.IsValid()))
	{
		return;
	}

	FRHIComputeShader* ComputeShader = Shader.GetComputeShader();
	if (!ensureVoxelSlow(ComputeShader))
	{
		return;
	}

	const int32 MaxStreamingPages = INLINE_LAMBDA
	{
		static IConsoleVariable* CVarStreamingPoolSize = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite.Streaming.StreamingPoolSize"));
		check(CVarStreamingPoolSize);

		int32 StreamingPoolSizeInMB = 0;
		CVarStreamingPoolSize->GetValue(StreamingPoolSizeInMB);

		return (uint64(StreamingPoolSizeInMB) << 20) >> NANITE_STREAMING_PAGE_GPU_SIZE_BITS;
	};

	const auto NaniteShadingParameters = INLINE_LAMBDA
	{
		FNaniteShadingUniformParameters Parameters;
		Parameters.ClusterPageData = PassParameters.ClusterPageData;
		Parameters.VisibleClustersSWHW = PassParameters.VisibleClustersSWHW;

		///////////////////////////////////////////////////////////////////////////////////////////
		// TRICKY, MIGHT CAUSE ISSUES IN FUTURE ENGINE RELEASES: Dummy values to pass validation //
		///////////////////////////////////////////////////////////////////////////////////////////

		Parameters.HierarchyBuffer = PassParameters.VisibleClustersSWHW;

#if VOXEL_ENGINE_VERSION >= 507
		Parameters.AssemblyTransforms = PassParameters.AssemblyTransforms;
#endif

		Parameters.ShadingMask = PassParameters.VisBuffer64;
		Parameters.VisBuffer64 = PassParameters.VisBuffer64;
		Parameters.DbgBuffer64 = PassParameters.VisBuffer64;
		Parameters.DbgBuffer32 = PassParameters.VisBuffer64;
#if VOXEL_ENGINE_VERSION < 506
		Parameters.RayTracingDataBuffer = PassParameters.VisibleClustersSWHW;
#endif
		Parameters.ShadingBinData = PassParameters.VisibleClustersSWHW;
		Parameters.MultiViewIndices = PassParameters.VisibleClustersSWHW;
		Parameters.MultiViewRectScaleOffsets = PassParameters.VisibleClustersSWHW;
		Parameters.InViews = PassParameters.VisibleClustersSWHW;

		return CreateUniformBufferImmediate(Parameters, UniformBuffer_SingleFrame);
	};

	const TUniformBufferRef<FNaniteRasterUniformParameters> NaniteRasterParameters = INLINE_LAMBDA
	{
		FNaniteRasterUniformParameters Parameters;
		Parameters.PageConstants = FIntVector4(0, MaxStreamingPages, 0, 0);
		return CreateUniformBufferImmediate(Parameters, UniformBuffer_SingleFrame);
	};

	const TUniformBufferRef<FVoxelMaterialSelectionParameters> VoxelParameters = INLINE_LAMBDA
	{
		FVoxelMaterialSelectionParameters Parameters;
		Parameters.UniqueId = UniqueId;

		{
			VOXEL_SCOPE_COUNTER("RenderIndexToShadingBin");

			TVoxelStaticArray<uint32, 128>& RenderIndexToShadingBin = ReinterpretCastRef<TVoxelStaticArray<uint32, 128>>(Parameters.RenderIndexToShadingBin);
			FMemory::Memset(RenderIndexToShadingBin, 0xFF);

			TVoxelMap<const FMaterialRenderProxy*, FNaniteShadingBin> MaterialToShadingBin;
			{
				const FNaniteShadingPipelineMap& ShadingPipelineMap = Scene.NaniteShadingPipelines[ENaniteMeshPass::BasePass].GetShadingPipelineMap();

				MaterialToShadingBin.Reserve(ShadingPipelineMap.Num());

				for (const auto& It : ShadingPipelineMap)
				{
					if (!ensureVoxelSlow(It.Value.ShadingPipeline))
					{
						continue;
					}
					FNaniteShadingPipeline& ShadingPipeline = *It.Value.ShadingPipeline;

					const Experimental::FHashElementId ShadingBinId = ShadingPipelineMap.FindId(ShadingPipeline);
					if (!ensureVoxelSlow(ShadingBinId.IsValid()))
					{
						continue;
					}

					if (const FNaniteShadingBin* ExistingShadingBin = MaterialToShadingBin.Find(ShadingPipeline.MaterialProxy))
					{
						const FNaniteShadingPipeline& ExistingShadingPipeline = ShadingPipelineMap.GetByElementId(Experimental::FHashElementId(ExistingShadingBin->BinId)).Key;
						ensure(ExistingShadingPipeline.MaterialProxy == ShadingPipeline.MaterialProxy);
						ensure(ExistingShadingPipeline.Material == ShadingPipeline.Material);
						ensure(ExistingShadingPipeline.ComputeShader == ShadingPipeline.ComputeShader);
						continue;
					}

					MaterialToShadingBin.Add_EnsureNew(ShadingPipeline.MaterialProxy) = FNaniteShadingBin
					{
						ShadingBinId.GetIndex(),
						It.Value.BinIndex
					};
				}
			}

			for (const FVoxelSurfaceType SurfaceType : UsedSurfaceTypes)
			{
				const FVoxelMaterialRenderIndex RenderIndex = MegaMaterialProxy->GetRenderIndex(SurfaceType);

				const TSharedPtr<FVoxelMaterialInstanceRef> Instance = GetMaterialInstance(RenderIndex);
				if (!ensureVoxelSlow(Instance))
				{
					continue;
				}

				const UMaterialInterface* MaterialObjectAtIndex = Instance->GetInstance();
				if (!ensureVoxelSlow(MaterialObjectAtIndex))
				{
					continue;
				}

				const FMaterialRenderProxy* MaterialRenderProxyAtIndex = MaterialObjectAtIndex->GetRenderProxy();
				if (!ensureVoxelSlow(MaterialRenderProxyAtIndex))
				{
					continue;
				}

				const FNaniteShadingBin* ShadingBin = MaterialToShadingBin.Find(MaterialRenderProxyAtIndex);
				if (!ShadingBin)
				{
					continue;
				}

				if (!ensure(RenderIndexToShadingBin.IsValidIndex(RenderIndex.Index)))
				{
					continue;
				}

				RenderIndexToShadingBin[RenderIndex.Index] = ShadingBin->BinIndex;
			}
		}

		Parameters.VisBuffer64 = PassParameters.VisBuffer64;
		Parameters.ShadingMask = PassParameters.ShadingMask;

		return CreateUniformBufferImmediate(Parameters, UniformBuffer_SingleFrame);
	};

	FMeshDrawShaderBindings ShaderBindings;

	FMeshProcessorShaders MeshProcessorShaders;
	MeshProcessorShaders.ComputeShader = Shader;
	ShaderBindings.Initialize(MeshProcessorShaders);

	FMeshDrawSingleShaderBindings SingleShaderBindings = ShaderBindings.GetSingleShaderBindings(SF_Compute);

	SingleShaderBindings.Add(Shader->GetUniformBufferParameter<FViewUniformShaderParameters>(), View.ViewUniformBuffer);
	SingleShaderBindings.Add(Shader->GetUniformBufferParameter<FSceneUniformParameters>(), PassParameters.SceneUniformBuffer->GetRHI());
	SingleShaderBindings.Add(Shader->GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), PrimitiveUniformBuffer->GetUniformBufferRHI());
	SingleShaderBindings.Add(Shader->GetUniformBufferParameter<FVoxelMaterialSelectionParameters>(), VoxelParameters);

	Shader->GetShaderBindings(
		View.Family->Scene,
		View.GetFeatureLevel(),
		*MaterialRenderProxy,
		Material,
		SingleShaderBindings);

	RHICmdList.SetStaticUniformBuffers(FUniformBufferStaticBindings(
		NaniteRasterParameters,
		NaniteShadingParameters));

	SetComputePipelineState(RHICmdList, ComputeShader);

	ShaderBindings.SetOnCommandList(RHICmdList, ComputeShader);

	RHICmdList.DispatchComputeShader(
		FMath::DivideAndRoundUp(Extent.X, 8),
		FMath::DivideAndRoundUp(Extent.Y, 8),
		1);
}