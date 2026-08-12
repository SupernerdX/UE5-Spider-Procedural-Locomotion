// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadata.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionGetVoxelMetadata.generated.h"

UCLASS(DisplayName = "Get Voxel Metadata")
class VOXEL_API UMaterialExpressionGetVoxelMetadata : public UMaterialExpression
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelMetadata> Metadata;

	// Will interpolate float typed metadata values
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bInterpolateMetadata = true;

	UPROPERTY()
	int32 MetadataIndex = -1;

public:
	UMaterialExpressionGetVoxelMetadata();

	//~ Begin UMaterialExpression Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual UObject* GetReferencedTexture() const override;
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