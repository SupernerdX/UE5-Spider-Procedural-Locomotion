// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampRef.h"
#include "VoxelInstancedStampComponent.generated.h"

USTRUCT()
struct FVoxelDeprecatedInstancedStamp
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxelInstancedStruct Stamp;

	UPROPERTY()
	FTransform Transform;
};

UCLASS(ClassGroup = Voxel, meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelInstancedStampComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	virtual ~UVoxelInstancedStampComponent() override;

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
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End USceneComponent Interface

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateAllStamps();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateStamp(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateStamps(const TArray<int32>& IndicesToUpdate);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	int32 AddStamp(const FVoxelStampRef& NewStamp);

	UFUNCTION(BlueprintPure, Category = "Voxel", DisplayName = "Get Stamp")
	FVoxelStampRef GetStamp(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVoxelStampRef SetStamp(int32 Index, const FVoxelStampRef& NewStamp);

	// Remove the stamp at Index by clearing it
	// Indices won't change
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void RemoveStamp(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void ClearStamps();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	int32 NumStamps() const
	{
		return PrivateStamps.Num();
	}

	void Reserve(int32 NewNumStamps);
	// Stamp refs won't be copied and need to be unique
	void AddStamps_NoCopy(TVoxelArray<FVoxelStampRef>&& NewStamps);
	int32 FindStampIndex(const FVoxelStampRef& StampRef) const;

public:
#if WITH_EDITOR
	void DuplicateStamp(int32 Index);
	void SetActiveInstance(const FVoxelStampRef& StampRef) const;
#endif

private:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ShowOnlyInnerProperties, SplitStampProperties))
	TArray<FVoxelStampRef> PrivateStamps;

	UPROPERTY()
	TArray<FVoxelDeprecatedInstancedStamp> InstancedStamps_DEPRECATED;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Config")
	mutable uint32 ActiveInstance = 0;
#endif

	friend class FVoxelInstancedStampComponentCustomization;
	friend struct FVoxelInstancedStampComponentInstanceData;
};

USTRUCT()
struct FVoxelInstancedStampComponentInstanceData : public FSceneComponentInstanceData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FVoxelStampRef> Stamps;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	uint32 ActiveInstance = 0;
#endif

public:
	FVoxelInstancedStampComponentInstanceData() = default;
	explicit FVoxelInstancedStampComponentInstanceData(const UVoxelInstancedStampComponent* Component);

protected:
	//~ Begin FSceneComponentInstanceData Interface
	virtual bool ContainsData() const override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual void ApplyToComponent(UActorComponent* Component, ECacheApplyPhase CacheApplyPhase) override;
	//~ End FSceneComponentInstanceData Interface
};