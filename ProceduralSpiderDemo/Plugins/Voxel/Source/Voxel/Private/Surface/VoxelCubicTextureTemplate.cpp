// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelCubicTextureTemplate.h"
#include "Engine/Texture2D.h"

DEFINE_VOXEL_FACTORY(UVoxelCubicTextureTemplate);

const FVoxelCubicTextureSlot* UVoxelCubicTextureTemplate::FindTextureSlot(const FGuid& SlotGuid) const
{
	for (const FVoxelCubicTextureSlot& Slot : TextureSlots)
	{
		if (Slot.Guid == SlotGuid)
		{
			return &Slot;
		}
	}
	return nullptr;
}

const FVoxelCubicScalarSlot* UVoxelCubicTextureTemplate::FindScalarSlot(const FGuid& SlotGuid) const
{
	for (const FVoxelCubicScalarSlot& Slot : ScalarSlots)
	{
		if (Slot.Guid == SlotGuid)
		{
			return &Slot;
		}
	}
	return nullptr;
}

const FVoxelCubicVectorSlot* UVoxelCubicTextureTemplate::FindVectorSlot(const FGuid& SlotGuid) const
{
	for (const FVoxelCubicVectorSlot& Slot : VectorSlots)
	{
		if (Slot.Guid == SlotGuid)
		{
			return &Slot;
		}
	}
	return nullptr;
}

FName UVoxelCubicTextureTemplate::GetTextureSlotName(const FGuid& SlotGuid) const
{
	if (const FVoxelCubicTextureSlot* Slot = FindTextureSlot(SlotGuid))
	{
		return Slot->Name;
	}
	return NAME_None;
}

FName UVoxelCubicTextureTemplate::GetScalarSlotName(const FGuid& SlotGuid) const
{
	if (const FVoxelCubicScalarSlot* Slot = FindScalarSlot(SlotGuid))
	{
		return Slot->Name;
	}
	return NAME_None;
}

FName UVoxelCubicTextureTemplate::GetVectorSlotName(const FGuid& SlotGuid) const
{
	if (const FVoxelCubicVectorSlot* Slot = FindVectorSlot(SlotGuid))
	{
		return Slot->Name;
	}
	return NAME_None;
}

UTexture2D* UVoxelCubicTextureTemplate::GetDefaultTexture(const FGuid& SlotGuid) const
{
	if (const FVoxelCubicTextureSlot* Slot = FindTextureSlot(SlotGuid))
	{
		return Slot->DefaultTexture;
	}
	return nullptr;
}

float UVoxelCubicTextureTemplate::GetDefaultScalar(const FGuid& SlotGuid) const
{
	if (const FVoxelCubicScalarSlot* Slot = FindScalarSlot(SlotGuid))
	{
		return Slot->DefaultValue;
	}
	return 0.0f;
}

FLinearColor UVoxelCubicTextureTemplate::GetDefaultVector(const FGuid& SlotGuid) const
{
	if (const FVoxelCubicVectorSlot* Slot = FindVectorSlot(SlotGuid))
	{
		return Slot->DefaultValue;
	}
	return FLinearColor::Black;
}

#if WITH_EDITOR
void UVoxelCubicTextureTemplate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	EnsureValidGuids();

	OnTemplateChanged.Broadcast();
}

void UVoxelCubicTextureTemplate::EnsureValidGuids()
{
	for (FVoxelCubicTextureSlot& Slot : TextureSlots)
	{
		if (!Slot.Guid.IsValid())
		{
			Slot.Guid = FGuid::NewGuid();
		}
	}

	for (FVoxelCubicScalarSlot& Slot : ScalarSlots)
	{
		if (!Slot.Guid.IsValid())
		{
			Slot.Guid = FGuid::NewGuid();
		}
	}

	for (FVoxelCubicVectorSlot& Slot : VectorSlots)
	{
		if (!Slot.Guid.IsValid())
		{
			Slot.Guid = FGuid::NewGuid();
		}
	}
}
#endif
