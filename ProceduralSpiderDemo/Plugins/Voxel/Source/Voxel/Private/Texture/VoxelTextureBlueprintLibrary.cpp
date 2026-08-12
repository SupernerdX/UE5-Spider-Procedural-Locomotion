// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Texture/VoxelTextureBlueprintLibrary.h"
#include "Texture/VoxelTexture.h"
#include "Texture/VoxelTextureData.h"
#include "RHIGPUReadback.h"
#include "TextureResource.h"
#include "Engine/TextureRenderTarget2D.h"

TVoxelFuture<UVoxelTexture*> UVoxelTextureBlueprintLibrary::CreateVoxelTextureFromRenderTarget(UTextureRenderTarget2D* RenderTarget)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!RenderTarget)
	{
		VOXEL_MESSAGE(Error, "RenderTarget is null");
		return {};
	}

	const FTextureRenderTargetResource* Resource = RenderTarget->GameThread_GetRenderTargetResource();
	if (!Resource)
	{
		VOXEL_MESSAGE(Error, "RenderTarget resource is null");
		return {};
	}

	return Voxel::RenderTask([Resource](FRHICommandListImmediate& RHICmdList) -> TVoxelFuture<UVoxelTexture*>
	{
		const FTextureRHIRef TextureRHI = Resource->GetShaderResourceTexture();
		if (!TextureRHI)
		{
			VOXEL_MESSAGE(Error, "Failed to read render target: TextureRHI is null");
			return {};
		}

		const int32 SizeX = TextureRHI->GetSizeX();
		const int32 SizeY = TextureRHI->GetSizeY();
		const EPixelFormat Format = TextureRHI->GetFormat();
		const int32 BlockBytes = GPixelFormats[Format].BlockBytes;

		TVoxelArray<uint8> Bytes;
		FVoxelUtilities::SetNumFast(Bytes, SizeX * SizeY * BlockBytes);

		{
			VOXEL_SCOPE_COUNTER("Copy data");

			RHICmdList.Transition(FRHITransitionInfo(TextureRHI, ERHIAccess::SRVMask, ERHIAccess::CopySrc));

			FRHIGPUTextureReadback Readback("CreateVoxelTextureFromRenderTarget");

			Readback.EnqueueCopy(
				RHICmdList,
				TextureRHI,
				FIntVector(0, 0, 0),
				0,
				TextureRHI->GetSizeXYZ());

			// Sync the GPU. Unfortunately we can't use the fences because not all RHIs implement them yet.
			RHICmdList.BlockUntilGPUIdle();
			RHICmdList.FlushResources();

			int32 RowPitchInPixels = 0;
			const uint8* LockedData = static_cast<const uint8*>(Readback.Lock(RowPitchInPixels));

			const TConstVoxelArrayView<uint8> SourceData(LockedData, SizeY * RowPitchInPixels * BlockBytes);

			for (int32 Y = 0; Y < SizeY; Y++)
			{
				FVoxelUtilities::Memcpy(
					Bytes.View().Slice(Y * SizeX * BlockBytes, SizeX * BlockBytes),
					SourceData.Slice(Y * RowPitchInPixels * BlockBytes, SizeX * BlockBytes));
			}

			Readback.Unlock();

			RHICmdList.Transition(FRHITransitionInfo(TextureRHI, ERHIAccess::CopySrc, ERHIAccess::SRVMask));
		}

		const ETextureSourceFormat SourceFormat = INLINE_LAMBDA
		{
			switch (Format)
			{
			case PF_R8: return TSF_G8;
			case PF_B8G8R8A8: return TSF_BGRA8;
			case PF_FloatRGBA: return TSF_RGBA16F;
			case PF_G16: return TSF_G16;
			case PF_A32B32G32R32F: return TSF_RGBA32F;
			case PF_R16F: return TSF_R16F;
			case PF_R32_FLOAT: return TSF_R32F;
			default:
			{
				ensureVoxelSlow(false);
				VOXEL_MESSAGE(Error, "Unsupported format for CreateVoxelTextureFromRenderTarget: {0}", GPixelFormats[Format].Name);
				return TSF_Invalid;
			}
			}
		};

		if (SourceFormat == TSF_Invalid)
		{
			return {};
		}

		const TSharedPtr<FVoxelBuffer> Buffer = FVoxelTextureData::CreateBufferFromTexture(
			Bytes,
			SourceFormat,
			SizeX,
			SizeY);

		if (!ensureVoxelSlow(Buffer))
		{
			return {};
		}

		const TSharedRef<const FVoxelTextureData> TextureData = FVoxelTextureData::CreateFromBuffer(
			SizeX,
			SizeY,
			Buffer.ToSharedRef());

		return Voxel::GameTask([=]
		{
			return UVoxelTexture::Create(TextureData);
		});
	});
}

FVoxelFuture UVoxelTextureBlueprintLibrary::K2_CreateVoxelTextureFromRenderTarget(UVoxelTexture*& Texture, UTextureRenderTarget2D* RenderTarget)
{
	Texture = nullptr;

	return CreateVoxelTextureFromRenderTarget(RenderTarget).Then_GameThread([&Texture](UVoxelTexture* NewTexture)
	{
		Texture = NewTexture;
	});
}