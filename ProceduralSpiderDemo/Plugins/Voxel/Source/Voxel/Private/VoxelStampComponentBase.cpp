// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampComponentBase.h"
#include "VoxelStampComponentUtilities.h"
#include "VoxelStampComponentInterface.h"
#include "VoxelInvalidationCallstack.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	Voxel::OnRefreshAll.AddLambda([]
	{
		ForEachObjectOfClass_Copy<UVoxelStampComponentBase>([&](UVoxelStampComponentBase& Component)
		{
			Component.UpdateStamp();
		});
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelStampComponentBase::OnRegister()
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnRegister();

	ApplyComponentChangesToStamp();
}

void UVoxelStampComponentBase::OnUnregister()
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnUnregister();

	ApplyComponentChangesToStamp();
}

void UVoxelStampComponentBase::PostLoad()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostLoad();

	if (const FVoxelStampRef Stamp = GetStamp_Internal())
	{
		Stamp->FixupProperties();

		if (const IVoxelStampComponentInterface* Interface = Cast<IVoxelStampComponentInterface>(this))
		{
			Stamp->FixupComponents(*Interface);
		}
	}
}

void UVoxelStampComponentBase::PostInitProperties()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject) ||
		GetOuter()->HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	if (const FVoxelStampRef Stamp = GetStamp_Internal())
	{
		Stamp->FixupProperties();

		if (const IVoxelStampComponentInterface* Interface = Cast<IVoxelStampComponentInterface>(this))
		{
			Stamp->FixupComponents(*Interface);
		}
	}
}

void UVoxelStampComponentBase::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

void UVoxelStampComponentBase::OnVisibilityChanged()
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnVisibilityChanged();

	ApplyComponentChangesToStamp();
}

void UVoxelStampComponentBase::OnHiddenInGameChanged()
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnHiddenInGameChanged();

	ApplyComponentChangesToStamp();
}

void UVoxelStampComponentBase::OnActorVisibilityChanged()
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnActorVisibilityChanged();

	ApplyComponentChangesToStamp();
}

void UVoxelStampComponentBase::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);

	// Async otherwise crash in MarkActorComponentForNeededEndOfFrameUpdate
	Voxel::GameTask_Async(MakeWeakObjectPtrLambda(this, [this]
	{
		ApplyComponentChangesToStamp();
	}));
}

void UVoxelStampComponentBase::UpdateBounds()
{
	VOXEL_FUNCTION_COUNTER();

	Super::UpdateBounds();

	ApplyComponentChangesToStamp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelStampComponentBase::PostEditUndo()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditUndo();

	if (const FVoxelStampRef Stamp = GetStamp_Internal())
	{
		Stamp->FixupProperties();

		if (const IVoxelStampComponentInterface* Interface = Cast<IVoxelStampComponentInterface>(this))
		{
			Stamp->FixupComponents(*Interface);
		}
	}

	UpdateStamp();
}

void UVoxelStampComponentBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	if (const FVoxelStampRef Stamp = GetStamp_Internal())
	{
		Stamp->FixupProperties();

		if (const IVoxelStampComponentInterface* Interface = Cast<IVoxelStampComponentInterface>(this))
		{
			Stamp->FixupComponents(*Interface);
		}
	}

	{
		FVoxelInvalidationScope Scope("Property changed: " + PropertyChangedEvent.GetPropertyName().ToString());

		UpdateStamp();
	}

	// Will reinstantiate the blueprint component, need to fixup before
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelStampComponentBase::UpdateStamp()
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelInvalidationScope Scope(this);

	const FVoxelStampRef Stamp = GetStamp_Internal();
	if (Stamp)
	{
		if (const USceneComponent* Component = Stamp->GetComponent().Resolve())
		{
			ensure(Component == this);
		}
	}

	PreUpdateStamp();

	if (Stamp.IsValid() &&
		FVoxelStampComponentUtilities::ShouldRender(this))
	{
		Stamp->Transform = GetComponentTransform();

		if (!Stamp.IsRegistered())
		{
			Stamp.Register(*this);
		}
		else
		{
			Stamp.Update();
		}
	}
	else
	{
		if (Stamp.IsRegistered())
		{
			Stamp.Unregister();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelStampComponentBase::ApplyComponentChangesToStamp()
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelStampRef Stamp = GetStamp_Internal();

	if (Stamp.IsValid() &&
		FVoxelStampComponentUtilities::ShouldRender(this))
	{
		if (!Stamp->Transform.Equals(GetComponentTransform()) ||
			!Stamp.IsRegistered())
		{
			UpdateStamp();
		}
	}
	else
	{
		if (Stamp.IsRegistered())
		{
			UpdateStamp();
		}
	}
}