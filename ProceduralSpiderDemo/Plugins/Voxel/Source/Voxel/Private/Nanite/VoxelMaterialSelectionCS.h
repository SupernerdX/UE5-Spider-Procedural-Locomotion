// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "MeshMaterialShader.h"
#include "SceneUniformBuffer.h"

class FPrimitiveUniformShaderParameters;

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FVoxelMaterialSelectionParameters, )
	SHADER_PARAMETER_SCALAR_ARRAY(uint32, RenderIndexToShadingBin, [128])
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<UlongType>, VisBuffer64)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<uint>, ShadingMask)
	SHADER_PARAMETER(uint32, UniqueId)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

class FVoxelMaterialSelectionCS : public FMaterialShader
{
public:
	DECLARE_SHADER_TYPE(FVoxelMaterialSelectionCS, Material);

	FVoxelMaterialSelectionCS() = default;
	explicit FVoxelMaterialSelectionCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters);

	static void ModifyCompilationEnvironment(
		const FMaterialShaderPermutationParameters& Parameters,
		FShaderCompilerEnvironment& OutEnvironment);
};

#define bUsedWithVoxelMaterialSelection bUsedWithVirtualHeightfieldMesh