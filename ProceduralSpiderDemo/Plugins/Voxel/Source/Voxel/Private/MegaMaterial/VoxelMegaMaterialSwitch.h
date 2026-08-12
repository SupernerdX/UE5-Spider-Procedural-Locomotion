// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "MaterialValueType.h"
#include "MegaMaterial/MaterialExpressionVoxelMegaMaterialInternalSwitch.h"

struct FVoxelHLSLMaterialTranslator;

#if WITH_EDITOR
class FVoxelMegaMaterialSwitch
{
public:
	// Add a new version when changing MATERIAL_VERTEX_PARAMETERS_VOXEL_VERSION or MATERIAL_PIXEL_PARAMETERS_VOXEL_VERSION
	// This will force mega materials to be re-compiled
	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion,
		AddSmoothBlends,
		ChunkIndices,
		NaniteIndirection,
		AddStateTracking,
		ExtendedDataSupport
	);

	const UMaterialExpressionVoxelMegaMaterialInternalSwitch& Switch;
	FMaterialCompiler& Compiler;
	FVoxelHLSLMaterialTranslator& Translator;

	FVoxelMegaMaterialSwitch(
		const UMaterialExpressionVoxelMegaMaterialInternalSwitch& Switch,
		FMaterialCompiler& Compiler);

public:
	int32 Compile();

	int32 CompileCustomOutput(
		FExpressionInput (FVoxelMegaMaterialSwitchInputs::*InputPtr),
		EMaterialValueType ValueType);

private:
	const FString UniqueTag = "VOXEL_TAG_C5BD7C84EB4";
	bool bHasVoxelHookDefine = false;
	TVoxelSet<FString> Includes;

	int32 Indent = 0;

	struct FIndentScope
	{
		FVoxelMegaMaterialSwitch& Switch;

		explicit FIndentScope(FVoxelMegaMaterialSwitch& Switch)
			: Switch(Switch)
		{
			Switch.Indent++;
		}
		~FIndentScope()
		{
			Switch.Indent--;
		}
	};

	void AddVoxelHookDefine();
	void AddInclude(const FString& Include);
	void AddLineImpl(FString String);

private:
	bool IsVertex() const;
	bool HasVoxelTag() const;
	void AddVoxelTag();
	TVoxelSet<EMaterialProperty> GetConnectedProperties() const;

private:
	TVoxelMap<EMaterialProperty, FString> PropertyToDefaultCode;

	void InitializeDefaultProperties();
	void ComputeDisplacementAndLayerMask(bool bComputeSmoothness);
	void ComputeProperties(TConstVoxelArrayView<EMaterialProperty> Properties);
};

FORCEINLINE FArchive& operator<<(FArchive& Ar, EMaterialProperty& Property)
{
	return Ar << ReinterpretCastRef<int32>(Property);
}
#endif