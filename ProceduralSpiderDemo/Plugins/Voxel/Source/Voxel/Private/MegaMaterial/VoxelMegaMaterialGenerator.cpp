// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelMegaMaterialGenerator.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialPrivateAccess.h"
#include "MegaMaterial/MaterialExpressionGetVoxelRandom.h"
#include "MegaMaterial/MaterialExpressionGetVoxelMetadata.h"
#include "MegaMaterial/MaterialExpressionVoxelNaniteMaterialHook.h"
#include "MegaMaterial/MaterialExpressionVoxelMegaMaterialSwitch.h"
#include "MegaMaterial/MaterialExpressionVoxelMegaMaterialInternalSwitch.h"
#include "VoxelMaterialGenerator.h"
#include "Surface/VoxelSurfaceTypeAsset.h"

#if WITH_EDITOR
#include "MaterialDomain.h"
#include "MaterialShared.h"
#include "Engine/Texture2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionComment.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionShadingModel.h"
#include "Materials/MaterialAttributeDefinitionMap.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpressionSetMaterialAttributes.h"
#include "Materials/MaterialExpressionRuntimeVirtualTextureOutput.h"
#include "Materials/MaterialExpressionSingleLayerWaterMaterialOutput.h"
#endif

#if WITH_EDITOR
DEFINE_PRIVATE_ACCESS(UMaterialInstance, StaticParametersRuntime)

#if !INTELLISENSE_PARSER
UClass* UMaterialExpressionRuntimeVirtualTextureOutput::GetPrivateStaticClass()
{
	static UClass* Class = FindObjectChecked<UClass>(nullptr, TEXT("/Script/Engine.MaterialExpressionRuntimeVirtualTextureOutput"));
	check(Class);
	return Class;
}
#endif
#endif

