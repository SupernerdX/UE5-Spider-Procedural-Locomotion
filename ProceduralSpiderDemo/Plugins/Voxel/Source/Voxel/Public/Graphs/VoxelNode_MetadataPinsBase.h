// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelMetadataRef.h"
#include "VoxelNode_MetadataPinsBase.generated.h"

USTRUCT(meta = (Abstract))
struct VOXEL_API FVoxelNode_MetadataPinsBase : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	FVoxelNode_MetadataPinsBase();

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