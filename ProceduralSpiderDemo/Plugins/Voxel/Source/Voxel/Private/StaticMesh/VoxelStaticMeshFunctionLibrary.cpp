// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "StaticMesh/VoxelStaticMeshFunctionLibrary.h"
#include "StaticMesh/VoxelStaticMeshData.h"
#include "VoxelStaticMeshFunctionLibraryImpl.ispc.generated.h"

void FVoxelStaticMeshRefPinType::Convert(
	const bool bSetObject,
	TVoxelObjectPtr<UVoxelStaticMesh>& OutObject,
	UVoxelStaticMesh& InObject,
	FVoxelStaticMeshRef& Struct)
{
	if (bSetObject)
	{
		OutObject = Struct.Object;
	}
	else
	{
		Struct.Object = InObject;
		Struct.MeshData = InObject.GetData();

		for (const FVoxelStaticMeshMetadata& Metadata : InObject.Metadatas)
		{
			if (!Metadata.Metadata ||
				!Metadata.Data.Buffer ||
				!ensure(Metadata.Data.Buffer->Num_Slow() == Struct.MeshData->Points.Num()) ||
				!ensure(Metadata.Data.Buffer->GetInnerType() == Metadata.Metadata->GetInnerType()))
			{
				continue;
			}

			const FVoxelMetadataRef MetadataRef(Metadata.Metadata);

			if (Struct.MetadataRefToBuffer.Contains(MetadataRef))
			{
				continue;
			}

			Struct.MetadataRefToBuffer.Add_EnsureNew(MetadataRef, Metadata.Data.Buffer);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox UVoxelStaticMeshFunctionLibrary::GetVoxelStaticMeshBounds(
	const FVoxelStaticMeshRef& Mesh,
	const float Scale) const
{
	if (!Mesh.MeshData)
	{
		VOXEL_MESSAGE(Error, "{0}: Mesh is null", this);
		return {};
	}

	return Mesh.MeshData->MeshBounds.Extend(1).Scale(Mesh.MeshData->VoxelSize).Scale(Scale);
}