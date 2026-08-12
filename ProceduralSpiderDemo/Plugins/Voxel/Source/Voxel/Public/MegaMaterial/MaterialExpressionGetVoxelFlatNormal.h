// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "MaterialValueType.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionGetVoxelFlatNormal.generated.h"

UCLASS(DisplayName = "Get Voxel Flat Normal")
class VOXEL_API UMaterialExpressionGetVoxelFlatNormal : public UMaterialExpression
{
	GENERATED_BODY()

public:
	UMaterialExpressionGetVoxelFlatNormal();

	//~ Begin UMaterialExpression Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
#if VOXEL_ENGINE_VERSION >= 506
	virtual EMaterialValueType GetOutputValueType(int32 OutputIndex) override
	{
		return MCT_Float3;
	}
#else
	virtual uint32 GetOutputType(int32 OutputIndex) override
	{
		return MCT_Float3;
	}
#endif
	virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
	//~ End UMaterialExpression Interface
};