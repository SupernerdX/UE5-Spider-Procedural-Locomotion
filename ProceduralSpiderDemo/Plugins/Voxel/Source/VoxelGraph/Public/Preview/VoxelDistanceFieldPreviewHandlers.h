// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Preview/VoxelPreviewHandler.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelDistanceFieldPreviewHandlers.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelPreviewHandler_DistanceField_Base : public FVoxelPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual void BuildStats(const FAddStat& AddStat) override;
	//~ End FVoxelPreviewHandler Interface

	virtual FString GetValueAt(int32 Index, bool bFullValue) const VOXEL_PURE_VIRTUAL({});
	virtual FString GetMinValue(bool bFullValue) const VOXEL_PURE_VIRTUAL({});
	virtual FString GetMaxValue(bool bFullValue) const VOXEL_PURE_VIRTUAL({});
};

USTRUCT(DisplayName = "Distance Field")
struct VOXELGRAPH_API FVoxelPreviewHandler_DistanceField_Float : public FVoxelPreviewHandler_DistanceField_Base
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return Type.Is<FVoxelFloatBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual FString GetValueAt(int32 Index, bool bFullValue) const override;
	virtual FString GetMinValue(bool bFullValue) const override;
	virtual FString GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelFloatBuffer> Buffer = FVoxelFloatBuffer::MakeSharedDefault();
	float MinValue = 0.f;
	float MaxValue = 0.f;
};

USTRUCT(DisplayName = "Distance Field")
struct VOXELGRAPH_API FVoxelPreviewHandler_DistanceField_Double : public FVoxelPreviewHandler_DistanceField_Base
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return Type.Is<FVoxelDoubleBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual FString GetValueAt(int32 Index, bool bFullValue) const override;
	virtual FString GetMinValue(bool bFullValue) const override;
	virtual FString GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelDoubleBuffer> Buffer = FVoxelDoubleBuffer::MakeSharedDefault();
	double MinValue = 0.f;
	double MaxValue = 0.f;
};