// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "EdMode.h"
#include "Sculpt/VoxelTool.h"

class FChange;
class AVoxelSculptActorBase;

struct FVoxelToolEdMode : public FEdMode
{
public:
	FVoxelToolEdMode() = default;

	//~ Begin UEdMode Interface
	virtual void Enter() override;
	virtual void Exit() override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;
	virtual bool CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;

	virtual bool UsesToolkits() const override { return true; }
	//~ End UEdMode Interface

	UVoxelTool* GetTool(const TSubclassOf<UVoxelTool>& Class);

	AVoxelSculptActorBase* GetSculptActor() const
	{
		return WeakSculptActor.Resolve();
	}
	void SetSculptActor(AVoxelSculptActorBase* NewSculptActor);
	void SwitchSculptActor(AVoxelSculptActorBase* NewSculptActor);

	const TVoxelMap<TSubclassOf<UVoxelTool>, TObjectPtr<UVoxelTool>>& GetClassToTool();

private:
	void InitializeTools();
	void StartSculpting();
	void StopSculpting();
	void DoEdit();
	static FRay ComputeRay(FEditorViewportClient* ViewportClient, int32 X, int32 Y);

private:
	TVoxelMap<TSubclassOf<UVoxelTool>, TObjectPtr<UVoxelTool>> ClassToTool;

public:
	TObjectPtr<UVoxelTool> ActiveTool;
	TSubclassOf<UVoxelTool> ActiveToolClass;

private:
	FRay LastRay;
	bool bIsClicking = false;
	TUniquePtr<FChange> Change;
	TVoxelObjectPtr<AVoxelSculptActorBase> WeakSculptActor;
	FVoxelFuture Future;

	double LastEditTime = 0.f;
	bool bReinitializeTools = false;
	TOptional<FIntVector2> LastMousePos;

	friend class FVoxelToolToolkit;
};