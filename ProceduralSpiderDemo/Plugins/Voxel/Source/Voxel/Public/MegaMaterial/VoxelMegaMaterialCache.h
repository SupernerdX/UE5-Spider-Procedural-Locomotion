// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMegaMaterialCache.generated.h"

class UVoxelMegaMaterial;
class UVoxelMegaMaterialGeneratedData;

UCLASS()
class VOXEL_API UVoxelMegaMaterialCache : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	static UVoxelMegaMaterialGeneratedData& GetGeneratedData(UVoxelMegaMaterial& MegaMaterial);
#endif

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bAutoSave = true;

#if WITH_EDITOR
	void AutoSaveIfEnabled() const;
#endif

public:
	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface

private:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TArray<TObjectPtr<UVoxelMegaMaterialGeneratedData>> GeneratedDatas;

	UPROPERTY()
	int32 Version = 0;
#endif
};