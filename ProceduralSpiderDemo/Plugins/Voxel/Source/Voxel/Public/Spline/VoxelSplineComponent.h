// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Components/SplineComponent.h"
#include "VoxelSplineComponent.generated.h"

class UVoxelSplineMetadata;

UCLASS(meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelSplineComponent : public USplineComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UVoxelSplineMetadata> Metadata;

	UVoxelSplineComponent();

	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface

	//~ Begin USplineComponent Interface
	virtual USplineMetadata* GetSplinePointsMetadata() override;
	virtual const USplineMetadata* GetSplinePointsMetadata() const override;
	//~ End USplineComponent Interface
};