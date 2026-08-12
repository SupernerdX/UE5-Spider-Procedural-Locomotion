// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelTextureManager.h"
#include "Render/VoxelTexturePool.h"
#include "VoxelMetadataMaterialType.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "MegaMaterial/VoxelRenderMaterial.h"

#include "Engine/Texture2D.h"
#include "Engine/Texture2DArray.h"
#include "TextureResource.h"
#include "Materials/MaterialInstanceDynamic.h"

FVoxelTextureManager::FVoxelTextureManager(const FVoxelMegaMaterialProxy& MegaMaterialProxy)
	: MetadataIndexToMetadata(MegaMaterialProxy.GetMetadataIndexToMetadata())
{
	VOXEL_FUNCTION_COUNTER();

	ChunkIndicesBufferPool = MakeShared<FVoxelTexturePool>(
		sizeof(int32),
		PF_R32_SINT,
		TEXT("Voxel.ChunkIndices"));

	MaterialBufferPool = MakeShared<FVoxelTexturePool>(
		sizeof(FVoxelRenderMaterial),
		PF_R32G32B32A32_UINT,
		TEXT("Voxel.Materials"));

	for (const FVoxelMetadataRef& Metadata : MetadataIndexToMetadata)
	{
		const TVoxelOptional<EVoxelMetadataMaterialType> MaterialType = Metadata.GetMaterialType();
		if (!ensure(MaterialType))
		{
			continue;
		}

		const int32 TypeSize = FVoxelMetadataMaterialType::GetTypeSize(*MaterialType);
		const EPixelFormat PixelFormat = FVoxelMetadataMaterialType::GetPixelFormat(*MaterialType);

		const TSharedRef<FVoxelTexturePool> BufferPool = MakeShared<FVoxelTexturePool>(
			TypeSize,
			PixelFormat,
			"Voxel.Metadata." + Metadata.GetName());

		MetadataToBufferPool.Add_EnsureNew(Metadata, BufferPool);
	}

	// Copy cubic texture arrays from proxy
	for (const auto& It : MegaMaterialProxy.GetCubicTextureArrays())
	{
		CubicTextureArrays.Add_EnsureNew(It.Key, It.Value);
	}

	// Build cubic slice lookup texture if we have cubic surfaces
	const TVoxelMap<FVoxelMaterialRenderIndex, int32>& RenderIndexToSlice = MegaMaterialProxy.GetRenderIndexToCubicSliceIndex();
	if (RenderIndexToSlice.Num() > 0)
	{
		// Create lookup data - 256 entries, -1 means not a cubic surface
		TVoxelArray<int32> LookupData;
		FVoxelUtilities::SetNumFast(LookupData, 256);
		FVoxelUtilities::SetAll(LookupData, -1);

		for (const auto& It : RenderIndexToSlice)
		{
			if (It.Key.Index < 256)
			{
				LookupData[It.Key.Index] = It.Value;
			}
		}

		// Create the lookup texture
		UTexture2D* LookupTexture = UTexture2D::CreateTransient(256, 1, PF_R32_SINT, "CubicSliceLookup");
		CubicSliceLookupTexture = LookupTexture;

		if (LookupTexture)
		{
			LookupTexture->Filter = TF_Nearest;
			LookupTexture->AddressX = TA_Clamp;
			LookupTexture->AddressY = TA_Clamp;
			LookupTexture->SRGB = false;

			// Lock and copy data
			FTexture2DMipMap& Mip = LookupTexture->GetPlatformData()->Mips[0];
			void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(Data, LookupData.GetData(), 256 * sizeof(int32));
			Mip.BulkData.Unlock();

			LookupTexture->UpdateResource();
		}
	}
}

void FVoxelTextureManager::UpdateInstance(UMaterialInstanceDynamic& Instance) const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (UTexture2D* Texture = ChunkIndicesBufferPool->GetTexture_GameThread())
	{
		Instance.SetTextureParameterValue(
			STATIC_FNAME("VOXEL_ChunkIndices_Texture"),
			Texture);

		Instance.SetScalarParameterValue(
			STATIC_FNAME("VOXEL_ChunkIndices_TextureSizeLog2"),
			FVoxelUtilities::ExactLog2(Texture->GetSizeX()));
	}

	if (UTexture2D* Texture = MaterialBufferPool->GetTexture_GameThread())
	{
		Instance.SetTextureParameterValue(
			STATIC_FNAME("VOXEL_Materials_Texture"),
			Texture);

		Instance.SetScalarParameterValue(
			STATIC_FNAME("VOXEL_Materials_TextureSizeLog2"),
			FVoxelUtilities::ExactLog2(Texture->GetSizeX()));
	}

	for (int32 Index = 0; Index < MetadataIndexToMetadata.Num(); Index++)
	{
		const TSharedPtr<FVoxelTexturePool> BufferPool = MetadataToBufferPool.FindRef(MetadataIndexToMetadata[Index]);
		if (!BufferPool)
		{
			continue;
		}

		UTexture2D* Texture = BufferPool->GetTexture_GameThread();
		if (!Texture)
		{
			continue;
		}

		Instance.SetTextureParameterValue(
			FName(STATIC_FNAME("VOXEL_Metadata_Texture"), Index + 1),
			Texture);

		Instance.SetScalarParameterValue(
			FName(STATIC_FNAME("VOXEL_Metadata_TextureSizeLog2"), Index + 1),
			FVoxelUtilities::ExactLog2(Texture->GetSizeX()));
	}

	// Bind cubic texture arrays - property name matches material parameter name
	for (const auto& It : CubicTextureArrays)
	{
		if (UTexture2DArray* TextureArray = It.Value.Resolve())
		{
			Instance.SetTextureParameterValue(It.Key, TextureArray);
		}
	}

	// Bind cubic slice lookup texture
	if (UTexture2D* LookupTexture = CubicSliceLookupTexture.Resolve())
	{
		Instance.SetTextureParameterValue(
			STATIC_FNAME("VOXEL_CubicSliceLookup"),
			LookupTexture);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelTextureManager::ProcessUploads()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	ChunkIndicesBufferPool->ProcessUploads();
	MaterialBufferPool->ProcessUploads();

	for (const auto& It : MetadataToBufferPool)
	{
		It.Value->ProcessUploads();
	}
}

void FVoxelTextureManager::AddReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_FUNCTION_COUNTER();

	ChunkIndicesBufferPool->AddReferencedObjects(Collector);
	MaterialBufferPool->AddReferencedObjects(Collector);

	for (const auto& It : MetadataToBufferPool)
	{
		It.Value->AddReferencedObjects(Collector);
	}

	// Keep cubic textures alive
	for (auto& It : CubicTextureArrays)
	{
		TObjectPtr<UTexture2DArray> TextureArray = It.Value.Resolve();
		Collector.AddReferencedObject(TextureArray);
	}

	{
		TObjectPtr<UTexture2D> LookupTexture = CubicSliceLookupTexture.Resolve();
		Collector.AddReferencedObject(LookupTexture);
	}
}