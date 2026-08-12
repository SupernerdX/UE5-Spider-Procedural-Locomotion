// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelHeightSculptCanvas.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Sculpt/Height/VoxelHeightChunks.h"
#include "Sculpt/Height/VoxelHeightChunkData.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"
#include "Sculpt/Height/VoxelSculptHeightCache.h"
#include "Sculpt/Height/VoxelHeightHeightChunk.h"
#include "Sculpt/Height/VoxelSculptHeightContext.h"
#include "Sculpt/Height/VoxelHeightChunkSamplers.h"
#include "Sculpt/Height/VoxelSculptHeightTreeBuilder.h"
#include "Sculpt/Height/VoxelHeightChunkTreeIterator.h"
#include "Sculpt/Height/VoxelHeightSculptPreviousHeightProvider.h"

TVoxelFuture<FVoxelHeightSculptCanvas> FVoxelHeightSculptCanvas::CreateCanvas(
	const TSharedRef<IVoxelBulkLoader>& BulkLoader,
	const TSharedRef<const FVoxelSculptHeightData>& SculptData,
	const FVoxelSculptHeightContext& Context,
	const FVoxelIntBox2D& ChunkKeyBounds,
	const FVoxelBulkHash& RootHash)
{
	VOXEL_FUNCTION_COUNTER();

	return
		SculptData->LoadNearChunks(BulkLoader, ChunkKeyBounds, RootHash)
		.Then_AsyncThread([=](const TVoxelMap<FIntPoint, TSharedPtr<const FVoxelHeightNearChunk>>& KeyToNearChunk)
		{
			return MakeShared<FVoxelHeightSculptCanvas>(
				ChunkKeyBounds,
				SculptData,
				Context,
				KeyToNearChunk);
		});
}

FVoxelHeightSculptCanvas::FVoxelHeightSculptCanvas(
	const FVoxelIntBox2D& ChunkKeyBounds,
	const TSharedRef<const FVoxelSculptHeightData>& PreviousSculptData,
	const FVoxelSculptHeightContext& Context,
	const TVoxelMap<FIntPoint, TSharedPtr<const FVoxelHeightNearChunk>>& KeyToPreviousChunk)
	: SizeInChunks(ChunkKeyBounds.Size())
	, Size(SizeInChunks * ChunkSize)
	, NumVoxels(Size.X * Size.Y)
	, ChunkKeyOffset(ChunkKeyBounds.Min)
	, PreviousSculptData(PreviousSculptData)
	, PreviousHeightProvider(Context.PreviousHeightProvider)
	, KeyToPreviousChunk(KeyToPreviousChunk)
	, ScaleZ(Context.ScaleZ)
	, OffsetZ(Context.OffsetZ)
	, bRelativeHeights(Context.bStoreRelativeHeights)
{
	VOXEL_FUNCTION_COUNTER_NUM(NumVoxels);

	for (const auto& It : KeyToPreviousChunk)
	{
		checkVoxelSlow(FVoxelIntBox2D(0, SizeInChunks).ShiftBy(ChunkKeyOffset).Contains(It.Key));
	}

	FVoxelUtilities::SetNumFast(Heights, NumVoxels);

	{
		VOXEL_SCOPE_COUNTER("Query heights");

		FVoxelIntBox2D(0, SizeInChunks).ParallelIterate([&](const FIntPoint& ChunkKey)
		{
			QueryChunkHeights(ChunkKey);
		});
	}

	QuerySurfaceTypesMetadatas();
}

