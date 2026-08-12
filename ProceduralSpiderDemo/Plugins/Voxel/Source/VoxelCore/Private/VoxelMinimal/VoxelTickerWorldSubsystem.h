// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelTickerWorldSubsystem.generated.h"

UCLASS()
class UVoxelTickerWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin UWorldSubsystem Interface
	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;
	//~ End UWorldSubsystem Interface
};