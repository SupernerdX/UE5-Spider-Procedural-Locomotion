// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelAllocator.h"
#include "VoxelMinimal.h"

class FVoxelTexturePool;

class FVoxelTexturePoolRef
{
public:
	FVoxelTexturePoolRef(
		FVoxelTexturePool& Pool,
		const FVoxelAllocation& Allocation);
	~FVoxelTexturePoolRef();
	UE_NONCOPYABLE(FVoxelTexturePoolRef);

	VOXEL_COUNT_INSTANCES();

	FORCEINLINE int64 Num() const
	{
		return Allocation.Num;
	}
	FORCEINLINE int64 GetIndex() const
	{
		return Allocation.Index;
	}

private:
	const TWeakPtr<FVoxelTexturePool> WeakPool;
	const FVoxelAllocation Allocation;
};

class FVoxelTexturePool : public TSharedFromThis<FVoxelTexturePool>
{
public:
	const int32 BytesPerElement;
	const EPixelFormat PixelFormat;
	const FString Name;
	const int64 MaxTextureSize = GMaxTextureDimensions;

	FVoxelTexturePool(
		int32 BytesPerElement,
		EPixelFormat PixelFormat,
		const FString& Name);

	virtual ~FVoxelTexturePool();

	void ProcessUploads();
	void AddReferencedObjects(FReferenceCollector& Collector);

public:
	FORCEINLINE UTexture2D* GetTexture_GameThread() const
	{
		checkVoxelSlow(IsInGameThread());
		return Texture;
	}
	FORCEINLINE FTextureRHIRef GetTextureRHI_RenderThread() const
	{
		checkVoxelSlow(IsInParallelRenderingThread());
		return TextureRHI_RenderThread;
	}

private:
	const FName AllocatedMemory_Name;
	const FName UsedMemory_Name;
	const FName PaddingMemory_Name;

	FVoxelCounter64 AllocatedMemory;
	FVoxelCounter64 UsedMemory;
	FVoxelCounter64 PaddingMemory;

	FVoxelCounter64 AllocatedMemory_Reported;
	FVoxelCounter64 UsedMemory_Reported;
	FVoxelCounter64 PaddingMemory_Reported;

	void UpdateStats();

public:
	TSharedRef<FVoxelTexturePoolRef> Upload_AnyThread(
		const FSharedVoidPtr& Owner,
		TConstVoxelArrayView64<uint8> Data);

	TSharedRef<FVoxelTexturePoolRef> Upload_AnyThread(TVoxelArray<uint8> Data);

	template<typename T>
	TSharedRef<FVoxelTexturePoolRef> Upload_AnyThread(TVoxelArray<T> Data)
	{
		check(sizeof(T) == BytesPerElement);

		const TSharedRef<TVoxelArray<T>> SharedData = MakeSharedCopy(MoveTemp(Data));

		return this->Upload_AnyThread(
			MakeSharedVoidRef(SharedData),
			SharedData->template View<uint8>());
	}

private:
	FVoxelAllocator Allocator{ int64(1) << 32 };

	TObjectPtr<UTexture2D> Texture;
	FTextureRHIRef TextureRHI_RenderThread;

	struct FUpload
	{
		FSharedVoidPtr Owner;
		TConstVoxelArrayView64<uint8> Data;
		TSharedPtr<FVoxelTexturePoolRef> TextureRef;
	};
	FVoxelCriticalSection CriticalSection;
	TVoxelArray<FUpload> Uploads_RequiresLock;

	friend FVoxelTexturePoolRef;
};