// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGSpawnActorWithVoxelGraph.h"
#include "PCGVoxelGraphParameterOverrides.h"
#include "VoxelPCGHelpers.h"
#include "VoxelParameterOverridesOwner.h"

#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGManagedResource.h"
#include "PCGSubsystem.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "Grid/PCGPartitionActor.h"
#include "Helpers/PCGActorHelpers.h"
#include "Helpers/PCGHelpers.h"
#include "Helpers/PCGPointDataPartition.h"
#include "Metadata/Accessors/IPCGAttributeAccessor.h"
#include "Metadata/Accessors/PCGAttributeAccessorKeys.h"
#include "Metadata/Accessors/PCGAttributeAccessorHelpers.h"

#include "Engine/Engine.h"
#include "UObject/Package.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"

bool AllowActorReuse()
{
	static IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(TEXT("pcg.Actor.AllowReuse"));
	if (ConsoleVariable)
	{
		return true;
	}

	return ConsoleVariable->GetBool();
}

class FPCGSpawnActorWithVoxelGraphPartitionByAttribute : public FPCGDataPartitionBase<FPCGSpawnActorWithVoxelGraphPartitionByAttribute, TSubclassOf<AActor>>
{
public:
	FPCGSpawnActorWithVoxelGraphPartitionByAttribute(FName InSpawnAttribute)
		: FPCGDataPartitionBase<FPCGSpawnActorWithVoxelGraphPartitionByAttribute, TSubclassOf<AActor>>()
		, SpawnAttribute(InSpawnAttribute)
	{
	}

	bool InitializeForData(const UPCGData* InData, UPCGData* OutData)
	{
		if (!InData || !InData->IsA<UPCGPointData>())
		{
			return false;
		}

		FPCGAttributePropertyInputSelector InputSource;
		InputSource.SetAttributeName(SpawnAttribute);
		InputSource = InputSource.CopyAndFixLast(InData);
		SpawnAttributeAccessor = PCGAttributeAccessorHelpers::CreateConstAccessor(InData, InputSource);
		SpawnAttributeKeys = PCGAttributeAccessorHelpers::CreateConstKeys(InData, InputSource);

		return SpawnAttributeAccessor.IsValid() && SpawnAttributeKeys.IsValid();
	}

	void AddToPartitionData(FPCGDataPartitionBase::Element* SelectedElement, const UPCGData* ParentData, int32 Index)
	{
		// Already checked in initialize that this is a valid cast
		const UPCGPointData* ParentPointData = static_cast<const UPCGPointData*>(ParentData);

		check(SelectedElement);
		if (!SelectedElement->PartitionData)
		{
			UPCGPointData* PartitionData = NewObject<UPCGPointData>();
			PartitionData->InitializeFromData(ParentPointData);
			SelectedElement->PartitionData = PartitionData;
		}

		check(SelectedElement->PartitionData);
		static_cast<UPCGPointData*>(SelectedElement->PartitionData)->GetMutablePoints().Add(ParentPointData->GetPoints()[Index]);
	}

	FPCGDataPartitionBase::Element* Select(int32 Index)
	{
		FSoftClassPath ActorPath;
		TSoftClassPtr<AActor> ActorClassSoftPtr;

		if (SpawnAttributeAccessor->Get<FSoftClassPath>(ActorPath, Index, *SpawnAttributeKeys))
		{
			ActorClassSoftPtr = TSoftClassPtr<AActor>(ActorPath);
		}
		else
		{
			FString ActorPathString;
			if (SpawnAttributeAccessor->Get<FString>(ActorPathString, Index, *SpawnAttributeKeys))
			{
				ActorPath = FSoftClassPath(ActorPathString);
				ActorClassSoftPtr = TSoftClassPtr<AActor>(ActorPath);
			}
		}

		if (!ActorPath.IsValid())
		{
			return nullptr;
		}

		UClass* ActorClass = ActorClassSoftPtr.LoadSynchronous();

		if (!ActorClass)
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(ActorPath.TryLoad());
			if (Blueprint)
			{
				ActorClass = Blueprint->GeneratedClass.Get();
			}
		}

