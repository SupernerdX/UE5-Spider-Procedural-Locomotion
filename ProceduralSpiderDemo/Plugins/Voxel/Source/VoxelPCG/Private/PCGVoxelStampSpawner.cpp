// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGVoxelStampSpawner.h"
#include "Graphs/VoxelHeightGraphStamp.h"
#include "Graphs/VoxelVolumeGraphStamp.h"
#include "VoxelPCGHelpers.h"
#include "VoxelPCGTracker.h"
#include "VoxelStampManager.h"
#include "VoxelInvalidationCallstack.h"
#include "VoxelInstancedStampComponent.h"
#include "PCGVoxelGraphParameterOverrides.h"
#include "PCGManagedVoxelInstancedStampComponent.h"

#include "PCGEdge.h"
#include "PCGComponent.h"
#include "Data/PCGPointData.h"
#include "Helpers/PCGHelpers.h"

#if VOXEL_ENGINE_VERSION >= 506
#include "Data/PCGPointArrayData.h"
#endif

TArray<FPCGPinProperties> UPCGVoxelStampSpawnerSettings::InputPinProperties() const
{
	return DefaultPointInputPinProperties();
}

TArray<FPCGPinProperties> UPCGVoxelStampSpawnerSettings::OutputPinProperties() const
{
	return DefaultPointOutputPinProperties();
}

FPCGElementPtr UPCGVoxelStampSpawnerSettings::CreateElement() const
{
	return MakeShared<FPCGVoxelStampSpawnerElement>();
}

void UPCGVoxelStampSpawnerSettings::PostLoad()
{
	Super::PostLoad();

	if (Template_DEPRECATED.IsValid())
	{
		NewTemplate = FVoxelStampRef::New(Template_DEPRECATED.Get<FVoxelStamp>());
	}
	Template_DEPRECATED.Reset();

	if (NewTemplate)
	{
		NewTemplate->FixupProperties();
	}
}

void UPCGVoxelStampSpawnerSettings::PostInitProperties()
{
	Super::PostInitProperties();

	if (NewTemplate)
	{
		NewTemplate->FixupProperties();
	}
}

void UPCGVoxelStampSpawnerSettings::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
void UPCGVoxelStampSpawnerSettings::PostEditUndo()
{
	Super::PostEditUndo();

	if (NewTemplate)
	{
		NewTemplate->FixupProperties();
	}
}

void UPCGVoxelStampSpawnerSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	NewTemplate->FixupProperties();

	for (const FProperty* Property : PropertyChangedEvent.PropertyChain)
	{
		if (Property == &FindFPropertyChecked(UPCGVoxelStampSpawnerSettings, NewTemplate))
		{
			GraphChangeId = FGuid::NewGuid();
			OnSettingsChangedDelegate.Broadcast(this, EPCGChangeType::Settings);

			// Recalculate CRC
			FPropertyChangedEvent Event(nullptr);
			Super::PostEditChangeProperty(Event);
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FString UPCGVoxelStampSpawnerSettings::GetNodeDebugInfo() const
{
	FString Name = GetName();
#if WITH_EDITOR
	Name = GetDefaultNodeTitle().ToString();
#endif

	const UVoxelLayer* Layer = nullptr;
	if (const TSharedPtr<const FVoxelHeightStamp> HeightStamp = NewTemplate.ToSharedPtr<FVoxelHeightStamp>())
	{
		Layer = HeightStamp->Layer;
	}
	if (const TSharedPtr<const FVoxelVolumeStamp> VolumeStamp = NewTemplate.ToSharedPtr<FVoxelVolumeStamp>())
	{
		Layer = VolumeStamp->Layer;
	}

	Name += " [Layer: " + (Layer ? Layer->GetName() : "None") + "]";
	return Name;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FPCGVoxelStampSpawnerElement::CanExecuteOnlyOnMainThread(FPCGContext* Context) const
{
	return
		!Context ||
		Context->CurrentPhase == EPCGExecutionPhase::Execute ||
		Context->CurrentPhase == EPCGExecutionPhase::PrepareData;
}

FPCGContext* FPCGVoxelStampSpawnerElement::CreateContext()
{
	return new FPCGVoxelStampSpawnerContext();
}

bool FPCGVoxelStampSpawnerElement::PrepareDataInternal(FPCGContext* InContext) const
{
	FPCGVoxelStampSpawnerContext* Context = static_cast<FPCGVoxelStampSpawnerContext*>(InContext);
	const UPCGVoxelStampSpawnerSettings* Settings = Context->GetInputSettings<UPCGVoxelStampSpawnerSettings>();
	check(Settings);

	UPCGComponent* Component = GetPCGComponent(*Context);
	if (!ensure(Component))
	{
		return true;
	}

#if WITH_EDITOR
	// In editor, we always want to generate this data for inspection & to prevent caching issues
	const bool bGenerateOutput = true;
#else
	const bool bGenerateOutput = Context->Node && Context->Node->IsOutputPinConnected(PCGPinConstants::DefaultOutputLabel);
#endif

	if (!Context->bReuseCheckDone)
	{
		// Compute CRC if it has not been computed (it likely isn't, but this is to futureproof this)
		if (!Context->DependenciesCrc.IsValid())
		{
#if VOXEL_ENGINE_VERSION < 506
			GetDependenciesCrc(Context->InputData, Settings, Component, Context->DependenciesCrc);
#else
			GetDependenciesCrc(FPCGGetDependenciesCrcParams(&Context->InputData, Settings, Context->ExecutionSource.Get()), Context->DependenciesCrc);
#endif
		}

		if (Context->DependenciesCrc.IsValid())
		{
			TArray<UPCGManagedVoxelInstancedStampComponent*> ManagedVoxelInstancedStampComponents;
			Component->ForEachManagedResource([&ManagedVoxelInstancedStampComponents, &Context, Settings](UPCGManagedResource* InResource)
			{
				if (UPCGManagedVoxelInstancedStampComponent* Resource = Cast<UPCGManagedVoxelInstancedStampComponent>(InResource))
				{
					if (Resource->GetSettingsUID() == Settings->GetStableUID() &&
						Resource->GetCrc().IsValid() &&
						Resource->GetCrc() == Context->DependenciesCrc)
					{
						ManagedVoxelInstancedStampComponents.Add(Resource);
					}
				}
			});

			for (UPCGManagedVoxelInstancedStampComponent* InstancedStampComponent : ManagedVoxelInstancedStampComponents)
			{
				InstancedStampComponent->MarkAsReused();
			}

			if (!ManagedVoxelInstancedStampComponents.IsEmpty())
			{
				Context->bSkippedDueToReuse = true;
			}
		}

		Context->bReuseCheckDone = true;
	}

	if (!bGenerateOutput &&
		Context->bSkippedDueToReuse)
	{
		return true;
	}

	if (bGenerateOutput)
	{
		TArray<FPCGTaggedData> Inputs = Context->InputData.UE_506_SWITCH(GetInputs, GetAllSpatialInputs)();
		TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

		for (const FPCGTaggedData& Input : Inputs)
		{
			const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);

			if (!SpatialData)
			{
				PCGE_LOG(Error, GraphAndLog, INVTEXT("Invalid input data"));
				continue;
			}

#if VOXEL_ENGINE_VERSION >= 506
			const UPCGPointArrayData* PointData = SpatialData->ToPointArrayData(Context);
			if (!PointData)
			{
				PCGE_LOG(Error, GraphAndLog, INVTEXT("Unable to get point data from input"));
				continue;
			}
#else
			const UPCGPointData* PointData = SpatialData->ToPointData(Context);
			if (!PointData)
			{
				PCGE_LOG(Error, GraphAndLog, INVTEXT("Unable to get point data from input"));
				continue;
			}
#endif

			const AActor* TargetActor = Settings->TargetActor.Get() ? Settings->TargetActor.Get() : Context->GetTargetActor(nullptr);
			if (!TargetActor)
			{
				PCGE_LOG(Error, GraphAndLog, INVTEXT("Invalid target actor. Ensure TargetActor member is initialized when creating SpatialData."));
				continue;
			}

			FPCGTaggedData& Output = Outputs.Add_GetRef(Input);

			Output.Data = PointData;
		}
	}

	return true;
}

bool FPCGVoxelStampSpawnerElement::ExecuteInternal(FPCGContext* InContext) const
{
	VOXEL_FUNCTION_COUNTER();

	FPCGVoxelStampSpawnerContext* Context = static_cast<FPCGVoxelStampSpawnerContext*>(InContext);
	const UPCGVoxelStampSpawnerSettings* Settings = Context->GetInputSettings<UPCGVoxelStampSpawnerSettings>();
	check(Settings);

	if (Context->bSkippedDueToReuse ||
		!Settings->NewTemplate)
	{
		return true;
	}

	UPCGComponent* Component = GetPCGComponent(*Context);
	if (!ensure(Component))
	{
		return true;
	}

	AActor* TargetActor = Settings->TargetActor.Get() ? Settings->TargetActor.Get() : Context->GetTargetActor(nullptr);

	if (!TargetActor)
	{
		PCGE_LOG(Error, GraphAndLog, INVTEXT("Invalid target actor. Ensure TargetActor member is initialized when creating SpatialData."));
		return true;
	}

	UPCGManagedVoxelInstancedStampComponent* Resource = GetOrCreateManagedComponent(
		TargetActor,
		Component,
		Settings->GetStableUID());

	check(Resource);
	Resource->SetCrc(Context->DependenciesCrc);

	UVoxelInstancedStampComponent* InstancedStampComponent = Resource->GetComponent();
	check(InstancedStampComponent);

	if (const USceneComponent* SceneComponent = TargetActor->GetRootComponent())
	{
		InstancedStampComponent->Mobility = SceneComponent->Mobility;
	}

	TVoxelSet<UVoxelPCGSettings*> PreviousSettings;
	{
		VOXEL_SCOPE_COUNTER("PreviousSettings");

		TVoxelSet<const UPCGNode*> PreviousNodes;
		PreviousNodes.Reserve(64);
		PreviousSettings.Reserve(16);

		const TFunction<void(const UPCGNode&)> Traverse = [&](const UPCGNode& Node)
		{
			if (PreviousNodes.Contains(&Node))
			{
				return;
			}
			PreviousNodes.Add_CheckNew(&Node);

			if (UVoxelPCGSettings* OtherSettings = Cast<UVoxelPCGSettings>(Node.GetSettings()))
			{
				PreviousSettings.Add(OtherSettings);
			}

			for (const UPCGPin* InputPin : Node.GetInputPins())
			{
				for (const UPCGEdge* Edge : InputPin->Edges)
				{
					const UPCGPin* Pin = Edge->InputPin;
					if (!Pin)
					{
						continue;
					}

					const UPCGNode* OtherNode = Pin->Node;
					if (!OtherNode)
					{
						continue;
					}

					Traverse(*OtherNode);
				}
			}
		};

		if (Context->Node)
		{
			Traverse(*Context->Node);
		}
	}

	const TSharedRef<FVoxelInvalidationCallstack> Callstack = FVoxelInvalidationFrame_PCG::Create(
		Component,
		*Settings,
		FName(Settings->GetNodeDebugInfo()));

	for (const UVoxelPCGSettings* OtherSettings : PreviousSettings)
	{
		Callstack->AddCaller(FVoxelInvalidationFrame_PCG::Create(
			Component,
			*OtherSettings,
			FName(OtherSettings->GetNodeDebugInfo())));
	}

	FVoxelInvalidationScope Scope(Callstack);

	const TArray<FPCGTaggedData> Inputs = Context->InputData.UE_506_SWITCH(GetInputs, GetAllSpatialInputs)();
	for (const FPCGTaggedData& Input : Inputs)
	{
		const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);
		if (!SpatialData)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("Invalid input data"));
			continue;
		}

#if VOXEL_ENGINE_VERSION >= 506
		const UPCGBasePointData* PointData = SpatialData->ToBasePointData(Context);
		if (!PointData)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("Unable to get point data from input"));
			continue;
		}

		const int32 NumPoints = PointData->GetNumPoints();

		const auto GetPointTransform = [&](const int32 Index) -> const FTransform&
		{
			return PointData->GetTransform(Index);
		};
