// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "StaticMesh/VoxelMeshStamp.h"
#include "StaticMesh/VoxelStaticMesh.h"
#include "StaticMesh/VoxelStaticMeshData.h"
#include "VoxelStampUtilities.h"
#include "VoxelNormalMetadata.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelStampMeshPreviewComponent.h"
#include "Surface/VoxelSurfaceTypeInterface.h"

#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"

#include "VoxelMeshStampImpl.ispc.generated.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UObject* FVoxelMeshStamp::GetAsset() const
{
	return NewMesh;
}

void FVoxelMeshStamp::FixupProperties()
{
	Super::FixupProperties();

#if WITH_EDITOR
	UVoxelStaticMesh::Migrate(Mesh, NewMesh);
	UVoxelSurfaceTypeInterface::Migrate(Material, SurfaceType);
#endif
}

#if WITH_EDITOR
void FVoxelMeshStamp::SetupPreview(IPreview& Preview) const
{
	VOXEL_FUNCTION_COUNTER();

	UVoxelStampMeshPreviewComponent* PreviewComponent = Preview.GetComponent<UVoxelStampMeshPreviewComponent>();
	if (!ensure(PreviewComponent))
	{
		return;
	}

	PreviewComponent->SetRelativeTransform(FTransform(
		FQuat::Identity,
		FVector::ZeroVector,
		FVector(1.01f)));

	PreviewComponent->SetStaticMesh(NewMesh ? NewMesh->Mesh.LoadSynchronous() : nullptr);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelMeshStampRuntime::Initialize(FVoxelDependencyCollector& DependencyCollector)
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(IsInGameThread());

	if (!Stamp.NewMesh)
	{
		return false;
	}

	MeshData = Stamp.NewMesh->GetData();

#if WITH_EDITOR
	Stamp.NewMesh->OnChanged_EditorOnly.Add(MakeWeakPtrDelegate(this, [this]
	{
		RequestUpdate();
	}));
#endif

	if (!MeshData)
	{
		return false;
	}

	SurfaceType = FVoxelSurfaceType(Stamp.SurfaceType);
	MetadataOverrides = Stamp.MetadataOverrides.CreateRuntime();

	for (const FVoxelStaticMeshMetadata& Metadata : Stamp.NewMesh->Metadatas)
	{
		if (!Metadata.Metadata ||
			!Metadata.Data.Buffer ||
			!ensure(Metadata.Data.Buffer->Num_Slow() == MeshData->Points.Num()) ||
			!ensure(Metadata.Data.Buffer->GetInnerType() == Metadata.Metadata->GetInnerType()))
		{
			continue;
		}

		const FVoxelMetadataRef MetadataRef(Metadata.Metadata);

		if (MetadataOverrides->MetadataToValue.Contains(MetadataRef) ||
			MetadataRefToBuffer.Contains(MetadataRef))
		{
			continue;
		}

		MetadataRefToBuffer.Add_EnsureNew(MetadataRef, Metadata.Data.Buffer);
	}

	return true;
}

