// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "PCGContext.h"
#include "PCGSettings.h"
#include "PCGWaitForVoxelWorld.generated.h"

UCLASS()
class VOXELPCG_API UPCGWaitForVoxelWorldSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	//~ Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override
	{
		return "WaitForVoxelWorld";
	}
	virtual FText GetDefaultNodeTitle() const override
	{
		return INVTEXT("Wait For Voxel World");
	}
	virtual FText GetNodeTooltipText() const override
	{
		return INVTEXT("Waits until the voxel world is ready to render");
	}
	virtual EPCGSettingsType GetType() const override
	{
		return EPCGSettingsType::ControlFlow;
	}
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
#if VOXEL_ENGINE_VERSION >= 507
 	virtual FPCGDataTypeIdentifier GetCurrentPinTypesID(const UPCGPin* InPin) const override;
#else
 	virtual EPCGDataType GetCurrentPinTypes(const UPCGPin* InPin) const override;
#endif

	virtual FPCGElementPtr CreateElement() const override;

	virtual void Serialize(FArchive& Ar) override;
	//~ End UPCGSettings interface
};

 struct FPCGWaitForVoxelWorldContext : public FPCGContext
 {
 	FSharedVoidRef SharedVoid = MakeSharedVoid();
 };

 class FPCGWaitForVoxelWorldElement : public IPCGElement
 {
 public:
	//~ Begin IPCGElement Interface
	virtual FPCGContext* CreateContext() override;
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override;
 	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override;
 	virtual bool PrepareDataInternal(FPCGContext* Context) const override;
 	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	//~ End IPCGElement Interface
 };