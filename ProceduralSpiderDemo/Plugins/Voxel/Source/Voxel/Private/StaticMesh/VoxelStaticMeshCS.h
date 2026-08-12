// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "MeshMaterialShader.h"
#include "StaticMesh/VoxelStaticMeshMetadata.h"

class FPrimitiveUniformShaderParameters;

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FVoxelStaticMeshCSParameters, )
	SHADER_PARAMETER_SRV(Buffer<float4>, Vertices)
	SHADER_PARAMETER_SRV(Buffer<float4>, TangentX)
	SHADER_PARAMETER_SRV(Buffer<float4>, TangentY)
	SHADER_PARAMETER_SRV(Buffer<float4>, TangentZ)
	SHADER_PARAMETER_SRV(Buffer<float4>, Colors)
	SHADER_PARAMETER_SRV(Buffer<float2>, TextureCoordinates0)
	SHADER_PARAMETER_SRV(Buffer<float2>, TextureCoordinates1)
	SHADER_PARAMETER_SRV(Buffer<float2>, TextureCoordinates2)
	SHADER_PARAMETER_SRV(Buffer<float2>, TextureCoordinates3)
	SHADER_PARAMETER(int32, Attribute)
	SHADER_PARAMETER_UAV(RWBuffer<float4>, OutData)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

class FVoxelStaticMeshCS : public FMaterialShader
{
public:
	DECLARE_SHADER_TYPE(FVoxelStaticMeshCS, Material);
	LAYOUT_FIELD(FShaderUniformBufferParameter, Parameter);

	FVoxelStaticMeshCS() = default;
	explicit FVoxelStaticMeshCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters);

	static void ModifyCompilationEnvironment(
		const FMaterialShaderPermutationParameters& Parameters,
		FShaderCompilerEnvironment& OutEnvironment);

	void GetShaderBindings(
		const FSceneInterface* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		const FRHIUniformBuffer& ViewUniformBuffer,
		const FRHIUniformBuffer& PrimitiveParameters,
		const TUniformBufferRef<FVoxelStaticMeshCSParameters>& Parameters) const;

public:
	static TVoxelFuture<TSharedPtr<FVoxelBuffer>> Compute(
		UMaterialInterface& MaterialObject,
		const FVoxelPinType& InnerType,
		const TVoxelArray<FVector4f>& Vertices,
		const TVoxelArray<FVector4f>& TangentX,
		const TVoxelArray<FVector4f>& TangentY,
		const TVoxelArray<FVector4f>& TangentZ,
		const TVoxelArray<FVector4f>& Colors,
		const TVoxelArray<TVoxelArray<FVector2f>>& TextureCoordinates,
		EVoxelStaticMeshMetadataAttribute Attribute,
		int32 NumRetries = 0);
};