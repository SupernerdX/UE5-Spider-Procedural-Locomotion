// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelObjectPinType.h"
#include "VoxelTerminalBuffer.h"
#include "VoxelSoftObjectPathBuffer.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelSoftObjectPath
{
	GENERATED_BODY()

	FMinimalName Path;

	FVoxelSoftObjectPath() = default;

	FVoxelSoftObjectPath(const FSoftObjectPath& ObjectPath)
	{
		Path = FMinimalName(FName(ObjectPath.GetAssetPath().ToString()));
	}

	operator FSoftObjectPath() const
	{
		return FSoftObjectPath(FName(Path).ToString());
	}
};
checkStatic(sizeof(FVoxelSoftObjectPath) == 8);

USTRUCT()
struct VOXELGRAPH_API FVoxelSoftClassPath : public FVoxelSoftObjectPath
{
	GENERATED_BODY()

	FVoxelSoftClassPath() = default;

	FVoxelSoftClassPath(const FSoftClassPath& ClassPath)
		: FVoxelSoftObjectPath(FSoftObjectPath(ClassPath))
	{
	}

	operator FSoftClassPath() const
	{
		return FSoftClassPath(FName(Path).ToString());
	}
};
checkStatic(sizeof(FVoxelSoftClassPath) == 8);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelSoftObjectPath);
DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelSoftClassPath);

USTRUCT()
struct VOXELGRAPH_API FVoxelSoftObjectPathPinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelSoftObjectPath, UObject)
	{
		if (bSetObject)
		{
			OutObject = FSoftObjectPath(Struct).TryLoad();
		}
		else
		{
			Struct = FSoftObjectPath(&InObject);
		}
	}
};

USTRUCT()
struct VOXELGRAPH_API FVoxelSoftClassPathPinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelSoftClassPath, UClass)
	{
		if (bSetObject)
		{
			OutObject = CastEnsured<UClass>(FSoftClassPath(Struct).TryLoad());
		}
		else
		{
			Struct = FSoftClassPath(&InObject);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelSoftObjectPathBuffer, FVoxelSoftObjectPath);
DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelSoftClassPathBuffer, FVoxelSoftClassPath);

USTRUCT(DisplayName = "Soft Object Path Buffer")
struct VOXELGRAPH_API FVoxelSoftObjectPathBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelSoftObjectPathBuffer, FVoxelSoftObjectPath);
};

USTRUCT(DisplayName = "Soft Class Path Buffer")
struct VOXELGRAPH_API FVoxelSoftClassPathBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelSoftClassPathBuffer, FVoxelSoftClassPath);
};