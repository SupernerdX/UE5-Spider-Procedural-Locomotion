// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSurfaceTypeInterface.generated.h"

class UVoxelSurfaceTypeAsset;

UCLASS(BlueprintType, Abstract)
class VOXEL_API UVoxelSurfaceTypeInterface : public UVoxelAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	FGuid Guid;

public:
	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostRename(UObject* OldOuter, const FName OldName) override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
#endif
	//~ End UObject Interface

public:
#if WITH_EDITOR
	static UVoxelSurfaceTypeAsset* Migrate(UObject* Material);

	static void Migrate(
		TObjectPtr<UObject>& Material,
		TObjectPtr<UVoxelSurfaceTypeInterface>& SurfaceType);

	static void Migrate(
		TArray<TObjectPtr<UMaterialInterface>>& Materials,
		TArray<TObjectPtr<UVoxelSurfaceTypeAsset>>& SurfaceTypes);
#endif
};