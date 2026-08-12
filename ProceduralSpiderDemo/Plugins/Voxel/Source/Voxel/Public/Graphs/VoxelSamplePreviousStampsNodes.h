// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelMetadataRef.h"
#include "Graphs/VoxelGraphStamp.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "VoxelSamplePreviousStampsNodes.generated.h"

USTRUCT(meta = (Abstract))
struct VOXEL_API FVoxelNode_SamplePreviousStampsBase : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	FVoxelNode_SamplePreviousStampsBase();

	//~ Begin FVoxelNode Interface
	virtual void Initialize(FInitializer& Initializer) override;
	virtual void PostSerialize() override;
#if WITH_EDITOR
	virtual EPostEditChange PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End FVoxelNode Interface

public:
	struct FMetadataPin
	{
		FVoxelMetadataRef MetadataRef;
		FPinRef_Output PinRef;
	};
	TVoxelArray<FMetadataPin> MetadataPins;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<TObjectPtr<UVoxelMetadata>> MetadatasToQuery;

	void FixupMetadataPins();
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(Category = "Stamp", meta = (AllowList = "Height, HeightSpline", Keywords = "get"))
struct VOXEL_API FVoxelNode_SamplePreviousHeightStamps : public FVoxelNode_SamplePreviousStampsBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelDoubleVector2DBuffer, Position, nullptr, PositionPin);
	VOXEL_OUTPUT_PIN(FVoxelFloatBuffer, Height);
	VOXEL_OUTPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceType);

public:
	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(Category = "Stamp", meta = (AllowList = "Volume, VolumeSpline", Keywords = "get"))
struct VOXEL_API FVoxelNode_SamplePreviousVolumeStamps : public FVoxelNode_SamplePreviousStampsBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelDoubleVectorBuffer, Position, nullptr, PositionPin);
	VOXEL_OUTPUT_PIN(FVoxelFloatBuffer, Distance);
	VOXEL_OUTPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceType);

public:
	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(Category = "Stamp", meta = (AllowList = "SmartSurfaceType", Keywords = "get sample query surface previous stamps"))
struct VOXEL_API FVoxelNode_SampleTerrain : public FVoxelNode_SamplePreviousStampsBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelDoubleVectorBuffer, Position, nullptr, PositionPin);
	VOXEL_OUTPUT_PIN(FVoxelFloatBuffer, Distance);
	VOXEL_OUTPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceType);

public:
	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};