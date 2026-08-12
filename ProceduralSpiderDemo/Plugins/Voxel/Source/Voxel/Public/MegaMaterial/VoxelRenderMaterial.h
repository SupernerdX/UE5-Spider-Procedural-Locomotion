// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelRenderMaterial.generated.h"

USTRUCT()
struct VOXEL_API FVoxelMaterialRenderIndex
{
	GENERATED_BODY()

public:
	UPROPERTY()
	uint8 Index = 0;

	FVoxelMaterialRenderIndex() = default;
	explicit FVoxelMaterialRenderIndex(const uint8 Index)
		: Index(Index)
	{
	}
	explicit FVoxelMaterialRenderIndex(const int32 Index)
		: Index(Index)
	{
		checkVoxelSlow(FVoxelUtilities::IsValidUINT8(Index));
	}

public:
	FORCEINLINE bool operator==(const FVoxelMaterialRenderIndex& Other) const
	{
		return Index == Other.Index;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelMaterialRenderIndex& RenderIndex)
	{
		return RenderIndex.Index;
	}

	FORCEINLINE bool operator<(const FVoxelMaterialRenderIndex& Other) const
	{
		return Index < Other.Index;
	}
	FORCEINLINE bool operator>(const FVoxelMaterialRenderIndex& Other) const
	{
		return Index > Other.Index;
	}
};

struct VOXEL_API FVoxelRenderMaterial
{
	TVoxelStaticArray<FVoxelMaterialRenderIndex, 8> Indices{ ForceInit };
	TVoxelStaticArray<uint8, 8> Alphas{ ForceInit };
};