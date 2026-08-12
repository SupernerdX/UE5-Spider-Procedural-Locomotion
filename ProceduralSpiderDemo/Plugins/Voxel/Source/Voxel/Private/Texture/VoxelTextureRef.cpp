// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Texture/VoxelTextureRef.h"
#include "VoxelDependency.h"

void FVoxelTextureRefPinType::Convert(
	const bool bSetObject,
	TVoxelObjectPtr<UVoxelTexture>& OutObject,
	UVoxelTexture& InObject,
	FVoxelTextureRef& Struct)
{
	if (bSetObject)
	{
		OutObject = Struct.WeakTexture;
	}
	else
	{
		Struct.WeakTexture = InObject;
		Struct.TextureData = InObject.GetData();

#if WITH_EDITOR
		if (!InObject.Dependency)
		{
			InObject.Dependency = FVoxelDependency::Create(InObject.GetName());
		}

		Struct.Dependency = InObject.Dependency;
#endif
	}
}