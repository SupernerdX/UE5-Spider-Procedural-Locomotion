// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Tools/UEdMode.h"
#include "Tools/LegacyEdModeInterfaces.h"
#include "VoxelSelectionEdMode.generated.h"

class AVoxelWorld;
struct FVoxelStampRuntime;

UCLASS()
class UVoxelSelectionEdModeSettings : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class UVoxelSelectionEdMode : public UEdMode, public ILegacyEdModeViewportInterface
{
	GENERATED_BODY()

public:
	UVoxelSelectionEdMode();

	//~ Begin UEdMode Interface
	virtual bool IsCompatibleWith(FEditorModeID OtherModeID) const override
	{
		return true;
	}
	virtual bool UsesToolkits() const override
	{
		return false;
	}

	virtual bool HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
	//~ End UEdMode Interface

private:
	TVoxelSet<TWeakPtr<const FVoxelStampRuntime>> IgnoredStamps;
	FVector LastViewLocation;
	FRotator LastViewRotation;

	TVoxelObjectPtr<AVoxelWorld> WeakWorld;
	TVoxelObjectPtr<UInstancedStaticMeshComponent> WeakISMC;
};