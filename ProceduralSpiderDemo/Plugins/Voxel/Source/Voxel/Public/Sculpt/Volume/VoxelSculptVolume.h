// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampComponentBase.h"
#include "Sculpt/VoxelSculptActorBase.h"
#include "Sculpt/Volume/VoxelSculptVolumeContext.h"
#include "Sculpt/Volume/VoxelVolumeSculptStampRef.h"
#include "VoxelSculptVolume.generated.h"

struct FVoxelVolumeSculptSave;
class AVoxelSculptVolume;
class UVoxelSculptVolumeAsset;
class IVoxelSculptVolumeDataSource;

UCLASS(Within=VoxelSculptVolume)
class VOXEL_API UVoxelSculptVolumeComponent : public UVoxelStampComponentBase
{
	GENERATED_BODY()

public:
	// Optional, if set save data will be stored externally
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UVoxelSculptVolumeAsset> ExternalAsset;

	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	TSet<TObjectPtr<UObject>> ReferencedAssets;

public:
	FVoxelVolumeSculptStampRef GetStamp() const;

	//~ Begin UVoxelStampComponentBase Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual FVoxelStampRef GetStamp_Internal() const override;
	//~ End UVoxelStampComponentBase Interface

private:
	UPROPERTY(EditAnywhere, Category = "Config", DisplayName = "Stamp", meta = (ShowOnlyInnerProperties, HideTransform, SplitStampProperties))
	FVoxelVolumeSculptStampRef PrivateStamp;

	friend class UVoxelStampComponent;
	friend class AVoxelSculptVolume;
};

UCLASS()
class VOXEL_API AVoxelSculptVolume : public AVoxelSculptActorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bReplicateSculptData = false;

public:
	AVoxelSculptVolume();

	//~ Begin AActor Interface
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;
	//~ End AActor Interface

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVoxelVolumeSculptStampRef GetStamp() const;

	UVoxelSculptVolumeComponent& GetComponent() const;
	TSharedRef<FVoxelDependency3D> GetDependency() const;
	TSharedRef<IVoxelBulkLoader> GetBulkLoader() const;
	TVoxelBulkRef<FVoxelSculptVolumeData> GetSculptData() const;

public:
	TVoxelOptional<FVoxelSculptVolumeContext> GetSculptContext() const;

	void SetSculptData(
		const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData,
		const TSharedRef<IVoxelBulkLoader>& NewBulkLoader);

	void SetSculptData(const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData);

	FVoxelFuture ApplyModifier(const TSharedRef<FVoxelVolumeModifier>& Modifier);
	virtual void ClearSculptCache() override;

public:
	TVoxelFuture<FVoxelVolumeSculptSave> GetSave(bool bCompress = true) const;
	FVoxelFuture LoadFromSave(const FVoxelVolumeSculptSave& Save);

private:
	UPROPERTY(VisibleAnywhere, Category = "Config")
	TObjectPtr<UVoxelSculptVolumeComponent> Component;

	mutable TSharedPtr<FVoxelSculptVolumeCache> Cache;

	TSharedPtr<FVoxelDependency3D> Dependency;
	TSharedPtr<IVoxelSculptVolumeDataSource> DataSource;
	TFunction<void()> UnregisterDataSource;

	friend UVoxelSculptVolumeComponent;
};