// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/MaterialExpressionNaniteDisplacementSwitch.h"
#include "Materials/HLSLMaterialTranslator.h"
#include "MaterialCompiler.h"
#include "VoxelHLSLMaterialTranslator.h"

UMaterialExpressionNaniteDisplacementSwitch::UMaterialExpressionNaniteDisplacementSwitch()
{
#if WITH_EDITORONLY_DATA
	MenuCategories.Add(INVTEXT("Voxel Plugin"));
#endif
}

void UMaterialExpressionNaniteDisplacementSwitch::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
int32 UMaterialExpressionNaniteDisplacementSwitch::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	FVoxelHLSLMaterialTranslator& Writer = *static_cast<FVoxelHLSLMaterialTranslator*>(Compiler);

	int32 DefaultIndex = Default.Compile(Compiler);
	if (DefaultIndex == -1)
	{
		return Compiler->Errorf(TEXT("Missing input Default"));
	}

	int32 NaniteIndex = Nanite.Compile(Compiler);
	if (NaniteIndex == -1)
	{
		return Compiler->Errorf(TEXT("Missing input Nanite"));
	}

	const EMaterialValueType Type = FMath::Max(
		Compiler->GetType(DefaultIndex),
		Compiler->GetType(NaniteIndex));

	DefaultIndex = Compiler->ForceCast(DefaultIndex, Type, MFCF_ExactMatch);
	NaniteIndex = Compiler->ForceCast(NaniteIndex, Type, MFCF_ExactMatch);

	const EDerivativeStatus DefaultDerivativeStatus = Writer.GetDerivativeStatus(DefaultIndex);
	const EDerivativeStatus NaniteDerivativeStatus = Writer.GetDerivativeStatus(NaniteIndex);

	EDerivativeStatus DerivativeStatus = EDerivativeStatus::NotValid;
	if (DefaultDerivativeStatus == EDerivativeStatus::Zero &&
		NaniteDerivativeStatus == EDerivativeStatus::Zero)
	{
		DerivativeStatus = EDerivativeStatus::Zero;
	}
	else if (
		DefaultDerivativeStatus == EDerivativeStatus::Valid &&
		NaniteDerivativeStatus == EDerivativeStatus::Valid)
	{
		DerivativeStatus = EDerivativeStatus::Valid;
	}
	else if (
		DefaultDerivativeStatus == EDerivativeStatus::Zero &&
		NaniteDerivativeStatus == EDerivativeStatus::Valid)
	{
		DerivativeStatus = EDerivativeStatus::Valid;
	}
	else if (
		DefaultDerivativeStatus == EDerivativeStatus::Valid &&
		NaniteDerivativeStatus == EDerivativeStatus::Zero)
	{
		DerivativeStatus = EDerivativeStatus::Valid;
	}

	// We need to add the indices in a comment otherwise the code will be falsely reused by other statements
	const int32 ResultIndex = Writer.AddCodeChunkInnerDeriv(
		*FString::Printf(TEXT("(%s)0 /* Default %d Nanite %d */"),
			Writer.HLSLTypeString(Type),
			DefaultIndex,
			NaniteIndex),
		*FString::Printf(TEXT("(%s)0 /* Default %d Nanite %d */"),
			Writer.HLSLTypeStringDeriv(Type, DerivativeStatus),
			DefaultIndex,
			NaniteIndex),
		Type,
		false,
		DerivativeStatus);

	FString Code = R"(
#if NANITE_TESSELLATION
	ResultNanite = Nanite;
#else
	ResultDefault = Default;
#endif
)";

	if (DefaultDerivativeStatus == DerivativeStatus)
	{
		Code.ReplaceInline(TEXT("ResultDefault"), *Writer.GetParameterCodeRaw(ResultIndex), ESearchCase::CaseSensitive);
		Code.ReplaceInline(TEXT("Default"), *Writer.GetParameterCodeRaw(DefaultIndex), ESearchCase::CaseSensitive);
	}
	else
	{
		// Also works when derivative is zero, as the derivative value will already be set to 0
		Code.ReplaceInline(TEXT("ResultDefault"), *Writer.GetParameterCode(ResultIndex), ESearchCase::CaseSensitive);
		Code.ReplaceInline(TEXT("Default"), *Writer.GetParameterCode(DefaultIndex), ESearchCase::CaseSensitive);
	}

	if (NaniteDerivativeStatus == DerivativeStatus)
	{
		Code.ReplaceInline(TEXT("ResultNanite"), *Writer.GetParameterCodeRaw(ResultIndex), ESearchCase::CaseSensitive);
		Code.ReplaceInline(TEXT("Nanite"), *Writer.GetParameterCodeRaw(NaniteIndex), ESearchCase::CaseSensitive);
	}
	else
	{
		// Also works when derivative is zero, as the derivative value will already be set to 0
		Code.ReplaceInline(TEXT("ResultNanite"), *Writer.GetParameterCode(ResultIndex), ESearchCase::CaseSensitive);
		Code.ReplaceInline(TEXT("Nanite"), *Writer.GetParameterCode(NaniteIndex), ESearchCase::CaseSensitive);
	}

	Writer.AddCodeChunk(MCT_VoidStatement, *Code);

	return ResultIndex;
}

void UMaterialExpressionNaniteDisplacementSwitch::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add("Nanite Displacement Switch");
}
#endif