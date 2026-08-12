// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampRef.h"
#include "VoxelStampComponentBase.generated.h"

UCLASS(NotBlueprintType, NotBlueprintable)
class VOXEL_API UVoxelStampComponentBase : public USceneComponent
{
	GENERATED_BODY()

public:
	//~ Begin USceneComponent Interface
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void OnVisibilityChanged() override;
	virtual void OnHiddenInGameChanged() override;
	virtual void OnActorVisibilityChanged() override;
	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;
	virtual void UpdateBounds() override;
#if WITH_EDITOR
	virtual void PostEditUndo() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End USceneComponent Interface

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateStamp();

protected:
	virtual void PreUpdateStamp() {}
	virtual FVoxelStampRef GetStamp_Internal() const VOXEL_PURE_VIRTUAL({});

private:
	// Update stamp if component properties changed
	// eg, IsVisible or Transform
	void ApplyComponentChangesToStamp();
};