TSharedRef<const FVoxelSculptHeightData> FVoxelHeightSculptCanvas::Finalize(const float TargetPrecision)
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(TVoxelSet<FVoxelMetadataRef>(MetadataRefsToWrite).Num() == MetadataRefsToWrite.Num());

	// Only write in the inner part
	const FVoxelIntBox2D ChunkKeysToWrite = FVoxelIntBox2D(1, SizeInChunks - 1);
	ensure(ChunkKeysToWrite.IsValid());

	TVoxelMap<FIntPoint, FVoxelHeightChunkData> KeyToChunkData;
	{
		KeyToChunkData.Reserve(SizeInChunks.X * SizeInChunks.Y);

		ChunkKeysToWrite.ShiftBy(ChunkKeyOffset).Iterate([&](const FIntPoint& ChunkKey)
		{
			FVoxelHeightChunkData ChunkData;

			if (const FVoxelHeightNearChunk* PreviousChunk = KeyToPreviousChunk.FindSmartPtr(ChunkKey))
			{
				ChunkData = PreviousChunk->ChunkData;
			}

			KeyToChunkData.Add_EnsureNew(ChunkKey, ChunkData);
		});
	}

	if (bWriteHeights)
	{
		VOXEL_SCOPE_COUNTER("Heights");

		if (bRelativeHeights)
		{
			VOXEL_SCOPE_COUNTER("Relative");

			TVoxelArray<float> PreviousHeights = QueryPreviousHeights();
			Voxel::ParallelFor(Heights.Num(), [&](const int32 Index)
			{
				const float PreviousHeight = PreviousHeights[Index];

				if (FVoxelUtilities::IsNaN(PreviousHeight))
				{
					return;
				}

				float& Height = Heights[Index];
				if (PreviousHeight == Height)
				{
					Height = FVoxelUtilities::NaNf();
					return;
				}

				Height -= PreviousHeight;
				Height = (Height - OffsetZ) / ScaleZ;
			});
		}
		else
		{
			Voxel::ParallelFor(Heights.Num(), [&](const int32 Index)
			{
				float& Height = Heights[Index];

				if (FVoxelUtilities::IsNaN(Height))
				{
					return;
				}

				Height = (Height - OffsetZ) / ScaleZ;
			});
		}

		ChunkKeysToWrite.ParallelIterate([&](const FIntPoint& ChunkKey)
		{
			KeyToChunkData[ChunkKeyOffset + ChunkKey].HeightChunk = FVoxelHeightHeightChunk::CreatePacked(
				Heights,
				ChunkKey * ChunkSize,
				Size,
				TargetPrecision,
				bRelativeHeights);
		});
	}

	if (bWriteSurfaceTypes &&
		SurfaceTypes)
	{
		VOXEL_SCOPE_COUNTER("Surface types");

		ChunkKeysToWrite.ParallelIterate([&](const FIntPoint& ChunkKey)
		{
			KeyToChunkData[ChunkKeyOffset + ChunkKey].SurfaceTypeChunk = FVoxelHeightSurfaceTypeChunk::Create(
				SurfaceTypes->Alphas,
				SurfaceTypes->Blends,
				ChunkKey * ChunkSize,
				Size);
		});
	}

	for (const FVoxelMetadataRef& MetadataRef : MetadataRefsToWrite)
	{
		const FMetadata* Metadata = MetadataRefToMetadata.Find(MetadataRef);
		if (!Metadata)
		{
			continue;
		}

		VOXEL_SCOPE_COUNTER_FORMAT("Metadata: %s", *MetadataRef.GetName());

		ChunkKeysToWrite.ParallelIterate([&](const FIntPoint& ChunkKey)
		{
			TVoxelArray<FVoxelHeightChunkData::FMetadataChunk>& MetadataChunks = KeyToChunkData[ChunkKeyOffset + ChunkKey].MetadataChunks;

			const TVoxelRefCountPtr<FVoxelHeightMetadataChunk> Chunk = FVoxelHeightMetadataChunk::Create(
				Metadata->Alphas,
				*Metadata->Buffer,
				ChunkKey * ChunkSize,
				Size);

			if (!Chunk)
			{
				MetadataChunks.RemoveAll([&](const FVoxelHeightChunkData::FMetadataChunk& MetadataChunk)
				{
					return MetadataChunk.MetadataRef == MetadataRef;
				});

				return;
			}

			for (FVoxelHeightChunkData::FMetadataChunk& MetadataChunk : MetadataChunks)
			{
				if (MetadataChunk.MetadataRef == MetadataRef)
				{
					MetadataChunk.Chunk = Chunk;
					return;
				}
			}

			MetadataChunks.Add(FVoxelHeightChunkData::FMetadataChunk
			{
				MetadataRef,
				Chunk
			});
		});
	}

	TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>> NewKeyToFarChunk = FVoxelSculptHeightTreeBuilder::Build(
		PreviousSculptData->GetKeyToFarChunk(),
		FVoxelIntBox2D(ChunkSize, Size - ChunkSize).ShiftBy(ChunkKeyOffset * ChunkSize),
		KeyToChunkData,
		TargetPrecision);

	return FVoxelSculptHeightData::Create(MoveTemp(NewKeyToFarChunk));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightSculptCanvas::QueryChunkHeights(const FIntPoint& ChunkKey)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelHeightHeightChunk* SculptChunk = INLINE_LAMBDA -> const FVoxelHeightHeightChunk*
	{
		const FVoxelHeightNearChunk* PreviousChunk = KeyToPreviousChunk.FindSmartPtr(ChunkKeyOffset + ChunkKey);
		if (!PreviousChunk)
		{
			return nullptr;
		}

		return PreviousChunk->ChunkData.HeightChunk.Get();
	};

	if (!SculptChunk)
	{
		PreviousHeightProvider->Query(ChunkKeyOffset + ChunkKey, [&](const TConstVoxelArrayView<float> PreviousHeights)
		{
			PreviousHeights.CopyTo2D<ChunkSize>(
				Heights.View(),
				ChunkKey * ChunkSize,
				Size);
		});

		return;
	}

	if (SculptChunk->bIsRelative)
	{
		PreviousHeightProvider->Query(ChunkKeyOffset + ChunkKey, [&](const TConstVoxelArrayView<float> PreviousHeights)
		{
			for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
			{
				for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
				{
					const int32 IndexInChunk = FVoxelUtilities::Get2DIndex<int32>(
						ChunkSize,
						IndexX,
						IndexY);

					const int32 IndexInSculpt = FVoxelUtilities::Get2DIndex<int32>(
						Size,
						ChunkKey.X * ChunkSize + IndexX,
						ChunkKey.Y * ChunkSize + IndexY);

					const float SculptHeight = SculptChunk->GetHeight(IndexInChunk);
					float Height = PreviousHeights[IndexInChunk];

					if (!FVoxelUtilities::IsNaN(SculptHeight))
					{
						Height += SculptHeight * ScaleZ + OffsetZ;
					}

					Heights[IndexInSculpt] = Height;
				}
			}
		});
	}
	else
	{
		for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
		{
			for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
			{
				const int32 IndexInSculpt = FVoxelUtilities::Get2DIndex<int32>(
					Size,
					ChunkKey.X * ChunkSize + IndexX,
					ChunkKey.Y * ChunkSize + IndexY);

				const int32 IndexInChunk = FVoxelUtilities::Get2DIndex<int32>(
					ChunkSize,
					IndexX,
					IndexY);

				float Height = SculptChunk->GetHeight(IndexInChunk);
				Height = Height * ScaleZ + OffsetZ;

				Heights[IndexInSculpt] = Height;
			}
		}
	}
}

