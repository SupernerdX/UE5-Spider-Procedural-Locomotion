// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelCubicTextureArrayData.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialGeneratedData.h"
#include "MegaMaterial/MaterialExpressionGetVoxelMetadata.h"
#include "Surface/VoxelCubicSurfaceTypeAsset.h"
#include "Surface/VoxelCubicTextureTemplate.h"
#include "VoxelMaterialGenerator.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DArray.h"

#if WITH_EDITOR
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionComponentMask.h"
#endif

void FVoxelCubicTextureArrayData::Reset()
{
	Template = nullptr;
	SlotToTextureArray.Reset();
	SlotToScalarLookup.Reset();
	SlotToVectorLookup.Reset();
	CubicSurfaceToSliceIndex.Reset();
	SlotNames.Reset();
}

#if WITH_EDITOR
bool FVoxelCubicTextureArrayData::ValidateTemplate(
	const TArray<TObjectPtr<UVoxelCubicSurfaceTypeAsset>>& CubicSurfaceTypes,
	UVoxelCubicTextureTemplate*& OutTemplate,
	FString& OutError)
{
	OutTemplate = nullptr;

	for (const UVoxelCubicSurfaceTypeAsset* SurfaceType : CubicSurfaceTypes)
	{
		if (!SurfaceType ||
			!SurfaceType->Template)
		{
			continue;
		}

		if (!OutTemplate)
		{
			OutTemplate = SurfaceType->Template;
		}
		else if (OutTemplate != SurfaceType->Template)
		{
			OutError = FString::Printf(
				TEXT("Cubic surface types must use the same template. Found %s and %s"),
				*OutTemplate->GetName(),
				*SurfaceType->Template->GetName());
			return false;
		}
	}

	if (!OutTemplate)
	{
		OutError = "No valid cubic surface types with Template found";
		return false;
	}

	return true;
}

