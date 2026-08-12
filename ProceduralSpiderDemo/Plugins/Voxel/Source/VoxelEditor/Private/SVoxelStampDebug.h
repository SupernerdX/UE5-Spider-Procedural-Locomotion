// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelStackLayer.h"
#include "Misc/NotifyHook.h"
#include "VoxelVolumeStamp.h"
#include "VoxelViewportInterface.h"
#include "SVoxelStampDebug.generated.h"

class AVoxelWorld;
class FVoxelState;
class FVoxelLayers;
class UBoxComponent;
class SVoxelViewport;
class UVoxelStampComponent;
struct FVoxelBreadcrumbs;

UCLASS()
class UVoxelStampDebugSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Config)
	FTransform Transform;

	UPROPERTY(EditAnywhere, Category = Config)
	int32 LOD = 0;

	UPROPERTY(EditAnywhere, Category = Config)
	float BoxSize = 500;
};

#if 0 // TODO
USTRUCT(meta = (Internal))
struct FVoxelDebugVolumeStamp : public FVoxelVolumeStamp
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	int32 LOD = 0;
	FVoxelBox Bounds;
	FTransform Transform;
	TSharedPtr<FVoxelLayers> Layers;
	FVoxelWeakStackLayer LayerToSample;
	FVoxelWeakStampRef LastStampNextRef;
	TFunction<void(const TSharedRef<FVoxelBreadcrumb>&)> ProcessBreadcrumb;
};

USTRUCT()
struct FVoxelDebugVolumeStampRuntime : public FVoxelVolumeStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_BODY(FVoxelDebugVolumeStamp)

	//~ Begin FVoxelVolumeStampRuntime Interface
	virtual FVoxelBox GetLocalBounds() const override;

	virtual void Apply(
		const FVoxelQuery& Query,
		TVoxelArrayView<float> Distances,
		const FVoxelMetadataView& Metadata,
		TConstVoxelArrayView<double> PositionsX,
		TConstVoxelArrayView<double> PositionsY,
		TConstVoxelArrayView<double> PositionsZ,
		const FVoxelBox& FilterBounds,
		float MaxDistance,
		float DistanceScale,
		const FMatrix& PositionToStamp) const override;
	//~ End FVoxelVolumeStampRuntime Interface
};
#endif

class SVoxelStampDebug
	: public SCompoundWidget
	, public FNotifyHook
	, public FGCObject
	, public IVoxelViewportInterface
{
public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(
		const FArguments& Args,
		const FVoxelState& State,
		const FVector& Position);

	//~ Begin SWidget Interface
	virtual void Tick(
		const FGeometry& AllottedGeometry,
		double InCurrentTime,
		float InDeltaTime) override;
	//~ End SWidget Interface

	//~ Begin FNotifyHook Interface
	virtual void NotifyPostChange(
		const FPropertyChangedEvent& PropertyChangedEvent,
		FProperty* PropertyThatChanged) override;
	//~ End FNotifyHook Interface

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return "SVoxelStampDebug"; }
	//~ End FGCObject Interface

	//~ Begin IVoxelViewportInterface Interface
	virtual bool ShowFloor() const override
	{
		return false;
	}
	//~ End IVoxelViewportInterface Interface

private:
	TSharedPtr<FVoxelLayers> Layers;
	FVoxelWeakStackLayer LayerToSample;

private:
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<SVoxelViewport> Viewport;
	TObjectPtr<UVoxelStampDebugSettings> Settings;
	TSharedPtr<STreeView<TSharedPtr<FVoxelBreadcrumbs>>> TreeView;

	TArray<TSharedPtr<FVoxelBreadcrumbs>> RootBreadcrumbs;
	FVoxelWeakStampRef LastStampNextRef;

	TVoxelObjectPtr<AVoxelWorld> WeakVoxelWorld;
	TVoxelObjectPtr<UBoxComponent> WeakBoundsComponent;
	TVoxelObjectPtr<UVoxelStampComponent> WeakStampComponent;

	void SetLastStampNextRef(const FVoxelWeakStampRef& NewLastStampNextRef);
	void UpdateBreadcrumbs(const TSharedRef<FVoxelBreadcrumbs>& RootBreadcrumb);
};