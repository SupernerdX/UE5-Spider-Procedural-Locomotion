// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampComponentInterface.generated.h"

UINTERFACE()
class VOXEL_API UVoxelStampComponentInterface : public UInterface
{
	GENERATED_BODY()
};

class VOXEL_API IVoxelStampComponentInterface  : public IInterface
{
	GENERATED_BODY()

public:
	virtual USceneComponent* FindComponent(UClass* Class) const = 0;

	template<typename T>
	T* FindComponent() const
	{
		return CastChecked<T>(this->FindComponent(StaticClassFast<T>()), ECastCheckedType::NullAllowed);
	}
};