FVoxelBox FVoxelMeshStampRuntime::GetLocalBounds() const
{
	return MeshData->MeshBounds.Extend(1).Scale(MeshData->VoxelSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMeshStampRuntime::Apply(
	const FVoxelVolumeBulkQuery& Query,
	const FVoxelVolumeTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	if (!AffectShape())
	{
		return;
	}

	const ispc::EVoxelVolumeBlendMode BlendMode = INLINE_LAMBDA
	{
		switch (Stamp.BlendMode)
		{
		default: ensure(false);
		case EVoxelVolumeBlendMode::Additive: return ispc::EVoxelVolumeBlendMode::VolumeBlendMode_Additive;
		case EVoxelVolumeBlendMode::Subtractive: return ispc::EVoxelVolumeBlendMode::VolumeBlendMode_Subtractive;
		case EVoxelVolumeBlendMode::Intersect: return ispc::EVoxelVolumeBlendMode::VolumeBlendMode_Intersect;
		case EVoxelVolumeBlendMode::Override: return ispc::EVoxelVolumeBlendMode::VolumeBlendMode_Override;
		}
	};

	ispc::VoxelMeshStamp_ApplyGrid(
		Query.ISPC(),
		StampToQuery.ISPC(),
		BlendMode,
		Stamp.bApplyOnVoid,
		Stamp.Smoothness,
		Stamp.bUseTricubic,
		GetISPCValue(MeshData->Size),
		MeshData->VoxelSize,
		GetISPCValue(MeshData->Origin),
		MeshData->DistanceField.GetData());
}

void FVoxelMeshStampRuntime::Apply(
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const ispc::EVoxelVolumeBlendMode BlendMode = INLINE_LAMBDA
	{
		switch (Stamp.BlendMode)
		{
		default: ensure(false);
		case EVoxelVolumeBlendMode::Additive: return ispc::EVoxelVolumeBlendMode::VolumeBlendMode_Additive;
		case EVoxelVolumeBlendMode::Subtractive: return ispc::EVoxelVolumeBlendMode::VolumeBlendMode_Subtractive;
		case EVoxelVolumeBlendMode::Intersect: return ispc::EVoxelVolumeBlendMode::VolumeBlendMode_Intersect;
		case EVoxelVolumeBlendMode::Override: return ispc::EVoxelVolumeBlendMode::VolumeBlendMode_Override;
		}
	};

	const bool bComputeSurfaceTypes =
		AffectSurfaceType() &&
		Query.bQuerySurfaceTypes;

	const bool bComputeMetadatas = INLINE_LAMBDA
	{
		if (!AffectMetadata() ||
			Query.MetadatasToQuery.Num() == 0)
		{
			return false;
		}

		for (const auto& It : MetadataRefToBuffer)
		{
			if (Query.MetadatasToQuery.Contains(It.Key))
			{
				return true;
			}
		}

		return MetadataOverrides->ShouldCompute(Query);
	};

	if (!bComputeSurfaceTypes &&
		!bComputeMetadatas)
	{
		if (AffectShape())
		{
			ispc::VoxelMeshStamp_ApplySparse(
				Query.ISPC(),
				StampToQuery.ISPC(),
				nullptr,
				true,
				false,
				BlendMode,
				Stamp.bApplyOnVoid,
				Stamp.Smoothness,
				Stamp.bUseTricubic,
				GetISPCValue(MeshData->Size),
				MeshData->VoxelSize,
				GetISPCValue(MeshData->Origin),
				MeshData->DistanceField.GetData());
		}

		return;
	}

	FVoxelFloatBuffer Alphas;
	Alphas.Allocate(Query.Num());

	ispc::VoxelMeshStamp_ApplySparse(
		Query.ISPC(),
		StampToQuery.ISPC(),
		Alphas.GetData(),
		AffectShape(),
		true,
		BlendMode,
		Stamp.bApplyOnVoid,
		Stamp.Smoothness,
		Stamp.bUseTricubic,
		GetISPCValue(MeshData->Size),
		MeshData->VoxelSize,
		GetISPCValue(MeshData->Origin),
		MeshData->DistanceField.GetData());

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

		for (const auto& It : MetadataRefToBuffer)
		{
			if (!Query.MetadatasToQuery.Contains(It.Key))
			{
				continue;
			}

			if (const FVoxelFloatBuffer* Buffer = It.Value->As<FVoxelFloatBuffer>())
			{
				FVoxelFloatBuffer& IndirectBuffer = Query.IndirectMetadata.FindChecked<FVoxelFloatBuffer>(It.Key);
				checkVoxelSlow(IndirectBuffer.Num() == Query.IndirectDistances.Num());

				ispc::VoxelMeshStamp_ApplyMetadata_Float(
					Query.ISPC(),
					StampToQuery.ISPC(),
					IndirectBuffer.GetData(),
					Alphas.GetData(),
					GetISPCValue(MeshData->Size),
					MeshData->VoxelSize,
					GetISPCValue(MeshData->Origin),
					MeshData->PointIndices.GetData(),
					Buffer->GetData());

				continue;
			}

			if (const FVoxelLinearColorBuffer* Buffer = It.Value->As<FVoxelLinearColorBuffer>())
			{
				FVoxelLinearColorBuffer& IndirectBuffer = Query.IndirectMetadata.FindChecked<FVoxelLinearColorBuffer>(It.Key);
				checkVoxelSlow(IndirectBuffer.Num() == Query.IndirectDistances.Num());

				ispc::VoxelMeshStamp_ApplyMetadata_LinearColor(
					Query.ISPC(),
					StampToQuery.ISPC(),
					IndirectBuffer.R.GetData(),
					IndirectBuffer.G.GetData(),
					IndirectBuffer.B.GetData(),
					IndirectBuffer.A.GetData(),
					Alphas.GetData(),
					GetISPCValue(MeshData->Size),
					MeshData->VoxelSize,
					GetISPCValue(MeshData->Origin),
					MeshData->PointIndices.GetData(),
					Buffer->R.GetData(),
					Buffer->G.GetData(),
					Buffer->B.GetData(),
					Buffer->A.GetData());

				continue;
			}

			if (const FVoxelNormalBuffer* Buffer = It.Value->As<FVoxelNormalBuffer>())
			{
				FVoxelNormalBuffer& IndirectBuffer = Query.IndirectMetadata.FindChecked<FVoxelNormalBuffer>(It.Key);
				checkVoxelSlow(IndirectBuffer.Num() == Query.IndirectDistances.Num());

				ispc::VoxelMeshStamp_ApplyMetadata_Normal(
					Query.ISPC(),
					StampToQuery.ISPC(),
					IndirectBuffer.GetData(),
					Alphas.GetData(),
					GetISPCValue(MeshData->Size),
					MeshData->VoxelSize,
					GetISPCValue(MeshData->Origin),
					MeshData->PointIndices.GetData(),
					Buffer->GetData());

				continue;
			}

			ensure(false);
		}
	}
}