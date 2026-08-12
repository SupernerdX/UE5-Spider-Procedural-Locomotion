// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampComponentBase.h"
#include "Sculpt/VoxelSculptActorBase.h"
#include "Sculpt/Height/VoxelSculptHeightContext.h"
#include "Sculpt/Height/VoxelHeightSculptStampRef.h"
#include "VoxelSculptHeight.generated.h"

struct FVoxelHeightSculptSave;
class AVoxelSculptHeight;
class UVoxelSculptHeightAsset;
class IVoxelSculptHeightDataSource;

UCLASS(Within=VoxelSculptHeight)
class VOXEL_API UVoxelSculptHeightComponent : public UVoxelStampComponentBase
{
	GENERATED_BODY()

public:
	// Optional, if set save data will be stored externally
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UVoxelSculptHeightAsset> ExternalAsset;

	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	TSet<TObjectPtr<UObject>> ReferencedAssets;

public:
	FVoxelHeightSculptStampRef GetStamp() const;

	//~ Begin UVoxelStampComponentBase Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual FVoxelStampRef GetStamp_Internal() const override;
	//~ End UVoxelStampComponentBase Interface

private:
	UPROPERTY(EditAnywhere, Category = "Config", DisplayName = "Stamp", meta = (ShowOnlyInnerProperties, HideTransform, SplitStampProperties))
	FVoxelHeightSculptStampRef PrivateStamp;

	friend class UVoxelStampComponent;
	friend class AVoxelSculptHeight;
};

UCLASS()
class VOXEL_API AVoxelSculptHeight : public AVoxelSculptActorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bReplicateSculptData = false;

public:
	AVoxelSculptHeight();

	//~ Begin AActor Interface
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;
	//~ End AActor Interface

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVoxelHeightSculptStampRef GetStamp() const;

	UVoxelSculptHeightComponent& GetComponent() const;
	TSharedRef<FVoxelDependency2D> GetDependency() const;
	TSharedRef<IVoxelBulkLoader> GetBulkLoader() const;
	TVoxelBulkRef<FVoxelSculptHeightData> GetSculptData() const;

public:
	TVoxelOptional<FVoxelSculptHeightContext> GetSculptContext() const;

	void SetSculptData(
		const TVoxelBulkRef<FVoxelSculptHeightData>& NewData,
		const TSharedRef<IVoxelBulkLoader>& NewBulkLoader);

	void SetSculptData(const TVoxelBulkRef<FVoxelSculptHeightData>& NewData);

	FVoxelFuture ApplyModifier(const TSharedRef<FVoxelHeightModifier>& Modifier);
	virtual void ClearSculptCache() override;

public:
	TVoxelFuture<FVoxelHeightSculptSave> GetSave(bool bCompress = true) const;
	FVoxelFuture LoadFromSave(const FVoxelHeightSculptSave& Save);

private:
	UPROPERTY(VisibleAnywhere, Category = "Config")
	TObjectPtr<UVoxelSculptHeightComponent> Component;

	mutable TSharedPtr<FVoxelSculptHeightCache> Cache;

	TSharedPtr<FVoxelDependency2D> Dependency;
	TSharedPtr<IVoxelSculptHeightDataSource> DataSource;
	TFunction<void()> UnregisterDataSource;

	friend UVoxelSculptHeightComponent;
};