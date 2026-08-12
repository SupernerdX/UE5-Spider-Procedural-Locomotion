// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelMegaMaterialSwitch.h"
#include "VoxelHLSLMaterialTranslator.h"
#include "Materials/MaterialAttributeDefinitionMap.h"

#define AddLine(Format, ...) AddLineImpl(FString::Printf(TEXT(Format), ##__VA_ARGS__))

#if WITH_EDITOR
FVoxelMegaMaterialSwitch::FVoxelMegaMaterialSwitch(
	const UMaterialExpressionVoxelMegaMaterialInternalSwitch& Switch,
	FMaterialCompiler& Compiler)
	: Switch(Switch)
	, Compiler(Compiler)
	, Translator(static_cast<FVoxelHLSLMaterialTranslator&>(Compiler))
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 FVoxelMegaMaterialSwitch::Compile()
{
	VOXEL_FUNCTION_COUNTER();

	ON_SCOPE_EXIT
	{
		if (Translator.VTStacks.Num() > 128)
		{
			Compiler.Errorf(TEXT("Too many Virtual Textures used: %d"), Translator.VTStacks.Num());
		}
	};

	if (Compiler.GetCurrentShaderFrequency() != SF_Vertex &&
		Compiler.GetCurrentShaderFrequency() != SF_Pixel)
	{
		// TODO
		return -1;
	}

	const EMaterialProperty CurrentProperty = FMaterialAttributeDefinitionMap::GetProperty(Compiler.GetMaterialAttribute());
	if (!HasVoxelTag())
	{
		AddVoxelTag();

		InitializeDefaultProperties();
		ComputeDisplacementAndLayerMask(CurrentProperty != MP_WorldPositionOffset);

		TVoxelArray<EMaterialProperty> Properties;
		if (IsVertex())
		{
			Properties.Add(MP_WorldPositionOffset);
			Properties.Add(MP_CustomizedUVs0);
			Properties.Add(MP_CustomizedUVs1);
			Properties.Add(MP_CustomizedUVs2);
			Properties.Add(MP_CustomizedUVs3);
			Properties.Add(MP_CustomizedUVs4);
			Properties.Add(MP_CustomizedUVs5);
			Properties.Add(MP_CustomizedUVs6);
			Properties.Add(MP_CustomizedUVs7);
		}
		else
		{
			// Compute normal first
			Properties.Add(MP_Normal);

			for (int32 PropertyIndex = 0; PropertyIndex < int32(MP_MAX); PropertyIndex++)
			{
				const EMaterialProperty Property = EMaterialProperty(PropertyIndex);

				if (Property == MP_Displacement ||
					Property == MP_CustomOutput ||
					Property == MP_FrontMaterial ||
					Property == MP_MaterialAttributes)
				{
					continue;
				}

				Properties.AddUnique(Property);
			}
		}

		ComputeProperties(Properties);

		// Forbid reuse in AttributePostProcess
		FVoxelMaterialTranslatorNoCodeReuseScope::DisableFutureReuse(Translator);
	}

	const EMaterialValueType ValueType = FMaterialAttributeDefinitionMap::GetValueType(CurrentProperty);

	if (CurrentProperty == MP_PixelDepthOffset)
	{
		if (!Switch.bEnablePixelDepthOffset)
		{
			return -1;
		}

		if (Switch.Target != EVoxelMegaMaterialTarget::NonNanite)
		{
			// PixelDepthOffset breaks displacement
			return -1;
		}
	}

	if (CurrentProperty == MP_BaseColor &&
		!IsVertex())
	{
		AddLine("#ifdef VOXEL_MEGA_MATERIAL_DEBUG");
		AddLine("#if VOXEL_MEGA_MATERIAL_DEBUG && VOXEL_HOOK");
		ON_SCOPE_EXIT
		{
			AddLine("#endif");
			AddLine("#endif");
		};

		AddLine("if (Parameters.Voxel_DebugMode == 1)");
		AddLine("{");
		{
			FIndentScope Scope(*this);

			AddLine("switch (NumLayersQueried)");
			AddLine("{");
			AddLine(" case 0: Voxel_BaseColor = float3(1.0, 1.0, 1.0); break;");
			AddLine(" case 1: Voxel_BaseColor = float3(0.0, 1.0, 0.0); break;");
			AddLine(" case 2: Voxel_BaseColor = float3(0.0, 1.0, 1.0); break;");
			AddLine(" case 3: Voxel_BaseColor = float3(1.0, 1.0, 0.0); break;");
			AddLine(" case 4: Voxel_BaseColor = float3(1.0, 0.5, 0.0); break;");
			AddLine(" case 5: Voxel_BaseColor = float3(1.0, 0.0, 0.0); break;");
			AddLine(" case 6: Voxel_BaseColor = float3(1.0, 0.0, 1.0); break;");
			AddLine(" case 7: Voxel_BaseColor = float3(0.5, 0.0, 1.0); break;");
			AddLine(" case 8: Voxel_BaseColor = float3(0.0, 0.0, 1.0); break;");
			AddLine(" default: Voxel_BaseColor = float3(0.0, 0.0, 0.0); break;");
			AddLine("}");
		}
		AddLine("}");

		AddLine("if (Parameters.Voxel_DebugMode == 2)");
		AddLine("{");
		{
			FIndentScope Scope(*this);

			AddLine("switch (NumNeighborsQueried)");
			AddLine("{");
			AddLine("case 0: Voxel_BaseColor = float3(1, 1, 1); break;");
			AddLine("case 1: Voxel_BaseColor = float3(0, 1, 0); break;");
			AddLine("case 2: Voxel_BaseColor = float3(1, 1, 0); break;");
			AddLine("case 3: Voxel_BaseColor = float3(1, 0, 0); break;");
			AddLine("case 4: Voxel_BaseColor = float3(1, 0, 1); break;");
			AddLine("default: Voxel_BaseColor = float3(0, 0, 0); break;");
			AddLine("}");
		}
		AddLine("}");

		AddLine("if (Parameters.Voxel_DebugMode == 3)");
		AddLine("{");
		{
			FIndentScope Scope(*this);

			AddLine("Voxel_BaseColor = DitherBiasDebug;");
		}
		AddLine("}");
	}

	if (CurrentProperty == MP_Displacement)
	{
		AddLine("#if VOXEL_HOOK");
		AddLine("const float VoxelDisplacement = Parameters.Voxel_Displacement;");
		AddLine("#else");
		AddLine("const float VoxelDisplacement = 0;");
		AddLine("#endif");

		return Translator.AddCodeChunk(ValueType, TEXT("VoxelDisplacement"));
	}

	const TVoxelSet<EMaterialProperty> ConnectedProperties = GetConnectedProperties();
	if (!ConnectedProperties.Contains(CurrentProperty))
	{
		// Ensure FMaterialAttributesInput::CompileWithDefault does SetConnectedProperty properly
		return -1;
	}

	return Translator.AddCodeChunk(
		ValueType,
		TEXT("Voxel_%s"),
		*FMaterialAttributeDefinitionMap::GetAttributeName(Compiler.GetMaterialAttribute()));
}