#if WITH_EDITOR
bool FVoxelMegaMaterialGenerator::GenerateMaterialForTarget(
	const TVoxelObjectPtr<const UObject> ErrorOwner,
	const FVoxelMegaMaterialTargetState& State,
	UMaterial& NewMaterial,
	UMaterialInstanceConstant& NewMaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();

	check(NewMaterialInstance.Parent == &NewMaterial);

	UTexture2D* DitherNoiseTexture = State.MegaMaterial.DitherNoiseTexture.LoadSynchronous();

	TUniquePtr<FMaterialUpdateContext> UpdateContext = MakeUnique<FMaterialUpdateContext>();
	UpdateContext->AddMaterial(&NewMaterial);
	UpdateContext->AddMaterialInstance(&NewMaterialInstance);

	NewMaterial.bUseMaterialAttributes = true;
	NewMaterial.bTangentSpaceNormal = false;
	NewMaterial.BlendMode = State.MegaMaterial.bGenerateMaskedMaterial ? BLEND_Masked : BLEND_Opaque;
	NewMaterial.TwoSided = State.MegaMaterial.bGenerateTwoSidedMaterial;
	NewMaterial.bHasPixelAnimation = State.MegaMaterial.bSetHasPixelAnimation;

	FVoxelUtilities::ClearMaterialExpressions(NewMaterial);

	TVoxelSet<EMaterialShadingModel> ShadingModels;
	for (const auto& It : State.IndexToSurfaceType)
	{
		ShadingModels.Add(It.Value.MaterialInterface.Material.ShadingModel);
	}

	if (ShadingModels.Num() == 1)
	{
		PrivateAccess::ShadingModel(NewMaterial) = ShadingModels.GetUniqueValue();
	}
	else
	{
		PrivateAccess::ShadingModel(NewMaterial) = MSM_FromMaterialExpression;

		if (ShadingModels.Contains(MSM_Unlit))
		{
			VOXEL_MESSAGE(Error, "{0}: unlit materials cannot be used with lit materials", ErrorOwner);
		}
	}

	TVoxelArray<TFunction<void(FMaterialInstanceParameterUpdateContext&)>> DelayedCopyParameterValues;

	if (State.MegaMaterial.CustomOutputsMaterial.Material.Object)
	{
		VOXEL_SCOPE_COUNTER_FNAME(State.MegaMaterial.CustomOutputsMaterial.Object->GetFName());

		const FString Prefix = "VOXELCUSTOMOUTPUT_";

		FVoxelMaterialGenerator Generator(
			ErrorOwner,
			NewMaterial,
			"VOXELCUSTOMOUTPUT_",
			false,
			ShouldDuplicateFunction);

		if (!ensure(Generator.CopyExpressions(*State.MegaMaterial.CustomOutputsMaterial.Material.Object)))
		{
			return false;
		}

		DelayedCopyParameterValues.Add([&State, &NewMaterialInstance, Prefix](FMaterialInstanceParameterUpdateContext& InstanceUpdateContext)
		{
			FVoxelUtilities::CopyParameterValues(
				InstanceUpdateContext,
				NewMaterialInstance,
				*State.MegaMaterial.CustomOutputsMaterial.Object,
				Prefix);
		});

		if (const FVoxelOptionalIntBox2D Bounds = Generator.GetBounds())
		{
			Generator.MoveExpressions(FIntPoint
			{
				-700 - Bounds->Max.X,
				-1000 - Bounds->Size().Y - Bounds->Min.Y
			});

			WrapInComment(
				NewMaterial,
				*Generator.GetBounds(),
				FString::Printf(TEXT("CustomOutputs: %s"), *State.MegaMaterial.CustomOutputsMaterial.Object->GetName()));
		}
	}

	bool bNeedRVTOutput = false;
	bool bNeedSingleLayerOutput = false;
	TVoxelMap<FVoxelMaterialRenderIndex, FVoxelMegaMaterialSwitchInputs> MaterialIndexToInputs;

	int32 PositionY = 0;
	for (const auto& It : State.IndexToSurfaceType)
	{
		const FVoxelMaterialRenderIndex RenderIndex = It.Key;
		const FVoxelSurfaceTypeState& SurfaceType = It.Value;

		if (!ensure(SurfaceType.Object) ||
			!ensure(SurfaceType.MaterialInterface.Material.Object))
		{
			continue;
		}

		const UMaterial& Material = *SurfaceType.MaterialInterface.Material.Object;

		VOXEL_SCOPE_COUNTER_FNAME(SurfaceType.Object->GetFName());

		const FString Prefix = "VOXELMATERIAL_" + FString::FromInt(RenderIndex.Index) + "_";

		FVoxelMaterialGenerator Generator(
			ErrorOwner,
			NewMaterial,
			Prefix,
			true,
			ShouldDuplicateFunction);

		FVoxelMegaMaterialSwitchInputs& Inputs = MaterialIndexToInputs.Add_EnsureNew(RenderIndex);
		{
			const TVoxelOptional<FMaterialAttributesInput> Input = Generator.CopyExpressions(Material);
			if (!ensure(Input))
			{
				return false;
			}

			Inputs.Attributes = FExpressionInput(*Input);
		}

		int32 AttributeIndex = 0;

		const auto TranslateFloat1 = [&](FExpressionInput Input, const float Constant)
		{
			if (Input.Expression)
			{
				Input.Expression = Generator.GetNewExpression(Input.Expression);
				ensure(Input.Expression);
				return Input;
			}

			UMaterialExpressionConstant& Expression = Generator.NewExpression<UMaterialExpressionConstant>();
			Expression.R = Constant;
			Expression.MaterialExpressionEditorX = Material.EditorX + 250;
			Expression.MaterialExpressionEditorY = Material.EditorY + (AttributeIndex++) * 50;

			FExpressionInput Result;
			Result.Expression = &Expression;
			return Result;
		};
		const auto TranslateFloat3 = [&](FExpressionInput Input, const FVector Constant)
		{
			if (Input.Expression)
			{
				Input.Expression = Generator.GetNewExpression(Input.Expression);
				ensure(Input.Expression);
				return Input;
			}

			UMaterialExpressionConstant3Vector& Expression = Generator.NewExpression<UMaterialExpressionConstant3Vector>();
			Expression.Constant = FLinearColor(Constant);
			Expression.MaterialExpressionEditorX = Material.EditorX + 250;
			Expression.MaterialExpressionEditorY = Material.EditorY + (AttributeIndex++) * 50;

			FExpressionInput Result;
			Result.Expression = &Expression;
			return Result;
		};
		const auto TranslateFloat4 = [&](FExpressionInput Input, const FLinearColor Constant)
		{
			if (Input.Expression)
			{
				Input.Expression = Generator.GetNewExpression(Input.Expression);
				ensure(Input.Expression);
				return Input;
			}

			UMaterialExpressionConstant4Vector& Expression = Generator.NewExpression<UMaterialExpressionConstant4Vector>();
			Expression.Constant = Constant;
			Expression.MaterialExpressionEditorX = Material.EditorX + 250;
			Expression.MaterialExpressionEditorY = Material.EditorY + (AttributeIndex++) * 50;

			FExpressionInput Result;
			Result.Expression = &Expression;
			return Result;
		};

		{
			const UMaterialExpressionRuntimeVirtualTextureOutput* RuntimeVirtualTextureOutput = INLINE_LAMBDA -> UMaterialExpressionRuntimeVirtualTextureOutput*
			{
				for (UMaterialExpression* Expression : FVoxelUtilities::GetMaterialExpressions(Material))
				{
					if (UMaterialExpressionRuntimeVirtualTextureOutput* Typed = Cast<UMaterialExpressionRuntimeVirtualTextureOutput>(Expression))
					{
						return Typed;
					}
				}

				return nullptr;
			};

			if (RuntimeVirtualTextureOutput)
			{
				bNeedRVTOutput = true;

				AttributeIndex = 0;

				// Match UMaterialExpressionRuntimeVirtualTextureOutput::Compile

				Inputs.BaseColor = TranslateFloat3(RuntimeVirtualTextureOutput->BaseColor, FVector(0.f, 0.f, 0.f));
				Inputs.Specular = TranslateFloat1(RuntimeVirtualTextureOutput->Specular, 0.5f);
				Inputs.Roughness = TranslateFloat1(RuntimeVirtualTextureOutput->Roughness, 0.5f);
				Inputs.Normal = TranslateFloat3(RuntimeVirtualTextureOutput->Normal, FVector(0.f, 0.f, 1.f));
				Inputs.WorldHeight = TranslateFloat1(RuntimeVirtualTextureOutput->WorldHeight, 0.f);
				Inputs.Opacity = TranslateFloat1(RuntimeVirtualTextureOutput->Opacity, 1.f);
				Inputs.Mask = TranslateFloat1(RuntimeVirtualTextureOutput->Mask, 1.f);
				Inputs.Displacement = TranslateFloat1(RuntimeVirtualTextureOutput->Displacement, 0.f);
				Inputs.Mask4 = TranslateFloat4(RuntimeVirtualTextureOutput->Mask4, FLinearColor(0.f, 0.f, 0.f, 0.f));
			}
		}

		{
			const UMaterialExpressionSingleLayerWaterMaterialOutput* SingleLayerWaterMaterialOutput = INLINE_LAMBDA -> UMaterialExpressionSingleLayerWaterMaterialOutput*
			{
				for (UMaterialExpression* Expression : FVoxelUtilities::GetMaterialExpressions(Material))
				{
					if (UMaterialExpressionSingleLayerWaterMaterialOutput* Typed = Cast<UMaterialExpressionSingleLayerWaterMaterialOutput>(Expression))
					{
						return Typed;
					}
				}

				return nullptr;
			};

			if (SingleLayerWaterMaterialOutput)
			{
				bNeedSingleLayerOutput = true;

				AttributeIndex = 0;

				// Match UMaterialExpressionSingleLayerWaterMaterialOutput::Compile

				Inputs.ScatteringCoefficients = TranslateFloat3(SingleLayerWaterMaterialOutput->ScatteringCoefficients, FVector(0.f, 0.f, 0.f));
				Inputs.AbsorptionCoefficients = TranslateFloat3(SingleLayerWaterMaterialOutput->AbsorptionCoefficients, FVector(0.f, 0.f, 0.f));
				Inputs.PhaseG = TranslateFloat1(SingleLayerWaterMaterialOutput->PhaseG, 0.0f);
				Inputs.ColorScaleBehindWater = TranslateFloat1(SingleLayerWaterMaterialOutput->ColorScaleBehindWater, 1.f);
			}
		}

		if (ShadingModels.Num() > 1 &&
			SurfaceType.MaterialInterface.Material.ShadingModel != MSM_FromMaterialExpression)
		{
			UMaterialExpressionShadingModel& ShadingModel = Generator.NewExpression<UMaterialExpressionShadingModel>();
			ShadingModel.ShadingModel = PrivateAccess::ShadingModel(Material);
			ShadingModel.MaterialExpressionEditorX = Material.EditorX + 250;
			ShadingModel.MaterialExpressionEditorY = Material.EditorY + 100;

			UMaterialExpressionSetMaterialAttributes& SetMaterialAttributes = Generator.NewExpression<UMaterialExpressionSetMaterialAttributes>();
			SetMaterialAttributes.AttributeSetTypes = { FMaterialAttributeDefinitionMap::GetID(MP_ShadingModel) };
			SetMaterialAttributes.Inputs[0] = Inputs.Attributes;
			SetMaterialAttributes.Inputs.Emplace_GetRef().Expression = &ShadingModel;
			SetMaterialAttributes.MaterialExpressionEditorX = Material.EditorX + 500;
			SetMaterialAttributes.MaterialExpressionEditorY = Material.EditorY;

			Inputs.Attributes.Expression = &SetMaterialAttributes;
		}

		DelayedCopyParameterValues.Add([&NewMaterialInstance, SurfaceType, Prefix](FMaterialInstanceParameterUpdateContext& InstanceUpdateContext)
		{
			FVoxelUtilities::CopyParameterValues(
				InstanceUpdateContext,
				NewMaterialInstance,
				*SurfaceType.MaterialInterface.Object,
				Prefix);
		});

		Generator.ForeachExpression([&](UMaterialExpression& Expression)
		{
			UMaterialExpressionGetVoxelRandom* GetVoxelRandom = Cast<UMaterialExpressionGetVoxelRandom>(&Expression);
			if (!GetVoxelRandom)
			{
				return;
			}

			ensure(GetVoxelRandom->SurfaceTypeSeed.Seed.IsEmpty());
			GetVoxelRandom->SurfaceTypeSeed = SurfaceType.Seed;
		});

		if (const FVoxelOptionalIntBox2D Bounds = Generator.GetBounds())
		{
			Generator.MoveExpressions(FIntPoint
			{
				-700 - Bounds->Max.X,
				PositionY - Bounds->Min.Y
			});

			PositionY += Bounds->Size().Y + 1500;

			WrapInComment(
				NewMaterial,
				*Generator.GetBounds(),
				FString::Printf(TEXT("Material %d: %s"),
					RenderIndex.Index,
					*SurfaceType.MaterialInterface.Object->GetName()));
		}
	}

	NewMaterial.EditorX = 5700;
	NewMaterial.EditorY = PositionY / 2;

	{
		UMaterialExpressionVoxelMegaMaterialInternalSwitch& Switch = FVoxelUtilities::CreateMaterialExpression<UMaterialExpressionVoxelMegaMaterialInternalSwitch>(NewMaterial);
		Switch.MaterialExpressionEditorX = 5400;
		Switch.MaterialExpressionEditorY = PositionY / 2;
		Switch.Target = State.Target;
		Switch.bEnablePixelDepthOffset = State.MegaMaterial.bEnablePixelDepthOffset;
		Switch.bEnableSmoothBlends = INLINE_LAMBDA
		{
			if (State.Target == EVoxelMegaMaterialTarget::Lumen)
			{
				return false;
			}

			if (!State.MegaMaterial.bEnableSmoothBlends)
			{
				return false;
			}

			for (const auto& It : State.IndexToSurfaceType)
			{
				if (It.Value.BlendSmoothness > 0)
				{
					return true;
				}
			}

			// Disable if all smoothnesses are 0
			return false;
		};

		Switch.bEnableDitherNoiseTexture = State.MegaMaterial.bEnableDitherNoiseTexture && DitherNoiseTexture != nullptr;

		// Default material
		Switch.IndexToInputs.Add(FVoxelMaterialRenderIndex(0), {});

		float MaxMagnitude = 0;
		for (const auto& It : State.IndexToSurfaceType)
		{
			const FVoxelMaterialRenderIndex RenderIndex = It.Key;
			const FVoxelSurfaceTypeState& SurfaceType = It.Value;

			MaxMagnitude = FMath::Max(MaxMagnitude, SurfaceType.MaterialInterface.DisplacementScaling.Magnitude);

			FVoxelMegaMaterialSwitchInputs Inputs = MaterialIndexToInputs[RenderIndex];
			Inputs.bTangentSpaceNormal = SurfaceType.MaterialInterface.Material.bTangentSpaceNormal;
			Inputs.DisplacementScaling = SurfaceType.MaterialInterface.DisplacementScaling;

			ensure(!Switch.IndexToInputs.Contains(RenderIndex));
			Switch.IndexToInputs.Add(RenderIndex, Inputs);
		}

		NewMaterial.DisplacementScaling.Magnitude = MaxMagnitude;
		NewMaterial.DisplacementScaling.Center = 0.5f;

		NewMaterial.GetEditorOnlyData()->MaterialAttributes.Expression = &Switch;

		if (bNeedRVTOutput)
		{
			UMaterialExpressionRuntimeVirtualTextureOutput& Output = FVoxelUtilities::CreateMaterialExpression<UMaterialExpressionRuntimeVirtualTextureOutput>(NewMaterial);
			Output.MaterialExpressionEditorX = 5700;
			Output.MaterialExpressionEditorY = PositionY / 2 + 70;
			Output.BaseColor.Connect(1, &Switch);
			Output.Specular.Connect(2, &Switch);
			Output.Roughness.Connect(3, &Switch);
			Output.Normal.Connect(4, &Switch);
			Output.WorldHeight.Connect(5, &Switch);
			Output.Opacity.Connect(6, &Switch);
			Output.Mask.Connect(7, &Switch);
			Output.Displacement.Connect(8, &Switch);
			Output.Mask4.Connect(9, &Switch);
		}

		if (bNeedSingleLayerOutput)
		{
			UMaterialExpressionSingleLayerWaterMaterialOutput& Output = FVoxelUtilities::CreateMaterialExpression<UMaterialExpressionSingleLayerWaterMaterialOutput>(NewMaterial);
			Output.MaterialExpressionEditorX = 5700;
			Output.MaterialExpressionEditorY = PositionY / 2 + 140;
			Output.ScatteringCoefficients.Connect(10, &Switch);
			Output.AbsorptionCoefficients.Connect(11, &Switch);
			Output.PhaseG.Connect(12, &Switch);
			Output.ColorScaleBehindWater.Connect(13, &Switch);
		}
	}

	if (!AddAttributePostProcess(
		ErrorOwner,
		State.MegaMaterial,
		NewMaterial))
	{
		return false;
	}

	for (UMaterialExpression* Expression : FVoxelUtilities::GetMaterialExpressions_Recursive(NewMaterial))
	{
		if (UMaterialExpressionVoxelMegaMaterialSwitch* Switch = Cast<UMaterialExpressionVoxelMegaMaterialSwitch>(Expression))
		{
			if (!ensure(Expression->GetTypedOuter<UMaterial>() == &NewMaterial))
			{
				continue;
			}

			ensure(!Switch->bIsMegaMaterial);
			Switch->bIsMegaMaterial = true;

			Switch->Default = {};
		}

		if (UMaterialExpressionGetVoxelMetadata* GetVoxelMetadata = Cast<UMaterialExpressionGetVoxelMetadata>(Expression))
		{
			if (!ensure(Expression->GetTypedOuter<UMaterial>() == &NewMaterial))
			{
				continue;
			}

			ensure(GetVoxelMetadata->MetadataIndex == -1);

			if (GetVoxelMetadata->Metadata)
			{
				GetVoxelMetadata->MetadataIndex = State.MetadataIndexToMetadata.Find(GetVoxelMetadata->Metadata);
				ensure(GetVoxelMetadata->MetadataIndex != -1);
			}
		}
	}

	NewMaterial.PostEditChange();

	{
		// FMaterialInstanceParameterUpdateContext does not clear static switches
		PrivateAccess::StaticParametersRuntime(NewMaterialInstance).Empty();
		if (UMaterialInstanceEditorOnlyData* EditorOnly = NewMaterialInstance.GetEditorOnlyData())
		{
			EditorOnly->StaticParameters.Empty();
		}

		FMaterialInstanceParameterUpdateContext InstanceUpdateContext(&NewMaterialInstance, EMaterialInstanceClearParameterFlag::All);

		for (const TFunction<void(FMaterialInstanceParameterUpdateContext&)>& CopyParameterValues : DelayedCopyParameterValues)
		{
			CopyParameterValues(InstanceUpdateContext);
		}

		ApplyBlendSmoothness(
			State.IndexToSurfaceType,
			NewMaterial,
			NewMaterialInstance);

		TGuardValue<bool> Guard(GIsEditor, true);
		NewMaterialInstance.SetTextureParameterValueEditorOnly(FName("VOXEL_PixelNoiseTexture"), DitherNoiseTexture);
	}

	// Reduce SRVs
	FVoxelUtilities::MergeIdenticalTextureParameters(NewMaterial, NewMaterialInstance);

	NewMaterial.PostEditChange();

	return true;
}

