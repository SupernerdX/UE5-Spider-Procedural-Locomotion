// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/MaterialExpressionGetVoxelMetadata.h"
#include "Materials/MaterialExpressionCustom.h"
#include "VoxelRuntimePinValue.h"
#include "MaterialCompiler.h"
#include "Engine/Texture2D.h"

UMaterialExpressionGetVoxelMetadata::UMaterialExpressionGetVoxelMetadata()
{
	MenuCategories.Add(INVTEXT("Voxel Plugin"));

	Outputs.Reset();
	Outputs.Add({ "Value" });
}

void UMaterialExpressionGetVoxelMetadata::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

UObject* UMaterialExpressionGetVoxelMetadata::GetReferencedTexture() const
{
	return FVoxelTextureUtilities::GetDefaultTexture2D();
}

#if WITH_EDITOR
#if VOXEL_ENGINE_VERSION >= 506
EMaterialValueType UMaterialExpressionGetVoxelMetadata::GetOutputValueType(int32 OutputIndex)
#else
uint32 UMaterialExpressionGetVoxelMetadata::GetOutputType(int32 OutputIndex)
#endif
{
	if (!Metadata)
	{
		return MCT_Float;
	}

	const TVoxelOptional<EVoxelMetadataMaterialType> Type = Metadata->GetMaterialType();
	if (!Type)
	{
		return MCT_Float;
	}

	return FVoxelMetadataMaterialType::GetMaterialValueType(Type.GetValue());
}

