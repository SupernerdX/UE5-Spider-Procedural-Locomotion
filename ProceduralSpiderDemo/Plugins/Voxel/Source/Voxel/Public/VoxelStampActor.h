// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampRef.h"
#include "VoxelStampActor.generated.h"

class UVoxelStampComponent;

UCLASS(NotBlueprintable)
class VOXEL_API AVoxelStampActor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelStampActor();

	UVoxelStampComponent& GetStampComponent() const;

	//~ Begin AActor Interface
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
#endif
	//~ End AActor Interface

public:
	// When enabled, will automatically update actor label according to stamp properties
	UPROPERTY(EditAnywhere, Category = "Misc")
	bool bAutoUpdateLabel = true;

	UPROPERTY(EditAnywhere, Category = "Misc", meta = (EditCondition = "bAutoUpdateLabel"))
	FString LabelPrefix;

public:
	UFUNCTION(BlueprintPure, Category = "Voxel")
	FVoxelStampRef GetStamp() const;

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVoxelStampRef SetStamp(const FVoxelStampRef& NewStamp);
	void SetStamp(const FVoxelStamp& NewStamp);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateStamp();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel", meta = (AllowPrivateAccess))
	TObjectPtr<UVoxelStampComponent> StampComponent;
};