bool FVoxelMegaMaterialGenerator::GenerateMaterial(
	const TVoxelObjectPtr<const UObject> ErrorOwner,
	const FVoxelMegaMaterialMaterialState& State,
	UMaterial& NewMaterial,
	UMaterialInstanceConstant& NewMaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();
	check(NewMaterialInstance.Parent == &NewMaterial);

	const UMaterial& Material = *State.SurfaceType.MaterialInterface.Material.Object;

	const TUniquePtr<FMaterialUpdateContext> UpdateContext = MakeUnique<FMaterialUpdateContext>();
	UpdateContext->AddMaterial(&NewMaterial);
	UpdateContext->AddMaterialInstance(&NewMaterialInstance);

	for (FProperty& Property : GetClassProperties<UMaterial>())
	{
		if (!Property.HasAllPropertyFlags(CPF_Edit) ||
			Property.HasAnyPropertyFlags(CPF_InstancedReference))
		{
			continue;
		}

		if (Property.GetName().StartsWith("bUsedWith"))
		{
			CastFieldChecked<FBoolProperty>(Property).SetPropertyValue_InContainer(&NewMaterial, false);
			continue;
		}

		Property.CopyCompleteValue_InContainer(&NewMaterial, &Material);
	}

#if VOXEL_ENGINE_VERSION >= 506
	// Otherwise "Cooking a material resource that doesn't have a valid ShaderMap"
	NewMaterial.bUsedWithStaticMesh = true;
#endif

	NewMaterial.bUsedWithNanite = true;
	NewMaterial.bEnableTessellation = true;

	if (State.MegaMaterial.bSetHasPixelAnimation)
	{
		NewMaterial.bHasPixelAnimation = true;
	}

	NewMaterialInstance.BasePropertyOverrides = {};

	for (const FVoxelMaterialInstanceBaseState& MaterialInstance : ReverseIterate(State.SurfaceType.MaterialInterface.MaterialInstanceHierarchy))
	{
		static_assert(sizeof(FMaterialInstanceBasePropertyOverrides) == 32, "Need to update code below");

#define COPY(Override, Value) \
		if (MaterialInstance.BasePropertyOverrides.Override) \
		{ \
			NewMaterialInstance.BasePropertyOverrides.Override = true; \
			NewMaterialInstance.BasePropertyOverrides.Value = MaterialInstance.BasePropertyOverrides.Value; \
		}

		COPY(bOverride_OpacityMaskClipValue, OpacityMaskClipValue);
		COPY(bOverride_BlendMode, BlendMode);
		COPY(bOverride_ShadingModel, ShadingModel);
		COPY(bOverride_DitheredLODTransition, DitheredLODTransition);
		COPY(bOverride_CastDynamicShadowAsMasked, bCastDynamicShadowAsMasked);
		COPY(bOverride_TwoSided, TwoSided);
		COPY(bOverride_bIsThinSurface, bIsThinSurface);
		COPY(bOverride_OutputTranslucentVelocity, bOutputTranslucentVelocity);
		COPY(bOverride_bHasPixelAnimation, bHasPixelAnimation);
		COPY(bOverride_bEnableTessellation, bEnableTessellation);
		COPY(bOverride_DisplacementScaling, DisplacementScaling);
		COPY(bOverride_bEnableDisplacementFade, bEnableDisplacementFade);
		COPY(bOverride_DisplacementFadeRange, DisplacementFadeRange);
		COPY(bOverride_MaxWorldPositionOffsetDisplacement, MaxWorldPositionOffsetDisplacement);
#if VOXEL_ENGINE_VERSION >= 506
		COPY(bOverride_CompatibleWithLumenCardSharing, bCompatibleWithLumenCardSharing);
#endif

#undef COPY
	}

	NewMaterial.bUseMaterialAttributes = true;

	FVoxelUtilities::ClearMaterialExpressions(NewMaterial);

	TVoxelArray<TFunction<void(FMaterialInstanceParameterUpdateContext&)>> DelayedCopyParameterValues;

	FVoxelMaterialGenerator Generator(
		ErrorOwner,
		NewMaterial,
		{},
		false,
		ShouldDuplicateFunction);

	const TVoxelOptional<FMaterialAttributesInput> Input = Generator.CopyExpressions(Material);
	if (!ensure(Input))
	{
		return false;
	}

	Generator.ForeachExpression([&](UMaterialExpression& Expression)
	{
		UMaterialExpressionGetVoxelRandom* GetVoxelRandom = Cast<UMaterialExpressionGetVoxelRandom>(&Expression);
		if (!GetVoxelRandom)
		{
			return;
		}

		ensure(GetVoxelRandom->SurfaceTypeSeed.Seed.IsEmpty());
		GetVoxelRandom->SurfaceTypeSeed = State.SurfaceType.Seed;
	});

	NewMaterial.EditorX = Material.EditorX + 500;
	NewMaterial.EditorY = Material.EditorY;

	{
		UMaterialExpressionVoxelNaniteMaterialHook& Hook = FVoxelUtilities::CreateMaterialExpression<UMaterialExpressionVoxelNaniteMaterialHook>(NewMaterial);
		Hook.MaterialExpressionEditorX = Material.EditorX + 200;
		Hook.MaterialExpressionEditorY = Material.EditorY;
		Hook.Input = FExpressionInput(*Input);

		NewMaterial.GetEditorOnlyData()->MaterialAttributes.Expression = &Hook;
	}

	if (!AddAttributePostProcess(
		ErrorOwner,
		State.MegaMaterial,
		NewMaterial))
	{
		return false;
	}

	for (UMaterialExpression* Expression : FVoxelUtilities::GetMaterialExpressions_Recursive(NewMaterial))
	{
		if (UMaterialExpressionGetVoxelMetadata* GetVoxelMetadata = Cast<UMaterialExpressionGetVoxelMetadata>(Expression))
		{
			if (!ensure(Expression->GetTypedOuter<UMaterial>() == &NewMaterial))
			{
				continue;
			}

			ensure(GetVoxelMetadata->MetadataIndex == -1);

			if (GetVoxelMetadata->Metadata)
			{
				GetVoxelMetadata->MetadataIndex = State.MetadataIndexToMetadata.Find(GetVoxelMetadata->Metadata);
				ensure(GetVoxelMetadata->MetadataIndex != -1);
			}
		}
	}

	NewMaterial.PostEditChange();

	FMaterialInstanceParameterUpdateContext InstanceUpdateContext(&NewMaterialInstance, EMaterialInstanceClearParameterFlag::All);

	FVoxelUtilities::CopyParameterValues(
		InstanceUpdateContext,
		NewMaterialInstance,
		*State.SurfaceType.MaterialInterface.Object,
		{});

	return true;
}

