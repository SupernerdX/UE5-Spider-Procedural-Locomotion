// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValue.h"
#include "VoxelMetadataRef.h"
#include "VoxelNodeEvaluator.h"
#include "VoxelMetadataOverrides.generated.h"

struct FVoxelGraphQueryImpl;
struct FVoxelStampSparseQuery;
struct FVoxelOutputNode_MetadataBase;
struct FVoxelRuntimeMetadataOverrides;

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMetadataOverride
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelMetadata> Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelPinValue Value;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMetadataOverrides
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<FVoxelMetadataOverride> Overrides;

public:
	void Fixup();

	TSharedRef<FVoxelRuntimeMetadataOverrides> CreateRuntime() const;

	TSharedRef<FVoxelRuntimeMetadataOverrides> CreateRuntime(
		const FVoxelGraphQueryImpl& GraphQuery,
		const TVoxelNodeEvaluator<FVoxelOutputNode_MetadataBase>& Evaluator) const;
};

struct VOXEL_API FVoxelRuntimeMetadataOverrides
{
	struct FMetadataValue
	{
		int32 PinIndex = -1;
		TSharedPtr<const FVoxelBuffer> Constant;

		bool operator==(const FMetadataValue& Other) const;
	};
	TVoxelMap<FVoxelMetadataRef, FMetadataValue> MetadataToValue;

	bool ShouldCompute(const FVoxelStampSparseQuery& Query) const;
	bool operator==(const FVoxelRuntimeMetadataOverrides& Other) const;
};