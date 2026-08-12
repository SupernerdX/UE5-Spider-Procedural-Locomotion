// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "PCGContext.h"
#include "PCGSettings.h"
#include "VoxelStampRef.h"
#include "Heightmap/VoxelHeightmapStamp.h"
#include "Metadata/PCGObjectPropertyOverride.h"
#include "PCGVoxelStampSpawner.generated.h"

class UPCGManagedVoxelInstancedStampComponent;

UCLASS()
class VOXELPCG_API UPCGVoxelStampSpawnerSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	//~ Begin UPCGSettings Interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return "VoxelStampSpawner"; }
	virtual FText GetDefaultNodeTitle() const override { return INVTEXT("Voxel Stamp Spawner"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spawner; }
#endif

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;

	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditUndo() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	//~ End UPCGSettings Interface

public:
	FString GetNodeDebugInfo() const;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Config, DisplayName = Template)
	FVoxelStampRef NewTemplate = FVoxelStampRef::New(FVoxelHeightmapStamp());

	UPROPERTY()
	FVoxelInstancedStruct Template_DEPRECATED;

	UPROPERTY(BlueprintReadWrite, Category = Settings, meta = (PCG_Overridable))
	TSoftObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	TArray<FPCGObjectPropertyOverrideDescription> SpawnedStampPropertyOverrideDescriptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	TArray<FPCGObjectPropertyOverrideDescription> SpawnedGraphParameterOverrideDescriptions;

	// Specify a list of functions to be called on the target actor after instances are spawned. Functions need to be parameter-less and with "CallInEditor" flag enabled.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TArray<FName> PostProcessFunctionNames;

#if WITH_EDITORONLY_DATA
	// Specify unique ID, for graph change, so that cached CRC for settings would differ
	UPROPERTY(SkipSerialization)
	FGuid GraphChangeId;
#endif
};

struct VOXELPCG_API FPCGVoxelStampSpawnerContext : public FPCGContext
{
	bool bReuseCheckDone = false;
	bool bSkippedDueToReuse = false;
};

class VOXELPCG_API FPCGVoxelStampSpawnerElement : public IPCGElement
{
public:
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override;
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }

protected:
	virtual FPCGContext* CreateContext() override;
	virtual bool PrepareDataInternal(FPCGContext* InContext) const override;
	virtual bool ExecuteInternal(FPCGContext* InContext) const override;
#if VOXEL_ENGINE_VERSION >= 506
	virtual bool SupportsBasePointDataInputs(FPCGContext* InContext) const override { return true; }
#endif

private:
	static UPCGManagedVoxelInstancedStampComponent* GetOrCreateManagedComponent(
		AActor* InTargetActor,
		UPCGComponent* InSourceComponent,
		uint64 SettingsUID);
};