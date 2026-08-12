// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionNaniteDisplacementSwitch.generated.h"

UCLASS()
class VOXEL_API UMaterialExpressionNaniteDisplacementSwitch : public UMaterialExpression
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (ToolTip = "Input will be used when not computing Nanite displacement"))
	FExpressionInput Default;

	UPROPERTY(meta = (ToolTip = "Input will be used when computing Nanite displacement"))
	FExpressionInput Nanite;

public:
	UMaterialExpressionNaniteDisplacementSwitch();

	//~ Begin UMaterialExpression Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
	//~ End UMaterialExpression Interface
};