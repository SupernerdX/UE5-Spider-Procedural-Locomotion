// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelExposedSeed.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionGetVoxelRandom.generated.h"

// Returns a float between Min and Max, unique per voxel surface
UCLASS(DisplayName = "Get Voxel Random")
class VOXEL_API UMaterialExpressionGetVoxelRandom : public UMaterialExpression
{
	GENERATED_BODY()

public:
	// Unique seed to further randomize SurfaceTypeSeed
	UPROPERTY(EditAnywhere, Category = "Config")
	FVoxelExposedSeed Seed;

	// Automatically assigned by the mega material
	UPROPERTY(VisibleAnywhere, Category = "Config")
	FVoxelExposedSeed SurfaceTypeSeed;

	UPROPERTY(EditAnywhere, Category = "Config")
	float Min = 0.f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float Max = 1.f;

public:
	UMaterialExpressionGetVoxelRandom();

	//~ Begin UMaterialExpression Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
#if VOXEL_ENGINE_VERSION >= 506
	virtual EMaterialValueType GetOutputValueType(int32 OutputIndex) override;
#else
	virtual uint32 GetOutputType(int32 OutputIndex) override;
#endif
	virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
	//~ End UMaterialExpression Interface
};