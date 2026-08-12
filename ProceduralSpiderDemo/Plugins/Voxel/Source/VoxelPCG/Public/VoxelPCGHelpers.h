// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "PCGContext.h"
#include "PCGSettings.h"
#include "VoxelPCGHelpers.generated.h"

class FVoxelPCGOutput;
class FVoxelPCGElement;
class FVoxelInvalidationQueue;

UCLASS(ClassGroup = (Voxel))
class VOXELPCG_API UVoxelPCGSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	//~ Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const final override;
	virtual FText GetDefaultNodeTitle() const final override;
	virtual FText GetNodeTooltipText() const final override;
#endif
	virtual bool CanCullTaskIfUnwired() const override;
	virtual FPCGElementPtr CreateElement() const final override;

	virtual void Serialize(FArchive& Ar) override;
	virtual FString GetNodeDebugInfo() const;
	//~ End UPCGSettings interface

public:
	virtual TSharedPtr<FVoxelPCGOutput> CreateOutput(FPCGContext& Context) const VOXEL_PURE_VIRTUAL({});

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable))
	bool bTrackLayerChanges = true;
};

struct VOXELPCG_API FVoxelPCGContext final : public FPCGContext
{
public:
	FVoxelPCGContext() = default;
	virtual ~FVoxelPCGContext() override;

	TSharedPtr<FVoxelPCGOutput> GetOutput() const;

private:
	TSharedPtr<FVoxelPCGOutput> PrivateOutput;

	friend FVoxelPCGElement;
};

class VOXELPCG_API FVoxelPCGOutput : public TSharedFromThis<FVoxelPCGOutput>
{
public:
	FVoxelPCGOutput() = default;
	virtual ~FVoxelPCGOutput() = default;

public:
	FORCEINLINE int32 GetSeed() const
	{
		return PrivateSeed;
	}
	FORCEINLINE FVoxelDependencyCollector& GetDependencyCollector() const
	{
		return *PrivateDependencyCollector;
	}
	TSharedRef<FVoxelDependencyCollector> GetSharedDependencyCollector() const
	{
		return PrivateDependencyCollector.ToSharedRef();
	}
	FORCEINLINE FPCGContext* GetOwner() const
	{
		return PrivateOwner;
	}

public:
	virtual FVoxelFuture Run() const = 0;

private:
	FPCGContext* PrivateOwner = nullptr;
	TVoxelOptional<FVoxelFuture> PrivateFuture;
	TSharedPtr<FVoxelDependencyCollector> PrivateDependencyCollector;
	TSharedPtr<FVoxelInvalidationQueue> PrivateInvalidationQueue;
	int32 PrivateSeed = 0;
	TSharedPtr<FVoxelTaskContext> PrivateContext;

	friend FVoxelPCGContext;
	friend FVoxelPCGElement;
};

class VOXELPCG_API FVoxelPCGElement : public IPCGElement
{
public:
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override
	{
		return true;
	}
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override
	{
		return true;
	}

private:
	virtual FPCGContext* CreateContext() override;

#if VOXEL_ENGINE_VERSION < 506
	virtual void GetDependenciesCrc(
		const FPCGDataCollection& InInput,
		const UPCGSettings* InSettings,
		UPCGComponent* InComponent,
		FPCGCrc& OutCrc) const override;
#else
	virtual void GetDependenciesCrc(
		const FPCGGetDependenciesCrcParams& InParams,
		FPCGCrc& OutCrc) const override;
#endif

	virtual bool PrepareDataInternal(FPCGContext* Context) const override;
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

VOXELPCG_API UPCGComponent* GetPCGComponent(const FPCGContext& Context);