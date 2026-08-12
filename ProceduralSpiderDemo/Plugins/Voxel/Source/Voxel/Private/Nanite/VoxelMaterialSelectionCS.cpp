// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMaterialSelectionCS.h"
#include "MeshDrawShaderBindings.h"
#include "DataDrivenShaderPlatformInfo.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FVoxelMaterialSelectionParameters, "MaterialSelectionParameters");

FVoxelMaterialSelectionCS::FVoxelMaterialSelectionCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FMaterialShader(Initializer)
{
}

bool FVoxelMaterialSelectionCS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
	if (!IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6))
	{
		return false;
	}

	return
		Parameters.MaterialParameters.bIsSpecialEngineMaterial ||
		Parameters.MaterialParameters.bIsUsedWithVirtualHeightfieldMesh;
}

void FVoxelMaterialSelectionCS::ModifyCompilationEnvironment(
	const FMaterialShaderPermutationParameters& Parameters,
	FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("VOXEL_MATERIAL_SELECTION"), 1);
	OutEnvironment.SetDefine(TEXT("IS_NANITE_PASS"), 1);
	OutEnvironment.SetDefine(TEXT("USE_ANALYTIC_DERIVATIVES"), 1);
	OutEnvironment.SetDefine(TEXT("NANITE_USE_UNIFORM_BUFFER"), 1);
	OutEnvironment.SetDefine(TEXT("NANITE_USE_VIEW_UNIFORM_BUFFER"), 1);
	OutEnvironment.SetDefine(TEXT("NANITE_USE_RASTER_UNIFORM_BUFFER"), 1);
	OutEnvironment.SetDefine(TEXT("NANITE_USE_SHADING_UNIFORM_BUFFER"), 1);
	OutEnvironment.SetDefine(TEXT("VF_SUPPORTS_PRIMITIVE_SCENE_DATA"), 1);

	OutEnvironment.CompilerFlags.Add(CFLAG_ForceDXC);
	OutEnvironment.CompilerFlags.Add(CFLAG_HLSL2021);

	// "implicit truncation of vector type" on Texture2DArraySampleGrad
	//OutEnvironment.CompilerFlags.Add(CFLAG_WarningsAsErrors);
}

IMPLEMENT_MATERIAL_SHADER_TYPE(
	,
	FVoxelMaterialSelectionCS,
	TEXT("/Plugin/Voxel/VoxelMaterialSelectionCS.usf"),
	TEXT("Main"),
	SF_Compute);