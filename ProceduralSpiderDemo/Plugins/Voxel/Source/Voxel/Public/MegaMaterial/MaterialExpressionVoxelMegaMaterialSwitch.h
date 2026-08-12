// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionVoxelMegaMaterialSwitch.generated.h"

UCLASS()
class VOXEL_API UMaterialExpressionVoxelMegaMaterialSwitch : public UMaterialExpression
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (ToolTip = "Input will be used when rendering as a regular material"))
	FExpressionInput Default;

	UPROPERTY(meta = (ToolTip = "Input will be used when rendering through a mega material"))
	FExpressionInput Voxel;

	UPROPERTY()
	bool bIsMegaMaterial = false;

public:
	UMaterialExpressionVoxelMegaMaterialSwitch();

	//~ Begin UMaterialExpression Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
	//~ End UMaterialExpression Interface
};