bool FVoxelCubicTextureArrayData::RebuildTextureArrays(
	UObject* Outer,
	const TArray<TObjectPtr<UVoxelCubicSurfaceTypeAsset>>& CubicSurfaceTypes,
	FString& OutError)
{
	VOXEL_FUNCTION_COUNTER();

	Reset();

	if (CubicSurfaceTypes.Num() == 0)
	{
		return true;
	}

	// Validate all surfaces use the same template
	UVoxelCubicTextureTemplate* ValidatedTemplate = nullptr;
	if (!ValidateTemplate(CubicSurfaceTypes, ValidatedTemplate, OutError))
	{
		return false;
	}
	Template = ValidatedTemplate;

	// Collect valid surfaces (those with template assigned)
	TVoxelArray<UVoxelCubicSurfaceTypeAsset*> ValidSurfaces;
	for (UVoxelCubicSurfaceTypeAsset* Surface : CubicSurfaceTypes)
	{
		if (Surface &&
			Surface->Template == Template)
		{
			ValidSurfaces.Add(Surface);
		}
	}

	if (ValidSurfaces.Num() == 0)
	{
		return true;
	}

	// Build surface index mapping first (used by all lookup types)
	{
		int32 SliceIndex = 0;
		for (UVoxelCubicSurfaceTypeAsset* Surface : ValidSurfaces)
		{
			CubicSurfaceToSliceIndex.Add(Surface, SliceIndex);
			SliceIndex++;
		}
	}

	// Build texture arrays for texture slots
	if (Template->TextureSlots.Num() > 0)
	{
		// First pass: collect and validate resolution for texture slots
		FIntPoint RequiredResolution = FIntPoint::ZeroValue;
		for (const UVoxelCubicSurfaceTypeAsset* Surface : ValidSurfaces)
		{
			for (const FVoxelCubicTextureSlot& Slot : Template->TextureSlots)
			{
				UTexture2D* Texture = Surface->GetTexture(Slot.Guid);
				if (!Texture)
				{
					continue;
				}

				const FIntPoint TextureSize(
					Texture->GetSizeX(),
					Texture->GetSizeY());

				if (RequiredResolution == FIntPoint::ZeroValue)
				{
					RequiredResolution = TextureSize;
				}
				else if (RequiredResolution != TextureSize)
				{
					OutError = FString::Printf(
						TEXT("Texture resolution mismatch in %s slot %s: expected %dx%d, got %dx%d"),
						*Surface->GetName(),
						*Slot.Name.ToString(),
						RequiredResolution.X, RequiredResolution.Y,
						TextureSize.X, TextureSize.Y);
					return false;
				}
			}
		}

		// Create texture arrays for each texture slot (if we have any textures)
		if (RequiredResolution != FIntPoint::ZeroValue)
		{
			for (const FVoxelCubicTextureSlot& Slot : Template->TextureSlots)
			{
				SlotNames.Add(Slot.Name);

				FVoxelCubicTextureArrayInfo& ArrayInfo = SlotToTextureArray.Add(Slot.Guid);
				ArrayInfo.SlotName = Slot.Name;
				ArrayInfo.Resolution = RequiredResolution;
				ArrayInfo.SliceCount = ValidSurfaces.Num();

				// Create the texture array
				ArrayInfo.TextureArray = NewObject<UTexture2DArray>(
					Outer,
					FName("CubicTextureArray_" + Slot.Name.ToString()),
					RF_Transient);

				ArrayInfo.TextureArray->MipGenSettings = TMGS_FromTextureGroup;
				ArrayInfo.TextureArray->Filter = Template->bUseNearestFilter ? TF_Nearest : TF_Default;
				ArrayInfo.TextureArray->SourceTextures.Reserve(ArrayInfo.SliceCount);
			}

			// Populate slices
			for (const UVoxelCubicSurfaceTypeAsset* Surface : ValidSurfaces)
			{
				for (const FVoxelCubicTextureSlot& Slot : Template->TextureSlots)
				{
					const FVoxelCubicTextureArrayInfo& ArrayInfo = SlotToTextureArray[Slot.Guid];

					UTexture2D* Texture = Surface->GetTexture(Slot.Guid);
					ArrayInfo.TextureArray->SourceTextures.Add(Texture);
				}
			}

			// Update texture arrays to compile them
			for (const auto& It : SlotToTextureArray)
			{
				It.Value.TextureArray->UpdateSourceFromSourceTextures();
				It.Value.TextureArray->UpdateResource();
			}
		}
	}

	// Build scalar lookup textures
	for (const FVoxelCubicScalarSlot& Slot : Template->ScalarSlots)
	{
		FVoxelCubicScalarLookupInfo& LookupInfo = SlotToScalarLookup.Add(Slot.Guid);
		LookupInfo.SlotName = Slot.Name;
		LookupInfo.EntryCount = ValidSurfaces.Num();

		// Create lookup texture (Nx1 R32F)
		LookupInfo.LookupTexture = NewObject<UTexture2D>(
			Outer,
			FName("CubicScalarLookup_" + Slot.Name.ToString()),
			RF_Transient);

		LookupInfo.LookupTexture->CompressionSettings = TC_HDR;
		LookupInfo.LookupTexture->SRGB = false;
		LookupInfo.LookupTexture->Filter = TF_Nearest;
		LookupInfo.LookupTexture->MipGenSettings = TMGS_NoMipmaps;
		LookupInfo.LookupTexture->AddressX = TA_Clamp;
		LookupInfo.LookupTexture->AddressY = TA_Clamp;

		// Create platform data and initialize
		const int32 Width = FMath::Max(1, ValidSurfaces.Num());
		LookupInfo.LookupTexture->Source.Init(Width, 1, 1, 1, TSF_RGBA16F);

		// Fill pixel data
		uint8* MipData = LookupInfo.LookupTexture->Source.LockMip(0);
		FFloat16Color* Pixels = reinterpret_cast<FFloat16Color*>(MipData);

		for (int32 Index = 0; Index < ValidSurfaces.Num(); Index++)
		{
			const float Value = ValidSurfaces[Index]->GetScalar(Slot.Guid);
			Pixels[Index] = FFloat16Color(FLinearColor(Value, 0.0f, 0.0f, 1.0f));
		}

		LookupInfo.LookupTexture->Source.UnlockMip(0);
		LookupInfo.LookupTexture->UpdateResource();
	}

	// Build vector lookup textures
	for (const FVoxelCubicVectorSlot& Slot : Template->VectorSlots)
	{
		FVoxelCubicVectorLookupInfo& LookupInfo = SlotToVectorLookup.Add(Slot.Guid);
		LookupInfo.SlotName = Slot.Name;
		LookupInfo.EntryCount = ValidSurfaces.Num();

		// Create lookup texture (Nx1 RGBA16F)
		LookupInfo.LookupTexture = NewObject<UTexture2D>(
			Outer,
			FName("CubicVectorLookup_" + Slot.Name.ToString()),
			RF_Transient);

		LookupInfo.LookupTexture->CompressionSettings = TC_HDR;
		LookupInfo.LookupTexture->SRGB = false;
		LookupInfo.LookupTexture->Filter = TF_Nearest;
		LookupInfo.LookupTexture->MipGenSettings = TMGS_NoMipmaps;
		LookupInfo.LookupTexture->AddressX = TA_Clamp;
		LookupInfo.LookupTexture->AddressY = TA_Clamp;

		// Create platform data and initialize
		const int32 Width = FMath::Max(1, ValidSurfaces.Num());
		LookupInfo.LookupTexture->Source.Init(Width, 1, 1, 1, TSF_RGBA16F);

		// Fill pixel data
		uint8* MipData = LookupInfo.LookupTexture->Source.LockMip(0);
		FFloat16Color* Pixels = reinterpret_cast<FFloat16Color*>(MipData);

		for (int32 Index = 0; Index < ValidSurfaces.Num(); Index++)
		{
			const FLinearColor Value = ValidSurfaces[Index]->GetVector(Slot.Guid);
			Pixels[Index] = FFloat16Color(Value);
		}

		LookupInfo.LookupTexture->Source.UnlockMip(0);
		LookupInfo.LookupTexture->UpdateResource();
	}

	return true;
}

