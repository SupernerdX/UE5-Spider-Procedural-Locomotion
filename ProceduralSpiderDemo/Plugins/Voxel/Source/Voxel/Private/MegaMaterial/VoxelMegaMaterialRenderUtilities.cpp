// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelMegaMaterialRenderUtilities.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "VoxelMesh.h"
#include "VoxelMetadataMaterialType.h"
#include "Render/VoxelTexturePool.h"
#include "Render/VoxelTextureManager.h"

TSharedRef<const FVoxelMegaMaterialRenderData> FVoxelMegaMaterialRenderUtilities::BuildRenderData(
	const FVoxelMegaMaterialProxy& MegaMaterialProxy,
	const TSharedRef<FVoxelTextureManager>& TextureManager,
	const FVoxelMesh& Mesh)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<int32, TSharedRef<FVoxelTexturePoolRef>> IndexToTextureRef;

	{
		TVoxelArray<FVoxelRenderMaterial> RenderMaterials = MegaMaterialProxy.GetRenderMaterials(Mesh.SurfaceTypes);

		const TSharedRef<FVoxelTexturePoolRef> TextureRef = TextureManager->GetMaterialBufferPool().Upload_AnyThread(MoveTemp(RenderMaterials));

		IndexToTextureRef.Add_EnsureNew(0, TextureRef);
	}

	for (const auto& It : Mesh.MetadataToBuffer)
	{
		const FVoxelMetadataRef Metadata = It.Key;
		VOXEL_SCOPE_COUNTER_FNAME(Metadata.GetFName());

		const TVoxelOptional<EVoxelMetadataMaterialType> OptionalMaterialType = Metadata.GetMaterialType();
		if (!ensure(OptionalMaterialType))
		{
			continue;
		}
		const EVoxelMetadataMaterialType MaterialType = *OptionalMaterialType;

		TVoxelArray<uint8> Bytes;
		FVoxelUtilities::SetNumFast(Bytes, Mesh.Vertices.Num() * FVoxelMetadataMaterialType::GetTypeSize(MaterialType));

		Metadata.WriteMaterialData(*It.Value, Bytes, MaterialType);

		if (MaterialType == EVoxelMetadataMaterialType::Float32_3 ||
			MaterialType == EVoxelMetadataMaterialType::Int3)
		{
			VOXEL_SCOPE_COUNTER("Add component");

			TVoxelArray<uint8> NewBytes;
			FVoxelUtilities::SetNumFast(NewBytes, Mesh.Vertices.Num() * sizeof(FVector4f));

			for (int32 Index = 0; Index < Mesh.Vertices.Num(); Index++)
			{
				NewBytes.View<FVector4f>()[Index] = FVector4f(Bytes.View<FVector3f>()[Index], 0.f);
			}

			Bytes = MoveTemp(NewBytes);
		}

		const TSharedPtr<FVoxelTexturePool> BufferPool = TextureManager->GetMetadataBufferPool(Metadata);
		if (!ensure(BufferPool) ||
			!ensureVoxelSlow(BufferPool->PixelFormat == FVoxelMetadataMaterialType::GetPixelFormat(MaterialType)))
		{
			continue;
		}

		const int32 MetadataIndex = MegaMaterialProxy.GetMetadataIndexToMetadata().Find(Metadata);
		if (!ensure(MetadataIndex != -1))
		{
			continue;
		}


		IndexToTextureRef.Add_EnsureNew(
			MetadataIndex + 1,
			BufferPool->Upload_AnyThread(MoveTemp(Bytes)));
	}

	const TConstVoxelArrayView<FVoxelMetadataRef> Metadatas = MegaMaterialProxy.GetMetadataIndexToMetadata();

	const int32 NumAttributes = 1 + Metadatas.Num();

	TVoxelArray<int32> AttributeIndices;
	FVoxelUtilities::SetNumFast(AttributeIndices, NumAttributes);
	FVoxelUtilities::SetAll(AttributeIndices, -1);

	const TSharedRef<FVoxelMegaMaterialRenderData> RenderData = MakeShared<FVoxelMegaMaterialRenderData>();
	RenderData->TextureManager = TextureManager;

	for (const auto& It : IndexToTextureRef)
	{
		RenderData->TextureRefs.Add(It.Value);

		const int64 Index = It.Value->GetIndex();
		checkVoxelSlow(Index <= MAX_int32);
		AttributeIndices[It.Key] = int32(Index);
	}

	const TSharedRef<FVoxelTexturePoolRef> TextureRef = TextureManager->GetChunkIndicesBufferPool().Upload_AnyThread(MoveTemp(AttributeIndices));
	// Upload.Future will be waited on implicitly by the task context

	RenderData->TextureRefs.Add(TextureRef);
	RenderData->ChunkIndicesIndex = TextureRef->GetIndex();
	return RenderData;
}