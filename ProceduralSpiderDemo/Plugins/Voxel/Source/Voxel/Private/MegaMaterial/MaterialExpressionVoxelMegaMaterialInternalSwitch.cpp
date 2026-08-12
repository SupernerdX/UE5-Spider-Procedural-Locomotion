// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/MaterialExpressionVoxelMegaMaterialInternalSwitch.h"
#include "MegaMaterial/VoxelMegaMaterialSwitch.h"
#include "MaterialCompiler.h"
#include "Engine/Texture2D.h"
#include "VoxelHLSLMaterialTranslator.h"
#include "Materials/MaterialAttributeDefinitionMap.h"

UMaterialExpressionVoxelMegaMaterialInternalSwitch::UMaterialExpressionVoxelMegaMaterialInternalSwitch()
{
	Outputs.Reset();
	Outputs.Add(FExpressionOutput("Attributes"));

	Outputs.Add(FExpressionOutput("BaseColor"));
	Outputs.Add(FExpressionOutput("Specular"));
	Outputs.Add(FExpressionOutput("Roughness"));
	Outputs.Add(FExpressionOutput("Normal"));
	Outputs.Add(FExpressionOutput("WorldHeight"));
	Outputs.Add(FExpressionOutput("Opacity"));
	Outputs.Add(FExpressionOutput("Mask"));
	Outputs.Add(FExpressionOutput("Displacement"));
	Outputs.Add(FExpressionOutput("Mask4"));

	Outputs.Add(FExpressionOutput("ScatteringCoefficients"));
	Outputs.Add(FExpressionOutput("AbsorptionCoefficients"));
	Outputs.Add(FExpressionOutput("PhaseG"));
	Outputs.Add(FExpressionOutput("ColorScaleBehindWater"));
}

UObject* UMaterialExpressionVoxelMegaMaterialInternalSwitch::GetReferencedTexture() const
{
	return FVoxelTextureUtilities::GetDefaultTexture2D();
}

#if WITH_EDITOR
#if VOXEL_ENGINE_VERSION >= 506
EMaterialValueType UMaterialExpressionVoxelMegaMaterialInternalSwitch::GetOutputValueType(const int32 OutputIndex)
#else
uint32 UMaterialExpressionVoxelMegaMaterialInternalSwitch::GetOutputType(const int32 OutputIndex)
#endif
{
	switch (OutputIndex)
	{
	default: ensure(false); return MCT_Float;
	case 0: return MCT_MaterialAttributes;
	case 1: return MCT_Float3;
	case 2: return MCT_Float;
	case 3: return MCT_Float;
	case 4: return MCT_Float3;
	case 5: return MCT_Float;
	case 6: return MCT_Float;
	case 7: return MCT_Float;
	case 8: return MCT_Float;
	case 9: return MCT_Float4;
	case 10: return MCT_Float3;
	case 11: return MCT_Float3;
	case 12: return MCT_Float;
	case 13: return MCT_Float;
	}
}

#if VOXEL_ENGINE_VERSION >= 506
EMaterialValueType UMaterialExpressionVoxelMegaMaterialInternalSwitch::GetInputValueType(const int32 InputIndex)
#else
uint32 UMaterialExpressionVoxelMegaMaterialInternalSwitch::GetInputType(const int32 InputIndex)
#endif
{
	switch (InputIndex % 14)
	{
	default: ensure(false); return MCT_Float;
	case 0: return MCT_MaterialAttributes;
	case 1: return MCT_Float3;
	case 2: return MCT_Float;
	case 3: return MCT_Float;
	case 4: return MCT_Float3;
	case 5: return MCT_Float;
	case 6: return MCT_Float;
	case 7: return MCT_Float;
	case 8: return MCT_Float;
	case 9: return MCT_Float4;
	case 10: return MCT_Float3;
	case 11: return MCT_Float3;
	case 12: return MCT_Float;
	case 13: return MCT_Float;
	}
}

bool UMaterialExpressionVoxelMegaMaterialInternalSwitch::IsResultMaterialAttributes(const int32 OutputIndex)
{
	return OutputIndex == 0;
}

