// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "UObject/Interface.h"
#include "VoxelNavigationMeshInterface.generated.h"

class FVoxelNavigationMesh;

UINTERFACE()
class UVoxelNavigationMeshInterface : public UInterface
{
	GENERATED_BODY()
};

class VOXEL_API IVoxelNavigationMeshInterface
{
	GENERATED_BODY()

public:
	TSharedPtr<const FVoxelNavigationMesh> GetNavigationMesh() const
	{
		return NavigationMesh;
	}

	void SetNavigationMesh(const TSharedPtr<const FVoxelNavigationMesh>& NewNavigationMesh);
	virtual void UpdateNavigationMesh() = 0;

private:
	TSharedPtr<const FVoxelNavigationMesh> NavigationMesh;
};