#else
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

		const int32 NumPoints = PointData->GetNumPoints();

		const auto GetPointTransform = [&](const int32 Index) -> const FTransform&
		{
			return Points[Index].Transform;
		};
#endif

		FVoxelStampRef NewTemplate = Settings->NewTemplate.MakeCopy();

		FPCGVoxelObjectOverrides ActorOverrides(NewTemplate->As<FVoxelStamp>());
		ActorOverrides.Initialize(Settings->SpawnedStampPropertyOverrideDescriptions, NewTemplate->GetStruct(), PointData, Context);

		FPCGVoxelGraphParameterOverrides GraphOverrides;
		GraphOverrides.Initialize(Settings->SpawnedGraphParameterOverrideDescriptions, PointData, Context);

		TVoxelArray<FVoxelStampRef> NewStamps;
		{
			VOXEL_SCOPE_COUNTER_NUM("Build NewStamps", NumPoints);

			FVoxelUtilities::SetNum(NewStamps, NumPoints);

			for (int32 Index = 0; Index < NumPoints; Index++)
			{
				// Tricky: can't parallelize this, this modifies NewTemplate
				ActorOverrides.Apply(Index);

				if (FVoxelHeightGraphStamp* GraphStamp = NewTemplate->As<FVoxelHeightGraphStamp>())
				{
					GraphOverrides.Apply(GraphStamp, Index, Context);
				}
				if (FVoxelVolumeGraphStamp* GraphStamp = NewTemplate->As<FVoxelVolumeGraphStamp>())
				{
					GraphOverrides.Apply(GraphStamp, Index, Context);
				}

				NewTemplate->Transform = GetPointTransform(Index);

				NewStamps[Index] = NewTemplate.MakeCopy();
			}
		}

		InstancedStampComponent->AddStamps_NoCopy(MoveTemp(NewStamps));
	}

	for (UFunction* Function : PCGHelpers::FindUserFunctions(TargetActor->GetClass(), Settings->PostProcessFunctionNames, { UPCGFunctionPrototypes::GetPrototypeWithNoParams() }, Context))
	{
		TargetActor->ProcessEvent(Function, nullptr);
	}

	return true;
}

