// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelObjectPinType.h"
#include "Texture/VoxelTexture.h"
#include "VoxelTextureRef.generated.h"

class FVoxelTextureData;

USTRUCT()
struct VOXEL_API FVoxelTextureRef
{
	GENERATED_BODY()

	TVoxelObjectPtr<UVoxelTexture> WeakTexture;
	TSharedPtr<const FVoxelTextureData> TextureData;
#if WITH_EDITOR
	TSharedPtr<FVoxelDependency> Dependency;
#endif
};

DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelTextureRef);

USTRUCT()
struct VOXEL_API FVoxelTextureRefPinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelTextureRef, UVoxelTexture);
};