		if (ActorClass && ActorClass->IsChildOf<AActor>())
		{
			return &ElementMap.FindOrAdd(ActorClass);
		}

		return nullptr;
	}

	// Disables time-slicing altogether because the code isn't setup for this yet
	int32 TimeSlicingCheckFrequency() const { return std::numeric_limits<int32>::max(); }

public:
	TUniquePtr<const IPCGAttributeAccessor> SpawnAttributeAccessor;
	TUniquePtr<const IPCGAttributeAccessorKeys> SpawnAttributeKeys;
	FName SpawnAttribute = NAME_None;
};

void UPCGSpawnActorWithVoxelGraphSettings::SetTemplateActorClass(const TSubclassOf<AActor>& InTemplateActorClass)
{
#if WITH_EDITOR
	TeardownBlueprintEvent();
#endif // WITH_EDITOR

	TemplateActorClass = InTemplateActorClass;

#if WITH_EDITOR
	SetupBlueprintEvent();
	RefreshTemplateActor();
#endif // WITH_EDITOR
}

void UPCGSpawnActorWithVoxelGraphSettings::SetAllowTemplateActorEditing(bool bInAllowTemplateActorEditing)
{
	bAllowTemplateActorEditing = bInAllowTemplateActorEditing;

#if WITH_EDITOR
	RefreshTemplateActor();
#endif // WITH_EDITOR
}

FPCGElementPtr UPCGSpawnActorWithVoxelGraphSettings::CreateElement() const
{
	return MakeShared<FPCGSpawnActorWithVoxelGraphElement>();
}

void UPCGSpawnActorWithVoxelGraphSettings::BeginDestroy()
{
#if WITH_EDITOR
	TeardownBlueprintEvent();
#endif // WITH_EDITOR

	Super::BeginDestroy();
}

void UPCGSpawnActorWithVoxelGraphSettings::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

UPCGGraphInterface* UPCGSpawnActorWithVoxelGraphSettings::GetGraphInterfaceFromActorSubclass(TSubclassOf<AActor> InTemplateActorClass)
{
	if (!InTemplateActorClass || InTemplateActorClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}

	UPCGGraphInterface* Result = nullptr;

	AActor::ForEachComponentOfActorClassDefault<UPCGComponent>(InTemplateActorClass, [&](const UPCGComponent* PCGComponent)
	{
		// If there is no graph, there is no graph instance
		if (PCGComponent->GetGraph() && PCGComponent->bActivated)
		{
			Result = PCGComponent->GetGraphInstance();
			return false;
		}

		return true;
	});

	return Result;
}

#if WITH_EDITOR
void UPCGSpawnActorWithVoxelGraphSettings::SetupBlueprintEvent()
{
	if (UBlueprintGeneratedClass* BlueprintClass = Cast<UBlueprintGeneratedClass>(TemplateActorClass))
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintClass->ClassGeneratedBy))
		{
			Blueprint->OnChanged().AddUObject(this, &UPCGSpawnActorWithVoxelGraphSettings::OnBlueprintChanged);
		}
	}
}

void UPCGSpawnActorWithVoxelGraphSettings::TeardownBlueprintEvent()
{
	if (UBlueprintGeneratedClass* BlueprintClass = Cast<UBlueprintGeneratedClass>(TemplateActorClass))
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintClass->ClassGeneratedBy))
		{
			Blueprint->OnChanged().RemoveAll(this);
		}
	}
}
#endif

void UPCGSpawnActorWithVoxelGraphSettings::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	// Since the template actor editing is set to false by default, this needs to be corrected on post-load for proper deprecation
	if (TemplateActor)
	{
		bAllowTemplateActorEditing = true;
	}

	SetupBlueprintEvent();

	if (TemplateActorClass)
	{
		if (TemplateActor)
		{
			TemplateActor->ConditionalPostLoad();
		}
	}

	RefreshTemplateActor();
