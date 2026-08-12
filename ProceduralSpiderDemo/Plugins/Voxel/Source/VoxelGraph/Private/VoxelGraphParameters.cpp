// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphParameters.h"

namespace FVoxelGraphParameters
{
	DEFINE_VOXEL_INSTANCE_COUNTER(FUniformParameter);
	DEFINE_VOXEL_INSTANCE_COUNTER(FBufferParameter);
}

FVoxelGraphParameterManager* GVoxelGraphParameterManager = new FVoxelGraphParameterManager();

FVoxelGraphParameterManager::FBufferInfo FVoxelGraphParameterManager::GetBufferInfo(const int32 Index) const
{
	VOXEL_SCOPE_LOCK(CriticalSection);
	return BufferInfos_RequiresLock[Index];
}

int32 FVoxelGraphParameterManager::GetUniformIndexImpl(const FName Name)
{
	VOXEL_SCOPE_LOCK(CriticalSection);
	return Uniforms_RequiresLock.Add(Name).GetIndex();
}

int32 FVoxelGraphParameterManager::GetBufferIndexImpl(
	const FName Name,
	const FBufferInfo Info)
{
	VOXEL_SCOPE_LOCK(CriticalSection);

	FVoxelSetIndex Index = Buffers_RequiresLock.Find(Name);
	if (!Index.IsValid())
	{
		Index = Buffers_RequiresLock.Add(Name);

		BufferInfos_RequiresLock.SetNumZeroed(Index.GetIndex() + 1);
		BufferInfos_RequiresLock[Index.GetIndex()] = Info;
	}
	return Index.GetIndex();
}