int32 FVoxelMegaMaterialSwitch::CompileCustomOutput(
	FExpressionInput (FVoxelMegaMaterialSwitchInputs::*InputPtr),
	const EMaterialValueType ValueType)
{
	VOXEL_FUNCTION_COUNTER();

	ON_SCOPE_EXIT
	{
		if (Translator.VTStacks.Num() > 128)
		{
			Compiler.Errorf(TEXT("Too many Virtual Textures used: %d"), Translator.VTStacks.Num());
		}
	};

	if (Compiler.GetCurrentShaderFrequency() != SF_Pixel)
	{
		return -1;
	}

	ComputeDisplacementAndLayerMask(true);

	{
		const FString Type = INLINE_LAMBDA
		{
			switch (ValueType)
			{
			default: ensure(false);
			case MCT_Float: return "MaterialFloat";
			case MCT_Float2: return "MaterialFloat2";
			case MCT_Float3: return "MaterialFloat3";
			case MCT_Float4: return "MaterialFloat4";
			}
		};

		AddLine("%s VoxelValue = 0;", *Type);
	}

	{
		AddLine("#if VOXEL_HOOK");
		ON_SCOPE_EXIT
		{
			AddLine("#endif");
		};

		AddLine("#if !VOXEL_CHECK_QUAD");
		AddLine("BRANCH");
		AddLine("switch (Parameters.Voxel_LayerIndex)");
		AddLine("{");
		AddLine("#endif");
		ON_SCOPE_EXIT
		{
			AddLine("#if !VOXEL_CHECK_QUAD");
			AddLine("}");
			AddLine("#endif");
		};

		for (auto& It : Switch.IndexToInputs)
		{
			const FVoxelMaterialRenderIndex RenderIndex = It.Key;
			const FVoxelMegaMaterialSwitchInputs& Inputs = It.Value;
			FVoxelMaterialTranslatorNoCodeReuseScope NoCodeReuseScope(Translator);

			AddLine("#if VOXEL_CHECK_QUAD");
			AddLine("BRANCH");
			AddLine("if (VoxelLayerMask_Get(QuadLayerMask, %d))", RenderIndex.Index);
			AddLine("#else");
			AddLine("case %d:", RenderIndex.Index);
			AddLine("#endif");
			AddLine("{");
			ON_SCOPE_EXIT
			{
				AddLine("}");
				AddLine("#if !VOXEL_CHECK_QUAD");
				AddLine("break;");
				AddLine("#endif");
			};

			FIndentScope IndentScope(*this);

			const FString LocalCode = INLINE_LAMBDA -> FString
			{
				const int32 Index = ConstCast(Inputs.*InputPtr).Compile(&Compiler);
				if (Index == -1)
				{
					return "0";
				}

				const int32 FinalIndex = Compiler.ForceCast(Index, ValueType, MFCF_ExactMatch | MFCF_ReplicateValue);

				return Translator.GetParameterCode(FinalIndex);
			};

			AddLine("#if VOXEL_CHECK_QUAD");
			AddLine("FLATTEN");
			AddLine("if (%d == Parameters.Voxel_LayerIndex)", RenderIndex.Index);
			AddLine("#endif");

			AddLine("{");
			{
				FIndentScope LocalIndentScope(*this);
				AddLine("VoxelValue = %s;", *LocalCode);
			}
			AddLine("}");
		}
	}

	return Translator.AddCodeChunk(ValueType, TEXT("VoxelValue"));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMegaMaterialSwitch::AddVoxelHookDefine()
{
	if (bHasVoxelHookDefine)
	{
		return;
	}
	bHasVoxelHookDefine = true;

	const FString TargetDefine = INLINE_LAMBDA
	{
		switch (Switch.Target)
		{
		default: ensure(false);
		case EVoxelMegaMaterialTarget::NonNanite: return "VOXEL_MESH_VERTEX_FACTORY";
		case EVoxelMegaMaterialTarget::NaniteWPO: return "IS_NANITE_PASS && !NANITE_TESSELLATION";
		case EVoxelMegaMaterialTarget::NaniteDisplacement: return "IS_NANITE_PASS && NANITE_TESSELLATION";
		case EVoxelMegaMaterialTarget::NaniteMaterialSelection: return "VOXEL_MATERIAL_SELECTION";
		case EVoxelMegaMaterialTarget::Lumen: return "VOXEL_MESH_VERTEX_FACTORY";
		}
	};

	AddLine("#undef VOXEL_HOOK");
	AddLine("#define VOXEL_HOOK (MATERIAL_VERTEX_PARAMETERS_VOXEL_VERSION == 6 && MATERIAL_PIXEL_PARAMETERS_VOXEL_VERSION == 8 && %s)", *TargetDefine);
}

void FVoxelMegaMaterialSwitch::AddInclude(const FString& Include)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Includes.TryAdd(Include))
	{
		return;
	}

	UMaterialExpressionCustom* Custom = NewObject<UMaterialExpressionCustom>();
	Custom->Inputs.Reset();
	Custom->OutputType = CMOT_Float1;
	Custom->Code = "return 0;";
	Custom->IncludeFilePaths.Add(Include);

	TArray<int32> CustomInputs;
	Compiler.CustomExpression(Custom, 0, CustomInputs);
}

