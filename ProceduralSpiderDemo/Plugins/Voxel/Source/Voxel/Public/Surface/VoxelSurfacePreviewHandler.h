// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Preview/VoxelPreviewHandler.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "VoxelSurfacePreviewHandler.generated.h"

USTRUCT(DisplayName = "Surface")
struct VOXEL_API FVoxelPreviewHandler_Surface : public FVoxelPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<FVoxelSurfaceTypeBlend>() ||
			Type.Is<FVoxelSurfaceTypeBlendBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;
	virtual void BuildStats(const FAddStat& AddStat) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelSurfaceTypeBlendBuffer> Buffer = FVoxelSurfaceTypeBlendBuffer::MakeSharedDefault();
};