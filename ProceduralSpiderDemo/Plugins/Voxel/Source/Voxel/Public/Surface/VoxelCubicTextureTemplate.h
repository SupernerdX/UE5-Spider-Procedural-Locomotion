// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelCubicTextureTemplate.generated.h"

class UTexture2D;
class UMaterialInterface;

DECLARE_MULTICAST_DELEGATE(FOnCubicTextureTemplateChanged);

// Defines a single texture slot in a cubic texture template
USTRUCT(BlueprintType)
struct VOXEL_API FVoxelCubicTextureSlot
{
	GENERATED_BODY()

	// Display name for UI and material parameter binding
	UPROPERTY(EditAnywhere, Category = "Slot")
	FName Name;

	// Default texture used when surface type doesn't specify one
	UPROPERTY(EditAnywhere, Category = "Slot")
	TObjectPtr<UTexture2D> DefaultTexture;

	// Stable identifier - survives renames, used as map key in surface types
	// Hidden from editor, auto-generated
	UPROPERTY()
	FGuid Guid;
};

// Defines a single scalar parameter slot in a cubic texture template
USTRUCT(BlueprintType)
struct VOXEL_API FVoxelCubicScalarSlot
{
	GENERATED_BODY()

	// Display name for UI and material parameter binding
	UPROPERTY(EditAnywhere, Category = "Slot")
	FName Name;

	// Default value used when surface type doesn't specify one
	UPROPERTY(EditAnywhere, Category = "Slot")
	float DefaultValue = 0.0f;

	// Stable identifier - survives renames, used as map key in surface types
	// Hidden from editor, auto-generated
	UPROPERTY()
	FGuid Guid;
};

// Defines a single vector parameter slot in a cubic texture template
USTRUCT(BlueprintType)
struct VOXEL_API FVoxelCubicVectorSlot
{
	GENERATED_BODY()

	// Display name for UI and material parameter binding
	UPROPERTY(EditAnywhere, Category = "Slot")
	FName Name;

	// Default value used when surface type doesn't specify one
	UPROPERTY(EditAnywhere, Category = "Slot")
	FLinearColor DefaultValue = FLinearColor::Black;

	// Stable identifier - survives renames, used as map key in surface types
	// Hidden from editor, auto-generated
	UPROPERTY()
	FGuid Guid;
};

// Template asset that defines the texture slots for cubic surface types
// Create this asset first, then reference it from cubic surface type assets
UCLASS(BlueprintType, meta = (VoxelAssetType, AssetColor = Orange))
class VOXEL_API UVoxelCubicTextureTemplate : public UVoxelAsset
{
	GENERATED_BODY()

public:
	// Texture slots defined by this template
	UPROPERTY(EditAnywhere, Category = "Slots")
	TArray<FVoxelCubicTextureSlot> TextureSlots;

	// Scalar parameter slots - values are packed into a lookup texture
	UPROPERTY(EditAnywhere, Category = "Slots")
	TArray<FVoxelCubicScalarSlot> ScalarSlots;

	// Vector parameter slots - values are packed into a lookup texture
	UPROPERTY(EditAnywhere, Category = "Slots")
	TArray<FVoxelCubicVectorSlot> VectorSlots;

	// Material for cubic rendering - texture parameters with names matching
	// slot names will be bound to generated texture arrays
	UPROPERTY(EditAnywhere, Category = "Material")
	TObjectPtr<UMaterialInterface> Material;

	// If true, faces at the boundary of unsampled (NaN) regions will be hidden
	// If false (default), faces are shown when solid blocks are adjacent to unsampled cells
	UPROPERTY(EditAnywhere, Category = "Meshing")
	bool bHideFacesAtNaNBoundaries = false;

	// If true, use nearest neighbor filtering for a pixelated look
	// If false (default), use standard texture filtering
	UPROPERTY(EditAnywhere, Category = "Rendering")
	bool bUseNearestFilter = false;

public:
	// Find a texture slot by GUID
	const FVoxelCubicTextureSlot* FindTextureSlot(const FGuid& SlotGuid) const;

	// Find a scalar slot by GUID
	const FVoxelCubicScalarSlot* FindScalarSlot(const FGuid& SlotGuid) const;

	// Find a vector slot by GUID
	const FVoxelCubicVectorSlot* FindVectorSlot(const FGuid& SlotGuid) const;

	// Get the display name for a texture slot GUID
	FName GetTextureSlotName(const FGuid& SlotGuid) const;

	// Get the display name for a scalar slot GUID
	FName GetScalarSlotName(const FGuid& SlotGuid) const;

	// Get the display name for a vector slot GUID
	FName GetVectorSlotName(const FGuid& SlotGuid) const;

	// Get the default texture for a slot GUID
	UTexture2D* GetDefaultTexture(const FGuid& SlotGuid) const;

	// Get the default scalar value for a slot GUID
	float GetDefaultScalar(const FGuid& SlotGuid) const;

	// Get the default vector value for a slot GUID
	FLinearColor GetDefaultVector(const FGuid& SlotGuid) const;

#if WITH_EDITOR
public:
	// Broadcast when template slots or defaults change
	FOnCubicTextureTemplateChanged OnTemplateChanged;

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

private:
	// Ensures all slots have valid GUIDs
	void EnsureValidGuids();
#endif
};
