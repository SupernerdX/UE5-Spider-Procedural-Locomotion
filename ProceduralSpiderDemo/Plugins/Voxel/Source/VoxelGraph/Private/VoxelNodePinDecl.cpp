// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNodePinDecl.h"
#include "VoxelNode.h"

#if WITH_EDITOR
FString FVoxelNodeDefaultValueHelper::MakeObject(const FVoxelPinType& RuntimeType, const FString& Path)
{
#if VOXEL_DEBUG
	FVoxelPinValue Value(RuntimeType.GetPinDefaultValueType());
	check(Value.IsObject());

	Value.GetObject() = FSoftObjectPtr(Path).LoadSynchronous();

	ensure(
		Value.GetObject() &&
		Value.GetObject().IsA(Value.GetType().GetObjectClass()));
#endif
	return Path;
}

FString FVoxelNodeDefaultValueHelper::MakeBodyInstance(const ECollisionEnabled::Type CollisionEnabled)
{
	FBodyInstance BodyInstance;
	BodyInstance.SetCollisionEnabled(CollisionEnabled);
	return FVoxelPinValue::Make(BodyInstance).ExportToString();
}
#endif