#endif // WITH_EDITOR
}

#if WITH_EDITOR
EPCGChangeType UPCGSpawnActorWithVoxelGraphSettings::GetChangeTypeForProperty(const FName& InPropertyName) const
{
	EPCGChangeType ChangeType = Super::GetChangeTypeForProperty(InPropertyName);

	if (InPropertyName == GET_MEMBER_NAME_CHECKED(UPCGSpawnActorWithVoxelGraphSettings, TemplateActorClass) ||
		InPropertyName == GET_MEMBER_NAME_CHECKED(UPCGSpawnActorWithVoxelGraphSettings, bSpawnByAttribute))
	{
		ChangeType |= EPCGChangeType::Structural;
	}

	return ChangeType;
}
#endif // WITH_EDITOR

#if WITH_EDITOR
void UPCGSpawnActorWithVoxelGraphSettings::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange && PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UPCGSpawnActorWithVoxelGraphSettings, TemplateActorClass))
	{
		TeardownBlueprintEvent();
	}
}

void UPCGSpawnActorWithVoxelGraphSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property)
	{
		const FName& PropertyName = PropertyChangedEvent.Property->GetFName();

		if (PropertyName == GET_MEMBER_NAME_CHECKED(UPCGSpawnActorWithVoxelGraphSettings, TemplateActorClass))
		{
			SetupBlueprintEvent();
			RefreshTemplateActor();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UPCGSpawnActorWithVoxelGraphSettings, bAllowTemplateActorEditing))
		{
			RefreshTemplateActor();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UPCGSpawnActorWithVoxelGraphSettings::PreEditUndo()
{
	TeardownBlueprintEvent();

	Super::PreEditUndo();
}

void UPCGSpawnActorWithVoxelGraphSettings::PostEditUndo()
{
	Super::PostEditUndo();

	SetupBlueprintEvent();
	RefreshTemplateActor();
}

void UPCGSpawnActorWithVoxelGraphSettings::OnBlueprintChanged(UBlueprint* InBlueprint)
{
	RefreshTemplateActor();
	DirtyCache();
	OnSettingsChangedDelegate.Broadcast(this, EPCGChangeType::Settings);
}

void UPCGSpawnActorWithVoxelGraphSettings::RefreshTemplateActor()
{
	// Implementation note: this is similar to the child actor component implementation
	if (TemplateActorClass && bAllowTemplateActorEditing)
	{
		const bool bCreateNewTemplateActor = (!TemplateActor || TemplateActor->GetClass() != TemplateActorClass);

		if (bCreateNewTemplateActor)
		{
			AActor* NewTemplateActor = NewObject<AActor>(GetTransientPackage(), TemplateActorClass, NAME_None, RF_ArchetypeObject | RF_Transactional | RF_Public);

			if (TemplateActor)
			{
				UEngine::FCopyPropertiesForUnrelatedObjectsParams Options;
				Options.bNotifyObjectReplacement = true;
				UEngine::CopyPropertiesForUnrelatedObjects(TemplateActor, NewTemplateActor, Options);

				TemplateActor->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_ForceNoResetLoaders);

				TMap<UObject*, UObject*> OldToNew;
				OldToNew.Emplace(TemplateActor, NewTemplateActor);
				GEngine->NotifyToolsOfObjectReplacement(OldToNew);

				TemplateActor->MarkAsGarbage();
			}

			TemplateActor = NewTemplateActor;

			// Record initial object state in case we're in a transaction context.
			TemplateActor->Modify();

			// Outer to this object
			TemplateActor->Rename(nullptr, this, REN_DoNotDirty | REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
		}
	}
	else
	{
		if (TemplateActor)
		{
			TemplateActor->MarkAsGarbage();
		}

		TemplateActor = nullptr;
	}
}

#endif // WITH_EDITOR

bool FPCGSpawnActorWithVoxelGraphElement::ExecuteInternal(FPCGContext* InContext) const
{
	const UPCGSpawnActorWithVoxelGraphSettings* Settings = InContext->GetInputSettings<UPCGSpawnActorWithVoxelGraphSettings>();
	check(Settings);

	return SpawnAndPrepareSubgraphs(InContext, Settings);
}

bool FPCGSpawnActorWithVoxelGraphElement::SpawnAndPrepareSubgraphs(FPCGContext* Context, const UPCGSpawnActorWithVoxelGraphSettings* Settings) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCSpawnActorElement::Execute);

	// Early out
	if (!Settings->bSpawnByAttribute)
	{
		if (!Settings->TemplateActorClass || Settings->TemplateActorClass->HasAnyClassFlags(CLASS_Abstract))
		{
			const FText ClassName = Settings->TemplateActorClass ? FText::FromString(Settings->TemplateActorClass->GetFName().ToString()) : FText::FromName(NAME_None);
			PCGE_LOG(Error, GraphAndLog, FText::Format(INVTEXT("Invalid template actor class '{0}'"), ClassName));
			return true;
		}

		if (!ensure(!Settings->TemplateActor || Settings->TemplateActor->IsA(Settings->TemplateActorClass)))
		{
			return true;
		}
	}

	// Check if we can reuse existing resources - note that this is done on a per-settings basis when collapsed,
	// Otherwise we'll check against merged crc
	bool bFullySkippedDueToReuse = false;

	if (AllowActorReuse())
	{
		// Compute CRC if it has not been computed (it likely isn't, but this is to futureproof this)
		if (!Context->DependenciesCrc.IsValid())
		{
#if VOXEL_ENGINE_VERSION < 506
			GetDependenciesCrc(Context->InputData, Settings, Context->SourceComponent.Get(), Context->DependenciesCrc);
#else
			GetDependenciesCrc(FPCGGetDependenciesCrcParams(&Context->InputData, Settings, Context->ExecutionSource.Get()), Context->DependenciesCrc);
#endif
		}
	}

	// Pass-through exclusions & settings
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

