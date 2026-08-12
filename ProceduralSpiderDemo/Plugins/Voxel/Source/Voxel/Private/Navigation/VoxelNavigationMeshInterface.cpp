// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Navigation/VoxelNavigationMeshInterface.h"
#include "Navigation/VoxelNavigationMesh.h"

void IVoxelNavigationMeshInterface::SetNavigationMesh(const TSharedPtr<const FVoxelNavigationMesh>& NewNavigationMesh)
{
	NavigationMesh = NewNavigationMesh;

	if (NavigationMesh)
	{
		ensure(!NavigationMesh->IsEmpty());
		NavigationMesh->UpdateStats();
	}

	UpdateNavigationMesh();
}