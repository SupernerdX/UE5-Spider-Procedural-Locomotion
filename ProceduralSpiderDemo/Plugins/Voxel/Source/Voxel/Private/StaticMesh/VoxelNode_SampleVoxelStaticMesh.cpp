// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "StaticMesh/VoxelNode_SampleVoxelStaticMesh.h"
#include "Buffer/VoxelNormalBuffer.h"
#include "StaticMesh/VoxelStaticMeshData.h"
#include "VoxelNode_SampleVoxelStaticMeshImpl.ispc.generated.h"

void FVoxelNode_SampleVoxelStaticMesh::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelStaticMeshRef> Mesh = MeshPin.Get(Query);
	const TValue<FVoxelVectorBuffer> Position = PositionPin.Get(Query);
	const TValue<float> Scale = ScalePin.Get(Query);
	const TValue<bool> bUseTricubic = UseTricubicPin.Get(Query);

	VOXEL_GRAPH_WAIT(Position, Mesh, Scale, bUseTricubic)
	{
		FVoxelVectorBuffer LocalPosition = *Position;
		LocalPosition.ExpandConstants();

		const int32 Num = LocalPosition.Num();

		FVoxelFloatBuffer Result;
		if (DistancePin.ShouldCompute())
		{
			Result.Allocate(LocalPosition.Num());
		}

		struct FMetadata
		{
			TSharedPtr<const FVoxelBuffer> MeshMetadata;
			TSharedRef<FVoxelBuffer> Result;
		};
		TVoxelMap<FVoxelMetadataRef, FMetadata> MetadataToBuffers;
		MetadataToBuffers.Reserve(MetadataPins.Num());

		for (const FMetadataPin& MetadataPin : MetadataPins)
		{
			if (!MetadataPin.PinRef.ShouldCompute())
			{
				continue;
			}

			const TSharedPtr<const FVoxelBuffer> MeshMetadata = Mesh->MetadataRefToBuffer.FindRef(MetadataPin.MetadataRef);
			if (!MeshMetadata)
			{
				continue;
			}

			MetadataToBuffers.Add_EnsureNew(
				MetadataPin.MetadataRef,
				FMetadata
				{
					MeshMetadata,
					MetadataPin.MetadataRef.MakeDefaultBuffer(Num)
				});
		}

		if (DistancePin.ShouldCompute())
		{
			ispc::VoxelNode_SampleVoxelStaticMesh(
				LocalPosition.X.GetData(),
				LocalPosition.Y.GetData(),
				LocalPosition.Z.GetData(),
				Result.GetData(),
				LocalPosition.Num(),
				Scale,
				bUseTricubic,
				GetISPCValue(Mesh->MeshData->Size),
				Mesh->MeshData->VoxelSize,
				GetISPCValue(Mesh->MeshData->Origin),
				Mesh->MeshData->DistanceField.GetData());
		}

		if (MetadataToBuffers.Num() > 0)
		{
			for (const auto& It : MetadataToBuffers)
			{
				if (const FVoxelFloatBuffer* MeshBuffer = It.Value.MeshMetadata->As<FVoxelFloatBuffer>())
				{
					FVoxelFloatBuffer& ResultBuffer = It.Value.Result->AsChecked<FVoxelFloatBuffer>();
					ispc::VoxelNode_SampleVoxelStaticMesh_ApplyMetadata_Float(
						LocalPosition.X.GetData(),
						LocalPosition.Y.GetData(),
						LocalPosition.Z.GetData(),
						ResultBuffer.GetData(),
						LocalPosition.Num(),
						Scale,
						GetISPCValue(Mesh->MeshData->Size),
						Mesh->MeshData->VoxelSize,
						GetISPCValue(Mesh->MeshData->Origin),
						Mesh->MeshData->PointIndices.GetData(),
						MeshBuffer->GetData());
					continue;
				}

				if (const FVoxelLinearColorBuffer* MeshBuffer = It.Value.MeshMetadata->As<FVoxelLinearColorBuffer>())
				{
					FVoxelLinearColorBuffer& ResultBuffer = It.Value.Result->AsChecked<FVoxelLinearColorBuffer>();
					ispc::VoxelNode_SampleVoxelStaticMesh_ApplyMetadata_LinearColor(
						LocalPosition.X.GetData(),
						LocalPosition.Y.GetData(),
						LocalPosition.Z.GetData(),
						ResultBuffer.R.GetData(),
						ResultBuffer.G.GetData(),
						ResultBuffer.B.GetData(),
						ResultBuffer.A.GetData(),
						LocalPosition.Num(),
						Scale,
						GetISPCValue(Mesh->MeshData->Size),
						Mesh->MeshData->VoxelSize,
						GetISPCValue(Mesh->MeshData->Origin),
						Mesh->MeshData->PointIndices.GetData(),
						MeshBuffer->R.GetData(),
						MeshBuffer->G.GetData(),
						MeshBuffer->B.GetData(),
						MeshBuffer->A.GetData());
					continue;
				}

				if (const FVoxelNormalBuffer* MeshBuffer = It.Value.MeshMetadata->As<FVoxelNormalBuffer>())
				{
					FVoxelNormalBuffer& ResultBuffer = It.Value.Result->AsChecked<FVoxelNormalBuffer>();
					ispc::VoxelNode_SampleVoxelStaticMesh_ApplyMetadata_Normal(
						LocalPosition.X.GetData(),
						LocalPosition.Y.GetData(),
						LocalPosition.Z.GetData(),
						ResultBuffer.GetData(),
						LocalPosition.Num(),
						Scale,
						GetISPCValue(Mesh->MeshData->Size),
						Mesh->MeshData->VoxelSize,
						GetISPCValue(Mesh->MeshData->Origin),
						Mesh->MeshData->PointIndices.GetData(),
						MeshBuffer->GetData());
					continue;
				}

				ensure(false);
			}
		}

		if (DistancePin.ShouldCompute())
		{
			DistancePin.Set(Query, Result);
		}

		for (const FMetadataPin& MetadataPin : MetadataPins)
		{
			const FMetadata* Metadata = MetadataToBuffers.Find(MetadataPin.MetadataRef);
			if (!Metadata)
			{
				continue;
			}

			MetadataPin.PinRef.Set(Query, FVoxelRuntimePinValue::Make(Metadata->Result));
		}
	};
}