#if WITH_EDITOR
	const bool bGenerateOutputsWithActorReference = true;
#else
	const bool bGenerateOutputsWithActorReference = Context->Node && Context->Node->IsOutputPinConnected(PCGPinConstants::DefaultOutputLabel);
#endif

	UPCGComponent* Component = GetPCGComponent(*Context);
	if (!ensure(Component))
	{
		return true;
	}

	const bool bHasAuthority = !Component || (Component->GetOwner() && Component->GetOwner()->HasAuthority());

	TArray<FPCGTaggedData> Inputs = Context->InputData.UE_506_SWITCH(GetInputs, GetAllSpatialInputs)();
	for (const FPCGTaggedData& Input : Inputs)
	{
		const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);

		if (!SpatialData)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("Invalid input data"));
			continue;
		}

		AActor* TargetActor = Settings->RootActor.Get() ? Settings->RootActor.Get() : Context->GetTargetActor(nullptr);

		if (!TargetActor)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("Invalid target actor. Ensure TargetActor member is initialized when creating SpatialData."));
			continue;
		}

		// First, create target instance transforms
		const UPCGPointData* PointData = SpatialData->ToPointData(Context);

		if (!PointData)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("Unable to get point data from input"));
			continue;
		}

		const TArray<FPCGPoint>& Points = PointData->GetPoints();

		if (Points.IsEmpty())
		{
			PCGE_LOG(Verbose, LogOnly, INVTEXT("Skipped - no points"));
			continue;
		}

		auto SpawnOrCollapse = [this, bHasAuthority, &Context, &TargetActor](TSubclassOf<AActor> TemplateActorClass, AActor* TemplateActor, FPCGTaggedData& Output, const UPCGPointData* PointData, UPCGPointData* OutPointData)
		{
			const bool bSpawnedActorsRequireAuthority = (TemplateActor ? TemplateActor->GetIsReplicated() : CastChecked<AActor>(TemplateActorClass->GetDefaultObject())->GetIsReplicated());

			if (bHasAuthority || !bSpawnedActorsRequireAuthority)
			{
				SpawnActors(Context, TargetActor, TemplateActorClass, TemplateActor, Output, PointData, OutPointData);
			}
		};

		UPCGPointData* OutPointData = nullptr;
		if (bGenerateOutputsWithActorReference)
		{
			OutPointData = NewObject<UPCGPointData>();
			OutPointData->InitializeFromData(PointData);
		}

		FPCGTaggedData Output = Input;

		if (Settings->bSpawnByAttribute && (!bFullySkippedDueToReuse || bGenerateOutputsWithActorReference))
		{
			FPCGSpawnActorWithVoxelGraphPartitionByAttribute Selector(Settings->SpawnAttribute);
			int32 CurrentPointIndex = 0;

			// Selection is still needed if are fully skipped in order to write to the OutPointData.
			Selector.SelectMultiple(*Context, PointData, CurrentPointIndex, PointData->GetPoints().Num(), OutPointData);

			if (!bFullySkippedDueToReuse)
			{
				for (auto& Element : Selector.ElementMap)
				{
					FPCGTaggedData PartialInput = Input;
					PartialInput.Data = Element.Value.PartitionData;

					SpawnOrCollapse(Element.Key, nullptr, PartialInput, static_cast<UPCGPointData*>(Element.Value.PartitionData), OutPointData);
				}
			}
		}
		else if(!bFullySkippedDueToReuse)
		{
			// Spawn actors/populate ISM
			FPCGTaggedData InputCopy = Input;
			SpawnOrCollapse(Settings->TemplateActorClass, Settings->TemplateActor, InputCopy, PointData, OutPointData);
		}

		// Update the data in the output to the final data gathered
		if (OutPointData)
		{
			Output.Data = OutPointData;
		}

		// Finally, pass through the input, in all cases:
		// - if it's not merged, will be the input points directly
		// - if it's merged but there is no subgraph, will be the input points directly
		// - if it's merged and there is a subgraph, we'd need to pass the data for it to be given to the subgraph
		Outputs.Add(Output);
	}

	return true;
}

