// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/MaterialExpressionVoxelMegaMaterialSwitch.h"
#include "MaterialCompiler.h"

UMaterialExpressionVoxelMegaMaterialSwitch::UMaterialExpressionVoxelMegaMaterialSwitch()
{
#if WITH_EDITORONLY_DATA
	MenuCategories.Add(INVTEXT("Voxel Plugin"));
#endif
}

void UMaterialExpressionVoxelMegaMaterialSwitch::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
int32 UMaterialExpressionVoxelMegaMaterialSwitch::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	if (bIsMegaMaterial)
	{
		return Voxel.Compile(Compiler);
	}

	return Default.Compile(Compiler);
}

void UMaterialExpressionVoxelMegaMaterialSwitch::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add("Voxel Mega Material Switch");
}
#endif