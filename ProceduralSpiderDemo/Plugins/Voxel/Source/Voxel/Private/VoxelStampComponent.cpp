// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampComponent.h"
#include "VoxelStampComponentUtilities.h"
#include "VoxelLayer.h"
#include "VoxelVersion.h"
#include "VoxelSettings.h"
#include "VoxelStampActor.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"
#include "VoxelInvalidationCallstack.h"

#if WITH_EDITOR
#include "LevelEditor.h"
#endif

class FVoxelStampComponentSingleton : public FVoxelSingleton
{
public:
	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override
	{
#if WITH_EDITOR
		FCoreUObjectDelegates::OnObjectPropertyChanged.AddLambda([](UObject* Object, FPropertyChangedEvent&)
		{
			VOXEL_SCOPE_COUNTER("UVoxelStampComponent OnObjectPropertyChanged");

			const USceneComponent* Component = Cast<USceneComponent>(Object);
			if (!Component)
			{
				// Handle spline metadatas
				Component = Cast<USceneComponent>(Object->GetOuter());
			}
			if (!Component)
			{
				return;
			}

			UVoxelStampComponent* ParentComponent = Cast<UVoxelStampComponent>(Component->GetAttachParent());
			if (!ParentComponent)
			{
				return;
			}

			ParentComponent->UpdateStamp();
		});
#endif
	}
	//~ End FVoxelSingleton Interface
};
FVoxelStampComponentSingleton* GVoxelStampComponentSingleton = new FVoxelStampComponentSingleton();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelStampDummyPreviewComponent::UVoxelStampDummyPreviewComponent()
{
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

FBoxSphereBounds UVoxelStampDummyPreviewComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	VOXEL_FUNCTION_COUNTER();

	const UVoxelStampComponent* Component = Cast<UVoxelStampComponent>(GetAttachParent());
	if (!Component)
	{
		return Super::CalcBounds(LocalToWorld);
	}

	const TSharedPtr<const FVoxelStampRuntime> Runtime = Component->GetStamp().ResolveStampRuntime();
	if (!Runtime)
	{
		ensure(!Component->GetStamp().IsRegistered());

		// Stamp isn't registered yet, delay update until next frame where it should be registered
		// Don't try to register now, would cause a stack overflow
		FVoxelUtilities::DelayedCall(MakeWeakObjectPtrLambda(this, MakeWeakObjectPtrLambda(Component, [this, Component]
		{
			if (Component->GetStamp().IsRegistered())
			{
				ConstCast(this)->UpdateBounds();
			}
		})));

		return Super::CalcBounds(LocalToWorld);
	}

	FBox LocalBounds = FVoxelStampComponentUtilities::GetLocalBounds(*Runtime).ToFBox();
	if (Runtime->GetStamp().IsA<FVoxelHeightStamp>())
	{
		LocalBounds.Min.Z = FMath::Clamp((LocalBounds.Min.X + LocalBounds.Min.Y) / 2.f, -10000.f, 10000.f);
		LocalBounds.Max.Z = FMath::Clamp((LocalBounds.Max.X + LocalBounds.Max.Y) / 2.f, -10000.f, 10000.f);
	}

	return LocalBounds.TransformBy(LocalToWorld);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelStampComponent::~UVoxelStampComponent()
{
	ensure(!PrivateStamp.IsRegistered());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelStampComponent::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);

	if (Ar.CustomVer(GVoxelCustomVersionGUID) >= FVoxelVersion::RemoveSculptStamps)
	{
		return;
	}

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion,
		MakeBulkDataOptional,
		SerializeStampStruct,
		RemoveBulkData,
		RemoveVoxelWriter
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;

	if (Version < FVersion::MakeBulkDataOptional)
	{
		FByteBulkData BulkData;
		BulkData.Serialize(Ar, this);
		return;
	}
	if (Version < FVersion::RemoveBulkData)
	{
		bool bHasBulkData = false;
		Ar << bHasBulkData;

		if (Version >= FVersion::SerializeStampStruct)
		{
			UScriptStruct* Struct = nullptr;
			Ar << Struct;
		}

		TVoxelArray64<uint8> Data;
		Ar << Data;

		return;
	}

	FVoxelSerializationGuard Guard(Ar);

	if (!ensure(Ar.IsLoading()))
	{
		return;
	}

	if (Version < FVersion::RemoveVoxelWriter)
	{
		TVoxelArray64<uint8> Data;
		Ar << Data;
	}
}