void FPCGSpawnActorWithVoxelGraphElement::SpawnActors(FPCGContext* Context, AActor* TargetActor, TSubclassOf<AActor> InTemplateActorClass, AActor* InTemplateActor, FPCGTaggedData& Output, const UPCGPointData* PointData, UPCGPointData* OutPointData) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGSpawnActorWithVoxelGraphElement::ExecuteInternal::SpawnActors);
	check(Context && TargetActor && PointData);

	const TArray<FPCGPoint>& Points = PointData->GetPoints();
	if (Points.IsEmpty())
	{
		return;
	}

	UPCGComponent* Component = GetPCGComponent(*Context);
	if (!ensure(Component))
	{
		return;
	}

	int32 OutPointOffset = 0;
	FPCGMetadataAttribute<FSoftObjectPath>* ActorReferenceAttribute = nullptr;

	if (OutPointData)
	{
		OutPointOffset = OutPointData->GetMutablePoints().Num();
		OutPointData->GetMutablePoints().Append(Points);
		ActorReferenceAttribute = OutPointData->MutableMetadata()->FindOrCreateAttribute<FSoftObjectPath>(PCGPointDataConstants::ActorReferenceAttribute, FSoftObjectPath(), /*bAllowsInterpolation=*/false, /*bOverrideParent=*/false, /*bOverwriteIfTypeMismatch=*/false);
	}

	const UPCGSpawnActorWithVoxelGraphSettings* Settings = Context->GetInputSettings<UPCGSpawnActorWithVoxelGraphSettings>();
	check(Settings);

	const bool bForceDisableActorParsing = (Settings->bForceDisableActorParsing);

	AActor* TemplateActor = nullptr;
	if (InTemplateActor)
	{
		if (Settings->SpawnedActorPropertyOverrideDescriptions.IsEmpty())
		{
			TemplateActor = InTemplateActor;
		}
		else
		{
			TemplateActor = DuplicateObject(InTemplateActor, GetTransientPackage());
		}
	}
	else
	{
		if (Settings->SpawnedActorPropertyOverrideDescriptions.IsEmpty())
		{
			TemplateActor = Cast<AActor>(InTemplateActorClass->GetDefaultObject());
		}
		else
		{
			TemplateActor = NewObject<AActor>(GetTransientPackage(), InTemplateActorClass, NAME_None, RF_ArchetypeObject);
		}
	}

	check(TemplateActor);

	FPCGVoxelObjectOverrides ActorOverrides(TemplateActor);
	ActorOverrides.Initialize(Settings->SpawnedActorPropertyOverrideDescriptions, TemplateActor->GetClass(), PointData, Context);

	FPCGVoxelGraphParameterOverrides GraphOverrides;
	GraphOverrides.Initialize(Settings->SpawnedGraphParameterOverrideDescriptions, PointData, Context);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Template = TemplateActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.OverrideLevel = TargetActor->GetLevel();

	if (PCGHelpers::IsRuntimeOrPIE() || (Component && Component->IsInPreviewMode()))
	{
		SpawnParams.ObjectFlags |= RF_Transient;
	}

	const bool bForceCallGenerate = (Settings->GenerationTrigger == EPCGSpawnActorGenerationTrigger::ForceGenerate);
