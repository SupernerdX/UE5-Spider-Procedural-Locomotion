// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "VoxelShape.generated.h"

class FVoxelMetadataView;

USTRUCT(BlueprintType, meta = (Abstract))
struct VOXEL_API FVoxelShape : public FVoxelVirtualStruct
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	virtual FVoxelBox GetLocalBounds() const
	{
		return {};
	}

	virtual void Sample(
		TVoxelArrayView<float> OutDistances,
		const FVoxelDoubleVectorBuffer& Positions) const
	{
	}

#if WITH_EDITOR
	virtual void GetPreviewInfo(
		UStaticMesh*& OutMesh,
		FTransform& OutTransform) const
	{
	}
#endif
};