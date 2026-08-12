// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "Nodes/VoxelPreviewDataNode.h"
#include "VoxelPreviewDataNode_BaseMetadata.generated.h"

struct FVoxelQueryParameterNode_MetadataPin
{
	FVoxelMetadataRef MetadataRef;
	FVoxelNode::TPinRef_Input<FVoxelBuffer> PinRef;
};

USTRUCT(meta = (Abstract))
struct VOXEL_API FVoxelPreviewDataNode_BaseMetadata : public FVoxelPreviewDataNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	FVoxelPreviewDataNode_BaseMetadata();

	//~ Begin FVoxelQueryParameterNode Interface
	virtual void Initialize(FInitializer& Initializer) override;
	virtual void PostSerialize() override;
#if WITH_EDITOR
	virtual EPostEditChange PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End FVoxelQueryParameterNode Interface

	void FixupMetadataPins();

public:
	TVoxelArray<FVoxelQueryParameterNode_MetadataPin> MetadataPins;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<TObjectPtr<UVoxelMetadata>> MetadatasToQuery;
};