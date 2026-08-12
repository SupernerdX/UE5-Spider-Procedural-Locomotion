// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelShape.h"
#include "VoxelCubeShape.generated.h"

USTRUCT(meta = (ShortName = "Cube", Icon = "ClassIcon.Cube", Thumbnail = "ClassThumbnail.Cube", SortOrder = 1))
struct VOXEL_API FVoxelCubeShape : public FVoxelShape
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = 0))
	FVector Size = FVector(1000.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (UIMin = 0, UIMax = 1))
	float Roundness = 0.f;

public:
	//~ Begin FVoxelShape Interface
	virtual FVoxelBox GetLocalBounds() const override;

	virtual void Sample(
		TVoxelArrayView<float> OutDistances,
		const FVoxelDoubleVectorBuffer& Positions) const override;

#if WITH_EDITOR
	virtual void GetPreviewInfo(
		UStaticMesh*& OutMesh,
		FTransform& OutTransform) const override;
#endif
	//~ End FVoxelShape Interface
};