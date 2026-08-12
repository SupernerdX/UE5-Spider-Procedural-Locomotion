// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/MaterialExpressionGetVoxelFlatNormal.h"
#include "MaterialCompiler.h"
#include "Materials/MaterialExpressionCustom.h"

UMaterialExpressionGetVoxelFlatNormal::UMaterialExpressionGetVoxelFlatNormal()
{
	MenuCategories.Add(INVTEXT("Voxel Plugin"));

	Outputs.Reset();
	Outputs.Add({ "Value" });
}

void UMaterialExpressionGetVoxelFlatNormal::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
int32 UMaterialExpressionGetVoxelFlatNormal::Compile(FMaterialCompiler* Compiler, const int32 OutputIndex)
{
	VOXEL_FUNCTION_COUNTER();

	UMaterialExpressionCustom* Custom = NewObject<UMaterialExpressionCustom>();
	Custom->Code.Reset();
	Custom->Inputs.Reset();
	Custom->OutputType = CMOT_Float3;

	Custom->Code = R"(
#if MATERIAL_VERTEX_PARAMETERS_VOXEL_VERSION == 6 && MATERIAL_PIXEL_PARAMETERS_VOXEL_VERSION == 8 && (IS_NANITE_PASS || VOXEL_VERTEX_FACTORY)
#if VERTEX_SHADER
	#error "Get Voxel Flat Normal cannot be used in vertex shaders";
#else
	return Parameters.Voxel_FlatNormal;
#endif
#else
	return Parameters.TangentToWorld[2];
#endif)";

	TArray<int32> Inputs;
	return Compiler->CustomExpression(Custom, 0, Inputs);
}

void UMaterialExpressionGetVoxelFlatNormal::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add("Get Voxel Flat Normal");
}
#endif