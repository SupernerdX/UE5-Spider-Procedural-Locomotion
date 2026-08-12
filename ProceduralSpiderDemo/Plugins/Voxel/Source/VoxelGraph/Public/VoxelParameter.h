// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinType.h"
#include "VoxelParameter.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelParameter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FName Name;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FVoxelPinType Type;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FString Category;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (MultiLine = true))
	FString Description;

	UPROPERTY()
	TMap<FName, FString> MetaData;
#endif

	void Fixup();

#if WITH_EDITOR
	TMap<FName, FString> GetMetaData() const;
#endif
};