bool FVoxelCubicTextureArrayData::GenerateCubicMaterial(
	UVoxelMegaMaterial& MegaMaterial,
	FString& OutError)
{
	VOXEL_FUNCTION_COUNTER();

	// Early out if no template or no source material
	if (!Template ||
		!Template->Material)
	{
		MegaMaterial.GeneratedCubicMaterial = nullptr;
		MegaMaterial.GeneratedCubicMaterialInstance = nullptr;
		MegaMaterial.GeneratedCubicMaterialTranslucent = nullptr;
		MegaMaterial.GeneratedCubicMaterialTranslucentInstance = nullptr;
		return true;
	}

	// Check if any cubic surfaces use masked or translucent blend modes
	bool bHasMaskedSurfaces = false;
	bool bHasTranslucentSurfaces = false;
	for (const UVoxelCubicSurfaceTypeAsset* CubicSurface : MegaMaterial.CubicSurfaceTypes)
	{
		if (!CubicSurface)
		{
			continue;
		}

		if (CubicSurface->BlendMode == BLEND_Masked)
		{
			bHasMaskedSurfaces = true;
		}
		else if (CubicSurface->BlendMode == BLEND_Translucent)
		{
			bHasTranslucentSurfaces = true;
		}
	}

	const UMaterial* SourceMaterial = Template->Material->GetMaterial();
	if (!SourceMaterial)
	{
		OutError = "Template material has no base material";
		return false;
	}

	// Build slot name to texture array map for matching and assignment
	TVoxelMap<FName, UTexture2DArray*> SlotNameToTextureArray;
	for (const FVoxelCubicTextureSlot& Slot : Template->TextureSlots)
	{
		if (const FVoxelCubicTextureArrayInfo* ArrayInfo = SlotToTextureArray.Find(Slot.Guid))
		{
			SlotNameToTextureArray.Add_EnsureNew(Slot.Name, ArrayInfo->TextureArray);
		}
		else
		{
			SlotNameToTextureArray.Add_EnsureNew(Slot.Name, nullptr);
		}
	}

	// Build slot name to scalar lookup texture map
	TVoxelMap<FName, UTexture2D*> SlotNameToScalarLookup;
	for (const FVoxelCubicScalarSlot& Slot : Template->ScalarSlots)
	{
		if (const FVoxelCubicScalarLookupInfo* LookupInfo = SlotToScalarLookup.Find(Slot.Guid))
		{
			SlotNameToScalarLookup.Add_EnsureNew(Slot.Name, LookupInfo->LookupTexture);
		}
		else
		{
			SlotNameToScalarLookup.Add_EnsureNew(Slot.Name, nullptr);
		}
	}

	// Build slot name to vector lookup texture map
	TVoxelMap<FName, UTexture2D*> SlotNameToVectorLookup;
	for (const FVoxelCubicVectorSlot& Slot : Template->VectorSlots)
	{
		if (const FVoxelCubicVectorLookupInfo* LookupInfo = SlotToVectorLookup.Find(Slot.Guid))
		{
			SlotNameToVectorLookup.Add_EnsureNew(Slot.Name, LookupInfo->LookupTexture);
		}
		else
		{
			SlotNameToVectorLookup.Add_EnsureNew(Slot.Name, nullptr);
		}
	}

	// Create or reset generated material
	if (!MegaMaterial.GeneratedCubicMaterial)
	{
		MegaMaterial.GeneratedCubicMaterial = NewObject<UMaterial>(&MegaMaterial, NAME_None, RF_Transient);
	}
	FVoxelUtilities::ClearMaterialExpressions(*MegaMaterial.GeneratedCubicMaterial);

	// Copy basic material properties
	MegaMaterial.GeneratedCubicMaterial->bUseMaterialAttributes = SourceMaterial->bUseMaterialAttributes;
	// Use masked blend mode if any cubic surfaces use masked blend
	MegaMaterial.GeneratedCubicMaterial->BlendMode = bHasMaskedSurfaces ? BLEND_Masked : EBlendMode(SourceMaterial->BlendMode);
	MegaMaterial.GeneratedCubicMaterial->TwoSided = SourceMaterial->TwoSided;

	// Copy expressions from source material
	FVoxelMaterialGenerator Generator(
		nullptr,
		*MegaMaterial.GeneratedCubicMaterial,
		"",
		false,
		nullptr);

	const TVoxelOptional<FMaterialAttributesInput> MaterialOutput = Generator.CopyExpressions(*SourceMaterial);
	if (!MaterialOutput)
	{
		OutError = "Failed to copy expressions from source material";
		return false;
	}

	// Set MetadataIndex on GetVoxelMetadata nodes
	{
		const TArray<TObjectPtr<UVoxelMetadata>>& MetadataIndexToMetadata = MegaMaterial.GetGeneratedData_EditorOnly().MetadataIndexToMetadata;

		Generator.ForeachExpression([&](UMaterialExpression& Expression)
		{
			if (UMaterialExpressionGetVoxelMetadata* GetVoxelMetadata = Cast<UMaterialExpressionGetVoxelMetadata>(&Expression))
			{
				ensure(GetVoxelMetadata->MetadataIndex == -1);
				if (GetVoxelMetadata->Metadata)
				{
					GetVoxelMetadata->MetadataIndex = MetadataIndexToMetadata.Find(GetVoxelMetadata->Metadata);
					ensure(GetVoxelMetadata->MetadataIndex != -1);
				}
			}
		});
	}

	// Create texture/scalar parameter nodes for GetVoxelMaterialLayers inputs
	UMaterialExpressionTextureObjectParameter& ChunkIndicesTexture = Generator.NewExpression<UMaterialExpressionTextureObjectParameter>();
	ChunkIndicesTexture.ParameterName = "VOXEL_ChunkIndices_Texture";
	ChunkIndicesTexture.MaterialExpressionEditorX = -1000;
	ChunkIndicesTexture.MaterialExpressionEditorY = 0;

	UMaterialExpressionScalarParameter& ChunkIndicesSizeLog2 = Generator.NewExpression<UMaterialExpressionScalarParameter>();
	ChunkIndicesSizeLog2.ParameterName = "VOXEL_ChunkIndices_TextureSizeLog2";
	ChunkIndicesSizeLog2.MaterialExpressionEditorX = -1000;
	ChunkIndicesSizeLog2.MaterialExpressionEditorY = 100;

	UMaterialExpressionTextureObjectParameter& MaterialsTexture = Generator.NewExpression<UMaterialExpressionTextureObjectParameter>();
	MaterialsTexture.ParameterName = "VOXEL_Materials_Texture";
	MaterialsTexture.MaterialExpressionEditorX = -1000;
	MaterialsTexture.MaterialExpressionEditorY = 200;

	UMaterialExpressionScalarParameter& MaterialsSizeLog2 = Generator.NewExpression<UMaterialExpressionScalarParameter>();
	MaterialsSizeLog2.ParameterName = "VOXEL_Materials_TextureSizeLog2";
	MaterialsSizeLog2.MaterialExpressionEditorX = -1000;
	MaterialsSizeLog2.MaterialExpressionEditorY = 300;

	// Create custom HLSL node that calls GetVoxelMaterialLayers
	UMaterialExpressionCustom& LayerIndexNode = Generator.NewExpression<UMaterialExpressionCustom>();
	LayerIndexNode.MaterialExpressionEditorX = -600;
	LayerIndexNode.MaterialExpressionEditorY = 0;
	LayerIndexNode.OutputType = CMOT_Float1;
	LayerIndexNode.IncludeFilePaths.Add("/Plugin/Voxel/VoxelDisplacement.ush");
	LayerIndexNode.Code = R"(
uint4 LayerMask;
float Alphas[8];
GetVoxelMaterialLayers(
    Parameters,
    ChunkIndicesTexture,
    ChunkIndicesSizeLog2,
    MaterialsTexture,
    MaterialsSizeLog2,
    LayerMask,
    Alphas);
return VoxelLayerMask_GetLayer(LayerMask, 0, 8) - 1;)";

	// Add inputs to the custom node
	LayerIndexNode.Inputs.Empty();

	FCustomInput& Input0 = LayerIndexNode.Inputs.AddDefaulted_GetRef();
	Input0.InputName = "ChunkIndicesTexture";
	Input0.Input.Expression = &ChunkIndicesTexture;

	FCustomInput& Input1 = LayerIndexNode.Inputs.AddDefaulted_GetRef();
	Input1.InputName = "ChunkIndicesSizeLog2";
	Input1.Input.Expression = &ChunkIndicesSizeLog2;

	FCustomInput& Input2 = LayerIndexNode.Inputs.AddDefaulted_GetRef();
	Input2.InputName = "MaterialsTexture";
	Input2.Input.Expression = &MaterialsTexture;

	FCustomInput& Input3 = LayerIndexNode.Inputs.AddDefaulted_GetRef();
	Input3.InputName = "MaterialsSizeLog2";
	Input3.Input.Expression = &MaterialsSizeLog2;

	// Collect expressions that need transformation
	TVoxelArray<UMaterialExpressionTextureObjectParameter*> TextureObjectParams;
	TVoxelArray<UMaterialExpressionTextureSampleParameter2D*> TextureSampleParams;
	TVoxelArray<UMaterialExpressionTextureSample*> TextureSamplers;
	TVoxelArray<UMaterialExpressionScalarParameter*> ScalarParams;
	TVoxelArray<UMaterialExpressionVectorParameter*> VectorParams;

	Generator.ForeachExpression([&](UMaterialExpression& Expression)
	{
		if (UMaterialExpressionTextureObjectParameter* TextureParam = Cast<UMaterialExpressionTextureObjectParameter>(&Expression))
		{
			if (SlotNameToTextureArray.Contains(TextureParam->ParameterName))
			{
				TextureObjectParams.Add(TextureParam);
			}
		}
		else if (UMaterialExpressionTextureSampleParameter2D* SampleParam = Cast<UMaterialExpressionTextureSampleParameter2D>(&Expression))
		{
			if (SlotNameToTextureArray.Contains(SampleParam->ParameterName))
			{
				TextureSampleParams.Add(SampleParam);
			}
		}
		else if (UMaterialExpressionTextureSample* Sampler = Cast<UMaterialExpressionTextureSample>(&Expression))
		{
			TextureSamplers.Add(Sampler);
		}
		else if (UMaterialExpressionScalarParameter* ScalarParam = Cast<UMaterialExpressionScalarParameter>(&Expression))
		{
			if (SlotNameToScalarLookup.Contains(ScalarParam->ParameterName))
			{
				ScalarParams.Add(ScalarParam);
			}
		}
		else if (UMaterialExpressionVectorParameter* VectorParam = Cast<UMaterialExpressionVectorParameter>(&Expression))
		{
			if (SlotNameToVectorLookup.Contains(VectorParam->ParameterName))
			{
				VectorParams.Add(VectorParam);
			}
		}
	});

	// Case A: Transform TextureObjectParameter -> TextureSample connections
	// Find all TextureSample nodes connected to matching TextureObjectParameter nodes
	for (UMaterialExpressionTextureObjectParameter* TextureParam : TextureObjectParams)
	{
		// Assign the texture array to this parameter
		if (UTexture2DArray* TextureArray = SlotNameToTextureArray.FindRef(TextureParam->ParameterName))
		{
			TextureParam->Texture = TextureArray;
		}

		for (UMaterialExpressionTextureSample* Sampler : TextureSamplers)
		{
			// Check if this sampler's TextureObject input is connected to this parameter
			if (Sampler->TextureObject.Expression != TextureParam)
			{
				continue;
			}

			// Intercept the Coordinates input and append Layer0
			UMaterialExpressionAppendVector& AppendNode = Generator.NewExpression<UMaterialExpressionAppendVector>();
			AppendNode.MaterialExpressionEditorX = Sampler->MaterialExpressionEditorX - 200;
			AppendNode.MaterialExpressionEditorY = Sampler->MaterialExpressionEditorY;

			// Move original Coordinates input to AppendVector.A
			if (Sampler->Coordinates.Expression)
			{
				AppendNode.A = Sampler->Coordinates;
			}
			else
			{
				// Create default texture coordinate node
				UMaterialExpressionTextureCoordinate& TexCoord = Generator.NewExpression<UMaterialExpressionTextureCoordinate>();
				TexCoord.MaterialExpressionEditorX = AppendNode.MaterialExpressionEditorX - 200;
				TexCoord.MaterialExpressionEditorY = AppendNode.MaterialExpressionEditorY;
				AppendNode.A.Expression = &TexCoord;
			}

			// Connect Layer0 to AppendVector.B
			AppendNode.B.Expression = &LayerIndexNode;
			AppendNode.B.OutputIndex = 0; // Layer 0

			// Connect AppendVector to sampler's Coordinates
			Sampler->Coordinates.Expression = &AppendNode;
			Sampler->Coordinates.OutputIndex = 0;
		}
	}

	// Case B: Transform TextureSampleParameter2D nodes into TextureObjectParameter + TextureSample
	for (UMaterialExpressionTextureSampleParameter2D* SampleParam : TextureSampleParams)
	{
		// Create new TextureObjectParameter with same parameter name
		UMaterialExpressionTextureObjectParameter& NewTextureParam = Generator.NewExpression<UMaterialExpressionTextureObjectParameter>();
		NewTextureParam.ParameterName = SampleParam->ParameterName;
		NewTextureParam.MaterialExpressionEditorX = SampleParam->MaterialExpressionEditorX - 400;
		NewTextureParam.MaterialExpressionEditorY = SampleParam->MaterialExpressionEditorY;

		// Assign the texture array to this parameter
		if (UTexture2DArray* TextureArray = SlotNameToTextureArray.FindRef(SampleParam->ParameterName))
		{
			NewTextureParam.Texture = TextureArray;
		}

		// Create new TextureSample node
		UMaterialExpressionTextureSample& NewSampler = Generator.NewExpression<UMaterialExpressionTextureSample>();
		NewSampler.MaterialExpressionEditorX = SampleParam->MaterialExpressionEditorX;
		NewSampler.MaterialExpressionEditorY = SampleParam->MaterialExpressionEditorY;

		// Connect TextureObjectParameter to TextureSample.TextureObject
		NewSampler.TextureObject.Expression = &NewTextureParam;

		// Create AppendVector for UV + Layer0
		UMaterialExpressionAppendVector& AppendNode = Generator.NewExpression<UMaterialExpressionAppendVector>();
		AppendNode.MaterialExpressionEditorX = SampleParam->MaterialExpressionEditorX - 200;
		AppendNode.MaterialExpressionEditorY = SampleParam->MaterialExpressionEditorY;

		// Move original Coordinates input to AppendVector.A
		if (SampleParam->Coordinates.Expression)
		{
			AppendNode.A = SampleParam->Coordinates;
		}
		else
		{
			// Create default texture coordinate node
			UMaterialExpressionTextureCoordinate& TexCoord = Generator.NewExpression<UMaterialExpressionTextureCoordinate>();
			TexCoord.MaterialExpressionEditorX = AppendNode.MaterialExpressionEditorX - 200;
			TexCoord.MaterialExpressionEditorY = AppendNode.MaterialExpressionEditorY;
			AppendNode.A.Expression = &TexCoord;
		}

		// Connect Layer0 to AppendVector.B
		AppendNode.B.Expression = &LayerIndexNode;
		AppendNode.B.OutputIndex = 0; // Layer 0

		// Connect AppendVector to sampler's Coordinates
		NewSampler.Coordinates.Expression = &AppendNode;

		// Rewire all outputs from old SampleParam to new Sampler
		Generator.ForeachExpression([&](UMaterialExpression& Expression)
		{
			ForeachObjectReference(Expression, [&](UObject*& Object)
			{
				if (Object == SampleParam)
				{
					Object = &NewSampler;
				}
			});
		});

		// Also update material output if it references the old node
		if (MaterialOutput->Expression == SampleParam)
		{
			// This shouldn't happen typically, but handle it
			ConstCast(MaterialOutput.GetValue()).Expression = &NewSampler;
		}
	}

	// Remove old TextureSampleParameter2D nodes that were replaced
	for (UMaterialExpressionTextureSampleParameter2D* SampleParam : TextureSampleParams)
	{
		MegaMaterial.GeneratedCubicMaterial->GetExpressionCollection().RemoveExpression(SampleParam);
	}

	// Number of surfaces for lookup texture UV calculation
	const int32 NumSurfaces = FMath::Max(1, CubicSurfaceToSliceIndex.Num());
	const float InvNumSurfaces = 1.0f / float(NumSurfaces);
	const float HalfPixelOffset = 0.5f * InvNumSurfaces;

	// Transform ScalarParameter nodes into texture samples from lookup textures
	for (UMaterialExpressionScalarParameter* ScalarParam : ScalarParams)
	{
		UTexture2D* LookupTexture = SlotNameToScalarLookup.FindRef(ScalarParam->ParameterName);

		// Create TextureObjectParameter for lookup texture
		UMaterialExpressionTextureObjectParameter& LookupTextureParam = Generator.NewExpression<UMaterialExpressionTextureObjectParameter>();
		LookupTextureParam.ParameterName = FName("VOXEL_ScalarLookup_" + ScalarParam->ParameterName.ToString());
		LookupTextureParam.Texture = LookupTexture;
		LookupTextureParam.SamplerType = SAMPLERTYPE_LinearColor;
		LookupTextureParam.MaterialExpressionEditorX = ScalarParam->MaterialExpressionEditorX - 400;
		LookupTextureParam.MaterialExpressionEditorY = ScalarParam->MaterialExpressionEditorY;

		// Create texture sampler
		UMaterialExpressionTextureSample& LookupSampler = Generator.NewExpression<UMaterialExpressionTextureSample>();
		LookupSampler.MaterialExpressionEditorX = ScalarParam->MaterialExpressionEditorX - 200;
		LookupSampler.MaterialExpressionEditorY = ScalarParam->MaterialExpressionEditorY;
		LookupSampler.SamplerType = SAMPLERTYPE_LinearColor;
		LookupSampler.TextureObject.Expression = &LookupTextureParam;

		// Create UV coordinate: ((LayerIndex / NumSurfaces) + HalfPixelOffset, 0.5)
		// Divide LayerIndex by NumSurfaces
		UMaterialExpressionDivide& DivideNode = Generator.NewExpression<UMaterialExpressionDivide>();
		DivideNode.MaterialExpressionEditorX = ScalarParam->MaterialExpressionEditorX - 600;
		DivideNode.MaterialExpressionEditorY = ScalarParam->MaterialExpressionEditorY;
		DivideNode.A.Expression = &LayerIndexNode;
		DivideNode.ConstB = float(NumSurfaces);

		// Add half pixel offset
		UMaterialExpressionAdd& AddNode = Generator.NewExpression<UMaterialExpressionAdd>();
		AddNode.MaterialExpressionEditorX = ScalarParam->MaterialExpressionEditorX - 500;
		AddNode.MaterialExpressionEditorY = ScalarParam->MaterialExpressionEditorY;
		AddNode.A.Expression = &DivideNode;
		AddNode.ConstB = HalfPixelOffset;

		// Append V coordinate (0.5)
		UMaterialExpressionAppendVector& UVAppend = Generator.NewExpression<UMaterialExpressionAppendVector>();
		UVAppend.MaterialExpressionEditorX = ScalarParam->MaterialExpressionEditorX - 400;
		UVAppend.MaterialExpressionEditorY = ScalarParam->MaterialExpressionEditorY + 50;
		UVAppend.A.Expression = &AddNode;

		UMaterialExpressionConstant& VCoord = Generator.NewExpression<UMaterialExpressionConstant>();
		VCoord.R = 0.5f;
		VCoord.MaterialExpressionEditorX = ScalarParam->MaterialExpressionEditorX - 500;
		VCoord.MaterialExpressionEditorY = ScalarParam->MaterialExpressionEditorY + 100;
		UVAppend.B.Expression = &VCoord;

		// Connect UV to sampler
		LookupSampler.Coordinates.Expression = &UVAppend;

		// Mask to R channel
		UMaterialExpressionComponentMask& RMask = Generator.NewExpression<UMaterialExpressionComponentMask>();
		RMask.MaterialExpressionEditorX = ScalarParam->MaterialExpressionEditorX;
		RMask.MaterialExpressionEditorY = ScalarParam->MaterialExpressionEditorY;
		RMask.R = true;
		RMask.G = false;
		RMask.B = false;
		RMask.A = false;
		RMask.Input.Expression = &LookupSampler;

		// Rewire all references from old ScalarParam to RMask
		Generator.ForeachExpression([&](UMaterialExpression& Expression)
		{
			ForeachObjectReference(Expression, [&](UObject*& Object)
			{
				if (Object == ScalarParam)
				{
					Object = &RMask;
				}
			});
		});
	}

	// Remove old ScalarParameter nodes that were replaced
	for (UMaterialExpressionScalarParameter* ScalarParam : ScalarParams)
	{
		MegaMaterial.GeneratedCubicMaterial->GetExpressionCollection().RemoveExpression(ScalarParam);
	}

	// Transform VectorParameter nodes into texture samples from lookup textures
	for (UMaterialExpressionVectorParameter* VectorParam : VectorParams)
	{
		UTexture2D* LookupTexture = SlotNameToVectorLookup.FindRef(VectorParam->ParameterName);

		// Create TextureObjectParameter for lookup texture
		UMaterialExpressionTextureObjectParameter& LookupTextureParam = Generator.NewExpression<UMaterialExpressionTextureObjectParameter>();
		LookupTextureParam.ParameterName = FName("VOXEL_VectorLookup_" + VectorParam->ParameterName.ToString());
		LookupTextureParam.Texture = LookupTexture;
		LookupTextureParam.SamplerType = SAMPLERTYPE_LinearColor;
		LookupTextureParam.MaterialExpressionEditorX = VectorParam->MaterialExpressionEditorX - 400;
		LookupTextureParam.MaterialExpressionEditorY = VectorParam->MaterialExpressionEditorY;

		// Create texture sampler
		UMaterialExpressionTextureSample& LookupSampler = Generator.NewExpression<UMaterialExpressionTextureSample>();
		LookupSampler.MaterialExpressionEditorX = VectorParam->MaterialExpressionEditorX - 200;
		LookupSampler.MaterialExpressionEditorY = VectorParam->MaterialExpressionEditorY;
		LookupSampler.SamplerType = SAMPLERTYPE_LinearColor;
		LookupSampler.TextureObject.Expression = &LookupTextureParam;

		// Create UV coordinate: ((LayerIndex / NumSurfaces) + HalfPixelOffset, 0.5)
		// Divide LayerIndex by NumSurfaces
		UMaterialExpressionDivide& DivideNode = Generator.NewExpression<UMaterialExpressionDivide>();
		DivideNode.MaterialExpressionEditorX = VectorParam->MaterialExpressionEditorX - 600;
		DivideNode.MaterialExpressionEditorY = VectorParam->MaterialExpressionEditorY;
		DivideNode.A.Expression = &LayerIndexNode;
		DivideNode.ConstB = float(NumSurfaces);

		// Add half pixel offset
		UMaterialExpressionAdd& AddNode = Generator.NewExpression<UMaterialExpressionAdd>();
		AddNode.MaterialExpressionEditorX = VectorParam->MaterialExpressionEditorX - 500;
		AddNode.MaterialExpressionEditorY = VectorParam->MaterialExpressionEditorY;
		AddNode.A.Expression = &DivideNode;
		AddNode.ConstB = HalfPixelOffset;

		// Append V coordinate (0.5)
		UMaterialExpressionAppendVector& UVAppend = Generator.NewExpression<UMaterialExpressionAppendVector>();
		UVAppend.MaterialExpressionEditorX = VectorParam->MaterialExpressionEditorX - 400;
		UVAppend.MaterialExpressionEditorY = VectorParam->MaterialExpressionEditorY + 50;
		UVAppend.A.Expression = &AddNode;

		UMaterialExpressionConstant& VCoord = Generator.NewExpression<UMaterialExpressionConstant>();
		VCoord.R = 0.5f;
		VCoord.MaterialExpressionEditorX = VectorParam->MaterialExpressionEditorX - 500;
		VCoord.MaterialExpressionEditorY = VectorParam->MaterialExpressionEditorY + 100;
		UVAppend.B.Expression = &VCoord;

		// Connect UV to sampler
		LookupSampler.Coordinates.Expression = &UVAppend;

		// Rewire all references from old VectorParam to LookupSampler (use RGBA output)
		Generator.ForeachExpression([&](UMaterialExpression& Expression)
		{
			ForeachObjectReference(Expression, [&](UObject*& Object)
			{
				if (Object == VectorParam)
				{
					Object = &LookupSampler;
				}
			});
		});
	}

	// Remove old VectorParameter nodes that were replaced
	for (UMaterialExpressionVectorParameter* VectorParam : VectorParams)
	{
		MegaMaterial.GeneratedCubicMaterial->GetExpressionCollection().RemoveExpression(VectorParam);
	}

	// Connect the material output
	if (MegaMaterial.GeneratedCubicMaterial->bUseMaterialAttributes)
	{
		MegaMaterial.GeneratedCubicMaterial->GetEditorOnlyData()->MaterialAttributes = *MaterialOutput;
	}

	MegaMaterial.GeneratedCubicMaterial->PostEditChange();

	// Create or update material instance
	if (!MegaMaterial.GeneratedCubicMaterialInstance ||
		MegaMaterial.GeneratedCubicMaterialInstance->Parent != MegaMaterial.GeneratedCubicMaterial)
	{
		MegaMaterial.GeneratedCubicMaterialInstance = NewObject<UMaterialInstanceConstant>(&MegaMaterial, NAME_None, RF_Transient);
		MegaMaterial.GeneratedCubicMaterialInstance->Parent = MegaMaterial.GeneratedCubicMaterial;
	}

	// Set texture array parameters
	TGuardValue<bool> Guard(GIsEditor, true);
	for (const auto& It : SlotToTextureArray)
	{
		if (It.Value.TextureArray)
		{
			MegaMaterial.GeneratedCubicMaterialInstance->SetTextureParameterValueEditorOnly(
				It.Value.SlotName,
				It.Value.TextureArray);
		}
	}

	// Set scalar lookup texture parameters
	for (const auto& It : SlotToScalarLookup)
	{
		if (It.Value.LookupTexture)
		{
			const FName ParameterName = FName("VOXEL_ScalarLookup_" + It.Value.SlotName.ToString());
			MegaMaterial.GeneratedCubicMaterialInstance->SetTextureParameterValueEditorOnly(
				ParameterName,
				It.Value.LookupTexture);
		}
	}

	// Set vector lookup texture parameters
	for (const auto& It : SlotToVectorLookup)
	{
		if (It.Value.LookupTexture)
		{
			const FName ParameterName = FName("VOXEL_VectorLookup_" + It.Value.SlotName.ToString());
			MegaMaterial.GeneratedCubicMaterialInstance->SetTextureParameterValueEditorOnly(
				ParameterName,
				It.Value.LookupTexture);
		}
	}

	// Generate translucent variant if any surfaces need it
	// Uses the same base material but with blend mode override on the instance
	if (bHasTranslucentSurfaces)
	{
		// Create or update translucent material instance - parent to the same generated material
		if (!MegaMaterial.GeneratedCubicMaterialTranslucentInstance ||
			MegaMaterial.GeneratedCubicMaterialTranslucentInstance->Parent != MegaMaterial.GeneratedCubicMaterial)
		{
			MegaMaterial.GeneratedCubicMaterialTranslucentInstance = NewObject<UMaterialInstanceConstant>(&MegaMaterial, NAME_None, RF_Transient);
			MegaMaterial.GeneratedCubicMaterialTranslucentInstance->Parent = MegaMaterial.GeneratedCubicMaterial;
		}

		// Set blend mode override to translucent
		MegaMaterial.GeneratedCubicMaterialTranslucentInstance->BasePropertyOverrides.bOverride_BlendMode = true;
		MegaMaterial.GeneratedCubicMaterialTranslucentInstance->BasePropertyOverrides.BlendMode = BLEND_Translucent;

		// Set texture array parameters on translucent instance
		TGuardValue<bool> TranslucentGuard(GIsEditor, true);
		for (const auto& It : SlotToTextureArray)
		{
			if (It.Value.TextureArray)
			{
				MegaMaterial.GeneratedCubicMaterialTranslucentInstance->SetTextureParameterValueEditorOnly(
					It.Value.SlotName,
					It.Value.TextureArray);
			}
		}

		// Set scalar lookup texture parameters on translucent instance
		for (const auto& It : SlotToScalarLookup)
		{
			if (It.Value.LookupTexture)
			{
				const FName ParameterName = FName("VOXEL_ScalarLookup_" + It.Value.SlotName.ToString());
				MegaMaterial.GeneratedCubicMaterialTranslucentInstance->SetTextureParameterValueEditorOnly(
					ParameterName,
					It.Value.LookupTexture);
			}
		}

		// Set vector lookup texture parameters on translucent instance
		for (const auto& It : SlotToVectorLookup)
		{
			if (It.Value.LookupTexture)
			{
				const FName ParameterName = FName("VOXEL_VectorLookup_" + It.Value.SlotName.ToString());
				MegaMaterial.GeneratedCubicMaterialTranslucentInstance->SetTextureParameterValueEditorOnly(
					ParameterName,
					It.Value.LookupTexture);
			}
		}
	}
	else
	{
		// No translucent surfaces, clear the translucent instance
		MegaMaterial.GeneratedCubicMaterialTranslucentInstance = nullptr;
	}

	// Clear the separate translucent material - no longer used
	MegaMaterial.GeneratedCubicMaterialTranslucent = nullptr;

	return true;
}
#endif
