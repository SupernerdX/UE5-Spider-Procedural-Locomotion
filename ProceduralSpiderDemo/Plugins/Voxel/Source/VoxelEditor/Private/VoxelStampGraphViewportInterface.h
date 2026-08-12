// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraphToolkit.h"

class AVoxelWorld;
class AVoxelStampActor;
struct FVoxelStampRef;

class FVoxelStampGraphViewportInterface : public FVoxelGraphViewportInterface
{
public:
	//~ Begin FVoxelGraphViewportInterface Interface
	virtual void SetupPreview(
		UVoxelGraph& Graph,
		const TSharedRef<SVoxelViewport>& Viewport) override;

	virtual bool ShowFloor() const override { return false; }
	virtual FRotator GetInitialViewRotation() const override;
	virtual TOptional<float> GetInitialViewDistance() const override;
	//~ End FVoxelGraphViewportInterface Interface

	void UpdateStamp();

public:
	virtual void SetupVoxelWorld(AVoxelWorld& VoxelWorld) const {}
	virtual FVoxelStampRef MakeStampRef(UVoxelGraph& Graph) const = 0;
	virtual TOptional<float> GetBoundsSize(const FVoxelStampRef& StampRef) const = 0;

protected:
	TVoxelObjectPtr<AVoxelStampActor> WeakStampActor;
};