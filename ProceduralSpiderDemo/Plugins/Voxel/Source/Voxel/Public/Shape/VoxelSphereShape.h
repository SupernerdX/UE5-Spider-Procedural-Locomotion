// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelShape.h"
#include "VoxelSphereShape.generated.h"

USTRUCT(meta = (ShortName = "Sphere", Icon = "ClassIcon.Sphere", Thumbnail = "ClassThumbnail.Sphere", SortOrder = 0))
struct VOXEL_API FVoxelSphereShape : public FVoxelShape
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	double Radius = 1000.f;

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
