// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VertexFactory.h"

struct FVoxelMegaMaterialRenderData;

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FVoxelVertexFactoryGlobalParameters,)
	SHADER_PARAMETER(int32, VoxelDebugMode)

	SHADER_PARAMETER_SRV(Buffer<float4>, Vertices)
	SHADER_PARAMETER_SRV(Buffer<float2>, Normals)
	SHADER_PARAMETER_SRV(Buffer<uint>, Indices)

	SHADER_PARAMETER(int32, ChunkIndicesIndex)

	SHADER_PARAMETER_TEXTURE(Texture2D, ChunkIndices_Texture)
	SHADER_PARAMETER(int32, ChunkIndices_TextureSizeLog2)

	SHADER_PARAMETER_TEXTURE(Texture2D<uint4>, Materials_Texture)
	SHADER_PARAMETER(int32, Materials_TextureSizeLog2)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

class FVoxelVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FVoxelVertexFactoryShaderParameters, NonVirtual);

public:
	void GetElementShaderBindings(
		const FSceneInterface* Scene,
		const FSceneView* View,
		const FMeshMaterialShader* Shader,
		EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const FVertexFactory* VertexFactory,
		const FMeshBatchElement& BatchElement,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const;
};

class FVoxelVertexFactory : public FVertexFactory
{
public:
	int32 VoxelDebugMode = 0;
	FShaderResourceViewRHIRef Vertices;
	FShaderResourceViewRHIRef Normals;
	FShaderResourceViewRHIRef Indices;
	TSharedPtr<const FVoxelMegaMaterialRenderData> RenderData;

private:
	// Keep textures alive even after reallocation
	FTextureRHIRef ChunkIndices_Texture;
	FTextureRHIRef Materials_Texture;
	TUniformBufferRef<FVoxelVertexFactoryGlobalParameters> GlobalParametersUniformBuffer;

	friend FVoxelVertexFactoryShaderParameters;

public:
	using FVertexFactory::FVertexFactory;

	//~ Begin FVertexFactory Interface
	virtual bool SupportsPositionOnlyStream() const override
	{
		return true;
	}
	virtual bool SupportsPositionAndNormalOnlyStream() const override
	{
		return true;
	}
	//~ End FVertexFactory Interface

	//~ Begin FRenderResource Interface
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void ReleaseRHI() override;
	//~ End FRenderResource Interface

public:
	static void ModifyCompilationEnvironment(
		const FVertexFactoryShaderPermutationParameters& Parameters,
		FShaderCompilerEnvironment& OutEnvironment);

	static void GetPSOPrecacheVertexFetchElements(
		EVertexInputStreamType VertexInputStreamType,
		FVertexDeclarationElementList& Elements);
};

enum class EVoxelVertexFactoryType : uint8
{
	NoBarycentrics_BasePass,
	WithBarycentrics_BasePass,
	WithBarycentrics_BasePass_Debug
};

template<EVoxelVertexFactoryType>
class TVoxelVertexFactory : public FVoxelVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(TVoxelVertexFactory);

public:
	using FVoxelVertexFactory::FVoxelVertexFactory;

	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters);

	static void ModifyCompilationEnvironment(
		const FVertexFactoryShaderPermutationParameters& Parameters,
		FShaderCompilerEnvironment& OutEnvironment);
};