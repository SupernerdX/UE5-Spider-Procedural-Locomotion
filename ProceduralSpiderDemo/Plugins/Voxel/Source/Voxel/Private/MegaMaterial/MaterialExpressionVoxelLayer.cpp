// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/MaterialExpressionVoxelLayer.h"
#include "Materials/MaterialExpressionCustom.h"
#include "MaterialCompiler.h"

UMaterialExpressionVoxelLayer::UMaterialExpressionVoxelLayer()
{
	MenuCategories.Add(INVTEXT("Voxel Plugin"));

	Outputs.Reset();
	Outputs.Add({ "Layer Index" });
}

void UMaterialExpressionVoxelLayer::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
int32 UMaterialExpressionVoxelLayer::Compile(FMaterialCompiler* Compiler, const int32 OutputIndex)
{
	UMaterialExpressionCustom* Custom = NewObject<UMaterialExpressionCustom>();
	Custom->Code.Reset();
	Custom->Inputs.Reset();
	Custom->OutputType = CMOT_Float1;

	Custom->Code = R"(
#if MATERIAL_VERTEX_PARAMETERS_VOXEL_VERSION == 6 && MATERIAL_PIXEL_PARAMETERS_VOXEL_VERSION == 8
	return Parameters.Voxel_LayerIndex;
#else
	return 0;
#endif)";

	TArray<int32> Inputs;
	return Compiler->CustomExpression(Custom, 0, Inputs);
}

void UMaterialExpressionVoxelLayer::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add("Voxel Layer");
}
#endif