#if WITH_EDITOR
void UVoxelStampComponent::PostDuplicate(const EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	if (DuplicateMode != EDuplicateMode::Normal ||
		!PrivateStamp.IsValid())
	{
		return;
	}

	PrivateStamp->Priority = GetNewStampPriority(GetWorld(), PrivateStamp);

	if (GetDefault<UVoxelSettings>()->bRegenerateSeedOnStampDuplicate)
	{
		PrivateStamp->StampSeed.Randomize();
	}
}
#endif

TStructOnScope<FActorComponentInstanceData> UVoxelStampComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FVoxelStampComponentInstanceData>(this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStampRef UVoxelStampComponent::SetStamp(const FVoxelStampRef& NewStamp)
{
	VOXEL_FUNCTION_COUNTER();

	if (!NewStamp)
	{
		FVoxelInvalidationScope ScopeA(this);
		FVoxelInvalidationScope ScopeB("SetStamp");

		PrivateStamp = {};
		UpdateStamp();
		return PrivateStamp;
	}

	// Make a copy to avoid common issues with reusing the stamp in BP
	SetStamp(*NewStamp);
	return PrivateStamp;
}

void UVoxelStampComponent::SetStamp(const FVoxelStamp& NewStamp)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelInvalidationScope ScopeA(this);
	FVoxelInvalidationScope ScopeB("SetStamp");

	if (!NewStamp.Transform.Equals(FTransform::Identity) &&
		!NewStamp.Transform.Equals(GetComponentTransform()))
	{
		VOXEL_MESSAGE(
			Warning,
			"Stamp transform is not identity. "
			"When used on a stamp component, the stamp transform will be ignored and the component transform will be used instead. "
			"Set the stamp transform to identity before calling SetStamp to silence this warning.");
	}

	PrivateStamp = FVoxelStampRef::New(NewStamp);
	PrivateStamp->FixupProperties();
	PrivateStamp->FixupComponents(*this);

	UpdateStamp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USceneComponent* UVoxelStampComponent::FindComponent(UClass* Class) const
{
	VOXEL_FUNCTION_COUNTER();

	for (USceneComponent* Component : ChildComponents)
	{
		if (Component &&
			Component->IsA(Class))
		{
			return Component;
		}
	}

	for (USceneComponent* Component : GetAttachChildren())
	{
		if (ensure(Component) &&
			Component->IsA(Class))
		{
			return Component;
		}
	}

	const AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return nullptr;
	}

	// Look for any component that might not be registered yet
	for (UActorComponent* Component : Owner->GetComponents())
	{
		if (!ensure(Component) ||
			!Component->IsA(Class))
		{
			continue;
		}

		USceneComponent* SceneComponent = CastChecked<USceneComponent>(Component);
		if (SceneComponent->GetAttachParent() != this)
		{
			continue;
		}

		return SceneComponent;
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelStampComponent::OnActorLabelChanged()
{
	VOXEL_FUNCTION_COUNTER();

	AVoxelStampActor* Owner = Cast<AVoxelStampActor>(GetOwner());
	if (!ensureVoxelSlow(Owner))
	{
		return;
	}

	const FString LabelSuffix = GetStampActorLabel();

	FString Label = Owner->GetActorLabel();

	// Remove number suffix to handle duplicated actors
	while (
		Label.Len() > 0 &&
		LabelSuffix.Len() > 0 &&
		Label[Label.Len() - 1] != LabelSuffix[LabelSuffix.Len() - 1] &&
		FChar::IsDigit(Label[Label.Len() - 1]))
	{
		Label.RemoveAt(Label.Len() - 1);
	}

	Label.RemoveFromEnd(LabelSuffix);
	Label.TrimStartAndEndInline();
	Owner->LabelPrefix = Label;

	UpdateStampActorLabel();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 UVoxelStampComponent::GetNewStampPriority(const UWorld* World, const FVoxelStampRef& TargetStamp)
{
	return GetNewStampPriority(World, *TargetStamp);
}

int32 UVoxelStampComponent::GetNewStampPriority(const UWorld* World, const FVoxelStamp& TargetStamp)
{
	VOXEL_FUNCTION_COUNTER();

	if (GetDefault<UVoxelSettings>()->bDisableAutomaticPriority)
	{
		return TargetStamp.Priority;
	}

	const UVoxelLayer* Layer = nullptr;
	if (const FVoxelHeightStamp* HeightStamp = TargetStamp.As<FVoxelHeightStamp>())
	{
		Layer = HeightStamp->Layer;
	}
	else if (const FVoxelVolumeStamp* VolumeStamp = TargetStamp.As<FVoxelVolumeStamp>())
	{
		Layer = VolumeStamp->Layer;
	}

	int32 NewPriority = 0;
	ForEachObjectOfClass<UVoxelStampComponent>([&](const UVoxelStampComponent& Component)
	{
		if (Component.GetWorld() != World)
		{
			return;
		}

		const FVoxelStampRef& OtherStamp = Component.GetStamp();
		if (!OtherStamp ||
			OtherStamp->bExcludeFromPriorityIncrements)
		{
			return;
		}

		if (const FVoxelHeightStamp* HeightStamp = OtherStamp.As<FVoxelHeightStamp>())
		{
			if (HeightStamp->Layer != Layer)
			{
				return;
			}
		}
		else if (const FVoxelVolumeStamp* VolumeStamp = OtherStamp.As<FVoxelVolumeStamp>())
		{
			if (VolumeStamp->Layer != Layer)
			{
				return;
			}
		}

		NewPriority = FMath::Max(NewPriority, OtherStamp->Priority + 1);
	});
	return NewPriority;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelStampComponent::PreUpdateStamp()
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		UpdatePreview();
		UpdateStampActorLabel();
		UpdateRequiredComponents();
	}
#endif
}

FVoxelStampRef UVoxelStampComponent::GetStamp_Internal() const
{
	return PrivateStamp;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelStampComponent::UpdatePreview()
{
	VOXEL_FUNCTION_COUNTER();

	if (!GIsEditor)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	USceneComponent* PreviewComponent = nullptr;
	{
		TArray<USceneComponent*, TInlineAllocator<32>> PreviewComponents;
		Owner->GetComponents<USceneComponent>(PreviewComponents);

		for (USceneComponent* Component : PreviewComponents)
		{
			if (!Component->GetClass()->HasMetaData(STATIC_FNAME("VoxelPreviewComponent")))
			{
				continue;
			}

			if (!PreviewComponent)
			{
				PreviewComponent = Component;
			}
			else
			{
				// Stale preview component, destroy
				Component->DestroyComponent();
			}
		}
	}

	const UWorld* World = GetWorld();
	if (!PrivateStamp ||
		!World ||
		World->IsGameWorld())
	{
		if (PreviewComponent)
		{
			PreviewComponent->DestroyComponent();
			PreviewComponent = nullptr;
		}

		return;
	}

	struct FPreview : FVoxelStamp::IPreview
	{
		AActor* Owner = nullptr;
		UVoxelStampComponent* This = nullptr;
		USceneComponent* OldPreviewComponent = nullptr;
		USceneComponent* NewPreviewComponent = nullptr;

		virtual USceneComponent* GetComponent(UClass* Class) override
		{
			ensure(Class->HasMetaData(STATIC_FNAME("VoxelPreviewComponent")));

			if (OldPreviewComponent &&
				OldPreviewComponent->GetClass() == Class)
			{
				NewPreviewComponent = OldPreviewComponent;
				return NewPreviewComponent;
			}

			NewPreviewComponent = NewObject<USceneComponent>(Owner, Class, NAME_None, RF_Transient);

			if (!ensure(NewPreviewComponent))
			{
				return nullptr;
			}

			// Cleanup the InstanceComponents array,
			// it doesn't get cleaned up automatically when Transient components are instanced
			Owner->RemoveInstanceComponent(nullptr);

			Owner->AddInstanceComponent(NewPreviewComponent);

			NewPreviewComponent->SetupAttachment(This);
			NewPreviewComponent->SetRelativeTransform(FTransform::Identity);
			NewPreviewComponent->RegisterComponent();

			return NewPreviewComponent;
		}
	};

	FPreview Preview;
	Preview.Owner = Owner;
	Preview.This = this;
	Preview.OldPreviewComponent = PreviewComponent;
	PrivateStamp->SetupPreview(Preview);

	if (!Preview.NewPreviewComponent)
	{
		// Create a dummy component to fix double click to focus in editor
		Preview.GetComponent(UVoxelStampDummyPreviewComponent::StaticClass());
	}

	if (PreviewComponent == Preview.NewPreviewComponent)
	{
		return;
	}

	if (PreviewComponent)
	{
		PreviewComponent->DestroyComponent();
	}

	if (Owner->IsSelectedInEditor() &&
		CreationMethod == EComponentCreationMethod::Native)
	{
		FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditor.BroadcastComponentsEdited();
	}
}

DEFINE_PRIVATE_ACCESS(AActor, ActorLabel);

void UVoxelStampComponent::UpdateStampActorLabel() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!GIsEditor)
	{
		return;
	}

	AVoxelStampActor* Owner = Cast<AVoxelStampActor>(GetOwner());
	if (!Owner ||
		!GetDefault<UVoxelSettings>()->bEnableAutoStampActorLabeling ||
		!Owner->bAutoUpdateLabel)
	{
		return;
	}

	FString Label = GetStampActorLabel();

	if (!Owner->LabelPrefix.IsEmpty())
	{
		Label = Owner->LabelPrefix + " " + Label;
	}

	if (Label.Equals(Owner->GetActorLabel()))
	{
		return;
	}

	// Don't use SetActorLabel to avoid loops, as SetActorLabel will re-register all components
	PrivateAccess::ActorLabel(*Owner) = Label;

	FCoreDelegates::OnActorLabelChanged.Broadcast(Owner);
}

void UVoxelStampComponent::UpdateRequiredComponents()
{
	VOXEL_FUNCTION_COUNTER();

	if (!GIsEditor)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World ||
		World->IsGameWorld())
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return;
	}

	bool bTriggerRefresh = false;

	TVoxelArray<TSubclassOf<USceneComponent>> RequiredComponents;
	if (PrivateStamp)
	{
		RequiredComponents = PrivateStamp->GetRequiredComponents();
	}

	for (auto It = ChildComponents.CreateIterator(); It; ++It)
	{
		USceneComponent* Component = *It;
		if (!Component)
		{
			It.RemoveCurrent();
			continue;
		}

		if (RequiredComponents.Remove(Component->GetClass()))
		{
			continue;
		}

		Component->SetFlags(RF_Transient);
		Component->UnregisterComponent();
		Component->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		Owner->RemoveOwnedComponent(Component);
		OrphanChildComponents.Add(Component);

		bTriggerRefresh = true;
		It.RemoveCurrent();
	}

	for (UClass* Class : RequiredComponents)
	{
		USceneComponent* Result = INLINE_LAMBDA -> USceneComponent*
		{
			if (USceneComponent* Component = FindComponent(Class))
			{
				return Component;
			}

			for (auto It = OrphanChildComponents.CreateIterator(); It; ++It)
			{
				USceneComponent* Component = *It;
				if (!Component)
				{
					It.RemoveCurrent();
					continue;
				}

				if (!Component->IsA(Class))
				{
					continue;
				}
				It.RemoveCurrent();

				ensure(Component->HasAllFlags(RF_Transient));
				Component->ClearFlags(RF_Transient);

				Component->CreationMethod = EComponentCreationMethod::Native;

				Owner->AddOwnedComponent(Component);
				Owner->AddInstanceComponent(Component);

				Component->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
				Component->SetRelativeTransform(FTransform::Identity);
				Component->RegisterComponent();

				return Component;
			}

			for (UActorComponent* Component : Owner->GetComponents())
			{
				if (!Component ||
					!Component->IsA(Class) ||
					!ensureVoxelSlow(Component->CreationMethod == EComponentCreationMethod::Instance) ||
					!ensureVoxelSlow(Owner->GetInstanceComponents().Contains(Component)))
				{
					continue;
				}

				return CastChecked<USceneComponent>(Component);
			}

			USceneComponent* Component = NewObject<USceneComponent>(Owner, Class, NAME_None, RF_Transactional);
			if (!ensure(Component))
			{
				return nullptr;
			}

			Owner->AddInstanceComponent(Component);

			Component->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			Component->SetRelativeTransform(FTransform::Identity);
			Component->RegisterComponent();

			return Component;
		};

		if (Result)
		{
			bTriggerRefresh = true;
			ChildComponents.Add(Result);
		}
	}

	if (bTriggerRefresh &&
		Owner->IsSelectedInEditor() &&
		CreationMethod == EComponentCreationMethod::Native)
	{
		FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditor.BroadcastComponentsEdited();
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString UVoxelStampComponent::GetStampActorLabel() const
{
	VOXEL_FUNCTION_COUNTER();

	FString StampTypeName = "None";
	FString BlendModeName = "";
	FString IdentifierName = "";
	FString LayerName = "";
	FString Priority = "";

	if (PrivateStamp)
	{
		StampTypeName = PrivateStamp.GetStruct()->GetName();
		StampTypeName.RemoveFromStart("Voxel");
		StampTypeName.RemoveFromEnd("Stamp");

		IdentifierName = PrivateStamp->GetIdentifier();

		const UVoxelLayer* Layer = nullptr;
		if (const TSharedPtr<const FVoxelHeightStamp> HeightStamp = PrivateStamp.ToSharedPtr<FVoxelHeightStamp>())
		{
			Layer = HeightStamp->Layer;
			BlendModeName = GetEnumDisplayName(HeightStamp->BlendMode).ToString();
		}
		if (const TSharedPtr<const FVoxelVolumeStamp> VolumeStamp = PrivateStamp.ToSharedPtr<FVoxelVolumeStamp>())
		{
			Layer = VolumeStamp->Layer;
			BlendModeName = GetEnumDisplayName(VolumeStamp->BlendMode).ToString();
		}

		if (Layer)
		{
			LayerName = Layer->GetName();
		}

		Priority = FText::AsNumber(PrivateStamp->Priority).ToString();
	}

	TMap<FString, FStringFormatArg> Args;
	Args.Add("Type", StampTypeName);
	Args.Add("BlendMode", BlendModeName);
	Args.Add("Identifier", IdentifierName);
	Args.Add("Layer", LayerName);
	Args.Add("Priority", Priority);

	FString Label = FString::Format(*GetDefault<UVoxelSettings>()->StampActorLabelFormat, Args);
	Label.ReplaceInline(TEXT("__"), TEXT("_"));
	Label.ReplaceInline(TEXT("  "), TEXT(" "));
	return Label;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStampComponentInstanceData::FVoxelStampComponentInstanceData(const UVoxelStampComponent* Component)
	: Super(Component)
	, Stamp(Component->PrivateStamp.MakeCopy())
{
}

bool FVoxelStampComponentInstanceData::ContainsData() const
{
	return true;
}

void FVoxelStampComponentInstanceData::AddReferencedObjects(FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);
	Stamp.AddStructReferencedObjects(Collector);
}

void FVoxelStampComponentInstanceData::ApplyToComponent(UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase)
{
	Super::ApplyToComponent(Component, CacheApplyPhase);

	if (CacheApplyPhase != ECacheApplyPhase::PostUserConstructionScript)
	{
		return;
	}

	UVoxelStampComponent* StampComponent = CastChecked<UVoxelStampComponent>(Component);
	StampComponent->SetStamp(Stamp);
}