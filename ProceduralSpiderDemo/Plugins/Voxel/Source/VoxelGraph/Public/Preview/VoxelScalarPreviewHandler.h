// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Preview/VoxelPreviewHandler.h"
#include "VoxelScalarPreviewHandler.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelScalarPreviewHandler : public FVoxelPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bNormalize = true;

	FORCEINLINE float GetNormalizedValue(
		const float Value,
		const float MinValue,
		const float MaxValue) const
	{
		return bNormalize ? (Value - MinValue) / (MaxValue - MinValue) : Value;
	}

	virtual void BuildStats(const FAddStat& AddStat) override;

	virtual TArray<FString> GetMinValue(bool bFullValue) const VOXEL_PURE_VIRTUAL({});
	virtual TArray<FString> GetMaxValue(bool bFullValue) const VOXEL_PURE_VIRTUAL({});
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const VOXEL_PURE_VIRTUAL({});

protected:
	bool bUniform = false;
};

USTRUCT()
struct VOXELGRAPH_API FVoxelGrayscalePreviewHandler : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()
};