void FVoxelMegaMaterialSwitch::AddLineImpl(FString String)
{
	VOXEL_FUNCTION_COUNTER();

	TArray<FString> Lines;
	String.ParseIntoArrayLines(Lines, false);

	for (FString& Line : Lines)
	{
		Line.TrimStartAndEndInline();

		if (Line.StartsWith("#"))
		{
			Translator.AddCodeChunk(MCT_VoidStatement, TEXT("%s"), *Line);
			continue;
		}

		for (int32 Index = 0; Index < Indent; Index++)
		{
			Line = "\t" + Line;
		}

		Translator.AddCodeChunk(MCT_VoidStatement, TEXT("\t%s"), *Line);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelMegaMaterialSwitch::IsVertex() const
{
	ensure(
		Compiler.GetCurrentShaderFrequency() == SF_Vertex ||
		Compiler.GetCurrentShaderFrequency() == SF_Pixel);

	return Compiler.GetCurrentShaderFrequency() == SF_Vertex;
}

bool FVoxelMegaMaterialSwitch::HasVoxelTag() const
{
	for (const FShaderCodeChunk& Chunk : *Translator.CurrentScopeChunks)
	{
		if (Chunk.DefinitionFinite.Contains(UniqueTag))
		{
			return true;
		}
	}

	return false;
}

void FVoxelMegaMaterialSwitch::AddVoxelTag()
{
	ensure(!HasVoxelTag());

	AddLine("// %s", *UniqueTag);
}

TVoxelSet<EMaterialProperty> FVoxelMegaMaterialSwitch::GetConnectedProperties() const
{
	VOXEL_FUNCTION_COUNTER();

	const FShaderCodeChunk* FoundChunk = nullptr;
	for (const FShaderCodeChunk& Chunk : *Translator.CurrentScopeChunks)
	{
		if (!Chunk.DefinitionFinite.Contains(TEXTVIEW("// Connected properties:")))
		{
			continue;
		}

		ensure(!FoundChunk);
		FoundChunk = &Chunk;
	}

	if (!ensure(FoundChunk))
	{
		return {};
	}

	FString String = FoundChunk->DefinitionFinite.TrimStartAndEnd();
	String.RemoveFromStart("// Connected properties: ");

	const TVoxelArray<uint8> Bytes = FVoxelUtilities::HexToBlob(String);
	FVoxelReader Reader(Bytes);

	TVoxelSet<EMaterialProperty> ConnectedProperties;
	Reader << ConnectedProperties;

	ensure(Reader.IsAtEndWithoutError());
	return ConnectedProperties;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMegaMaterialSwitch::InitializeDefaultProperties()
{
	VOXEL_FUNCTION_COUNTER();

	// Always do this first
	for (int32 PropertyIndex = 0; PropertyIndex < int32(MP_MAX); PropertyIndex++)
	{
		const EMaterialProperty Property = EMaterialProperty(PropertyIndex);

		if (Property == MP_CustomOutput ||
			Property == MP_MaterialAttributes)
		{
			continue;
		}

		// Do this outside the scope, as FMaterialAttributesInput::CompileWithDefault will try to reuse it
		const int32 DefaultIndex = FMaterialAttributeDefinitionMap::CompileDefaultExpression(&Compiler, Property);
		const FString DefaultCode = Translator.GetParameterCode(DefaultIndex);

		PropertyToDefaultCode.Add_EnsureNew(Property, DefaultCode);
	}
}

void FVoxelMegaMaterialSwitch::ComputeDisplacementAndLayerMask(const bool bComputeSmoothness)
{
	VOXEL_FUNCTION_COUNTER();

	AddInclude("/Plugin/Voxel/VoxelDisplacement.ush");
	AddVoxelHookDefine();

	Compiler.PushMaterialAttribute(FMaterialAttributeDefinitionMap::GetID(MP_Displacement));
	ON_SCOPE_EXIT
	{
		ensure(Compiler.PopMaterialAttribute() == FMaterialAttributeDefinitionMap::GetID(MP_Displacement));
	};

	AddLine("#if VOXEL_HOOK");
	ON_SCOPE_EXIT
	{
		AddLine("#endif");
	};

	AddLine("#define IS_VOXEL_MEGA_MATERIAL 1");

	{
		AddLine("#if IS_NANITE_PASS");
		ON_SCOPE_EXIT
		{
			AddLine("#endif");
		};

		FIndentScope Scope(*this);

		{
			AddLine("const FCluster Cluster = GetCluster(Parameters.Voxel_PageIndex, Parameters.Voxel_ClusterIndex);");
			AddLine("Parameters.Voxel_ChunkIndicesIndex = GetChunkIndicesIndexInCluster(Cluster, Parameters.Voxel_ClusterIndex);");
		}

		{
			if (IsVertex())
			{
				AddLine("Parameters.Voxel_VertexIndexInChunk = GetVertexIndexInVoxelChunk(Cluster, Parameters.Voxel_ClusterIndex, Parameters.Voxel_VertexIndex);");
			}
			else
			{
				AddLine("Parameters.Voxel_VertexIndicesInChunk.r = GetVertexIndexInVoxelChunk(Cluster, Parameters.Voxel_ClusterIndex, Parameters.Voxel_TriIndices.r);");
				AddLine("Parameters.Voxel_VertexIndicesInChunk.g = GetVertexIndexInVoxelChunk(Cluster, Parameters.Voxel_ClusterIndex, Parameters.Voxel_TriIndices.g);");
				AddLine("Parameters.Voxel_VertexIndicesInChunk.b = GetVertexIndexInVoxelChunk(Cluster, Parameters.Voxel_ClusterIndex, Parameters.Voxel_TriIndices.b);");
			}
		}

		const int32 ChunkIndices_Texture = Compiler.TextureParameter(
			"VOXEL_ChunkIndices_Texture",
			FVoxelTextureUtilities::GetDefaultTexture2D(),
			SAMPLERTYPE_Color);

		const int32 ChunkIndices_TextureSizeLog2 = Compiler.ScalarParameter(
			"VOXEL_ChunkIndices_TextureSizeLog2",
			0.f);

		const int32 Materials_Texture = Compiler.TextureParameter(
			"VOXEL_Materials_Texture",
			FVoxelTextureUtilities::GetDefaultTexture2D(),
			SAMPLERTYPE_Color);

		const int32 Materials_TextureSizeLog2 = Compiler.ScalarParameter(
			"VOXEL_Materials_TextureSizeLog2",
			0.f);

		AddLine(R"(
			GetVoxelMaterialLayers(
				Parameters,
				%s,
				%s,
				%s,
				%s,
				Parameters.Voxel_LayerMask,
				Parameters.Voxel_LayerWeights);
			)",
			*Translator.GetParameterCode(ChunkIndices_Texture),
			*Translator.GetParameterCode(ChunkIndices_TextureSizeLog2),
			*Translator.GetParameterCode(Materials_Texture),
			*Translator.GetParameterCode(Materials_TextureSizeLog2));
	}

	AddLine("int NumLayersQueried = 0;");
	AddLine("int NumNeighborsQueried = 0;");
	AddLine("float DitherBiasDebug = 0;");

	AddLine(R"(
		float Displacements[8];
		Displacements[0] = -10.f;
		Displacements[1] = -10.f;
		Displacements[2] = -10.f;
		Displacements[3] = -10.f;
		Displacements[4] = -10.f;
		Displacements[5] = -10.f;
		Displacements[6] = -10.f;
		Displacements[7] = -10.f;
	)");

	AddLine(R"(
		float BlendSmoothnesses[8];
		BlendSmoothnesses[0] = 0.f;
		BlendSmoothnesses[1] = 0.f;
		BlendSmoothnesses[2] = 0.f;
		BlendSmoothnesses[3] = 0.f;
		BlendSmoothnesses[4] = 0.f;
		BlendSmoothnesses[5] = 0.f;
		BlendSmoothnesses[6] = 0.f;
		BlendSmoothnesses[7] = 0.f;
	)");

	for (auto& It : Switch.IndexToInputs)
	{
		const FVoxelMaterialRenderIndex RenderIndex = It.Key;
		const FVoxelMegaMaterialSwitchInputs& Inputs = It.Value;
		FVoxelMaterialTranslatorNoCodeReuseScope NoCodeReuseScope(Translator);

		AddLine("BRANCH");
		AddLine("if (VoxelLayerMask_Get(Parameters.Voxel_LayerMask, %d))", RenderIndex.Index);
		AddLine("{");
		ON_SCOPE_EXIT
		{
			AddLine("}");
		};

		FIndentScope IndentScope(*this);

		AddLine("NumLayersQueried++;");

		int32 Index = ConstCast(Inputs.Attributes).Compile(&Compiler);
		if (Index == -1)
		{
			Index = Compiler.Constant(0.f);
		}

		AddLine("float LocalDisplacement = (float)%s;", *Translator.GetParameterCode(Index));

		AddLine("LocalDisplacement = (saturate(LocalDisplacement) - %f) * %f;",
			Inputs.DisplacementScaling.Center,
			Inputs.DisplacementScaling.Magnitude);

		AddLine("LocalDisplacement = LocalDisplacement / %f + %f;",
			Switch.Material->DisplacementScaling.Magnitude,
			Switch.Material->DisplacementScaling.Center);

		AddLine("Displacements[VoxelLayerMask_GetLayerIndex(Parameters.Voxel_LayerMask, %d)] = LocalDisplacement;", RenderIndex.Index);

		const int32 BlendSmoothness = Compiler.ScalarParameter(FName(FString::Printf(TEXT("VOXEL_BlendSmoothness_%d"), RenderIndex.Index)), 0.f);

		AddLine("BlendSmoothnesses[VoxelLayerMask_GetLayerIndex(Parameters.Voxel_LayerMask, %d)] = %s;",
			RenderIndex.Index,
			*Translator.GetParameterCode(BlendSmoothness));

	}

	const int32 PixelNoiseTexture = Compiler.TextureParameter(
		"VOXEL_PixelNoiseTexture",
		FVoxelTextureUtilities::GetDefaultTexture2D(),
		SAMPLERTYPE_Color);

	if (Switch.bEnableSmoothBlends &&
		bComputeSmoothness)
	{
		AddLine(R"(
				GetVoxelDisplacement(
					GetPixelPosition(Parameters),
					%d,
					%s,
					Parameters.Voxel_LayerMask,
					Parameters.Voxel_LayerWeights,
					Displacements,
					BlendSmoothnesses,
					Parameters.Voxel_Displacement,
					Parameters.Voxel_LayerIndex,
					DitherBiasDebug);
			)",
			Switch.bEnableDitherNoiseTexture ? 1 : 0,
			*Translator.GetParameterCode(PixelNoiseTexture));
	}
	else
	{
		AddLine(R"(
				GetVoxelDisplacement_NoSmoothness(
					Parameters.Voxel_LayerMask,
					Parameters.Voxel_LayerWeights,
					Displacements,
					Parameters.Voxel_Displacement,
					Parameters.Voxel_LayerIndex);
			)");
	}

	AddLine(R"(
		#undef VOXEL_CHECK_QUAD
		#define VOXEL_CHECK_QUAD (PIXELSHADER && COMPILER_SUPPORTS_WAVE_BIT_ORAND)

		#if VOXEL_CHECK_QUAD
			uint4 QuadLayerMask = 0;

			VoxelLayerMask_Set(QuadLayerMask, Parameters.Voxel_LayerIndex);
			VoxelLayerMask_Set(QuadLayerMask, QuadReadAcrossX(Parameters.Voxel_LayerIndex));
			VoxelLayerMask_Set(QuadLayerMask, QuadReadAcrossY(Parameters.Voxel_LayerIndex));
			VoxelLayerMask_Set(QuadLayerMask, QuadReadAcrossDiagonal(Parameters.Voxel_LayerIndex));
		#endif
	)");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMegaMaterialSwitch::ComputeProperties(const TConstVoxelArrayView<EMaterialProperty> Properties)
{
	VOXEL_FUNCTION_COUNTER();

	for (const EMaterialProperty Property : Properties)
	{
		const EMaterialValueType ValueType = FMaterialAttributeDefinitionMap::GetValueType(Property);

		const FString Type = INLINE_LAMBDA
		{
			switch (ValueType)
			{
			default: ensure(false);
			case MCT_Float: return "MaterialFloat";
			case MCT_Float2: return "MaterialFloat2";
			case MCT_Float3: return "MaterialFloat3";
			case MCT_Float4: return "MaterialFloat4";
			case MCT_ShadingModel: return "uint";
			}
		};

		AddLine("%s Voxel_%s = 0;",
			*Type,
			*FMaterialAttributeDefinitionMap::GetAttributeName(Property));
	}

	TVoxelSet<EMaterialProperty> ConnectedProperties;

	AddLine("#if VOXEL_HOOK");
	ON_SCOPE_EXIT
	{
		AddLine("#endif");
	};

	AddLine("#if !VOXEL_CHECK_QUAD");
	AddLine("BRANCH");
	AddLine("switch (Parameters.Voxel_LayerIndex)");
	AddLine("{");
	AddLine("#endif");
	ON_SCOPE_EXIT
	{
		AddLine("#if !VOXEL_CHECK_QUAD");
		AddLine("}");
		AddLine("#endif");
	};

	for (auto& It : Switch.IndexToInputs)
	{
		const FVoxelMaterialRenderIndex RenderIndex = It.Key;
		const FVoxelMegaMaterialSwitchInputs& Inputs = It.Value;
		FVoxelMaterialTranslatorNoCodeReuseScope NoCodeReuseScope(Translator);

		AddLine("#if VOXEL_CHECK_QUAD");
		AddLine("BRANCH");
		AddLine("if (VoxelLayerMask_Get(QuadLayerMask, %d))", RenderIndex.Index);
		AddLine("#else");
		AddLine("case %d:", RenderIndex.Index);
		AddLine("#endif");
		AddLine("{");
		ON_SCOPE_EXIT
		{
			AddLine("}");
			AddLine("#if !VOXEL_CHECK_QUAD");
			AddLine("break;");
			AddLine("#endif");
		};

		FIndentScope IndentScope(*this);

		AddLine("NumNeighborsQueried++;");

		for (const EMaterialProperty Property : Properties)
		{
			const EMaterialValueType ValueType = FMaterialAttributeDefinitionMap::GetValueType(Property);
			const FGuid AttributeID = FMaterialAttributeDefinitionMap::GetID(Property);

			Compiler.PushMaterialAttribute(AttributeID);
			const EMaterialProperty MaterialPropertyBackup = Translator.MaterialProperty;
			Translator.MaterialProperty = Property;
			ON_SCOPE_EXIT
			{
				ensure(Compiler.PopMaterialAttribute() == AttributeID);

				ensure(Translator.MaterialProperty == Property);
				Translator.MaterialProperty = MaterialPropertyBackup;
			};

			bool bIsDefaultCode = false;
			FString LocalCode = INLINE_LAMBDA
			{
				const bool bShouldCompile = INLINE_LAMBDA
				{
					switch (Switch.Target)
					{
					default: ensure(false);
					case EVoxelMegaMaterialTarget::NonNanite:
					{
						return true;
					}
					case EVoxelMegaMaterialTarget::NaniteWPO:
					case EVoxelMegaMaterialTarget::NaniteDisplacement:
					case EVoxelMegaMaterialTarget::NaniteMaterialSelection:
					{
						return IsVertex();
					}
					case EVoxelMegaMaterialTarget::Lumen:
					{
						return
							Property == MP_BaseColor ||
							Property == MP_EmissiveColor;
					}
					}
				};

				if (!bShouldCompile)
				{
					bIsDefaultCode = true;
					return PropertyToDefaultCode[Property];
				}

				const int32 Index = ConstCast(Inputs.Attributes).Compile(&Compiler);
				if (Index == -1)
				{
					bIsDefaultCode = true;
					return PropertyToDefaultCode[Property];
				}

				const int32 FinalIndex = Compiler.ForceCast(Index, ValueType, MFCF_ExactMatch | MFCF_ReplicateValue);

				// SetMaterialAttribute & others will return default when unplugged, make sure to ignore that
				if (FinalIndex == FMaterialAttributeDefinitionMap::CompileDefaultExpression(&Compiler, AttributeID))
				{
					bIsDefaultCode = true;
				}

				return Translator.GetParameterCode(FinalIndex);
			};

			if (!bIsDefaultCode)
			{
				ConnectedProperties.Add(Property);
			}

			if (Property == MP_Normal)
			{
				FString Code = R"(
					#if MATERIAL_TANGENTSPACENORMAL
						// TransformTangentNormalToWorld handles normalization
					    float3 NewNormal = TransformTangentNormalToWorld(Parameters.TangentToWorld, LocalNormal);
					#else
					    float3 NewNormal = normalize(LocalNormal);
					#endif

					#if MATERIAL_TANGENTSPACENORMAL || TWO_SIDED_WORLD_SPACE_SINGLELAYERWATER_NORMAL
					    NewNormal *= Parameters.TwoSidedSign;
					#endif

					    Parameters.WorldNormal = NewNormal;
					    Output = NewNormal;
				)";

				const bool bTangentSpaceNormal =
					Inputs.bTangentSpaceNormal &&
					// Defaults are in the generated material space & thus not in tangent space,
					// see FMaterialAttributeDefinitionMap::CompileDefaultExpression above
					!bIsDefaultCode;

				Code.ReplaceInline(
					TEXT("MATERIAL_TANGENTSPACENORMAL"),
					bTangentSpaceNormal ? TEXT("1") : TEXT("0"));

				Code.ReplaceInline(TEXT("LocalNormal"), *LocalCode);

				const int32 NewIndex = Translator.AddCodeChunk(MCT_Float3, TEXT("0; // Ensure code is not reused: D55DD5967225"));
				LocalCode = Translator.GetParameterCode(NewIndex);

				Code.ReplaceInline(TEXT("Output"), *LocalCode);

				AddLine("{");
				{
					FIndentScope LocalIndentScope(*this);
					AddLine("%s", *Code);
				}
				AddLine("}");
			}

			AddLine("#if VOXEL_CHECK_QUAD");
			AddLine("FLATTEN");
			AddLine("if (%d == Parameters.Voxel_LayerIndex)", RenderIndex.Index);
			AddLine("#endif");

			AddLine("{");
			{
				FIndentScope LocalIndentScope(*this);

				AddLine("Voxel_%s = %s;",
					*FMaterialAttributeDefinitionMap::GetAttributeName(AttributeID),
					*LocalCode);
			}
			AddLine("}");
		}
	}

	FVoxelWriter Writer;
	Writer << ConnectedProperties;

	AddLine("// Connected properties: %s", *FVoxelUtilities::BlobToHex(Writer.Bytes));
}
#endif

#undef AddLine