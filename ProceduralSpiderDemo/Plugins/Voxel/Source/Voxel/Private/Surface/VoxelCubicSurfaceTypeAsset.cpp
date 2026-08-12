// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelCubicSurfaceTypeAsset.h"
#include "Surface/VoxelCubicTextureTemplate.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialGeneratedData.h"
#include "Engine/Texture2D.h"

DEFINE_VOXEL_FACTORY(UVoxelCubicSurfaceTypeAsset);

void UVoxelCubicSurfaceTypeAsset::SyncWithTemplate()
{
	if (!Template)
	{
		return;
	}

	// Sync texture slots
	{
		TVoxelSet<FGuid> ValidGuids;
		for (const FVoxelCubicTextureSlot& Slot : Template->TextureSlots)
		{
			ValidGuids.Add(Slot.Guid);

			FVoxelCubicTextureEntry* ExistingEntry = Textures.Find(Slot.Guid);
			if (!ExistingEntry)
			{
				// Create new entry with template default
				FVoxelCubicTextureEntry& NewEntry = Textures.Add(Slot.Guid);
				NewEntry.bIsEdited = false;
				NewEntry.Texture = Slot.DefaultTexture;
				NewEntry.CachedSlotName = Slot.Name;
			}
			else
			{
				// Update cached name for existing entry
				ExistingEntry->CachedSlotName = Slot.Name;

				// Update stored value to match template default when not edited
				if (!ExistingEntry->bIsEdited)
				{
					ExistingEntry->Texture = Slot.DefaultTexture;
				}
			}
		}

		for (auto It = Textures.CreateIterator(); It; ++It)
		{
			if (!ValidGuids.Contains(It.Key()))
			{
				It.RemoveCurrent();
			}
		}
	}

	// Sync scalar slots
	{
		TVoxelSet<FGuid> ValidGuids;
		for (const FVoxelCubicScalarSlot& Slot : Template->ScalarSlots)
		{
			ValidGuids.Add(Slot.Guid);

			FVoxelCubicScalarEntry* ExistingEntry = Scalars.Find(Slot.Guid);
			if (!ExistingEntry)
			{
				// Create new entry with template default
				FVoxelCubicScalarEntry& NewEntry = Scalars.Add(Slot.Guid);
				NewEntry.bIsEdited = false;
				NewEntry.Value = Slot.DefaultValue;
				NewEntry.CachedSlotName = Slot.Name;
			}
			else
			{
				// Update cached name for existing entry
				ExistingEntry->CachedSlotName = Slot.Name;

				// Update stored value to match template default when not edited
				if (!ExistingEntry->bIsEdited)
				{
					ExistingEntry->Value = Slot.DefaultValue;
				}
			}
		}

		for (auto It = Scalars.CreateIterator(); It; ++It)
		{
			if (!ValidGuids.Contains(It.Key()))
			{
				It.RemoveCurrent();
			}
		}
	}

	// Sync vector slots
	{
		TVoxelSet<FGuid> ValidGuids;
		for (const FVoxelCubicVectorSlot& Slot : Template->VectorSlots)
		{
			ValidGuids.Add(Slot.Guid);

			FVoxelCubicVectorEntry* ExistingEntry = Vectors.Find(Slot.Guid);
			if (!ExistingEntry)
			{
				// Create new entry with template default
				FVoxelCubicVectorEntry& NewEntry = Vectors.Add(Slot.Guid);
				NewEntry.bIsEdited = false;
				NewEntry.Value = Slot.DefaultValue;
				NewEntry.CachedSlotName = Slot.Name;
			}
			else
			{
				// Update cached name for existing entry
				ExistingEntry->CachedSlotName = Slot.Name;

				// Update stored value to match template default when not edited
				if (!ExistingEntry->bIsEdited)
				{
					ExistingEntry->Value = Slot.DefaultValue;
				}
			}
		}

		for (auto It = Vectors.CreateIterator(); It; ++It)
		{
			if (!ValidGuids.Contains(It.Key()))
			{
				It.RemoveCurrent();
			}
		}
	}
}

