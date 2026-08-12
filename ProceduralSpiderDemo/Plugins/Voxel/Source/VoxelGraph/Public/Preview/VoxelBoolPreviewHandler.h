// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Preview/VoxelPreviewHandler.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelBoolPreviewHandler.generated.h"

USTRUCT(DisplayName = "Bool")
struct VOXELGRAPH_API FVoxelPreviewHandler_Bool : public FVoxelPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<bool>() ||
			Type.Is<FVoxelBoolBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;
	virtual void BuildStats(const FAddStat& AddStat) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelBoolBuffer> Buffer = FVoxelBoolBuffer::MakeSharedDefault();
};