bool FVoxelMegaMaterialGenerator::ApplyBlendSmoothness(
	const TMap<FVoxelMaterialRenderIndex, FVoxelSurfaceTypeState>& IndexToSurfaceType,
	const UMaterial& NewMaterial,
	UMaterialInstanceConstant& MaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();

	bool bChanged = false;

	for (const auto& It : IndexToSurfaceType)
	{
		const FVoxelMaterialRenderIndex RenderIndex = It.Key;
		const FVoxelSurfaceTypeState& SurfaceType = It.Value;

		// Apply displacement scaling, but keep it relative to output magnitude just like displacement in FVoxelMegaMaterialSwitch
		const float Scale =
			SurfaceType.MaterialInterface.DisplacementScaling.Magnitude /
			NewMaterial.GetDisplacementScaling().Magnitude;

		const float NewValue = FMath::Max(SurfaceType.BlendSmoothness, 0.f) * Scale;

		const FName Name = FName(FString::Printf(TEXT("VOXEL_BlendSmoothness_%d"), RenderIndex.Index));

		float Value = 0.f;
		if (MaterialInstance.GetScalarParameterValue(Name, Value) &&
			Value == NewValue)
		{
			continue;
		}

		{
			TGuardValue<bool> Guard(GIsEditor, true);
			MaterialInstance.SetScalarParameterValueEditorOnly(Name, NewValue);
		}

		bChanged = true;
	}

	return bChanged;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelArray<UVoxelMetadata*> FVoxelMegaMaterialGenerator::GetUsedMetadatas(const UMaterial& Material)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelSet<UVoxelMetadata*> Metadatas;

	for (UMaterialExpression* Expression : FVoxelUtilities::GetMaterialExpressions_Recursive(Material))
	{
		const UMaterialExpressionGetVoxelMetadata* GetVoxelMetadata = Cast<UMaterialExpressionGetVoxelMetadata>(Expression);
		if (!GetVoxelMetadata)
		{
			continue;
		}
		ensure(GetVoxelMetadata->MetadataIndex == -1);

		if (!GetVoxelMetadata->Metadata)
		{
			continue;
		}

		Metadatas.Add(GetVoxelMetadata->Metadata);
	}

	return Metadatas.Array();
}

