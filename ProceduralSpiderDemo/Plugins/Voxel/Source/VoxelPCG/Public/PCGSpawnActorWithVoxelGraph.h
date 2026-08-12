// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "PCGSubgraph.h"
#include "Elements/PCGSpawnActor.h"
#include "PCGSpawnActorWithVoxelGraph.generated.h"

class UPCGPointData;

UCLASS()
class VOXELPCG_API UPCGSpawnActorWithVoxelGraphSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	/**
	 * Can specify a list of functions from the template class to be called on each actor spawned, in order. Need to have "CallInEditor" flag enabled
	 * and have either no parameters or exactly the parameters PCGPoint and PCGMetadata
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TArray<FName> PostSpawnFunctionNames;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bForceDisableActorParsing = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	EPCGSpawnActorGenerationTrigger GenerationTrigger = EPCGSpawnActorGenerationTrigger::Default;

	/** Warning: inheriting parent actor tags work only in non-collapsed actor hierarchies */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bInheritActorTags = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TArray<FName> TagsToAddOnActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = Settings, meta = (ShowInnerProperties, EditCondition = "bAllowTemplateActorEditing", EditConditionHides))
	TObjectPtr<AActor> TemplateActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	TArray<FPCGObjectPropertyOverrideDescription> SpawnedActorPropertyOverrideDescriptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	TArray<FPCGObjectPropertyOverrideDescription> SpawnedGraphParameterOverrideDescriptions;

	UPROPERTY(meta = (PCG_Overridable))
	TSoftObjectPtr<AActor> RootActor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	EPCGAttachOptions AttachOptions = EPCGAttachOptions::InFolder;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (InlineEditConditionToggle))
	bool bSpawnByAttribute = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (EditCondition = "bSpawnByAttribute"))
	FName SpawnAttribute = NAME_None;

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Settings, meta = (OnlyPlaceable, DisallowCreateNew, MustImplement = "/Script/VoxelGraph.VoxelParameterOverridesObjectOwner"))
	TSubclassOf<AActor> TemplateActorClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bAllowTemplateActorEditing = false;

public:
	virtual void PostLoad() override;
	virtual void BeginDestroy() override;
	virtual void Serialize(FArchive& Ar) override;

#if WITH_EDITOR
	void OnBlueprintChanged(UBlueprint* Blueprint);
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreEditUndo() override;
	virtual void PostEditUndo() override;
#endif // WITH_EDITOR

	//~Begin UPCGSettings interface

	void SetTemplateActorClass(const TSubclassOf<AActor>& InTemplateActorClass);
	void SetAllowTemplateActorEditing(bool bInAllowTemplateActorEditing);
	const TSubclassOf<AActor>& GetTemplateActorClass() const { return TemplateActorClass; }
	bool GetAllowTemplateActorEditing() const { return bAllowTemplateActorEditing; }

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("SpawnActorWithVoxelGraph")); }
	virtual FText GetDefaultNodeTitle() const override { return INVTEXT("Spawn Actor with Voxel Graph"); }
	virtual EPCGSettingsType GetType() const override;
#endif

protected:
#if WITH_EDITOR
	virtual EPCGChangeType GetChangeTypeForProperty(const FName& InPropertyName) const override;
#endif
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return DefaultPointInputPinProperties(); }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return DefaultPointOutputPinProperties(); }
	virtual FPCGElementPtr CreateElement() const override;
	//~End UPCGSettings interface

public:
	//~Begin UPCGBaseSubgraphSettings interface
	// When using spawn by attribute, the potential execution of subgraphs will be done in a dynamic manner
	virtual bool IsDynamicGraph() const { return bSpawnByAttribute; }

	static UPCGGraphInterface* GetGraphInterfaceFromActorSubclass(TSubclassOf<AActor> InTemplateActorClass);

protected:
#if WITH_EDITOR
	//~End UPCGBaseSubgraphSettings interface

	void SetupBlueprintEvent();
	void TeardownBlueprintEvent();

private:
	void RefreshTemplateActor();
#endif

	friend class FPCGSpawnActorWithVoxelGraphElement;
};

class FPCGSpawnActorWithVoxelGraphElement : public IPCGElement
{
public:
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }
	virtual bool IsPassthrough(const UPCGSettings* InSettings) const override { return !InSettings; }

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

private:
	bool SpawnAndPrepareSubgraphs(FPCGContext* Context, const UPCGSpawnActorWithVoxelGraphSettings* Settings) const;

	TArray<FName> GetNewActorTags(FPCGContext* Context, AActor* TargetActor, bool bInheritActorTags, const TArray<FName>& AdditionalTags) const;

	void SpawnActors(FPCGContext* Context, AActor* TargetActor, TSubclassOf<AActor> TemplateActorClass, AActor* TemplateActor, FPCGTaggedData& Output, const UPCGPointData* PointData, UPCGPointData* OutPointData) const;
};