#if WITH_EDITOR
	const bool bOnLoadCallGenerate = (Settings->GenerationTrigger == EPCGSpawnActorGenerationTrigger::Default);
#else
	const bool bOnLoadCallGenerate = (Settings->GenerationTrigger == EPCGSpawnActorGenerationTrigger::Default ||
		Settings->GenerationTrigger == EPCGSpawnActorGenerationTrigger::DoNotGenerateInEditor);
#endif
	UPCGSubsystem* Subsystem = (Component ? Component->GetSubsystem() : nullptr);

	// Try to reuse actors if they are preexisting
	TArray<UPCGManagedActors*> ReusedManagedActorsResources;
	FPCGCrc InputDependenciesCrc;
	if (AllowActorReuse())
	{
		FPCGDataCollection SingleInputCollection;
		SingleInputCollection.TaggedData.Add(Output);
		// TODO: review this, it might make more sense to do a full data crc here
		SingleInputCollection.ComputeCrcs(/*bFullDataCrc=*/false);

#if VOXEL_ENGINE_VERSION < 506
		GetDependenciesCrc(SingleInputCollection, Settings, Component, InputDependenciesCrc);
#else
		GetDependenciesCrc(FPCGGetDependenciesCrcParams(&SingleInputCollection, Settings, Context->ExecutionSource.Get()), InputDependenciesCrc);
#endif

		if (InputDependenciesCrc.IsValid())
		{
			Component->ForEachManagedResource([&ReusedManagedActorsResources, &InputDependenciesCrc](UPCGManagedResource* InResource)
			{
				if (UPCGManagedActors* Resource = Cast<UPCGManagedActors>(InResource))
				{
					if (Resource->GetCrc().IsValid() && Resource->GetCrc() == InputDependenciesCrc)
					{
						ReusedManagedActorsResources.Add(Resource);
					}
				}
			});
		}
	}

	TArray<AActor*> ProcessedActors;
	const bool bActorsHavePCGComponents = (UPCGSpawnActorWithVoxelGraphSettings::GetGraphInterfaceFromActorSubclass(InTemplateActorClass) != nullptr);

	if (!ReusedManagedActorsResources.IsEmpty())
	{
		// If the actors are fully independent, we might need to make sure to call Generate if the underlying graph has changed - e.g. if the actor is dirty
		for (UPCGManagedActors* ManagedActors : ReusedManagedActorsResources)
		{
			check(ManagedActors);

			if (!ManagedActors->IsMarkedUnused())
			{
				// TODO: Add Context back in with toggles. Revisit if the stack is added to the managed actors at creation
				PCGLog::LogWarningOnGraph(INVTEXT("Identical actor spawn occurred. It may be beneficial to re-check graph logic for identical spawn conditions (same actor at same location, etc) or repeated nodes."), nullptr);
			}

			ManagedActors->MarkAsReused();

			// There's no setup to be done, just generation if we're in the no-merge case, so keep track of these actors only in this case
			if (bActorsHavePCGComponents)
			{
				for (const TSoftObjectPtr<AActor>& ManagedActorPtr : ManagedActors->UE_506_SWITCH(GeneratedActors, GetConstGeneratedActors()))
				{
					if (AActor* ManagedActor = ManagedActorPtr.Get())
					{
						ProcessedActors.Add(ManagedActor);
					}
				}
			}
		}
	}
	else
	{
		TArray<FName> NewActorTags = GetNewActorTags(Context, TargetActor, Settings->bInheritActorTags, Settings->TagsToAddOnActors);

		// Create managed resource for actor tracking
		UPCGManagedActors* ManagedActors = NewObject<UPCGManagedActors>(Component);
		ManagedActors->SetCrc(InputDependenciesCrc);

		// If generated actors are not directly attached, place them in a subfolder for tidiness.
		FString GeneratedActorsFolderPath;
#if WITH_EDITOR
		PCGHelpers::GetGeneratedActorsFolderPath(TargetActor, GeneratedActorsFolderPath);
#endif

		const UFunction* FunctionPrototypeWithNoParams = UPCGFunctionPrototypes::GetPrototypeWithNoParams();
		const UFunction* FunctionPrototypeWithPointAndMetadata = UPCGFunctionPrototypes::GetPrototypeWithPointAndMetadata();

		const TArray<UFunction*> PostSpawnFunctions = PCGHelpers::FindUserFunctions(
			InTemplateActorClass,
			Settings->PostSpawnFunctionNames,
			{ FunctionPrototypeWithNoParams, FunctionPrototypeWithPointAndMetadata },
			Context);

		bool bAllActorOverridesSucceeded = true;
		bool bAllGraphOverridesSucceeded = true;

		for (int32 i = 0; i < Points.Num(); ++i)
		{
			const FPCGPoint& Point = Points[i];

			bAllActorOverridesSucceeded &= ActorOverrides.Apply(i);

			AActor* GeneratedActor = TargetActor->GetWorld()->SpawnActor(InTemplateActorClass, &Point.Transform, SpawnParams);

			if (!GeneratedActor)
			{
				PCGE_LOG(Error, GraphAndLog, FText::Format(INVTEXT("Failed to spawn actor on point with index {0}"), i));
				continue;
			}

			// HACK: until UE-62747 is fixed, we have to force set the scale after spawning the actor
			GeneratedActor->SetActorRelativeScale3D(Point.Transform.GetScale3D());
			GeneratedActor->Tags.Append(NewActorTags);
			PCGHelpers::AttachToParent(GeneratedActor, TargetActor, Settings->AttachOptions, Context, GeneratedActorsFolderPath);

			for (UFunction* PostSpawnFunction : PostSpawnFunctions)
			{
				if (PostSpawnFunction->IsSignatureCompatibleWith(FunctionPrototypeWithNoParams))
				{
					GeneratedActor->ProcessEvent(PostSpawnFunction, nullptr);
				}
				else if (PostSpawnFunction->IsSignatureCompatibleWith(FunctionPrototypeWithPointAndMetadata))
				{
					TPair<FPCGPoint, const UPCGMetadata*> PointAndMetadata = { Point, PointData->ConstMetadata() };
					GeneratedActor->ProcessEvent(PostSpawnFunction, &PointAndMetadata);
				}
			}

			bAllGraphOverridesSucceeded &= GraphOverrides.Apply(Cast<IVoxelParameterOverridesObjectOwner>(GeneratedActor), i, Context);

			ManagedActors->UE_506_SWITCH(GeneratedActors, GetMutableGeneratedActors()).Add(GeneratedActor);

			if (bActorsHavePCGComponents)
			{
				ProcessedActors.Add(GeneratedActor);
			}

			// Write to out data the actor reference
			if (OutPointData && ActorReferenceAttribute)
			{
				FPCGPoint& OutPoint = OutPointData->GetMutablePoints()[i + OutPointOffset];
				OutPointData->Metadata->InitializeOnSet(OutPoint.MetadataEntry);
				ActorReferenceAttribute->SetValue(OutPoint.MetadataEntry, FSoftObjectPath(GeneratedActor));
			}
		}

		if (!bAllActorOverridesSucceeded)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("At least one actor property override failed."));
		}

		if (!bAllGraphOverridesSucceeded)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("At least one graph parameter override failed."));
		}

		Component->AddToManagedResources(ManagedActors);

		PCGE_LOG(Verbose, LogOnly, FText::Format(INVTEXT("Generated {0} actors"), Points.Num()));
	}

	// Setup & Generate on PCG components if needed
	for (AActor* Actor : ProcessedActors)
	{
		TInlineComponentArray<UPCGComponent*, 1> PCGComponents;
		Actor->GetComponents(PCGComponents);

		for (UPCGComponent* PCGComponent : PCGComponents)
		{
#if WITH_EDITOR
			// For both pre-existing and new actors, we need to make sure we're inline with loading/generation as needed
			if (PCGComponent->GetEditingMode() != Component->GetEditingMode())
			{
				PCGComponent->SetEditingMode(/*CurrentEditingMode=*/Component->GetEditingMode(), /*SerializedEditingMode=*/Component->GetEditingMode());
				PCGComponent->ChangeTransientState(Component->GetEditingMode());
			}
#endif // WITH_EDITOR

			if (bForceDisableActorParsing)
			{
				PCGComponent->bParseActorComponents = false;
			}

			if (bForceCallGenerate || (bOnLoadCallGenerate && PCGComponent->GenerationTrigger == EPCGComponentGenerationTrigger::GenerateOnLoad))
			{
				if (Subsystem)
				{
#if !IS_MONOLITHIC && (PLATFORM_MAC || PLATFORM_LINUX)
					// Not supported due to linking errors
					ensure(false);
#else
					Subsystem->RegisterOrUpdatePCGComponent(PCGComponent);
#endif
				}
			}
		}
	}
}

