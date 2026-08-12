// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "PCGSettings.h"
#include "PCGContext.h"
#include "Async/PCGAsyncLoadingContext.h"
#include "Metadata/PCGObjectPropertyOverride.h"
#include "PCGApplyOnVoxelGraphSettings.generated.h"

UCLASS()
class VOXELPCG_API UPCGApplyOnVoxelGraphSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	//~ Begin UPCGSettings Interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("ApplyOnVoxelGraph")); }
	virtual FText GetDefaultNodeTitle() const override { return INVTEXT("Apply on Voxel Graph"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Generic; }
	virtual bool HasDynamicPins() const override { return true; }
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

	virtual FPCGElementPtr CreateElement() const override;

	virtual void Serialize(FArchive& Ar) override;
	//~ End UPCGSettings Interface

public:
	/** If something is connected in the In pin, will look for this attribute values to load, representing the object reference. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, PCG_DiscardPropertySelection, PCG_DiscardExtraSelection))
	FPCGAttributePropertyInputSelector ObjectReferenceAttribute;

	/** Override the default property values on the target actor. Applied before post-process functions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	TArray<FPCGObjectPropertyOverrideDescription> PropertyOverrideDescriptions;

	/** Opt-in option to silence errors when the path is Empty or nothing to extract. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Advanced")
	bool bSilenceErrorOnEmptyObjectPath = false;

	/** By default, object loading is asynchronous, can force it synchronous if needed. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Debug")
	bool bSynchronousLoad = false;
};

struct FPCGApplyOnVoxelGraphContext : public FPCGContext, public IPCGAsyncLoadingContext
{
	bool InitializeAndRequestLoad(
		FName InputPinName,
		const FPCGAttributePropertyInputSelector& InputAttributeSelector,
		const TArray<FSoftObjectPath>& StaticObjectPaths,
		bool bPersistAllData,
		bool bSilenceErrorOnEmptyObjectPath,
		bool bSynchronousLoad
	);

	TArray<TTuple<FSoftObjectPath, int32, int32>> PathsToObjectsAndDataIndex;
};

class FPCGApplyOnVoxelGraphElement : public IPCGElement, public IPCGAsyncLoadingContext
{
protected:
	virtual FPCGContext* CreateContext() override;

	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }
	virtual bool PrepareDataInternal(FPCGContext* InContext) const override;
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};