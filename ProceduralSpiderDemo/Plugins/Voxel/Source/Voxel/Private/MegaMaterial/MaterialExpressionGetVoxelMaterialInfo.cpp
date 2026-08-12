// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/MaterialExpressionGetVoxelMaterialInfo.h"
#include "Materials/MaterialExpressionCustom.h"
#include "MaterialCompiler.h"

UMaterialExpressionGetVoxelMaterialInfo::UMaterialExpressionGetVoxelMaterialInfo()
{
	MenuCategories.Add(INVTEXT("Voxel Plugin"));

	bShowOutputNameOnPin = true;

	Outputs.Reset();

	Outputs.Add({ "Layer 0" });
	Outputs.Add({ "Layer 1" });
	Outputs.Add({ "Layer 2" });
	Outputs.Add({ "Layer 3" });
	Outputs.Add({ "Layer 4" });
	Outputs.Add({ "Layer 5" });
	Outputs.Add({ "Layer 6" });
	Outputs.Add({ "Layer 7" });

	Outputs.Add({ "Weight 0" });
	Outputs.Add({ "Weight 1" });
	Outputs.Add({ "Weight 2" });
	Outputs.Add({ "Weight 3" });
	Outputs.Add({ "Weight 4" });
	Outputs.Add({ "Weight 5" });
	Outputs.Add({ "Weight 6" });
	Outputs.Add({ "Weight 7" });
}

void UMaterialExpressionGetVoxelMaterialInfo::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
int32 UMaterialExpressionGetVoxelMaterialInfo::Compile(FMaterialCompiler* Compiler, const int32 OutputIndex)
{
	UMaterialExpressionCustom* Custom = NewObject<UMaterialExpressionCustom>();
	Custom->Code.Reset();
	Custom->Inputs.Reset();
	Custom->OutputType = CMOT_Float1;

	Custom->Code = R"(
#if MATERIAL_VERTEX_PARAMETERS_VOXEL_VERSION == 6 && MATERIAL_PIXEL_PARAMETERS_VOXEL_VERSION == 8
	return TOKEN;
#else
	return 0;
#endif)";

	Custom->Code.ReplaceInline(TEXT("TOKEN"), *(INLINE_LAMBDA
	{
		if (OutputIndex < 8)
		{
			return "VoxelLayerMask_GetLayer(Parameters.Voxel_LayerMask, " + FString::FromInt(OutputIndex) + ")";
		}
		else
		{
			return "Parameters.Voxel_LayerWeights[" + FString::FromInt(OutputIndex - 8) + "]";
		}
	}));

	TArray<int32> Inputs;
	return Compiler->CustomExpression(Custom, 0, Inputs);
}

void UMaterialExpressionGetVoxelMaterialInfo::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add("Get Voxel Material Info");
}
#endif