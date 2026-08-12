// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Tools/UEdMode.h"
#include "BaseGizmos/GizmoActor.h"
#include "VoxelSimpleAssetToolkit.h"
#include "BaseGizmos/AxisPositionGizmo.h"
#include "Tools/LegacyEdModeInterfaces.h"
#include "Heightmap/VoxelHeightmap.h"
#include "VoxelHeightmapToolkit.generated.h"

class AVoxelWorld;
class AVoxelStampActor;
class SVoxelHeightmapList;

UCLASS()
class UVoxelHeightStampToolkitGizmoParameterSource : public UGizmoBaseFloatParameterSource
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<AVoxelStampActor> Actor;

	virtual void BeginModify() override;
	virtual float GetParameter() const override;
	virtual void SetParameter(float NewValue) override;
	virtual void EndModify() override;

private:
	TSharedPtr<FVoxelTransaction> Transaction;
	float InitialValue = 0.f;

public:
	template<typename T>
	static T* Construct(AVoxelStampActor* Actor)
	{
		T* NewSource = NewObject<T>();
		NewSource->Actor = Actor;
		return NewSource;
	}
};

UCLASS(Transient)
class AVoxelHeightStampToolkitGizmoActor : public AGizmoActor
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UGizmoCircleComponent> MinHeightComponent;

	AVoxelHeightStampToolkitGizmoActor();

	static AVoxelHeightStampToolkitGizmoActor* ConstructDefaultIntervalGizmo(UWorld* World, UGizmoViewContext* GizmoViewContext);
};

UCLASS()
class UVoxelHeightStampToolkitGizmo : public UInteractiveGizmo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UWorld> World;

	//~ Begin UInteractiveGizmo interface
	virtual void Setup() override;
	virtual void Shutdown() override;
	virtual void Tick(float DeltaTime) override;
	//~ End UInteractiveGizmo interface

	void SetActor(AVoxelStampActor* Actor);

private:
	UPROPERTY()
	TObjectPtr<AVoxelHeightStampToolkitGizmoActor> GizmoActor;

	UPROPERTY()
	TObjectPtr<AVoxelStampActor> HeightStampActor;

	UPROPERTY()
	TObjectPtr<UAxisPositionGizmo> MinHeightGizmo;
};

UCLASS()
class UVoxelHeightStampGizmoBuilder : public UInteractiveGizmoBuilder
{
	GENERATED_BODY()

public:
	virtual UInteractiveGizmo* BuildGizmo(const FToolBuilderState& SceneState) const override
	{
		UVoxelHeightStampToolkitGizmo* NewGizmo = NewObject<UVoxelHeightStampToolkitGizmo>(SceneState.GizmoManager);
		NewGizmo->World = SceneState.World;
		return NewGizmo;
	}
};

UCLASS()
class UVoxelHeightStampToolkitEdMode : public UEdMode, public ILegacyEdModeViewportInterface
{
	GENERATED_BODY()

public:
	UVoxelHeightStampToolkitEdMode();

	//~ Begin UEdMode Interface
	virtual bool IsCompatibleWith(FEditorModeID OtherModeID) const override
	{
		return true;
	}

	virtual bool UsesToolkits() const override
	{
		return true;
	}

	virtual void Enter() override;
	virtual void Exit() override;

	virtual bool Select(AActor* InActor, bool bInSelected) override;
	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
	//~ End UEdMode Interface

private:
	UPROPERTY()
	TObjectPtr<UInteractiveGizmo> InteractiveGizmo;

	UPROPERTY()
	TObjectPtr<UVoxelHeightStampGizmoBuilder> GizmoBuilder;

	void DestroyGizmos();
};

USTRUCT()
struct FVoxelHeightmapToolkit : public FVoxelSimpleAssetToolkit
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY()
	TObjectPtr<UVoxelHeightmap> Asset;

public:
	//~ Begin FVoxelSimpleAssetToolkit Interface
	virtual void Initialize() override;
	virtual void RegisterTabs(FRegisterTab RegisterTab) override;
	virtual bool ShowFloor() const override { return false; }
	virtual void Tick() override;
	virtual void PostEditChange(const FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostUndo() override;
	virtual void SetupPreview() override;
	virtual FName GetDefaultEditorModeId() const override { return "VoxelHeightStampToolkitEdMode"; }
	//~ End FVoxelSimpleAssetToolkit Interface

private:
	void UpdateMinHeightPlane() const;
	void UpdateStats();

private:
	TSharedPtr<SVoxelHeightmapList> HeightStampEntriesList;

private:
	UPROPERTY()
	TObjectPtr<AVoxelWorld> VoxelWorld;

	UPROPERTY()
	TObjectPtr<AVoxelStampActor> VoxelHeightStamp;

	UPROPERTY()
	TObjectPtr<AStaticMeshActor> MinHeightPlane;
};