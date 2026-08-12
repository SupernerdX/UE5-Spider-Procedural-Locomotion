// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelCubicTextureArrayData.generated.h"

class UMaterial;
class UMaterialInstanceConstant;
class UTexture2D;
class UTexture2DArray;
class UVoxelCubicSurfaceTypeAsset;
class UVoxelCubicTextureTemplate;
class UVoxelMegaMaterial;

// Information about a single generated texture array for a slot
USTRUCT()
struct FVoxelCubicTextureArrayInfo
{
	GENERATED_BODY()

	// The slot name (for display and shader binding)
	UPROPERTY()
	FName SlotName;

	// The generated texture array
	UPROPERTY()
	TObjectPtr<UTexture2DArray> TextureArray;

	// Resolution of textures in this array
	UPROPERTY()
	FIntPoint Resolution = FIntPoint::ZeroValue;

	// Number of slices (equals number of cubic surface types)
	UPROPERTY()
	int32 SliceCount = 0;
};

// Information about a scalar parameter lookup texture
// Each pixel stores the scalar value for one cubic surface type
USTRUCT()
struct FVoxelCubicScalarLookupInfo
{
	GENERATED_BODY()

	// The slot name (for display and shader binding)
	UPROPERTY()
	FName SlotName;

	// The generated lookup texture (Nx1 where N = number of surface types)
	UPROPERTY()
	TObjectPtr<UTexture2D> LookupTexture;

	// Number of entries (equals number of cubic surface types)
	UPROPERTY()
	int32 EntryCount = 0;
};

// Information about a vector parameter lookup texture
// Each pixel stores the RGBA value for one cubic surface type
USTRUCT()
struct FVoxelCubicVectorLookupInfo
{
	GENERATED_BODY()

	// The slot name (for display and shader binding)
	UPROPERTY()
	FName SlotName;

	// The generated lookup texture (Nx1 where N = number of surface types)
	UPROPERTY()
	TObjectPtr<UTexture2D> LookupTexture;

	// Number of entries (equals number of cubic surface types)
	UPROPERTY()
	int32 EntryCount = 0;
};

// Stores all generated texture arrays for cubic surface types
USTRUCT()
struct VOXEL_API FVoxelCubicTextureArrayData
{
	GENERATED_BODY()

	// The template all cubic surface types must use
	UPROPERTY()
	TObjectPtr<UVoxelCubicTextureTemplate> Template;

	// Map of slot GUID to generated texture array info
	UPROPERTY()
	TMap<FGuid, FVoxelCubicTextureArrayInfo> SlotToTextureArray;

	// Map of slot GUID to scalar lookup texture info
	UPROPERTY()
	TMap<FGuid, FVoxelCubicScalarLookupInfo> SlotToScalarLookup;

	// Map of slot GUID to vector lookup texture info
	UPROPERTY()
	TMap<FGuid, FVoxelCubicVectorLookupInfo> SlotToVectorLookup;

	// Map from cubic surface type asset to slice index in each texture array
	// The proxy will convert this to RenderIndex -> SliceIndex
	UPROPERTY()
	TMap<TObjectPtr<UVoxelCubicSurfaceTypeAsset>, int32> CubicSurfaceToSliceIndex;

	// Cached list of slot names (for iteration, in template order)
	UPROPERTY()
	TArray<FName> SlotNames;

public:
	void Reset();

#if WITH_EDITOR
	// Rebuild texture arrays from source data
	// Returns false and sets OutError if there's an issue
	bool RebuildTextureArrays(
		UObject* Outer,
		const TArray<TObjectPtr<UVoxelCubicSurfaceTypeAsset>>& CubicSurfaceTypes,
		FString& OutError);

	// Validate all surfaces use the same template
	static bool ValidateTemplate(
		const TArray<TObjectPtr<UVoxelCubicSurfaceTypeAsset>>& CubicSurfaceTypes,
		UVoxelCubicTextureTemplate*& OutTemplate,
		FString& OutError);

	// Generate material that samples from texture arrays
	// Transforms TextureObjectParameter/TextureSampleParameter2D nodes to use texture arrays
	// Stores generated material in MegaMaterial (not in this struct) to avoid invalidation loops
	bool GenerateCubicMaterial(
		UVoxelMegaMaterial& MegaMaterial,
		FString& OutError);
#endif
};
