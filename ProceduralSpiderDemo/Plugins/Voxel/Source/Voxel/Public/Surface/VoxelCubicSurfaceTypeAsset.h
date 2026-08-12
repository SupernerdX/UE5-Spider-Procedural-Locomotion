// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Surface/VoxelSurfaceTypeInterface.h"
#include "VoxelCubicSurfaceTypeAsset.generated.h"

class UTexture2D;
class UVoxelCubicTextureTemplate;

// Entry storing a texture assignment for a template slot
// Includes cached slot name for resilience if template is deleted
USTRUCT()
struct VOXEL_API FVoxelCubicTextureEntry
{
	GENERATED_BODY()

	// Whether this value was explicitly edited (not using template default)
	UPROPERTY()
	bool bIsEdited = false;

	// The texture assigned to this slot
	UPROPERTY(EditAnywhere, Category = "Texture")
	TObjectPtr<UTexture2D> Texture;

	// Cached from template - survives template deletion
	// Hidden from editor
	UPROPERTY()
	FName CachedSlotName;
};

// Entry storing a scalar value for a template slot
USTRUCT()
struct VOXEL_API FVoxelCubicScalarEntry
{
	GENERATED_BODY()

	// Whether this value was explicitly edited (not using template default)
	UPROPERTY()
	bool bIsEdited = false;

	// The scalar value for this slot
	UPROPERTY(EditAnywhere, Category = "Scalar")
	float Value = 0.0f;

	// Cached from template - survives template deletion
	UPROPERTY()
	FName CachedSlotName;
};

// Entry storing a vector value for a template slot
USTRUCT()
struct VOXEL_API FVoxelCubicVectorEntry
{
	GENERATED_BODY()

	// Whether this value was explicitly edited (not using template default)
	UPROPERTY()
	bool bIsEdited = false;

	// The vector value for this slot
	UPROPERTY(EditAnywhere, Category = "Vector")
	FLinearColor Value = FLinearColor::Black;

	// Cached from template - survives template deletion
	UPROPERTY()
	FName CachedSlotName;
};

// Surface type asset for cubic/block-based rendering
// References a template that defines texture slots, then provides textures for each slot
UCLASS(meta = (VoxelAssetType, AssetColor = Orange))
class VOXEL_API UVoxelCubicSurfaceTypeAsset : public UVoxelSurfaceTypeInterface
{
	GENERATED_BODY()

public:
	// Template that defines the texture slots for this surface type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelCubicTextureTemplate> Template;

	// Texture assignments keyed by slot GUID
	// Auto-populated from Template - only texture values are editable
	UPROPERTY(EditAnywhere, Category = "Textures", meta = (ReadOnlyKeys, EditCondition = "Template != nullptr", EditConditionHides))
	TMap<FGuid, FVoxelCubicTextureEntry> Textures;

	// Scalar assignments keyed by slot GUID
	// Auto-populated from Template - only scalar values are editable
	UPROPERTY(EditAnywhere, Category = "Scalars", meta = (ReadOnlyKeys, EditCondition = "Template != nullptr", EditConditionHides))
	TMap<FGuid, FVoxelCubicScalarEntry> Scalars;

	// Vector assignments keyed by slot GUID
	// Auto-populated from Template - only vector values are editable
	UPROPERTY(EditAnywhere, Category = "Vectors", meta = (ReadOnlyKeys, EditCondition = "Template != nullptr", EditConditionHides))
	TMap<FGuid, FVoxelCubicVectorEntry> Vectors;

	// Whether faces between same-type neighbors are culled
	// When false, internal faces render (e.g., for leaves with visible depth)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	bool bOccludesSameType = true;

	// Whether this block occludes faces from different-type neighbors
	// When false, adjacent blocks will show their faces against this block (e.g., stone next to glass)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	bool bOccludesOtherTypes = true;

	// Material blend mode for this surface type
	// Masked is recommended for most see-through effects (glass, leaves)
	// Translucent requires separate render pass and depth sorting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	TEnumAsByte<EBlendMode> BlendMode = BLEND_Opaque;

public:
	// Syncs Textures/Scalars/Vectors maps with Template:
	// - Adds entries for new slots
	// - Updates CachedSlotName for existing slots
	// - Optionally removes orphaned entries
	void SyncWithTemplate();

	// Gets texture for a slot, falling back to template default if not set
	UTexture2D* GetTexture(const FGuid& SlotGuid) const;

	// Gets scalar for a slot, falling back to template default if not set
	float GetScalar(const FGuid& SlotGuid) const;

	// Gets vector for a slot, falling back to template default if not set
	FLinearColor GetVector(const FGuid& SlotGuid) const;

public:
	//~ Begin UObject Interface
	virtual void PostLoad() override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface
};