TArray<FName> FPCGSpawnActorWithVoxelGraphElement::GetNewActorTags(FPCGContext* Context, AActor* TargetActor, bool bInheritActorTags, const TArray<FName>& AdditionalTags) const
{
	UPCGComponent* Component = GetPCGComponent(*Context);
	if (!ensure(Component))
	{
		return {};
	}

	TArray<FName> NewActorTags;
	// Prepare actor tags
	if (bInheritActorTags)
	{
		// Special case: if the current target actor is a partition, we'll reach out
		// and find the original actor tags
		if (APCGPartitionActor* PartitionActor = Cast<APCGPartitionActor>(TargetActor))
		{
			if (UPCGComponent* OriginalComponent = PartitionActor->GetOriginalComponent(Component))
			{
				check(OriginalComponent->GetOwner());
				NewActorTags = OriginalComponent->GetOwner()->Tags;
			}
		}
		else
		{
			NewActorTags = TargetActor->Tags;
		}
	}

	NewActorTags.AddUnique(PCGHelpers::DefaultPCGActorTag);

	for (const FName& AdditionalTag : AdditionalTags)
	{
		NewActorTags.AddUnique(AdditionalTag);
	}

	return NewActorTags;
}

#if WITH_EDITOR
EPCGSettingsType UPCGSpawnActorWithVoxelGraphSettings::GetType() const
{
	return EPCGSettingsType::Spawner;
}
#endif