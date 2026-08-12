// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelShape.h"
#include "VoxelPlaneShape.generated.h"

USTRUCT(meta = (ShortName = "Plane", Icon = "ClassIcon.Plane", Thumbnail = "ClassThumbnail.Plane", SortOrder = 2))
struct VOXEL_API FVoxelPlaneShape : public FVoxelShape
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = 0))
	FVector2D Size = FVector2D(1000.f);

	// Relative to Size, by how much to extend the distance field up and down
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", AdvancedDisplay, meta = (UIMin = 0, UIMax = 2))
	double Height = 1.f;

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