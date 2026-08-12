// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValue.h"
#include "VoxelParameter.h"
#include "Components/SplineComponent.h"
#include "VoxelSplineMetadata.generated.h"

class UVoxelGraph;

USTRUCT()
struct VOXEL_API FVoxelSplineMetadataValues
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxelParameter Parameter;

	UPROPERTY()
	FVoxelPinValue DefaultValue;

	UPROPERTY()
#if CPP
	TArray<TVoxelInstancedStruct<FVoxelPinValue>> Values;
#else
	TArray<FVoxelInstancedStruct> Values;
#endif
};

struct VOXEL_API FVoxelSplineMetadataRuntime
{
	TVoxelMap<FGuid, TVoxelArray<float>> GuidToFloatValues;
	TVoxelMap<FGuid, TVoxelArray<FVector2f>> GuidToVector2DValues;
	TVoxelMap<FGuid, TVoxelArray<FVector3f>> GuidToVectorValues;
};

UCLASS(Within=VoxelSplineComponent)
class VOXEL_API UVoxelSplineMetadata : public USplineMetadata
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FGuid, FVoxelSplineMetadataValues> GuidToValues;

public:
	void Fixup(const UVoxelGraph& Graph);
	TSharedRef<FVoxelSplineMetadataRuntime> GetRuntime() const;

public:
	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface

	//~ Begin USplineMetadata Interface
	virtual void InsertPoint(int32 Index, float t, bool bClosedLoop) override;
	virtual void UpdatePoint(int32 Index, float t, bool bClosedLoop) override;
	virtual void AddPoint(float InputKey) override;
	virtual void RemovePoint(int32 Index) override;
	virtual void DuplicatePoint(int32 Index) override;
	virtual void CopyPoint(const USplineMetadata* FromSplineMetadata, int32 FromIndex, int32 ToIndex) override;
	virtual void Reset(int32 NumPoints) override;
	virtual void Fixup(int32 NumPoints, USplineComponent*) override;
	//~ End USplineMetadata Interface
};