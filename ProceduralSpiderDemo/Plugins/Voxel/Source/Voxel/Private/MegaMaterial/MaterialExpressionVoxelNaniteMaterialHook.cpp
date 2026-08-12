// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MaterialExpressionVoxelNaniteMaterialHook.h"
#include "VoxelHLSLMaterialTranslator.h"
#include "MaterialCompiler.h"
#include "Engine/Texture2D.h"

UObject* UMaterialExpressionVoxelNaniteMaterialHook::GetReferencedTexture() const
{
	return FVoxelTextureUtilities::GetDefaultTexture2D();
}

#if WITH_EDITOR
int32 UMaterialExpressionVoxelNaniteMaterialHook::Compile(FMaterialCompiler* Compiler, const int32 OutputIndex)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelHLSLMaterialTranslator& Writer = *static_cast<FVoxelHLSLMaterialTranslator*>(Compiler);

	if (Compiler->GetCurrentShaderFrequency() != SF_Pixel)
	{
		return Input.Compile(Compiler);
	}

	UTexture2D* DefaultTexture = FVoxelTextureUtilities::GetDefaultTexture2D();
	if (!ensure(DefaultTexture))
	{
		return -1;
	}

	const FString HookTag = "VOXEL_HOOK_TAG_F65439F64BC";

	const bool bHasHook = INLINE_LAMBDA
	{
		for (const FShaderCodeChunk& Chunk : *Writer.CurrentScopeChunks)
		{
			if (Chunk.DefinitionFinite.Contains(HookTag))
			{
				return true;
			}
		}
		return false;
	};

	if (bHasHook)
	{
		return Input.Compile(Compiler);
	}

	{
		UMaterialExpressionCustom* Custom = NewObject<UMaterialExpressionCustom>();
		Custom->Inputs.Reset();
		Custom->OutputType = CMOT_Float1;
		Custom->Code = "return 0;";
		Custom->IncludeFilePaths.Add("/Plugin/Voxel/VoxelDisplacement.ush");
		Custom->IncludeFilePaths.Add("/Engine/Private/Nanite/NaniteDataDecode.ush");

		TArray<int32> CustomInputs;
		Compiler->CustomExpression(Custom, 0, CustomInputs);
	}

	Writer.AddCodeChunk(MCT_VoidStatement, TEXT("\t// %s"), *HookTag);

	FString Code = R"(
#if MATERIAL_VERTEX_PARAMETERS_VOXEL_VERSION == 6 && MATERIAL_PIXEL_PARAMETERS_VOXEL_VERSION == 8 && IS_NANITE_PASS
{
	const FCluster Cluster = GetCluster(Parameters.Voxel_PageIndex, Parameters.Voxel_ClusterIndex);

	Parameters.Voxel_ChunkIndicesIndex = GetChunkIndicesIndexInCluster(Cluster, Parameters.Voxel_ClusterIndex);

	Parameters.Voxel_VertexIndicesInChunk.r = GetVertexIndexInVoxelChunk(Cluster, Parameters.Voxel_ClusterIndex, Parameters.Voxel_TriIndices.r);
	Parameters.Voxel_VertexIndicesInChunk.g = GetVertexIndexInVoxelChunk(Cluster, Parameters.Voxel_ClusterIndex, Parameters.Voxel_TriIndices.g);
	Parameters.Voxel_VertexIndicesInChunk.b = GetVertexIndexInVoxelChunk(Cluster, Parameters.Voxel_ClusterIndex, Parameters.Voxel_TriIndices.b);

#define VOXEL_NANITE_MATERIAL 1
}
#endif
)";

	Writer.AddCodeChunk(MCT_VoidStatement, TEXT("%s"), *Code);

	return Input.Compile(Compiler);
}

void UMaterialExpressionVoxelNaniteMaterialHook::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add("Voxel Nanite Material Hook");
}

FExpressionInput* UMaterialExpressionVoxelNaniteMaterialHook::GetInput(const int32 InputIndex)
{
	if (InputIndex != 0)
	{
		return nullptr;
	}

	return &Input;
}

TArrayView<FExpressionInput*> UMaterialExpressionVoxelNaniteMaterialHook::GetInputsView()
{
	CachedInputs.Empty();
	CachedInputs.Add(&Input);
	return CachedInputs;
}
#endif