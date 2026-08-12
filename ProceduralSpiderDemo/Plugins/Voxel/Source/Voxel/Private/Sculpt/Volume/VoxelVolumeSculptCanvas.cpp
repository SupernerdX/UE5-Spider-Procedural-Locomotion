// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeSculptCanvas.h"
#include "Sculpt/Volume/VoxelVolumeChunks.h"
#include "Sculpt/Volume/VoxelVolumeChunkData.h"
#include "Sculpt/Volume/VoxelSculptVolumeCache.h"
#include "Sculpt/Volume/VoxelVolumeChunkSamplers.h"
#include "Sculpt/Volume/VoxelVolumeDistanceChunk.h"
#include "Sculpt/Volume/VoxelVolumeSculptUtilities.h"
#include "Sculpt/Volume/VoxelVolumeSculptPreviousDistanceProvider.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "Sculpt/Volume/VoxelSculptVolumeTreeBuilder.h"

TVoxelFuture<FVoxelVolumeSculptCanvas> FVoxelVolumeSculptCanvas::CreateCanvas(
	const TSharedRef<IVoxelBulkLoader>& BulkLoader,
	const TSharedRef<const FVoxelSculptVolumeData>& SculptData,
	const TSharedRef<IVoxelVolumeSculptPreviousDistanceProvider>& PreviousDistanceProvider,
	const FVoxelIntBox& ChunkKeyBounds,
	const FVoxelBulkHash& RootHash)
{
	VOXEL_FUNCTION_COUNTER();

	return
		SculptData->LoadNearChunks(BulkLoader, ChunkKeyBounds, RootHash)
		.Then_AsyncThread([=](const TVoxelMap<FIntVector, TSharedPtr<const FVoxelVolumeNearChunk>>& KeyToNearChunk)
		{
			return MakeShared<FVoxelVolumeSculptCanvas>(
				ChunkKeyBounds,
				SculptData,
				PreviousDistanceProvider,
				KeyToNearChunk);
		});
}

FVoxelVolumeSculptCanvas::FVoxelVolumeSculptCanvas(
	const FVoxelIntBox& ChunkKeyBounds,
	const TSharedRef<const FVoxelSculptVolumeData>& PreviousSculptData,
	const TSharedRef<IVoxelVolumeSculptPreviousDistanceProvider>& PreviousDistanceProvider,
	const TVoxelMap<FIntVector, TSharedPtr<const FVoxelVolumeNearChunk>>& KeyToPreviousChunk)
	: SizeInChunks(ChunkKeyBounds.Size())
	, Size(SizeInChunks * ChunkSize)
	, NumVoxels(Size.X * Size.Y * Size.Z)
	, ChunkKeyOffset(ChunkKeyBounds.Min)
	, PreviousSculptData(PreviousSculptData)
	, PreviousDistanceProvider(PreviousDistanceProvider)
	, KeyToPreviousChunk(KeyToPreviousChunk)
{
	VOXEL_FUNCTION_COUNTER_NUM(NumVoxels);

	for (const auto& It : KeyToPreviousChunk)
	{
		checkVoxelSlow(FVoxelIntBox(0, SizeInChunks).ShiftBy(ChunkKeyOffset).Contains(It.Key))
	}

	FVoxelUtilities::SetNumFast(Distances, NumVoxels);

	{
		VOXEL_SCOPE_COUNTER("Query distances");

		FVoxelIntBox(0, SizeInChunks).ParallelIterate([&](const FIntVector& ChunkKey)
		{
			QueryChunkDistances(ChunkKey);
		});
	}

	TVoxelArray<float> ClosestX;
	TVoxelArray<float> ClosestY;
	TVoxelArray<float> ClosestZ;

	FVoxelUtilities::JumpFlood(
		Size,
		Distances,
		ClosestX,
		ClosestY,
		ClosestZ);

	// Propagate now - shouldn't change any visible data but will auto fixup any "floating" data
	Propagate(ClosestX, ClosestY, ClosestZ);
}

