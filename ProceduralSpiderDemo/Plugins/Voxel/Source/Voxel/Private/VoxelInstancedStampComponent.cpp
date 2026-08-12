// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelInstancedStampComponent.h"
#include "VoxelStampComponentUtilities.h"
#include "VoxelInvalidationCallstack.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	Voxel::OnRefreshAll.AddLambda([]
	{
		ForEachObjectOfClass_Copy<UVoxelInstancedStampComponent>([&](UVoxelInstancedStampComponent& Component)
		{
			if (!Component.GetWorld())
			{
				return;
			}

			Component.UpdateAllStamps();
		});
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelInstancedStampComponent::~UVoxelInstancedStampComponent()
{
	for (const FVoxelStampRef& Stamp : PrivateStamps)
	{
		ensure(!Stamp.IsRegistered());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelInstancedStampComponent::OnRegister()
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnRegister();

	UpdateAllStamps();
}

void UVoxelInstancedStampComponent::OnUnregister()
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnUnregister();

	UpdateAllStamps();
}

void UVoxelInstancedStampComponent::PostLoad()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostLoad();

	if (InstancedStamps_DEPRECATED.Num() > 0)
	{
		for (const FVoxelDeprecatedInstancedStamp& InstancedStamp : InstancedStamps_DEPRECATED)
		{
			if (!InstancedStamp.Stamp.IsValid())
			{
				PrivateStamps.Add({});
				continue;
			}

			FVoxelStampRef Stamp = FVoxelStampRef::New(InstancedStamp.Stamp.Get<FVoxelStamp>());
			Stamp->Transform = InstancedStamp.Transform * GetComponentTransform();
			PrivateStamps.Add(Stamp);
		}
	}
	InstancedStamps_DEPRECATED.Empty();

	for (const FVoxelStampRef& Stamp : PrivateStamps)
	{
		if (Stamp)
		{
			Stamp->FixupProperties();
		}
	}
}

void UVoxelInstancedStampComponent::PostInitProperties()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostInitProperties();

	for (const FVoxelStampRef& Stamp : PrivateStamps)
	{
		if (Stamp)
		{
			Stamp->FixupProperties();
		}
	}
}

void UVoxelInstancedStampComponent::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion,
		RemoveBulkData
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;

	if (Version < FVersion::RemoveBulkData)
	{
		FByteBulkData BulkData;
		BulkData.Serialize(Ar, this);
		return;
	}
	ensure(Version == FVersion::LatestVersion);

	FVoxelSerializationGuard Guard(Ar);
}

void UVoxelInstancedStampComponent::OnVisibilityChanged()
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnVisibilityChanged();

	UpdateAllStamps();
}

void UVoxelInstancedStampComponent::OnHiddenInGameChanged()
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnHiddenInGameChanged();

	UpdateAllStamps();
}

void UVoxelInstancedStampComponent::OnActorVisibilityChanged()
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnActorVisibilityChanged();

	UpdateAllStamps();
}

void UVoxelInstancedStampComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);

	Voxel::GameTask(MakeWeakObjectPtrLambda(this, [this]
	{
		UpdateAllStamps();
	}));
}

void UVoxelInstancedStampComponent::UpdateBounds()
{
	VOXEL_FUNCTION_COUNTER();

	Super::UpdateBounds();

	UpdateAllStamps();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelInstancedStampComponent::PostEditUndo()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditUndo();

	for (const FVoxelStampRef& Stamp : PrivateStamps)
	{
		if (Stamp)
		{
			Stamp->FixupProperties();
		}
	}

	UpdateAllStamps();
}

void UVoxelInstancedStampComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	for (const FVoxelStampRef& Stamp : PrivateStamps)
	{
		if (Stamp)
		{
			Stamp->FixupProperties();
		}
	}

	const int32 ArrayIndex = PropertyChangedEvent.GetArrayIndex(GET_OWN_MEMBER_NAME(PrivateStamps).ToString());

	if (ArrayIndex == -1)
	{
		UpdateAllStamps();
	}
	else
	{
		UpdateStamp(ArrayIndex);
	}

	// Will reinstantiate the blueprint component, need to fixup before
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif

TStructOnScope<FActorComponentInstanceData> UVoxelInstancedStampComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FVoxelInstancedStampComponentInstanceData>(this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelInstancedStampComponent::UpdateAllStamps()
{
	VOXEL_FUNCTION_COUNTER_NUM(PrivateStamps.Num());
	FVoxelInvalidationScope Scope(this);

	if (FVoxelStampComponentUtilities::ShouldRender(this))
	{
		FVoxelStampRef::BulkRegister(PrivateStamps, *this);
	}
	else
	{
		FVoxelStampRef::BulkUnregister(PrivateStamps);
	}
}

void UVoxelInstancedStampComponent::UpdateStamp(const int32 Index)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelInvalidationScope Scope(this);

	if (!PrivateStamps.IsValidIndex(Index))
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid index passed to UpdateStamp: {1}", this, Index);
		return;
	}

	const FVoxelStampRef Stamp = PrivateStamps[Index];

	if (FVoxelStampComponentUtilities::ShouldRender(this))
	{
		if (Stamp &&
			!Stamp.IsRegistered())
		{
			Stamp.Register(*this);
		}
	}
	else
	{
		if (Stamp &&
			Stamp.IsRegistered())
		{
			Stamp.Unregister();
		}
	}
}

