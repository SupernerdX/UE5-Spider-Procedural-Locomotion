// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelAssetIcon.h"
#include "VoxelParameter.h"
#include "VoxelParameterOverridesOwner.h"
#include "VoxelGraph.generated.h"

class UVoxelTerminalGraph;
class FVoxelCompiledGraph;
struct FVoxelGraphMetadata;

USTRUCT()
struct VOXELGRAPH_API FVoxelEditedDocumentInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UEdGraph> EdGraph;

	UPROPERTY()
	FVector2D ViewLocation = FVector2D::ZeroVector;

	UPROPERTY()
	float ZoomAmount = -1.f;
};

constexpr FVoxelGuid GVoxelMainTerminalGraphGuid = VOXEL_GUID("00000000FFFFFFFF0000000029A672A2");
constexpr FVoxelGuid GVoxelEditorTerminalGraphGuid = VOXEL_GUID("00000000FFFFFFFF000000007FEB8321");

UCLASS(meta = (VoxelAssetType, AssetColor=Blue))
class VOXELGRAPH_API UVoxelGraph
	: public UVoxelAsset
	, public IVoxelParameterOverridesObjectOwner
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Config")
	FString Category;

	UPROPERTY(AssetRegistrySearchable, EditAnywhere, Category = "Config", DisplayName = "Tags")
	TSet<FName> GraphTags;

	UPROPERTY(AssetRegistrySearchable, EditAnywhere, Category = "Config")
	FString Description;

	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	FString DisplayNameOverride;

	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	bool bShowInContextMenu = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	FVoxelAssetIcon AssetIcon;

	UPROPERTY(Transient)
	bool bEnableNodeRangeStats = false;

	UPROPERTY(Transient)
	bool bEnableNodeValueStats = false;

	UPROPERTY()
	TArray<FVoxelEditedDocumentInfo> LastEditedDocuments;
#endif

	UVoxelGraph();

public:
	void Fixup();
	bool IsFunctionLibrary() const;
	FString GetGraphTypeName() const;
#if WITH_EDITOR
	FVoxelGraphMetadata GetMetadata() const;
#endif

public:
	bool HasMainTerminalGraph() const;

	UVoxelTerminalGraph& GetMainTerminalGraph();
	const UVoxelTerminalGraph& GetMainTerminalGraph() const;

	UVoxelTerminalGraph* GetMainTerminalGraph_CheckBaseGraphs();
	const UVoxelTerminalGraph* GetMainTerminalGraph_CheckBaseGraphs() const;

	UVoxelTerminalGraph& GetEditorTerminalGraph();
	const UVoxelTerminalGraph& GetEditorTerminalGraph() const;

public:
	TSharedRef<const FVoxelCompiledGraph> GetCompiledGraph(FVoxelDependencyCollector& DependencyCollector) const;

	static void LoadAllGraphs();

public:
	UVoxelTerminalGraph* FindTerminalGraph_NoInheritance(const FGuid& Guid);
	const UVoxelTerminalGraph* FindTerminalGraph_NoInheritance(const FGuid& Guid) const;

	void ForeachTerminalGraph_NoInheritance(TFunctionRef<void(UVoxelTerminalGraph&)> Lambda);
	void ForeachTerminalGraph_NoInheritance(TFunctionRef<void(const UVoxelTerminalGraph&)> Lambda) const;

	FGuid FindTerminalGraphGuid_NoInheritance(const UVoxelTerminalGraph* TerminalGraph) const;

public:
	UVoxelTerminalGraph* FindTerminalGraph(const FGuid& Guid);
	const UVoxelTerminalGraph* FindTerminalGraph(const FGuid& Guid) const;

	UVoxelTerminalGraph& FindTerminalGraphChecked(const FGuid& Guid);
	const UVoxelTerminalGraph& FindTerminalGraphChecked(const FGuid& Guid) const;

	TVoxelSet<FGuid> GetTerminalGraphs() const;
	const UVoxelTerminalGraph* FindTopmostTerminalGraph(const FGuid& Guid) const;

#if WITH_EDITOR
	UVoxelTerminalGraph& AddTerminalGraph(
		const FGuid& Guid,
		const UVoxelTerminalGraph* Template = nullptr);
	void RemoveTerminalGraph(const FGuid& Guid);
	void ReorderTerminalGraphs(TConstVoxelArrayView<FGuid> NewGuids);
#endif

public:
	const FVoxelParameter* FindParameter(const FGuid& Guid) const;
	const FVoxelParameter& FindParameterChecked(const FGuid& Guid) const;

	int32 NumParameters() const;
	TVoxelSet<FGuid> GetParameters() const;
	bool IsInheritedParameter(const FGuid& Guid) const;
	void ForeachParameter(TFunctionRef<void(const FGuid&, const FVoxelParameter&)> Lambda) const;

#if WITH_EDITOR
	void AddParameter(const FGuid& Guid, const FVoxelParameter& Parameter);
	void RemoveParameter(const FGuid& Guid);
	void UpdateParameter(const FGuid& Guid, TFunctionRef<void(FVoxelParameter&)> Update);
	void ReorderParameters(TConstVoxelArrayView<FGuid> NewGuids);
#endif

public:
	// Won't check for loops, prefer GetBaseGraphs instead
	UVoxelGraph* GetBaseGraph_Unsafe() const;

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetBaseGraph(UVoxelGraph* NewBaseGraph);
#endif

	// Includes self
	TVoxelInlineArray<UVoxelGraph*, 1> GetBaseGraphs();
	TVoxelInlineArray<const UVoxelGraph*, 1> GetBaseGraphs() const;

	// Includes self, slow
	TVoxelSet<UVoxelGraph*> GetChildGraphs_LoadedOnly();
	TVoxelSet<const UVoxelGraph*> GetChildGraphs_LoadedOnly() const;

public:
	//~ Begin UObject Interface
	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditUndo() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostRename(UObject* OldOuter, FName OldName) override;
#endif
	//~ End UObject Interface

	//~ Begin IVoxelParameterOverridesOwner Interface
	virtual bool ShouldForceEnableOverride(const FGuid& Guid) const override;
	virtual UVoxelGraph* GetGraph() const override { return ConstCast(this); }
	virtual FVoxelParameterOverrides& GetParameterOverrides() override;
	virtual FProperty* GetParameterOverridesProperty() const override;
	//~ End IVoxelParameterOverridesOwner Interface

#if WITH_EDITOR
	struct VOXELGRAPH_API FFactoryInfo
	{
		FString DisplayNameOverride;
		FString Description;
		FName Category;
		const FSlateBrush* Icon = nullptr;
		UVoxelGraph* Template = nullptr;

		FString GetDisplayName(const UClass* Class) const;
	};
	virtual FFactoryInfo GetFactoryInfo() VOXEL_PURE_VIRTUAL({});
#endif

	virtual UScriptStruct* GetOutputNodeStruct() const
	{
		return nullptr;
	}

private:
	UPROPERTY()
	TObjectPtr<UVoxelGraph> PrivateBaseGraph;

	UPROPERTY()
	FVoxelParameterOverrides ParameterOverrides;

	UPROPERTY()
	TMap<FGuid, TObjectPtr<UVoxelTerminalGraph>> GuidToTerminalGraph;

	UPROPERTY()
	TMap<FGuid, FVoxelParameter> GuidToParameter;

	mutable TSharedPtr<const FVoxelCompiledGraph> CachedCompiledGraph;
	mutable FSharedVoidPtr OnCompiledGraphChangedPtr;

	const TSharedRef<FVoxelDependency> CompiledGraphDependency = SharedRef_Null;

	friend class FVoxelGraphEnvironment;
};