UPCGManagedVoxelInstancedStampComponent* FPCGVoxelStampSpawnerElement::GetOrCreateManagedComponent(
	AActor* InTargetActor,
	UPCGComponent* InSourceComponent,
	uint64 SettingsUID)
{
	VOXEL_FUNCTION_COUNTER();

	check(InTargetActor && InSourceComponent);

	const auto AddTagsToComponent = [InSourceComponent](UVoxelInstancedStampComponent* InstancedStampComponent)
	{
		InstancedStampComponent->ComponentTags.AddUnique(PCGHelpers::DefaultPCGTag);
		InstancedStampComponent->ComponentTags.AddUnique(InSourceComponent->GetFName());
	};

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UPCGActorHelpers::GetOrCreateManagedISMC::FindMatchingMISMC);
		UPCGManagedVoxelInstancedStampComponent* MatchingResource = nullptr;
		InSourceComponent->ForEachManagedResource([&MatchingResource, &InTargetActor, SettingsUID](UPCGManagedResource* InResource)
		{
			// Early out if already found a match
			if (MatchingResource)
			{
				return;
			}

			if (UPCGManagedVoxelInstancedStampComponent* Resource = Cast<UPCGManagedVoxelInstancedStampComponent>(InResource))
			{
				if (Resource->GetSettingsUID() != SettingsUID ||
					!Resource->CanBeUsed())
				{
					return;
				}

				if (const UVoxelInstancedStampComponent* InstancedStampComponent = Resource->GetComponent())
				{
					if (IsValid(InstancedStampComponent) &&
						InstancedStampComponent->GetOwner() == InTargetActor)
					{
						MatchingResource = Resource;
					}
				}
			}
		});

		if (MatchingResource)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UPCGActorHelpers::GetOrCreateManagedISMC::MarkAsUsed);
			MatchingResource->MarkAsUsed();

			UVoxelInstancedStampComponent* InstancedStampComponent = Cast<UVoxelInstancedStampComponent>(MatchingResource->GeneratedComponent.Get());
			if (ensure(InstancedStampComponent))
			{
				InstancedStampComponent->Modify(!InSourceComponent->IsInPreviewMode());
				AddTagsToComponent(InstancedStampComponent);
			}

			return MatchingResource;
		}
	}

	// No matching VoxelStampInstancedComponent found, let's create a new one
	InTargetActor->Modify(!InSourceComponent->IsInPreviewMode());

	const EObjectFlags ObjectFlags = InSourceComponent->IsInPreviewMode() ? RF_Transient : RF_NoFlags;
	UVoxelInstancedStampComponent* InstancedStampComponent = NewObject<UVoxelInstancedStampComponent>(
		InTargetActor,
		UVoxelInstancedStampComponent::StaticClass(),
		MakeUniqueObjectName(InTargetActor, UVoxelInstancedStampComponent::StaticClass(), {}), ObjectFlags);

	InstancedStampComponent->RegisterComponent();
	InTargetActor->AddInstanceComponent(InstancedStampComponent);

	InstancedStampComponent->AttachToComponent(InTargetActor->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));

	AddTagsToComponent(InstancedStampComponent);

	// Create managed resource on source component
	UPCGManagedVoxelInstancedStampComponent* Resource = NewObject<UPCGManagedVoxelInstancedStampComponent>(InSourceComponent);
	Resource->SetComponent(InstancedStampComponent);
	if (InTargetActor->GetRootComponent())
	{
		Resource->SetRootLocation(InTargetActor->GetRootComponent()->GetComponentLocation());
	}

	Resource->SetSettingsUID(SettingsUID);
	InSourceComponent->AddToManagedResources(Resource);

	return Resource;
}