TVoxelArray<float> FVoxelHeightSculptCanvas::QueryPreviousHeights() const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<float> Result;
	FVoxelUtilities::SetNumFast(Result, NumVoxels);

	FVoxelIntBox2D(0, SizeInChunks).ParallelIterate([&](const FIntPoint& ChunkKey)
	{
		PreviousHeightProvider->Query(ChunkKeyOffset + ChunkKey, [&](const TConstVoxelArrayView<float> PreviousHeights)
		{
			PreviousHeights.CopyTo2D<ChunkSize>(
				Result.View(),
				ChunkKey * ChunkSize,
				Size);
		});
	});

	return Result;
}

void FVoxelHeightSculptCanvas::QuerySurfaceTypesMetadatas()
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<FIntPoint, const FVoxelHeightSurfaceTypeChunk*> KeyToSurfaceTypeChunk;
	TVoxelMap<FVoxelMetadataRef, TVoxelMap<FIntPoint, const FVoxelHeightMetadataChunk*>> MetadataRefToKeyToMetadataChunk;

	{
		VOXEL_SCOPE_COUNTER("Build maps");

		KeyToSurfaceTypeChunk.Reserve(KeyToPreviousChunk.Num());

		for (const auto& It :  KeyToPreviousChunk)
		{
			if (!It.Value)
			{
				continue;
			}

			const FVoxelHeightChunkData& ChunkData = It.Value->ChunkData;
			if (It.Value->ChunkData.SurfaceTypeChunk)
			{
				KeyToSurfaceTypeChunk.Add_EnsureNew(It.Key - ChunkKeyOffset, ChunkData.SurfaceTypeChunk.Get());
			}

			for (const auto& MetadataIt : ChunkData.MetadataChunks)
			{
				TVoxelMap<FIntPoint, const FVoxelHeightMetadataChunk*>& KeyToMetadataChunk = MetadataRefToKeyToMetadataChunk.FindOrAdd(MetadataIt.MetadataRef);
				if (KeyToMetadataChunk.Num() == 0)
				{
					KeyToMetadataChunk.Reserve(KeyToPreviousChunk.Num());
				}

				KeyToMetadataChunk.Add_EnsureNew(It.Key - ChunkKeyOffset, MetadataIt.Chunk.Get());
			}
		}
	}

	if (KeyToSurfaceTypeChunk.Num() == 0 &&
		MetadataRefToKeyToMetadataChunk.Num() == 0)
	{
		return;
	}

	FVoxelDoubleVector2DBuffer Positions;
	{
		VOXEL_SCOPE_COUNTER("Write positions");

		Positions.Allocate(Heights.Num());

		for (int32 Y = 0; Y < Size.Y; Y++)
		{
			for (int32 X = 0; X < Size.X; X++)
			{
				const int32 Index = FVoxelUtilities::Get2DIndex<int32>(Size, X, Y);
				Positions.Set(Index, FVector2D(X, Y));
			}
		}
	}

	if (KeyToSurfaceTypeChunk.Num() > 0)
	{
		VOXEL_SCOPE_COUNTER("Surface types");

		SurfaceTypes = FSurfaceTypes();

		FVoxelUtilities::SetNumFast(SurfaceTypes->Alphas, NumVoxels);
		FVoxelUtilities::SetNumFast(SurfaceTypes->Blends, NumVoxels);

		const TVoxelHeightChunkTreeIterator<const FVoxelHeightSurfaceTypeChunk> Iterator = TVoxelHeightChunkTreeIterator<const FVoxelHeightSurfaceTypeChunk>::Create(
			KeyToSurfaceTypeChunk,
			Positions,
			0.);

		FVoxelHeightChunkSamplers::ProcessSurfaceTypes(Iterator, [&](
			const int32 Index,
			const float Alpha,
			const FVoxelSurfaceTypeBlend& SurfaceType)
			{
				SurfaceTypes->Alphas[Index] = Alpha;
				SurfaceTypes->Blends[Index] = SurfaceType;
			});
	}

	for (const auto& It : MetadataRefToKeyToMetadataChunk)
	{
		const FVoxelMetadataRef MetadataRef = It.Key;

		FVoxelHeightMetadataChunk::SwitchType(MetadataRef, [&]<typename InnerType>()
		{
			VOXEL_SCOPE_COUNTER_FORMAT("Metadata %s", *MetadataRef.GetName());

			FMetadata& Metadata = MetadataRefToMetadata.Add_EnsureNew(MetadataRef);
			FVoxelUtilities::SetNumFast(Metadata.Alphas, NumVoxels);

			const TVoxelHeightChunkTreeIterator<const FVoxelHeightMetadataChunk> Iterator = TVoxelHeightChunkTreeIterator<const FVoxelHeightMetadataChunk>::Create(
				It.Value,
				Positions,
				0.);

			TVoxelBufferType<InnerType> Buffer;
			Buffer.Allocate(NumVoxels);
			Buffer.SetAllGeneric(MetadataRef.GetDefaultValue());

			FVoxelHeightChunkSamplers::ProcessMetadata<InnerType>(Iterator, [&](
				const int32 Index,
				const float Alpha,
				const InnerType Value)
				{
					Metadata.Alphas[Index] = Alpha;
					Buffer.Set(Index, Value);
				});

			Metadata.Buffer = MakeSharedCopy(MoveTemp(Buffer));
		});
	}
}