int32 UMaterialExpressionVoxelMegaMaterialInternalSwitch::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	switch (OutputIndex)
	{
	default: ensure(false); return -1;
	case 0: return FVoxelMegaMaterialSwitch(*this, *Compiler).Compile();
	case 1: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::BaseColor, MCT_Float3);
	case 2: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::Specular, MCT_Float);
	case 3: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::Roughness, MCT_Float);
	case 4: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::Normal, MCT_Float3);
	case 5: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::WorldHeight, MCT_Float);
	case 6: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::Opacity, MCT_Float);
	case 7: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::Mask, MCT_Float);
	case 8: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::Displacement, MCT_Float);
	case 9: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::Mask4, MCT_Float4);
	case 10: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::ScatteringCoefficients, MCT_Float3);
	case 11: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::AbsorptionCoefficients, MCT_Float3);
	case 12: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::PhaseG, MCT_Float);
	case 13: return FVoxelMegaMaterialSwitch(*this, *Compiler).CompileCustomOutput(&FVoxelMegaMaterialSwitchInputs::ColorScaleBehindWater, MCT_Float);
	}
}

void UMaterialExpressionVoxelMegaMaterialInternalSwitch::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add("Voxel Mega Material Switch");
}

FName UMaterialExpressionVoxelMegaMaterialInternalSwitch::GetInputName(const int32 InputIndex) const
{
	const FString Name = INLINE_LAMBDA
	{
		switch (InputIndex % FVoxelMegaMaterialSwitchInputs::Num)
		{
		default: ensure(false);
		case 0: return "Attributes";
		case 1: return "BaseColor";
		case 2: return "Specular";
		case 3: return "Roughness";
		case 4: return "Normal";
		case 5: return "WorldHeight";
		case 6: return "Opacity";
		case 7: return "Mask";
		case 8: return "Displacement";
		case 9: return "Mask4";
		case 10: return "ScatteringCoefficients";
		case 11: return "AbsorptionCoefficients";
		case 12: return "PhaseG";
		case 13: return "ColorScaleBehindWater";
		}
	};

	return FName(FString::Printf(TEXT("Output %d %s"), InputIndex / FVoxelMegaMaterialSwitchInputs::Num, *Name));
}

FExpressionInput* UMaterialExpressionVoxelMegaMaterialInternalSwitch::GetInput(const int32 InputIndex)
{
	if (!IndexToInputs.Contains(FVoxelMaterialRenderIndex(InputIndex / FVoxelMegaMaterialSwitchInputs::Num)))
	{
		return nullptr;
	}
	FVoxelMegaMaterialSwitchInputs& Inputs = IndexToInputs[FVoxelMaterialRenderIndex(InputIndex / FVoxelMegaMaterialSwitchInputs::Num)];

	switch (InputIndex % FVoxelMegaMaterialSwitchInputs::Num)
	{
	default: ensure(false); return nullptr;
	case 0: return &Inputs.Attributes;
	case 1: return &Inputs.BaseColor;
	case 2: return &Inputs.Specular;
	case 3: return &Inputs.Roughness;
	case 4: return &Inputs.Normal;
	case 5: return &Inputs.WorldHeight;
	case 6: return &Inputs.Opacity;
	case 7: return &Inputs.Mask;
	case 8: return &Inputs.Displacement;
	case 9: return &Inputs.Mask4;
	case 10: return &Inputs.ScatteringCoefficients;
	case 11: return &Inputs.AbsorptionCoefficients;
	case 12: return &Inputs.PhaseG;
	case 13: return &Inputs.ColorScaleBehindWater;
	}
}

TArrayView<FExpressionInput*> UMaterialExpressionVoxelMegaMaterialInternalSwitch::GetInputsView()
{
	CachedInputs.Empty();
	CachedInputs.Reserve(IndexToInputs.Num());

	for (auto& It : IndexToInputs)
	{
		CachedInputs.Add(&It.Value.Attributes);
		CachedInputs.Add(&It.Value.BaseColor);
		CachedInputs.Add(&It.Value.Specular);
		CachedInputs.Add(&It.Value.Roughness);
		CachedInputs.Add(&It.Value.Normal);
		CachedInputs.Add(&It.Value.WorldHeight);
		CachedInputs.Add(&It.Value.Opacity);
		CachedInputs.Add(&It.Value.Mask);
		CachedInputs.Add(&It.Value.Displacement);
		CachedInputs.Add(&It.Value.Mask4);
		CachedInputs.Add(&It.Value.ScatteringCoefficients);
		CachedInputs.Add(&It.Value.AbsorptionCoefficients);
		CachedInputs.Add(&It.Value.PhaseG);
		CachedInputs.Add(&It.Value.ColorScaleBehindWater);
	}

	return CachedInputs;
}
#endif