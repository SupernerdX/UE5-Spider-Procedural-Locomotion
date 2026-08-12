// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "PCGContext.h"
#include "PCGSettings.h"
#include "VoxelStackLayer.h"
#include "VoxelStampBehavior.h"
#include "VoxelHeightBlendMode.h"
#include "VoxelVolumeBlendMode.h"
#include "Async/PCGAsyncLoadingContext.h"
#include "Spline/VoxelHeightSplineGraph.h"
#include "PCGCreateVoxelSpline.generated.h"

class UVoxelLayer;
class AVoxelHeightSplineStamp;
class UVoxelVolumeSplineGraph;

UCLASS(BlueprintType, ClassGroup = (Voxel))
class VOXELPCG_API UPCGCreateVoxelSplineSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UPCGCreateVoxelSplineSettings();

	//~ Begin UPCGSettings Interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("CreateVoxelSpline")); }
	virtual FText GetDefaultNodeTitle() const override { return INVTEXT("Create Voxel Spline"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

	virtual FString GetAdditionalTitleInformation() const override;
	virtual FPCGElementPtr CreateElement() const override;

	virtual void Serialize(FArchive& Ar) override;
	//~ End UPCGSettings Interface

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	bool bVolumeSpline = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable, EditCondition = "!bVolumeSpline", EditConditionHides))
	TObjectPtr<UVoxelHeightSplineGraph> HeightGraph;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PCG_Overridable, EditCondition = "!bVolumeSpline", EditConditionHides))
	EVoxelHeightBlendMode HeightBlendMode = EVoxelHeightBlendMode::Max;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable, EditCondition = "bVolumeSpline", EditConditionHides))
	TObjectPtr<UVoxelVolumeSplineGraph> VolumeGraph;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PCG_Overridable, EditCondition = "bVolumeSpline", EditConditionHides))
	EVoxelVolumeBlendMode VolumeBlendMode = EVoxelVolumeBlendMode::Additive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (Units = cm, ClampMin = 0))
	float Smoothness = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	int32 Priority = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TObjectPtr<UVoxelLayer> Layer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	EVoxelStampBehavior StampBehavior = EVoxelStampBehavior::AffectAll;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	EPCGAttachOptions AttachOptions = EPCGAttachOptions::InFolder;
};

struct FPCGCreateVoxelSplineContext : public FPCGContext, public IPCGAsyncLoadingContext {};

class FPCGCreateVoxelSplineElement : public IPCGElement
{
protected:
	virtual FPCGContext* CreateContext() override;

	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override;
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override;
	virtual bool PrepareDataInternal(FPCGContext* InContext) const override;
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};