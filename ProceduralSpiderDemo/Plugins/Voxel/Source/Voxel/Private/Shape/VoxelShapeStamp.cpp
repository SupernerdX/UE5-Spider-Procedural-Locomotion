// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Shape/VoxelShapeStamp.h"
#include "VoxelStampUtilities.h"
#include "VoxelStampMeshPreviewComponent.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"

#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"

#if WITH_EDITOR
FString FVoxelShapeStamp::GetIdentifier() const
{
	if (!Shape)
	{
		return "None";
	}

	return Shape->GetStruct()->GetDisplayNameText().ToString();
}

void FVoxelShapeStamp::SetupPreview(IPreview& Preview) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Shape)
	{
		return;
	}

	UVoxelStampMeshPreviewComponent* PreviewComponent = Preview.GetComponent<UVoxelStampMeshPreviewComponent>();
	if (!ensure(PreviewComponent))
	{
		return;
	}

	UStaticMesh* PreviewMesh = nullptr;
	FTransform PreviewTransform;
	Shape->GetPreviewInfo(PreviewMesh, PreviewTransform);

	PreviewComponent->SetRelativeTransform(FTransform(
			FQuat::Identity,
			FVector::ZeroVector,
			FVector(1.01f)) *
		PreviewTransform);

	PreviewComponent->SetStaticMesh(PreviewMesh);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelShapeStampRuntime::Initialize(FVoxelDependencyCollector& DependencyCollector)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Stamp.Shape)
	{
		return false;
	}

	Shape = Stamp.Shape.ToSharedRef();
	SurfaceType = FVoxelSurfaceType(Stamp.SurfaceType);
	MetadataOverrides = Stamp.MetadataOverrides.CreateRuntime();

	return true;
}

FVoxelBox FVoxelShapeStampRuntime::GetLocalBounds() const
{
	return Shape->GetLocalBounds();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelShapeStampRuntime::Apply(
	const FVoxelVolumeBulkQuery& Query,
	const FVoxelVolumeTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	if (!AffectShape())
	{
		return;
	}

	const FVoxelDoubleVectorBuffer Positions = FVoxelStampUtilities::ComputePositions(
		Query,
		StampToQuery);

	FVoxelFloatBuffer NewDistances;
	NewDistances.Allocate(Query.Num());

	Shape->Sample(
		NewDistances.View(),
		Positions);

	FVoxelStampUtilities::ApplyDistances(
		*this,
		Query,
		StampToQuery,
		NewDistances);
}

void FVoxelShapeStampRuntime::Apply(
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const FVoxelDoubleVectorBuffer Positions = FVoxelStampUtilities::ComputePositions(
		Query,
		StampToQuery);

	const auto ComputeDistances = [&]
	{
		FVoxelFloatBuffer NewDistances;
		NewDistances.Allocate(Positions.Num());

		Shape->Sample(
			NewDistances.View(),
			Positions);

		return NewDistances;
	};

	const bool bComputeSurfaceTypes =
		AffectSurfaceType() &&
		Query.bQuerySurfaceTypes;

	const bool bComputeMetadatas =
		AffectMetadata() &&
		Query.MetadatasToQuery.Num() > 0 &&
		MetadataOverrides->ShouldCompute(Query);

	if (!bComputeSurfaceTypes &&
		!bComputeMetadatas)
	{
		if (AffectShape())
		{
			FVoxelStampUtilities::ApplyDistances(
				*this,
				Query,
				StampToQuery,
				ComputeDistances(),
				true,
				nullptr);
		}

		return;
	}

	FVoxelFloatBuffer Alphas;
	if (Stamp.BlendMode == EVoxelVolumeBlendMode::Override)
	{
		if (AffectShape())
		{
			FVoxelStampUtilities::ApplyDistances(
				*this,
				Query,
				StampToQuery,
				ComputeDistances(),
				true,
				nullptr);
		}

		Alphas = 1.f;
	}
	else
	{
		FVoxelStampUtilities::ApplyDistances(
			*this,
			Query,
			StampToQuery,
			ComputeDistances(),
			AffectShape(),
			&Alphas);
	}

	if (bComputeSurfaceTypes)
	{
		FVoxelStampUtilities::ApplySurfaceType(
			Query,
			SurfaceType,
			Alphas);
	}

	if (bComputeMetadatas)
	{
		FVoxelStampUtilities::ApplyMetadatas(
			Query,
			*MetadataOverrides,
			{},
			Alphas);
	}
}