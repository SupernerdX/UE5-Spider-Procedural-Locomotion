// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelHeightStampRef.h"
#include "VoxelNode_MetadataPinsBase.h"
#include "VoxelVolumeStampRef.h"
#include "Graphs/VoxelGraphStamp.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "VoxelSampleStampNodes.generated.h"

USTRUCT(Category = "Stamp", meta = (Abstract))
struct VOXEL_API FVoxelNode_SampleHeightStampBase : public FVoxelNode_MetadataPinsBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_OUTPUT_PIN(FVoxelFloatBuffer, Height);
	VOXEL_OUTPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceType);
	VOXEL_OUTPUT_PIN(FVoxelBox2D, Bounds, NoCache);

protected:
	void ComputeImpl(
		const FVoxelGraphQuery& Query,
		const FVoxelDoubleVector2DBuffer& InPosition,
		const FVoxelFloatBuffer& PreviousHeight,
		const TSharedPtr<const FVoxelHeightStampRuntime>& StampRuntime) const;
};

USTRUCT(Category = "Stamp", meta = (Abstract))
struct VOXEL_API FVoxelNode_SampleVolumeStampBase : public FVoxelNode_MetadataPinsBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_OUTPUT_PIN(FVoxelFloatBuffer, Distance);
	VOXEL_OUTPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceType);
	VOXEL_OUTPUT_PIN(FVoxelBox, Bounds, NoCache);

protected:
	void ComputeImpl(
		const FVoxelGraphQuery& Query,
		const FVoxelDoubleVectorBuffer& InPosition,
		const FVoxelFloatBuffer& PreviousDistance,
		const TSharedPtr<const FVoxelVolumeStampRuntime>& StampRuntime) const;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXEL_API FVoxelNode_SampleHeightStamp : public FVoxelNode_SampleHeightStampBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelDoubleVector2DBuffer, Position, nullptr, PositionPin);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, PreviousHeight, nullptr, HideDefault);

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	FVoxelHeightInstancedStampRef Stamp;

public:
	//~ Begin FVoxelNode Interface
#if WITH_EDITOR
	virtual FString GetDisplayName() const override;
#endif

	virtual void Initialize(FInitializer& Initializer) override;
	virtual void Compute(FVoxelGraphQuery Query) const override;

	virtual void ComputeNoCachePin(
		FVoxelGraphQuery Query,
		int32 PinIndex) const override;
	//~ End FVoxelNode Interface

private:
	TSharedPtr<const FVoxelHeightStampRuntime> StampRuntime;
};

USTRUCT()
struct VOXEL_API FVoxelNode_SampleVolumeStamp : public FVoxelNode_SampleVolumeStampBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelDoubleVectorBuffer, Position, nullptr, PositionPin);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, PreviousDistance, nullptr, HideDefault);

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	FVoxelVolumeInstancedStampRef Stamp;

public:
	//~ Begin FVoxelNode Interface
#if WITH_EDITOR
	virtual FString GetDisplayName() const override;
#endif

	virtual void Initialize(FInitializer& Initializer) override;
	virtual void Compute(FVoxelGraphQuery Query) const override;

	virtual void ComputeNoCachePin(
		FVoxelGraphQuery Query,
		int32 PinIndex) const override;
	//~ End FVoxelNode Interface

private:
	TSharedPtr<const FVoxelVolumeStampRuntime> StampRuntime;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXEL_API FVoxelNode_SampleHeightStampParameter : public FVoxelNode_SampleHeightStampBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelHeightGraphStampWrapper, Stamp, nullptr);
	VOXEL_INPUT_PIN(FVoxelDoubleVector2DBuffer, Position, nullptr, PositionPin);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, PreviousHeight, nullptr, HideDefault);

public:
	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;

	virtual void ComputeNoCachePin(
		FVoxelGraphQuery Query,
		int32 PinIndex) const override;
	//~ End FVoxelNode Interface
};

USTRUCT()
struct VOXEL_API FVoxelNode_SampleVolumeStampParameter : public FVoxelNode_SampleVolumeStampBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelVolumeGraphStampWrapper, Stamp, nullptr);
	VOXEL_INPUT_PIN(FVoxelDoubleVectorBuffer, Position, nullptr, PositionPin);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, PreviousDistance, nullptr, HideDefault);

public:
	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;

	virtual void ComputeNoCachePin(
		FVoxelGraphQuery Query,
		int32 PinIndex) const override;
	//~ End FVoxelNode Interface
};