void UVoxelInstancedStampComponent::UpdateStamps(const TArray<int32>& IndicesToUpdate)
{
	VOXEL_FUNCTION_COUNTER();

	for (const int32 Index : IndicesToUpdate)
	{
		UpdateStamp(Index);
	}
}

int32 UVoxelInstancedStampComponent::AddStamp(const FVoxelStampRef& NewStamp)
{
	VOXEL_FUNCTION_COUNTER();

	// Make a copy to avoid common issues with reusing the stamp in BP
	const int32 Index = PrivateStamps.Add(NewStamp.MakeCopy());
	UpdateStamp(Index);
	return Index;
}

FVoxelStampRef UVoxelInstancedStampComponent::GetStamp(const int32 Index)
{
	if (!PrivateStamps.IsValidIndex(Index))
	{
		return {};
	}

	return PrivateStamps[Index];
}

FVoxelStampRef UVoxelInstancedStampComponent::SetStamp(const int32 Index, const FVoxelStampRef& NewStamp)
{
	VOXEL_FUNCTION_COUNTER();

	if (!PrivateStamps.IsValidIndex(Index))
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid index passed to SetStamp: {1}", this, Index);
		return {};
	}

	// Make a copy to avoid common issues with reusing the stamp in BP
	PrivateStamps[Index] = NewStamp.MakeCopy();

	UpdateStamp(Index);
	return PrivateStamps[Index];
}

void UVoxelInstancedStampComponent::RemoveStamp(const int32 Index)
{
	VOXEL_FUNCTION_COUNTER();

	if (!PrivateStamps.IsValidIndex(Index))
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid index passed to RemoveStamp: {1}", this, Index);
		return;
	}

	PrivateStamps[Index] = {};

	UpdateStamp(Index);
}

void UVoxelInstancedStampComponent::ClearStamps()
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelInvalidationScope Scope(this);

	FVoxelStampRef::BulkUnregister(PrivateStamps);
	PrivateStamps.Empty();
}

void UVoxelInstancedStampComponent::Reserve(const int32 NewNumStamps)
{
	VOXEL_FUNCTION_COUNTER();

	PrivateStamps.Reserve(NewNumStamps);
}

void UVoxelInstancedStampComponent::AddStamps_NoCopy(TVoxelArray<FVoxelStampRef>&& NewStamps)
{
	VOXEL_FUNCTION_COUNTER_NUM(NewStamps.Num());

#if VOXEL_DEBUG
	for (const FVoxelStampRef& Stamp : NewStamps)
	{
		ensure(Stamp.IsSharedPtrUnique());
		ensure(!Stamp.IsRegistered());
	}
#endif

	if (FVoxelStampComponentUtilities::ShouldRender(this))
	{
		FVoxelStampRef::BulkRegister(NewStamps, *this);
	}

	if (PrivateStamps.Num() == 0)
	{
		PrivateStamps = MoveTemp(NewStamps);
	}
	else
	{
		PrivateStamps.Append(NewStamps);
	}
}

int32 UVoxelInstancedStampComponent::FindStampIndex(const FVoxelStampRef& StampRef) const
{
	VOXEL_FUNCTION_COUNTER();

	return PrivateStamps.Find(StampRef);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelInstancedStampComponent::DuplicateStamp(const int32 Index)
{
	VOXEL_FUNCTION_COUNTER();

	ActiveInstance = AddStamp(GetStamp(Index));
}

void UVoxelInstancedStampComponent::SetActiveInstance(const FVoxelStampRef& StampRef) const
{
	VOXEL_FUNCTION_COUNTER();

	const int32 Index = PrivateStamps.Find(StampRef);
	if (!ensureVoxelSlow(Index != -1))
	{
		return;
	}

	ActiveInstance = Index;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelInstancedStampComponentInstanceData::FVoxelInstancedStampComponentInstanceData(const UVoxelInstancedStampComponent* Component)
	: Super(Component)
#if WITH_EDITOR
	, ActiveInstance(Component->ActiveInstance)
#endif
{
	FVoxelUtilities::SetNum(Stamps, Component->PrivateStamps.Num());
	for (int32 Index = 0; Index < Stamps.Num(); Index++)
	{
		Stamps[Index] = Component->PrivateStamps[Index].MakeCopy();
	}
}

bool FVoxelInstancedStampComponentInstanceData::ContainsData() const
{
	return true;
}

void FVoxelInstancedStampComponentInstanceData::AddReferencedObjects(FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);
	for (FVoxelStampRef& Stamp : Stamps)
	{
		Stamp.AddStructReferencedObjects(Collector);
	}
}

void FVoxelInstancedStampComponentInstanceData::ApplyToComponent(UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase)
{
	Super::ApplyToComponent(Component, CacheApplyPhase);

	if (CacheApplyPhase != ECacheApplyPhase::PostUserConstructionScript)
	{
		return;
	}

	UVoxelInstancedStampComponent* StampComponent = CastChecked<UVoxelInstancedStampComponent>(Component);
	StampComponent->PrivateStamps = Stamps;

#if WITH_EDITOR
	StampComponent->ActiveInstance = ActiveInstance;
#endif
}