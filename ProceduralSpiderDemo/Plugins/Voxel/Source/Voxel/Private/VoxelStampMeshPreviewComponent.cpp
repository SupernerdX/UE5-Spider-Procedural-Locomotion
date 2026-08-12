// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampMeshPreviewComponent.h"
#include "Materials/MaterialInterface.h"

UVoxelStampMeshPreviewComponent::UVoxelStampMeshPreviewComponent()
{
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bDisallowNanite = true;

	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> PreviewMaterial(TEXT("/Engine/Engine_MI_Shaders/Instances/M_Shader_SimpleTranslucent_Invis.M_Shader_SimpleTranslucent_Invis"));

	for (int32 Index = 0; Index < 32; Index++)
	{
		SetMaterial(Index, PreviewMaterial.Get());
	}

#if WITH_EDITOR
	SetVisibility(FVoxelUtilities::IsActorSelected_AnyThread(GetOwner()));

	FVoxelUtilities::OnActorSelectionChanged(*GetOwner(), MakeWeakObjectPtrDelegate(this, [this](const bool bIsSelected)
	{
		SetVisibility(bIsSelected);
	}));
#endif
}