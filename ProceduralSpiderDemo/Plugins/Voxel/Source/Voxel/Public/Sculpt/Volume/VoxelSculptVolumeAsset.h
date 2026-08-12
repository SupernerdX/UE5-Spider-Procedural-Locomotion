// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "VoxelSculptVolumeAsset.generated.h"

class FVoxelUObjectBulkLoader;

UCLASS(meta = (VoxelAssetType, AssetColor=Red))
class VOXEL_API UVoxelSculptVolumeAsset : public UObject
{
	GENERATED_BODY()

public:
	// If true will stream data from disk as needed instead of loading everything into memory
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bEnableStreaming = false;

	// Max amount of memory we can waste before triggering a full rewrite when saving
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	int32 MaxWastedMB = 10;

	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	TSet<TObjectPtr<UObject>> ReferencedAssets;

public:
	TMulticastDelegate<void(const TVoxelBulkRef<FVoxelSculptVolumeData>&)> OnDataChanged;

	TSharedRef<IVoxelBulkLoader> GetBulkLoader() const;
	TVoxelBulkRef<FVoxelSculptVolumeData> GetData() const;
	void SetData(const TVoxelBulkRef<FVoxelSculptVolumeData>& Data);

public:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface

private:
	TSharedPtr<FVoxelUObjectBulkLoader> BulkLoader;
	TVoxelBulkPtr<FVoxelSculptVolumeData> PrivateData;
};