TVoxelArray<UVoxelMetadata*> FVoxelMegaMaterialGenerator::GetUsedMetadatas(const UMaterialFunction& MaterialFunction)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelSet<UVoxelMetadata*> Metadatas;

	for (UMaterialExpression* Expression : FVoxelUtilities::GetMaterialExpressions_Recursive(MaterialFunction))
	{
		const UMaterialExpressionGetVoxelMetadata* GetVoxelMetadata = Cast<UMaterialExpressionGetVoxelMetadata>(Expression);
		if (!GetVoxelMetadata)
		{
			continue;
		}
		ensure(GetVoxelMetadata->MetadataIndex == -1);

		if (!GetVoxelMetadata->Metadata)
		{
			continue;
		}

		Metadatas.Add(GetVoxelMetadata->Metadata);
	}

	return Metadatas.Array();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelMegaMaterialGenerator::AddAttributePostProcess(
	const TVoxelObjectPtr<const UObject> ErrorOwner,
	const FVoxelMegaMaterialState& MegaMaterial,
	UMaterial& Material)
{
	VOXEL_FUNCTION_COUNTER();

	if (!MegaMaterial.AttributePostProcess.Object)
	{
		return true;
	}

	{
		TArray<FFunctionExpressionInput> Inputs;
		TArray<FFunctionExpressionOutput> Outputs;
		MegaMaterial.AttributePostProcess.Object->GetInputsAndOutputs(Inputs, Outputs);

		if (Inputs.Num() != 1 ||
			Outputs.Num() != 1)
		{
			VOXEL_MESSAGE(Error, "{0}: MegaMaterial AttributePostProcess should have exactly one input and one output, both of the type MaterialAttributes", ErrorOwner);
			return false;
		}
	}

	FVoxelMaterialGenerator Generator(
		ErrorOwner,
		Material,
		{},
		false,
		ShouldDuplicateFunction);

	UMaterialFunction* NewFunction = Generator.DuplicateFunctionIfNeeded(*MegaMaterial.AttributePostProcess.Object);
	if (!ensureVoxelSlow(NewFunction))
	{
		return false;
	}

	UMaterialExpressionMaterialFunctionCall& FunctionCall = FVoxelUtilities::CreateMaterialExpression<UMaterialExpressionMaterialFunctionCall>(Material);
	FunctionCall.SetMaterialFunction(NewFunction);
	FunctionCall.MaterialExpressionEditorX = Material.EditorX;
	FunctionCall.MaterialExpressionEditorY = Material.EditorY;
	FunctionCall.FunctionInputs[0].Input = FExpressionInput(Material.GetEditorOnlyData()->MaterialAttributes);

	Material.EditorX += 200;

	Material.GetEditorOnlyData()->MaterialAttributes.Expression = &FunctionCall;

	return true;
}

void FVoxelMegaMaterialGenerator::WrapInComment(
	UMaterial& Material,
	const FVoxelIntBox2D& Bounds,
	const FString& Comment)
{
	const FVoxelIntBox2D FinalBounds = Bounds.Extend(100);

	UMaterialExpressionComment& Expression = FVoxelUtilities::CreateMaterialExpression<UMaterialExpressionComment>(Material);
	Expression.Text = Comment;
	Expression.SizeX = FinalBounds.Size().X + 200;
	Expression.SizeY = FinalBounds.Size().Y + 500;
	Expression.MaterialExpressionEditorX = FinalBounds.Min.X;
	Expression.MaterialExpressionEditorY = FinalBounds.Min.Y;
}

bool FVoxelMegaMaterialGenerator::ShouldDuplicateFunction(const UMaterialExpression& Expression)
{
	return
		Expression.IsA<UMaterialExpressionGetVoxelRandom>() ||
		Expression.IsA<UMaterialExpressionGetVoxelMetadata>() ||
		Expression.IsA<UMaterialExpressionVoxelMegaMaterialSwitch>();
}
#endif