int32 UMaterialExpressionGetVoxelMetadata::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	VOXEL_FUNCTION_COUNTER();

	UTexture2D* DefaultTexture = FVoxelTextureUtilities::GetDefaultTexture2D();
	if (!ensure(DefaultTexture))
	{
		return -1;
	}

	const FVoxelMetadataRef MetadataRef(Metadata);
	if (!MetadataRef)
	{
		return Compiler->Error(TEXT("Missing Metadata"));
	}

	const TVoxelOptional<EVoxelMetadataMaterialType> OptionalMaterialType = MetadataRef.GetMaterialType();
	if (!OptionalMaterialType)
	{
		return Compiler->Error(TEXT("Metadata cannot be used in a material"));
	}
	const EVoxelMetadataMaterialType MaterialType = OptionalMaterialType.GetValue();

	if (MetadataIndex == -1)
	{
		return FVoxelMetadataMaterialType::Constant(
			*Compiler,
			MetadataRef,
			MaterialType,
			MetadataRef.GetDefaultValue());
	}
	ensure(MetadataIndex >= 0);

	UMaterialExpressionCustom* Custom = NewObject<UMaterialExpressionCustom>();
	Custom->Code.Reset();
	Custom->Inputs.Reset();
	Custom->OutputType = FVoxelMetadataMaterialType::GetCustomMaterialOutputType(MaterialType);
	Custom->IncludeFilePaths.Add("/Plugin/Voxel/VoxelDisplacement.ush");

	TArray<int32> Inputs;

	{
		Inputs.Add(Compiler->TextureParameter(
			"VOXEL_ChunkIndices_Texture",
			DefaultTexture,
			SAMPLERTYPE_Color));

		Custom->Inputs.Add({ "ChunkIndices_Texture" });
	}

	{
		Inputs.Add(Compiler->ScalarParameter("VOXEL_ChunkIndices_TextureSizeLog2", 0.f));
		Custom->Inputs.Add({ "ChunkIndices_TextureSizeLog2" });
	}

	{
		Inputs.Add(Compiler->TextureParameter(
			FName("VOXEL_Metadata_Texture", MetadataIndex + 1),
			DefaultTexture,
			SAMPLERTYPE_Color));
		Custom->Inputs.Add({ "Metadata_Texture" });
	}

	{
		Inputs.Add(Compiler->ScalarParameter(FName("VOXEL_Metadata_TextureSizeLog2", MetadataIndex + 1), 0.f));
		Custom->Inputs.Add({ "Metadata_TextureSizeLog2" });
	}

	FString GetMetadataCode;

	if (MaterialType == EVoxelMetadataMaterialType::Normal)
	{
		Custom->Code = R"(
#if MATERIAL_VERTEX_PARAMETERS_VOXEL_VERSION == 6 && MATERIAL_PIXEL_PARAMETERS_VOXEL_VERSION == 8
	const int Offset = GetVoxelAttributeOffset(Parameters.Voxel_ChunkIndicesIndex, NUM_METADATAS, ChunkIndices_Texture, ChunkIndices_TextureSizeLog2, 1 + METADATA_INDEX);

	BRANCH
	if (Offset == -1)
	{
		return DEFAULT_METADATA;
	}

#if VERTEX_SHADER
	const int Index = Offset + Parameters.Voxel_VertexIndexInChunk;
	return GET_METADATA;
#else
	const int Index0 = Offset + Parameters.Voxel_VertexIndicesInChunk[0];
	const int Index1 = Offset + Parameters.Voxel_VertexIndicesInChunk[1];
	const int Index2 = Offset + Parameters.Voxel_VertexIndicesInChunk[2];

	return normalize(
		GET_METADATA_0 * Parameters.Voxel_Barycentrics[0] +
		GET_METADATA_1 * Parameters.Voxel_Barycentrics[1] +
		GET_METADATA_2 * Parameters.Voxel_Barycentrics[2]);
#endif
#else
	return 0;
#endif)";

		GetMetadataCode = "OctahedronToUnitVector(2.f * asfloat(Metadata_Texture[GetTextureIndex_Log2(IndexX, Metadata_TextureSizeLog2)].rg) - 1.f)";
	}
	else
	{
		Custom->Code = R"(
#if MATERIAL_VERTEX_PARAMETERS_VOXEL_VERSION == 6 && MATERIAL_PIXEL_PARAMETERS_VOXEL_VERSION == 8
	const int Offset = GetVoxelAttributeOffset(Parameters.Voxel_ChunkIndicesIndex, ChunkIndices_Texture, ChunkIndices_TextureSizeLog2, 1 + METADATA_INDEX);

	BRANCH
	if (Offset == -1)
	{
		return DEFAULT_METADATA;
	}

#if VERTEX_SHADER
	const int Index = Offset + Parameters.Voxel_VertexIndexInChunk;
	return GET_METADATA;
#else
	const int Index0 = Offset + Parameters.Voxel_VertexIndicesInChunk[0];
	const int Index1 = Offset + Parameters.Voxel_VertexIndicesInChunk[1];
	const int Index2 = Offset + Parameters.Voxel_VertexIndicesInChunk[2];

	const TYPE Value0 = GET_METADATA_0;
	const TYPE Value1 = GET_METADATA_1;
	const TYPE Value2 = GET_METADATA_2;

	if (INTERPOLATE)
	{
		return
			Value0 * Parameters.Voxel_Barycentrics[0] +
			Value1 * Parameters.Voxel_Barycentrics[1] +
			Value2 * Parameters.Voxel_Barycentrics[2];
	}
	else if (AVERAGE_VALUE)
	{
		return
			(Value0 +
			Value1 +
			Value2) / 3.f;
	}
	else
	{
		if (Parameters.Voxel_Barycentrics[0] >= Parameters.Voxel_Barycentrics[1] &&
			Parameters.Voxel_Barycentrics[0] >= Parameters.Voxel_Barycentrics[2])
		{
			return Value0;
		}

		if (Parameters.Voxel_Barycentrics[1] >= Parameters.Voxel_Barycentrics[2])
		{
			return Value1;
		}

		return Value2;
	}
#endif
#else
	return 0;
#endif)";

		{
			FString InterpolateValue;
			FString AverageValue;
			INLINE_LAMBDA
			{
				switch (MaterialType)
				{
				default: ensure(false);
				case EVoxelMetadataMaterialType::Float8_1:
				case EVoxelMetadataMaterialType::Float8_2:
				case EVoxelMetadataMaterialType::Float8_3:
				case EVoxelMetadataMaterialType::Float8_4:
				case EVoxelMetadataMaterialType::Float16_1:
				case EVoxelMetadataMaterialType::Float16_2:
				case EVoxelMetadataMaterialType::Float16_3:
				case EVoxelMetadataMaterialType::Float16_4:
				case EVoxelMetadataMaterialType::Float32_1:
				case EVoxelMetadataMaterialType::Float32_2:
				case EVoxelMetadataMaterialType::Float32_3:
				case EVoxelMetadataMaterialType::Float32_4:
				{
					if (bInterpolateMetadata)
					{
						InterpolateValue = "true";
						AverageValue = "false";
					}
					else
					{
						InterpolateValue = "false";
						AverageValue = "true";
					}
					break;
				}
				case EVoxelMetadataMaterialType::Int1:
				case EVoxelMetadataMaterialType::Int2:
				case EVoxelMetadataMaterialType::Int3:
				case EVoxelMetadataMaterialType::Int4:
				{
					InterpolateValue = "false";
					AverageValue = "false";
					break;
				}
				}
			};

			Custom->Code.ReplaceInline(
				TEXT("INTERPOLATE"),
				*InterpolateValue,
				ESearchCase::CaseSensitive);

			Custom->Code.ReplaceInline(
				TEXT("AVERAGE_VALUE"),
				*AverageValue,
				ESearchCase::CaseSensitive);
		}

		{
			const FString Code = INLINE_LAMBDA
			{
				switch (MaterialType)
				{
				default: ensure(false);
				case EVoxelMetadataMaterialType::Float8_1:
				case EVoxelMetadataMaterialType::Float16_1:
				case EVoxelMetadataMaterialType::Float32_1: return "float";
				case EVoxelMetadataMaterialType::Float8_2:
				case EVoxelMetadataMaterialType::Float16_2:
				case EVoxelMetadataMaterialType::Float32_2: return "float2";
				case EVoxelMetadataMaterialType::Float8_3:
				case EVoxelMetadataMaterialType::Float16_3:
				case EVoxelMetadataMaterialType::Float32_3: return "float3";
				case EVoxelMetadataMaterialType::Float8_4:
				case EVoxelMetadataMaterialType::Float16_4:
				case EVoxelMetadataMaterialType::Float32_4: return "float4";
				case EVoxelMetadataMaterialType::Int1: return "int";
				case EVoxelMetadataMaterialType::Int2: return "int2";
				case EVoxelMetadataMaterialType::Int3: return "int3";
				case EVoxelMetadataMaterialType::Int4: return "int4";
				}
			};

			Custom->Code.ReplaceInline(
				TEXT("TYPE"),
				*Code,
				ESearchCase::CaseSensitive);
		}

		GetMetadataCode = INLINE_LAMBDA -> FString
		{
			switch (MaterialType)
			{
			default: ensure(false);
			case EVoxelMetadataMaterialType::Float8_1:
			case EVoxelMetadataMaterialType::Float16_1:
			case EVoxelMetadataMaterialType::Float32_1: return "asfloat(Metadata_Texture[GetTextureIndex_Log2(IndexX, Metadata_TextureSizeLog2)].r)";
			case EVoxelMetadataMaterialType::Float8_2:
			case EVoxelMetadataMaterialType::Float16_2:
			case EVoxelMetadataMaterialType::Float32_2: return "asfloat(Metadata_Texture[GetTextureIndex_Log2(IndexX, Metadata_TextureSizeLog2)].rg)";
			case EVoxelMetadataMaterialType::Float8_3:
			case EVoxelMetadataMaterialType::Float16_3:
			case EVoxelMetadataMaterialType::Float32_3: return "asfloat(Metadata_Texture[GetTextureIndex_Log2(IndexX, Metadata_TextureSizeLog2)].rgb)";
			case EVoxelMetadataMaterialType::Float8_4:
			case EVoxelMetadataMaterialType::Float16_4:
			case EVoxelMetadataMaterialType::Float32_4: return "asfloat(Metadata_Texture[GetTextureIndex_Log2(IndexX, Metadata_TextureSizeLog2)].rgba)";
			case EVoxelMetadataMaterialType::Int1: return "float(asint(Metadata_Texture[GetTextureIndex_Log2(IndexX, Metadata_TextureSizeLog2)].r))";
			case EVoxelMetadataMaterialType::Int2: return "float2(asint(Metadata_Texture[GetTextureIndex_Log2(IndexX, Metadata_TextureSizeLog2)].rg))";
			case EVoxelMetadataMaterialType::Int3: return "float3(asint(Metadata_Texture[GetTextureIndex_Log2(IndexX, Metadata_TextureSizeLog2)].rgb))";
			case EVoxelMetadataMaterialType::Int4: return "float4(asint(Metadata_Texture[GetTextureIndex_Log2(IndexX, Metadata_TextureSizeLog2)].rgba))";
			}
		};
	}

	Custom->Code.ReplaceInline(TEXT("VERTEX_SHADER"), Compiler->GetCurrentShaderFrequency() == SF_Vertex
		? TEXT("1")
		: TEXT("0"));

	Custom->Code.ReplaceInline(
		TEXT("METADATA_INDEX"),
		*FString::FromInt(MetadataIndex),
		ESearchCase::CaseSensitive);

	{
		const FString Code = FVoxelMetadataMaterialType::Constant(
			MetadataRef,
			MaterialType,
			MetadataRef.GetDefaultValue());

		Custom->Code.ReplaceInline(
			TEXT("DEFAULT_METADATA"),
			*Code,
			ESearchCase::CaseSensitive);
	}

	Custom->Code.ReplaceInline(
		TEXT("GET_METADATA_0"),
		*GetMetadataCode.Replace(TEXT("IndexX"), TEXT("Index0"), ESearchCase::CaseSensitive),
		ESearchCase::CaseSensitive);

	Custom->Code.ReplaceInline(
		TEXT("GET_METADATA_1"),
		*GetMetadataCode.Replace(TEXT("IndexX"), TEXT("Index1"), ESearchCase::CaseSensitive),
		ESearchCase::CaseSensitive);

	Custom->Code.ReplaceInline(
		TEXT("GET_METADATA_2"),
		*GetMetadataCode.Replace(TEXT("IndexX"), TEXT("Index2"), ESearchCase::CaseSensitive),
		ESearchCase::CaseSensitive);

	Custom->Code.ReplaceInline(
		TEXT("GET_METADATA"),
		*GetMetadataCode.Replace(TEXT("IndexX"), TEXT("Index"), ESearchCase::CaseSensitive),
		ESearchCase::CaseSensitive);

	if (!ensureVoxelSlow(!Inputs.Contains(-1)))
	{
		return Compiler->Errorf(TEXT("Invalid texture index"));
	}

	return Compiler->CustomExpression(Custom, 0, Inputs);
}

void UMaterialExpressionGetVoxelMetadata::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add("Get Voxel Metadata: " + (Metadata ? Metadata->GetName() : "null"));
}
#endif