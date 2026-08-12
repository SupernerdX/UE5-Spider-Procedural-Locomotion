// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelTexture.generated.h"

class FVoxelTextureData;

UCLASS(BlueprintType, meta = (VoxelAssetType, AssetColor=Red))
class VOXEL_API UVoxelTexture : public UVoxelAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Source")
	TSoftObjectPtr<UTexture2D> Texture;
#endif

public:
#if WITH_EDITOR
	TSharedPtr<FVoxelDependency> Dependency;
#endif

public:
	static UVoxelTexture* Create(const TSharedRef<const FVoxelTextureData>& Data);

#if WITH_EDITOR
	void OnReimport();
#endif
	TSharedPtr<const FVoxelTextureData> GetData() const;

	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

private:
	mutable TSharedPtr<const FVoxelTextureData> PrivateData;
};