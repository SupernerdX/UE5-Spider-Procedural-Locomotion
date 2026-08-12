// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBuffer.h"
#include "Preview/VoxelPreviewHandler.h"
#include "VoxelGenericPreviewHandler.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelGenericPreviewHandler : public FVoxelPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;
	virtual bool SupportsType(const FVoxelPinType& Type) const override;
	virtual void BuildStats(const FAddStat& AddStat) override;
	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override {}
	//~ End FVoxelPreviewHandler Interface

private:
	FString GetValue(int32 Index) const;

private:
	TSharedPtr<const FVoxelBuffer> Buffer;
};