UTexture2D* UVoxelCubicSurfaceTypeAsset::GetTexture(const FGuid& SlotGuid) const
{
	// Only use stored value if it was explicitly edited
	if (const FVoxelCubicTextureEntry* Entry = Textures.Find(SlotGuid))
	{
		if (Entry->bIsEdited)
		{
			return Entry->Texture;
		}
	}

	// Use template default when not edited
	if (Template)
	{
		return Template->GetDefaultTexture(SlotGuid);
	}

	return nullptr;
}

float UVoxelCubicSurfaceTypeAsset::GetScalar(const FGuid& SlotGuid) const
{
	// Only use stored value if it was explicitly edited
	if (const FVoxelCubicScalarEntry* Entry = Scalars.Find(SlotGuid))
	{
		if (Entry->bIsEdited)
		{
			return Entry->Value;
		}
	}

	// Use template default when not edited
	if (Template)
	{
		return Template->GetDefaultScalar(SlotGuid);
	}

	return 0.0f;
}

FLinearColor UVoxelCubicSurfaceTypeAsset::GetVector(const FGuid& SlotGuid) const
{
	// Only use stored value if it was explicitly edited
	if (const FVoxelCubicVectorEntry* Entry = Vectors.Find(SlotGuid))
	{
		if (Entry->bIsEdited)
		{
			return Entry->Value;
		}
	}

	// Use template default when not edited
	if (Template)
	{
		return Template->GetDefaultVector(SlotGuid);
	}

	return FLinearColor::Black;
}

void UVoxelCubicSurfaceTypeAsset::PostLoad()
{
	Super::PostLoad();

	SyncWithTemplate();
}

void UVoxelCubicSurfaceTypeAsset::PostDuplicate(const EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	SyncWithTemplate();
}

#if WITH_EDITOR
void UVoxelCubicSurfaceTypeAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetMemberPropertyName();

	// Sync maps when template changes
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UVoxelCubicSurfaceTypeAsset, Template))
	{
		SyncWithTemplate();
	}

	// Mark entries as edited when their values change
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UVoxelCubicSurfaceTypeAsset, Textures) &&
		Template)
	{
		for (auto& It : Textures)
		{
			if (!It.Value.bIsEdited)
			{
				UTexture2D* DefaultTexture = Template->GetDefaultTexture(It.Key);
				if (It.Value.Texture != DefaultTexture)
				{
					It.Value.bIsEdited = true;
				}
			}
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UVoxelCubicSurfaceTypeAsset, Scalars) &&
		Template)
	{
		for (auto& It : Scalars)
		{
			if (!It.Value.bIsEdited)
			{
				const float DefaultValue = Template->GetDefaultScalar(It.Key);
				if (!FMath::IsNearlyEqual(It.Value.Value, DefaultValue))
				{
					It.Value.bIsEdited = true;
				}
			}
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UVoxelCubicSurfaceTypeAsset, Vectors) &&
		Template)
	{
		for (auto& It : Vectors)
		{
			if (!It.Value.bIsEdited)
			{
				const FLinearColor DefaultValue = Template->GetDefaultVector(It.Key);
				if (It.Value.Value != DefaultValue)
				{
					It.Value.bIsEdited = true;
				}
			}
		}
	}

	// Refresh mega materials that use this cubic surface when any values change
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UVoxelCubicSurfaceTypeAsset, Template) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UVoxelCubicSurfaceTypeAsset, Textures) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UVoxelCubicSurfaceTypeAsset, Scalars) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UVoxelCubicSurfaceTypeAsset, Vectors))
	{
		ForEachObjectOfClass_Copy<UVoxelMegaMaterial>([&](UVoxelMegaMaterial& MegaMaterial)
		{
			if (MegaMaterial.CubicSurfaceTypes.Contains(this))
			{
				MegaMaterial.GetGeneratedData_EditorOnly().QueueRebuild();
			}
		});
	}
}
#endif
