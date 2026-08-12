// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "PCGManagedResource.h"
#include "PCGManagedVoxelInstancedStampComponent.generated.h"

class UVoxelInstancedStampComponent;

UCLASS()
class VOXELPCG_API UPCGManagedVoxelInstancedStampComponent : public UPCGManagedComponent
{
	GENERATED_BODY()

public:
	//~ Begin UObject Interface
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface

	//~ Begin UPCGManagedResource Interface
	virtual bool ReleaseIfUnused(TSet<TSoftObjectPtr<AActor>>& OutActorsToDelete) override;
	//~ End UPCGManagedResource Interface

	//~ Begin UPCGManagedComponents Interface
	virtual void ResetComponent() override;
	virtual bool SupportsComponentReset() const override
	{
		return true;
	}
	virtual void MarkAsUsed() override;
	virtual void MarkAsReused() override;
	virtual void ForgetComponent() override;
	//~ End UPCGManagedComponents Interface

	UVoxelInstancedStampComponent* GetComponent() const;
	void SetComponent(UVoxelInstancedStampComponent* InComponent);

	void SetRootLocation(const FVector& InRootLocation);

	uint64 GetSettingsUID() const
	{
		return SettingsUID;
	}
	void SetSettingsUID(const uint64 InSettingsUID)
	{
		SettingsUID = InSettingsUID;
	}

protected:
	UPROPERTY()
	bool bHasRootLocation = false;

	UPROPERTY()
	FVector RootLocation = FVector::ZeroVector;

	// purposefully a value that will never happen in data
	UPROPERTY(Transient)
	uint64 SettingsUID = -1;

	// Cached raw pointer to ISM component
	mutable UVoxelInstancedStampComponent* CachedRawComponentPtr = nullptr;
};