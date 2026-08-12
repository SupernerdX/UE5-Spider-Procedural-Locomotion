// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Materials/MaterialExpression.h"
#include "MegaMaterial/VoxelRenderMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialTarget.h"
#include "MaterialExpressionVoxelMegaMaterialInternalSwitch.generated.h"

USTRUCT()
struct FVoxelMegaMaterialSwitchInputs
{
	GENERATED_BODY()

	static constexpr int32 Num = 14;

public:
	UPROPERTY()
	FExpressionInput Attributes;

public:
	/** Input for Base Color to output to virtual texture. */
	UPROPERTY()
	FExpressionInput BaseColor;

	/** Input for Specular to output to virtual texture. */
	UPROPERTY()
	FExpressionInput Specular;

	/** Input for Roughness to output to virtual texture. */
	UPROPERTY()
	FExpressionInput Roughness;

	/** Input for Surface Normal to output to virtual texture. */
	UPROPERTY()
	FExpressionInput Normal;

	/** Input for World Height to output to virtual texture. */
	UPROPERTY()
	FExpressionInput WorldHeight;

	/** Input for Opacity value used for blending to virtual texture. */
	UPROPERTY()
	FExpressionInput Opacity;

	/** Input for Mask to output to virtual texture. */
	UPROPERTY()
	FExpressionInput Mask;

	/** Input for World Height to output to virtual texture. */
	UPROPERTY()
	FExpressionInput Displacement;

	/** Input for Mask to output to virtual texture. */
	UPROPERTY()
	FExpressionInput Mask4;

public:
	/** Input for scattering coefficient describing how light scatter around and is absorbed. Valid range is [0,+inf[. Unit is 1/cm. */
	UPROPERTY()
	FExpressionInput ScatteringCoefficients;

	/** Input for scattering coefficient describing how light bounce is absorbed. Valid range is [0,+inf[. Unit is 1/cm. */
	UPROPERTY()
	FExpressionInput AbsorptionCoefficients;

	/** Input for phase function 'g' parameter describing how much forward(g>0) or backward (g<0) light scatter around. Valid range is [-1,1]. */
	UPROPERTY()
	FExpressionInput PhaseG;

	/** Input for custom color multiplier for scene color behind water. Can be used for caustics textures etc. Defaults to 1.0. Valid range is [0,+inf[. */
	UPROPERTY()
	FExpressionInput ColorScaleBehindWater;

public:
	UPROPERTY()
	bool bTangentSpaceNormal = false;

	UPROPERTY()
	FDisplacementScaling DisplacementScaling;
};

UCLASS(meta = (Private))
class UMaterialExpressionVoxelMegaMaterialInternalSwitch : public UMaterialExpression
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Config")
	EVoxelMegaMaterialTarget Target = {};

	UPROPERTY(VisibleAnywhere, Category = "Config")
	bool bEnablePixelDepthOffset = false;

	UPROPERTY(VisibleAnywhere, Category = "Config")
	bool bEnableSmoothBlends = false;

	UPROPERTY(VisibleAnywhere, Category = "Config")
	bool bEnableDitherNoiseTexture = false;

	UPROPERTY()
	TMap<FVoxelMaterialRenderIndex, FVoxelMegaMaterialSwitchInputs> IndexToInputs;

	UMaterialExpressionVoxelMegaMaterialInternalSwitch();

	//~ Begin UMaterialExpression Interface
	virtual UObject* GetReferencedTexture() const override;
	virtual bool CanReferenceTexture() const override { return true; }

#if WITH_EDITOR
#if VOXEL_ENGINE_VERSION >= 506
	virtual EMaterialValueType GetOutputValueType(int32 OutputIndex) override;
	virtual EMaterialValueType GetInputValueType(int32 InputIndex) override;
#else
	virtual uint32 GetOutputType(int32 OutputIndex) override;
	virtual uint32 GetInputType(int32 InputIndex) override;
#endif
	virtual bool IsResultMaterialAttributes(int32 OutputIndex) override;
	virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
	virtual FName GetInputName(int32 InputIndex) const override;
	virtual FExpressionInput* GetInput(int32 InputIndex) override;
	virtual TArrayView<FExpressionInput*> GetInputsView() override;
#endif
	//~ End UMaterialExpression Interface
};