TSharedRef<const FVoxelSculptVolumeData> FVoxelVolumeSculptCanvas::Finalize(
	const bool bStoreMovableDistances,
	const float MaxErrorPercentage)
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(TVoxelSet<FVoxelMetadataRef>(MetadataRefsToWrite).Num() == MetadataRefsToWrite.Num());

	// Only write in the inner part that's fully jump flooded
	const FVoxelIntBox ChunkKeysToWrite = FVoxelIntBox(1, SizeInChunks - 1);
	ensure(ChunkKeysToWrite.IsValid());

	TVoxelMap<FIntVector, FVoxelVolumeChunkData> KeyToChunkData;
	{
		KeyToChunkData.Reserve(SizeInChunks.X * SizeInChunks.Y * SizeInChunks.Z);

		ChunkKeysToWrite.ShiftBy(ChunkKeyOffset).Iterate([&](const FIntVector& ChunkKey)
		{
			FVoxelVolumeChunkData ChunkData;

			if (const FVoxelVolumeNearChunk* PreviousChunk = KeyToPreviousChunk.FindSmartPtr(ChunkKey))
			{
				ChunkData = PreviousChunk->ChunkData;
			}

			KeyToChunkData.Add_EnsureNew(ChunkKey, ChunkData);
		});
	}

	if (bWriteDistances)
	{
		VOXEL_SCOPE_COUNTER("Distances");

		FVoxelUtilities::JumpFlood(
			Size,
			Distances);

		if (bStoreMovableDistances)
		{
			VOXEL_SCOPE_COUNTER("Movable");

			TVoxelArray<float> PreviousDistances = QueryPreviousDistances();
			{
				VOXEL_SCOPE_COUNTER("Fixup PreviousDistances");

				const TVoxelArray<float> OldPreviousDistances = PreviousDistances;

				FVoxelUtilities::JumpFlood(Size, PreviousDistances);

				// Jump flooding might fail if we're too deep underground
				for (int32 Index = 0; Index < PreviousDistances.Num(); Index++)
				{
					float& PreviousDistance = PreviousDistances[Index];

					if (FVoxelUtilities::IsNaN(PreviousDistance))
					{
						PreviousDistance = OldPreviousDistances[Index];
					}
				}
			}

			TVoxelArray<float> AdditiveDistances;
			TVoxelArray<float> SubtractiveDistances;
			FVoxelVolumeSculptUtilities::DiffDistances(
				Distances,
				PreviousDistances,
				Size,
				AdditiveDistances,
				SubtractiveDistances);

			ChunkKeysToWrite.ParallelIterate([&](const FIntVector& ChunkKey)
			{
				KeyToChunkData[ChunkKeyOffset + ChunkKey].DistanceChunk = FVoxelVolumeDistanceChunk::CreateMovable(
					AdditiveDistances,
					SubtractiveDistances,
					ChunkKey * ChunkSize,
					Size);
			});
		}
		else
		{
			ChunkKeysToWrite.ParallelIterate([&](const FIntVector& ChunkKey)
			{
				KeyToChunkData[ChunkKeyOffset + ChunkKey].DistanceChunk = FVoxelVolumeDistanceChunk::CreatePacked(
					Distances,
					ChunkKey * ChunkSize,
					Size,
					MaxErrorPercentage);
			});
		}
	}

	if (bWriteSurfaceTypes &&
		SurfaceTypes)
	{
		VOXEL_SCOPE_COUNTER("Surface types");

		ChunkKeysToWrite.ParallelIterate([&](const FIntVector& ChunkKey)
		{
			KeyToChunkData[ChunkKeyOffset + ChunkKey].SurfaceTypeChunk = FVoxelVolumeSurfaceTypeChunk::Create(
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

		ChunkKeysToWrite.ParallelIterate([&](const FIntVector& ChunkKey)
		{
			TVoxelArray<FVoxelVolumeChunkData::FMetadataChunk>& MetadataChunks = KeyToChunkData[ChunkKeyOffset + ChunkKey].MetadataChunks;

			const TVoxelRefCountPtr<FVoxelVolumeMetadataChunk> Chunk = FVoxelVolumeMetadataChunk::Create(
				Metadata->Alphas,
				*Metadata->Buffer,
				ChunkKey * ChunkSize,
				Size);

			if (!Chunk)
			{
				MetadataChunks.RemoveAll([&](const FVoxelVolumeChunkData::FMetadataChunk& MetadataChunk)
				{
					return MetadataChunk.MetadataRef == MetadataRef;
				});

				return;
			}

			for (FVoxelVolumeChunkData::FMetadataChunk& MetadataChunk : MetadataChunks)
			{
				if (MetadataChunk.MetadataRef == MetadataRef)
				{
					MetadataChunk.Chunk = Chunk;
					return;
				}
			}

			MetadataChunks.Add(FVoxelVolumeChunkData::FMetadataChunk
			{
				MetadataRef,
				Chunk
			});
		});
	}

	TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>> NewKeyToFarChunk = FVoxelSculptVolumeTreeBuilder::Build(
		PreviousSculptData->GetKeyToFarChunk(),
		FVoxelIntBox(ChunkSize, Size - ChunkSize).ShiftBy(ChunkKeyOffset * ChunkSize),
		KeyToChunkData,
		MaxErrorPercentage);

	return FVoxelSculptVolumeData::Create(MoveTemp(NewKeyToFarChunk));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeSculptCanvas::Propagate(
	const TConstVoxelArrayView<float> ClosestX,
	const TConstVoxelArrayView<float> ClosestY,
	const TConstVoxelArrayView<float> ClosestZ)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<FIntVector, const FVoxelVolumeSurfaceTypeChunk*> KeyToSurfaceTypeChunk;
	TVoxelMap<FVoxelMetadataRef, TVoxelMap<FIntVector, const FVoxelVolumeMetadataChunk*>> MetadataRefToKeyToMetadataChunk;
	{
		VOXEL_SCOPE_COUNTER("Build maps");

		KeyToSurfaceTypeChunk.Reserve(KeyToPreviousChunk.Num());

		for (const auto& It : KeyToPreviousChunk)
		{
			const FVoxelVolumeChunkData& ChunkData = It.Value->ChunkData;

			if (ChunkData.SurfaceTypeChunk)
			{
				KeyToSurfaceTypeChunk.Add_EnsureNew(It.Key - ChunkKeyOffset, ChunkData.SurfaceTypeChunk.Get());
			}

			for (const auto& MetadataIt : ChunkData.MetadataChunks)
			{
				TVoxelMap<FIntVector, const FVoxelVolumeMetadataChunk*>& KeyToMetadataChunk = MetadataRefToKeyToMetadataChunk.FindOrAdd(MetadataIt.MetadataRef);
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

	FVoxelDoubleVectorBuffer Positions;
	{
		VOXEL_SCOPE_COUNTER("Write positions");

		Positions.Allocate(Distances.Num());

		for (int32 Z = 0; Z < Size.Z; Z++)
		{
			for (int32 Y = 0; Y < Size.Y; Y++)
			{
				for (int32 X = 0; X < Size.X; X++)
				{
					const int32 Index = FVoxelUtilities::Get3DIndex<int32>(Size, X, Y, Z);

					FVector Closest = FVector(
						ClosestX[Index],
						ClosestY[Index],
						ClosestZ[Index]);

					if (FVoxelUtilities::IsNaN(Closest.X) ||
						FVoxelUtilities::IsNaN(Closest.Y) ||
						FVoxelUtilities::IsNaN(Closest.Z))
					{
						Closest = FVector(X, Y, Z);
					}
					ensureVoxelSlowNoSideEffects(FVoxelBox(0, FVector(Size - 1)).Contains(Closest));

					Positions.Set(Index, Closest);
				}
			}
		}
	}

	if (KeyToSurfaceTypeChunk.Num() > 0)
	{
		VOXEL_SCOPE_COUNTER("Surface types");

		SurfaceTypes = FSurfaceTypes();

		FVoxelUtilities::SetNumFast(SurfaceTypes->Alphas, NumVoxels);
		FVoxelUtilities::SetNumFast(SurfaceTypes->Blends, NumVoxels);

		const TVoxelVolumeChunkTreeIterator<const FVoxelVolumeSurfaceTypeChunk> Iterator = TVoxelVolumeChunkTreeIterator<const FVoxelVolumeSurfaceTypeChunk>::Create(
			KeyToSurfaceTypeChunk,
			Positions,
			0.);

		FVoxelVolumeChunkSamplers::ProcessSurfaceTypes(Iterator, [&](
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

		FVoxelVolumeMetadataChunk::SwitchType(MetadataRef, [&]<typename InnerType>()
		{
			VOXEL_SCOPE_COUNTER_FORMAT("Metadata %s", *MetadataRef.GetName());

			FMetadata& Metadata = MetadataRefToMetadata.Add_EnsureNew(MetadataRef);
			FVoxelUtilities::SetNumFast(Metadata.Alphas, NumVoxels);

			const TVoxelVolumeChunkTreeIterator<const FVoxelVolumeMetadataChunk> Iterator = TVoxelVolumeChunkTreeIterator<const FVoxelVolumeMetadataChunk>::Create(
				It.Value,
				Positions,
				0.);

			TVoxelBufferType<InnerType> Buffer;
			Buffer.Allocate(NumVoxels);
			Buffer.SetAllGeneric(MetadataRef.GetDefaultValue());

			FVoxelVolumeChunkSamplers::ProcessMetadata<InnerType>(Iterator, [&](
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

void FVoxelVolumeSculptCanvas::QueryChunkDistances(const FIntVector& ChunkKey)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelVolumeDistanceChunk* SculptChunk = INLINE_LAMBDA -> const FVoxelVolumeDistanceChunk*
	{
		const FVoxelVolumeNearChunk* PreviousChunk = KeyToPreviousChunk.FindSmartPtr(ChunkKeyOffset + ChunkKey);
		if (!PreviousChunk)
		{
			return nullptr;
		}

		return PreviousChunk->ChunkData.DistanceChunk.Get();
	};

	if (!SculptChunk)
	{
		PreviousDistanceProvider->Query(ChunkKeyOffset + ChunkKey, [&](const TConstVoxelArrayView<float> PreviousDistances)
		{
			PreviousDistances.CopyTo3D<ChunkSize>(
				Distances.View(),
				ChunkKey * ChunkSize,
				Size);
		});

		return;
	}

	if (SculptChunk->bIsPacked)
	{
		for (int32 IndexZ = 0; IndexZ < ChunkSize; IndexZ++)
		{
			for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
			{
				for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
				{
					const int32 IndexInSculpt = FVoxelUtilities::Get3DIndex<int32>(
						Size,
						ChunkKey.X * ChunkSize + IndexX,
						ChunkKey.Y * ChunkSize + IndexY,
						ChunkKey.Z * ChunkSize + IndexZ);

					const int32 IndexInChunk = FVoxelUtilities::Get3DIndex<int32>(
						ChunkSize,
						IndexX,
						IndexY,
						IndexZ);

					Distances[IndexInSculpt] = SculptChunk->GetPackedDistance(IndexInChunk);
				}
			}
		}
	}
	else
	{
		PreviousDistanceProvider->Query(ChunkKeyOffset + ChunkKey, [&](const TConstVoxelArrayView<float> PreviousDistances)
		{
			for (int32 IndexZ = 0; IndexZ < ChunkSize; IndexZ++)
			{
				for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
				{
					for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
					{
						const int32 IndexInChunk = FVoxelUtilities::Get3DIndex<int32>(
							ChunkSize,
							IndexX,
							IndexY,
							IndexZ);

						const int32 IndexInSculpt = FVoxelUtilities::Get3DIndex<int32>(
							Size,
							ChunkKey.X * ChunkSize + IndexX,
							ChunkKey.Y * ChunkSize + IndexY,
							ChunkKey.Z * ChunkSize + IndexZ);

						const float AdditiveDistance = SculptChunk->GetMovable().AdditiveDistances[IndexInChunk];
						const float SubtractiveDistance = SculptChunk->GetMovable().SubtractiveDistances[IndexInChunk];

						// AdditiveDistance will be NaN if we only removed
						// SubtractiveDistance will be NaN if we only added

						float Distance = PreviousDistances[IndexInChunk];

						if (!FVoxelUtilities::IsNaN(AdditiveDistance))
						{
							if (FVoxelUtilities::IsNaN(Distance))
							{
								Distance = AdditiveDistance;
							}
							else
							{
								Distance = FMath::Min(Distance, AdditiveDistance);
							}
						}

						if (!FVoxelUtilities::IsNaN(SubtractiveDistance))
						{
							if (FVoxelUtilities::IsNaN(Distance))
							{
								Distance = -SubtractiveDistance;
							}
							else
							{
								Distance = FMath::Max(Distance, -SubtractiveDistance);
							}
						}

						Distances[IndexInSculpt] = Distance;
					}
				}
			}
		});
	}
}

TVoxelArray<float> FVoxelVolumeSculptCanvas::QueryPreviousDistances() const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<float> Result;
	FVoxelUtilities::SetNumFast(Result, NumVoxels);

	FVoxelIntBox(0, SizeInChunks).ParallelIterate([&](const FIntVector& ChunkKey)
	{
		PreviousDistanceProvider->Query(ChunkKeyOffset + ChunkKey, [&](const TConstVoxelArrayView<float> PreviousDistances)
		{
			PreviousDistances.CopyTo3D<ChunkSize>(
				Result.View(),
				ChunkKey * ChunkSize,
				Size);
		});
	});

	return Result;
}