// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterActor.h"
#include "Scatter/VoxelScatterGraph.h"
#include "Scatter/VoxelScatterActorRuntime.h"

bool AVoxelScatterActor::ShouldForceEnableOverride(const FGuid& Guid) const
{
	return true;
}

UVoxelGraph* AVoxelScatterActor::GetGraph() const
{
	return Graph;
}

FVoxelParameterOverrides& AVoxelScatterActor::GetParameterOverrides()
{
	return ParameterOverrides;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelScatterActor::BeginPlay()
{
	VOXEL_FUNCTION_COUNTER();

	Super::BeginPlay();

	if (!Runtime)
	{
		CreateRuntime();
	}
}

void AVoxelScatterActor::BeginDestroy()
{
	VOXEL_FUNCTION_COUNTER();

	if (Runtime)
	{
		DestroyRuntime();
	}

	Super::BeginDestroy();
}

void AVoxelScatterActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	VOXEL_FUNCTION_COUNTER();

	// In the editor, Destroyed is called but EndPlay isn't

	if (Runtime)
	{
		DestroyRuntime();
	}

	Super::EndPlay(EndPlayReason);
}

void AVoxelScatterActor::Destroyed()
{
	VOXEL_FUNCTION_COUNTER();

	if (Runtime)
	{
		DestroyRuntime();
	}

	Super::Destroyed();
}

void AVoxelScatterActor::OnConstruction(const FTransform& Transform)
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnConstruction(Transform);

	if (!Runtime)
	{
		CreateRuntime();
	}
}

void AVoxelScatterActor::PostLoad()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostLoad();

	FixupParameterOverrides();
}

void AVoxelScatterActor::PostInitProperties()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostInitProperties();

	FixupParameterOverrides();
}

void AVoxelScatterActor::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void AVoxelScatterActor::PostEditUndo()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditUndo();

	if (IsValid(this))
	{
		if (!Runtime)
		{
			CreateRuntime();
		}
	}
	else
	{
		if (Runtime)
		{
			DestroyRuntime();
		}
	}
}

void AVoxelScatterActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	FixupParameterOverrides();
	UpdateRuntime();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelScatterActor::CreateRuntime()
{
	VOXEL_FUNCTION_COUNTER();

	Runtime = FVoxelScatterActorRuntime::Create(*this);
}

void AVoxelScatterActor::DestroyRuntime()
{
	VOXEL_FUNCTION_COUNTER();

	if (!Runtime)
	{
		return;
	}

	Runtime->Destroy();
	Runtime.Reset();
}

void AVoxelScatterActor::UpdateRuntime()
{
	VOXEL_FUNCTION_COUNTER();

	DestroyRuntime();
	CreateRuntime();
}