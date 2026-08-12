// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelTexturePool.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelTexturePoolRef);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTexturePoolRef::FVoxelTexturePoolRef(
	FVoxelTexturePool& Pool,
	const FVoxelAllocation& Allocation)
	: WeakPool(Pool.AsWeak())
	, Allocation(Allocation)
{
	Pool.UsedMemory.Add(Allocation.Num * Pool.BytesPerElement);
	Pool.PaddingMemory.Add(Allocation.Padding * Pool.BytesPerElement);
}

FVoxelTexturePoolRef::~FVoxelTexturePoolRef()
{
	const TSharedPtr<FVoxelTexturePool> Pool = WeakPool.Pin();
	if (!Pool)
	{
		return;
	}

	Pool->UsedMemory.Subtract(Allocation.Num * Pool->BytesPerElement);
	Pool->PaddingMemory.Subtract(Allocation.Padding * Pool->BytesPerElement);

	Pool->Allocator.Free(Allocation);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTexturePool::FVoxelTexturePool(
	const int32 BytesPerElement,
	const EPixelFormat PixelFormat,
	const FString& Name)
	: BytesPerElement(BytesPerElement)
	, PixelFormat(PixelFormat)
	, Name(Name)
	, AllocatedMemory_Name(Name + " Allocated Memory")
	, UsedMemory_Name(Name + " Used Memory")
	, PaddingMemory_Name(Name + " Padding Memory")
{
	ensure(BytesPerElement % GPixelFormats[PixelFormat].BlockBytes == 0);
}

FVoxelTexturePool::~FVoxelTexturePool()
{
	Voxel_AddAmountToDynamicMemoryStat(AllocatedMemory_Name, -AllocatedMemory_Reported.Get());
	Voxel_AddAmountToDynamicMemoryStat(UsedMemory_Name, -UsedMemory_Reported.Get());
	Voxel_AddAmountToDynamicMemoryStat(PaddingMemory_Name, -PaddingMemory_Reported.Get());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelTexturePool::UpdateStats()
{
	const int64 AllocatedMemoryNew = AllocatedMemory.Get();
	const int64 UsedMemoryNew = UsedMemory.Get();
	const int64 PaddingMemoryNew = PaddingMemory.Get();

	const int64 AllocatedMemoryOld = AllocatedMemory_Reported.Set_ReturnOld(AllocatedMemoryNew);
	const int64 UsedMemoryOld = UsedMemory_Reported.Set_ReturnOld(UsedMemoryNew);
	const int64 PaddingMemoryOld = PaddingMemory_Reported.Set_ReturnOld(PaddingMemoryNew);

	Voxel_AddAmountToDynamicMemoryStat(AllocatedMemory_Name, AllocatedMemoryNew - AllocatedMemoryOld);
	Voxel_AddAmountToDynamicMemoryStat(UsedMemory_Name, UsedMemoryNew - UsedMemoryOld);
	Voxel_AddAmountToDynamicMemoryStat(PaddingMemory_Name, PaddingMemoryNew - PaddingMemoryOld);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelTexturePoolRef> FVoxelTexturePool::Upload_AnyThread(
	const FSharedVoidPtr& Owner,
	const TConstVoxelArrayView64<uint8> Data)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(Data.Num() > 0);
	checkVoxelSlow(Data.Num() % BytesPerElement == 0);

	const int64 Num = Data.Num() / BytesPerElement;

	const FVoxelAllocation Allocation = Allocator.Allocate(Num);

	const TSharedRef<FVoxelTexturePoolRef> TextureRef = MakeShared<FVoxelTexturePoolRef>(
		*this,
		Allocation);

	{
		VOXEL_SCOPE_LOCK(CriticalSection);

		Uploads_RequiresLock.Add(FUpload
		{
			Owner,
			Data,
			TextureRef
		});
	}

	return TextureRef;
}

TSharedRef<FVoxelTexturePoolRef> FVoxelTexturePool::Upload_AnyThread(TVoxelArray<uint8> Data)
{
	const TSharedRef<TVoxelArray<uint8>> SharedData = MakeSharedCopy(MoveTemp(Data));

	return this->Upload_AnyThread(
		MakeSharedVoidRef(SharedData),
		*SharedData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelTexturePool::ProcessUploads()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	ON_SCOPE_EXIT
	{
		UpdateStats();
	};

	TVoxelArray<FUpload> Uploads;
	{
		VOXEL_SCOPE_LOCK(CriticalSection);

		Uploads = MoveTemp(Uploads_RequiresLock);
	}

	if (Uploads.Num() == 0)
	{
		return;
	}

	const int64 Max = Allocator.GetMax();
	if (Max > FMath::Square(MaxTextureSize))
	{
		VOXEL_MESSAGE(Error, "Out of memory: {0}", Name);
		return;
	}

	UTexture2D* OldTexture = Texture;
	{
		// Do this after dequeuing all copies to make sure we allocate a big enough buffer for them
		const int32 Size = FMath::Max<int32>(1024, FMath::RoundUpToPowerOfTwo(FMath::CeilToInt(FMath::Sqrt(double(Max)))));

		AllocatedMemory.Set(FMath::Square<int64>(Size) * BytesPerElement);

		if (!Texture ||
			Texture->GetSizeX() != Size)
		{
			Texture = FVoxelTextureUtilities::CreateTexture2D(
				FName(Name + FString("_Texture")),
				Size,
				Size,
				false,
				TF_Default,
				PixelFormat);

			FVoxelTextureUtilities::RemoveBulkData(Texture);
		}
	}

	{
		UTexture2D* NewTexture = Texture;
		if (!ensure(NewTexture))
		{
			return;
		}

		if (OldTexture &&
			OldTexture != NewTexture &&
			ensure(NewTexture->GetSizeX() % OldTexture->GetSizeX() == 0) &&
			ensure(NewTexture->GetSizeY() % OldTexture->GetSizeY() == 0))
		{
			FTextureResource* OldResource = OldTexture->GetResource();
			FTextureResource* NewResource = NewTexture->GetResource();

			const int32 Scale = NewTexture->GetSizeX() / OldTexture->GetSizeX();
			ensure(Scale == NewTexture->GetSizeY() / OldTexture->GetSizeY());
			ensure(Scale > 1);
			ensure(FMath::IsPowerOfTwo(Scale));

			Voxel::RenderTask([OldResource, NewResource, Scale](FRHICommandListImmediate& RHICmdList)
			{
				VOXEL_SCOPE_COUNTER("FVoxelTextureBufferPool Reallocate");

				const int32 SizeX = OldResource->GetSizeX();
				const int32 SizeY = OldResource->GetSizeY();

				for (int32 Row = 0; Row < SizeY; Row++)
				{
					FRHICopyTextureInfo CopyInfo;
					CopyInfo.Size = FIntVector(SizeX, 1, 1);
					CopyInfo.SourcePosition.X = 0;
					CopyInfo.SourcePosition.Y = Row;
					CopyInfo.DestPosition.X = (Row % Scale) * SizeX;
					CopyInfo.DestPosition.Y = Row / Scale;

					RHICmdList.CopyTexture(
						OldResource->GetTextureRHI(),
						NewResource->GetTextureRHI(),
						CopyInfo);
				}
			});
		}
	}
	check(Texture);

	FTextureResource* Resource = Texture->GetResource();
	if (!ensure(Resource))
	{
		return;
	}

	Voxel::RenderTask(MakeWeakPtrLambda(this, [this, Resource, Uploads = MoveTemp(Uploads)]
	{
		VOXEL_FUNCTION_COUNTER();

		FRHITexture* TextureRHI = Resource->GetTexture2DRHI();
		if (!ensure(TextureRHI))
		{
			return;
		}

		TextureRHI_RenderThread = TextureRHI;

		const int32 TextureSize = TextureRHI->GetSizeX();

		for (const FUpload& Upload : Uploads)
		{
			int64 Offset = Upload.TextureRef->GetIndex();
			TConstVoxelArrayView64<uint8> Data = Upload.Data;

			while (Data.Num() > 0)
			{
				checkVoxelSlow(Data.Num() % BytesPerElement == 0);

				const int64 NumToCopy = FMath::Min(Data.Num() / BytesPerElement, TextureSize - (Offset % TextureSize));

				const FUpdateTextureRegion2D UpdateRegion(
					Offset % TextureSize,
					Offset / TextureSize,
					0,
					0,
					NumToCopy,
					1);

				RHIUpdateTexture2D_Safe(
					TextureRHI,
					0,
					UpdateRegion,
					NumToCopy * BytesPerElement,
					Data.LeftOf(NumToCopy * BytesPerElement));

				Offset += NumToCopy;
				Data = Data.RightOf(NumToCopy * BytesPerElement);
			}
		}
	}));
}

void FVoxelTexturePool::AddReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_FUNCTION_COUNTER();

	Collector.AddReferencedObject(Texture);
}