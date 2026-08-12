// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Engine/StaticMesh.h"
#include "VoxelObjectPinType.h"
#include "VoxelTerminalBuffer.h"
#include "VoxelGraphStaticMeshBuffer.generated.h"

USTRUCT(DisplayName = "Static Mesh")
struct VOXELGRAPH_API FVoxelGraphStaticMesh
{
	GENERATED_BODY()

	TVoxelObjectPtr<UStaticMesh> StaticMesh;

	FORCEINLINE bool operator==(const FVoxelGraphStaticMesh& Other) const
	{
		return StaticMesh == Other.StaticMesh;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelGraphStaticMesh& Mesh)
	{
		return GetTypeHash(Mesh.StaticMesh);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelGraphStaticMesh);

USTRUCT()
struct VOXELGRAPH_API FVoxelGraphStaticMeshPinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelGraphStaticMesh, UStaticMesh)
	{
		if (bSetObject)
		{
			OutObject = Struct.StaticMesh;
		}
		else
		{
			Struct.StaticMesh = &InObject;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelGraphStaticMeshBuffer, FVoxelGraphStaticMesh);

USTRUCT(DisplayName = "Static Mesh Buffer")
struct VOXELGRAPH_API FVoxelGraphStaticMeshBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelGraphStaticMeshBuffer, FVoxelGraphStaticMesh);
};