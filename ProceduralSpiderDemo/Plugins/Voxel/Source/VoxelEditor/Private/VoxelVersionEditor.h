// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Commandlets/Commandlet.h"
#include "VoxelVersionEditor.generated.h"

struct FVoxelVersionUtilities
{
	static bool UpgradeVoxelAssets();
};

UCLASS()
class UUpgradeVoxelAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	//~ Begin UCommandlet Interface
	virtual int32 Main(const FString& Params) override;
	//~ End UCommandlet Interface
};