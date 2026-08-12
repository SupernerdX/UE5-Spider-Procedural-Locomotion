// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "PCGContext.h"
#include "PCGSettings.h"
#include "VoxelNodeEvaluator.h"
#include "StructUtils/PropertyBag.h"
#include "VoxelParameterOverridesOwner.h"
#include "Async/PCGAsyncLoadingContext.h"
#include "Buffer/VoxelSoftObjectPathBuffer.h"
#include "PCGCallVoxelGraph.generated.h"

class UPCGPointData;
class UVoxelPCGGraph;
class FVoxelLayers;
class FVoxelSurfaceTypeTable;
struct FVoxelOutputNode_OutputPoints;
enum class EPCGMetadataTypes : uint8;

USTRUCT(BlueprintType)
struct FVoxelPCGObjectAttributeType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FPCGAttributePropertyInputSelector Attribute;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FVoxelPinType Type = FVoxelPinType::MakeStruct(FVoxelSoftObjectPath::StaticStruct());
};

UCLASS(BlueprintType, ClassGroup = (Voxel))
class VOXELPCG_API UPCGCallVoxelGraphSettings
	: public UPCGSettings
	, public IVoxelParameterOverridesObjectOwner
{
	GENERATED_BODY()

public:
	UPCGCallVoxelGraphSettings();

	//~ Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override
	{
		return "CallVoxelGraph";
	}
	virtual FText GetDefaultNodeTitle() const override
	{
		return INVTEXT("Call Voxel Graph");
	}
	virtual FText GetNodeTooltipText() const override
	{
		return INVTEXT("Calls a voxel graph to modify points.");
	}
	virtual EPCGSettingsType GetType() const override
	{
		return EPCGSettingsType::Blueprint;
	}
	virtual EPCGChangeType GetChangeTypeForProperty(const FName& InPropertyName) const override
	{
		return Super::GetChangeTypeForProperty(InPropertyName) | EPCGChangeType::Cosmetic;
	}
	virtual UObject* GetJumpTargetForDoubleClick() const override;
#endif

	virtual bool CanCullTaskIfUnwired() const override
	{
		return false;
	}

	virtual bool HasDynamicPins() const override { return true; }
	virtual bool HasFlippedTitleLines() const override { return true; }
	virtual FString GetAdditionalTitleInformation() const override;
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
#if WITH_EDITOR
	virtual TArray<FPCGSettingsOverridableParam> GatherOverridableParams() const override;
#endif
	virtual void FixingOverridableParamPropertyClass(FPCGSettingsOverridableParam& Param) const override;
	virtual FPCGElementPtr CreateElement() const override;
	//~ End UPCGSettings interface

	//~ Begin UObject Interface
	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditUndo() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

	//~ Begin IVoxelParameterOverridesOwner Interface
	virtual UVoxelGraph* GetGraph() const override;
	virtual bool ShouldForceEnableOverride(const FGuid& Guid) const override;
	virtual FVoxelParameterOverrides& GetParameterOverrides() override;
	virtual void FixupParameterOverrides() override;
	//~ End IVoxelParameterOverridesOwner Interface

#if WITH_EDITOR
	void FixupOnChangedDelegate();
#endif

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	TObjectPtr<UVoxelPCGGraph> Graph;

	// Assign types to object attributes
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", DisplayName = "Object Attributes Mapping")
	TArray<FVoxelPCGObjectAttributeType> ObjectAttributeToType;

	// By default, mapped object type attributes loading is asynchronous, can force it synchronous if needed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Debug")
	bool bSynchronousLoad = false;

	UPROPERTY()
	FVoxelParameterOverrides ParameterOverrides;

	UPROPERTY()
	FInstancedPropertyBag ParameterPinOverrides;

#if WITH_EDITOR
	FSharedVoidPtr OnTerminalGraphChangedPtr;
#endif
};

struct FPCGCallVoxelGraphContext : public FPCGContext, public IPCGAsyncLoadingContext
{
	TSharedPtr<FVoxelDependencyCollector> DependencyCollector;
	TVoxelNodeEvaluator<FVoxelOutputNode_OutputPoints> Evaluator;
	TSharedPtr<FVoxelLayers> Layers;
	TSharedPtr<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	FVoxelBox Bounds;

	TArray<const UPCGPointData*> InPointData;
	TArray<TVoxelMap<FName, TVoxelObjectPtr<UPCGPointData>>> OutPointData;

	TVoxelSet<FName> PinsToQuery;

	TArray<FVoxelFuture> FuturePoints;
	FSharedVoidRef SharedVoid = MakeSharedVoid();

	FInstancedPropertyBag ParameterPinOverrides;

	void InitializeUserParametersStruct();

	//~ Begin FPCGContext Interface
	virtual void* GetUnsafeExternalContainerForOverridableParam(const FPCGSettingsOverridableParam& InParam) override;
	virtual void AddExtraStructReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FPCGContext Interface
};

class FPCGCallVoxelGraphElement : public IPCGElement
{
public:
	//~ Begin IPCGElement Interface
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }

#if VOXEL_ENGINE_VERSION >= 506
	virtual FPCGContext* Initialize(const FPCGInitializeElementParams& InParams) override;
#else
	virtual FPCGContext* Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node) override;
#endif

	virtual bool IsCacheable(const UPCGSettings* InSettings) const override;
	virtual bool PrepareDataInternal(FPCGContext* InContext) const override;
	virtual bool ExecuteInternal(FPCGContext* InContext) const override;
	//~ End IPCGElement Interface

private:
	static FVoxelFuture Compute(
		const TVoxelNodeEvaluator<FVoxelOutputNode_OutputPoints>& Evaluator,
		const TSharedRef<FVoxelDependencyCollector>& DependencyCollector,
		const FVoxelBox& Bounds,
		const TVoxelSet<FName>& PinsToQuery,
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
		const TVoxelArray<FPCGPoint>& Points,
		const TVoxelMap<FName, EPCGMetadataTypes>& AttributeNameToType,
		const TVoxelMap<FName, TSharedPtr<FVoxelBuffer>>& AttributeNameToBuffer,
		const TVoxelMap<FName, TVoxelObjectPtr<UPCGPointData>>& PinToOutPointData);
};