// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "Sculpt/Volume/VoxelVolumeChunkSamplers.h"
#include "Sculpt/Volume/VoxelVolumeChunks.h"
#include "Sculpt/Volume/VoxelVolumeModifier.h"
#include "Sculpt/Volume/VoxelVolumeSculptCanvas.h"
#include "Sculpt/Volume/VoxelSculptVolumeContext.h"
#include "Sculpt/Volume/VoxelVolumeSculptCanvasWriter.h"
#include "Sculpt/Volume/VoxelSculptVolumeLegacyData.h"
#include "Sculpt/Volume/VoxelSculptVolumeTreeBuilder.h"
#include "VoxelDependency.h"
#include "VoxelTaskContext.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelShowVolumeSculptBounds, false,
	"voxel.volume.ShowSculptBounds",
	"Show bounds being sculpted");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelSculptVolumeData> FVoxelSculptVolumeData::Create(TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>> KeyToFarChunk)
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(KeyToFarChunk.AreKeySorted());

	const TSharedRef<FVoxelSculptVolumeData> Result = MakeShared<FVoxelSculptVolumeData>();
	Result->KeyToFarChunk = MoveTemp(KeyToFarChunk);
	Result->ComputeBounds();
	return Result;
}

void FVoxelSculptVolumeData::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion = 7
	);

	const int64 Offset = Ar.Tell();

	int32 Version = FVersion::LatestVersion;
	Ar << Version;

	if (Version < FVersion::FirstVersion)
	{
		Ar.Seek(Offset);

		TVoxelMap<FIntVector, TVoxelRefCountPtr<FVoxelVolumeDistanceChunk>> KeyToDistanceChunk;
		TVoxelMap<FIntVector, TVoxelRefCountPtr<FVoxelVolumeSurfaceTypeChunk>> KeyToSurfaceChunk;
		TVoxelMap<FVoxelMetadataRef, TVoxelMap<FIntVector, TVoxelRefCountPtr<FVoxelVolumeMetadataChunk>>> MetadataToKeyToMetadataChunk;
		SerializeLegacyData(
			Ar,
			KeyToDistanceChunk,
			KeyToSurfaceChunk,
			MetadataToKeyToMetadataChunk);

		TVoxelMap<FIntVector, FVoxelVolumeChunkData> KeyToNewChunkData;
		for (auto& It : KeyToDistanceChunk)
		{
			KeyToNewChunkData.FindOrAdd(It.Key).DistanceChunk = It.Value;
		}
		for (auto& It : KeyToSurfaceChunk)
		{
			KeyToNewChunkData.FindOrAdd(It.Key).SurfaceTypeChunk = It.Value;
		}
		for (const auto& MetadataIt : MetadataToKeyToMetadataChunk)
		{
			for (const auto& It : MetadataIt.Value)
			{
				KeyToNewChunkData.FindOrAdd(It.Key).MetadataChunks.Add(FVoxelVolumeChunkData::FMetadataChunk
				{
					MetadataIt.Key,
					It.Value
				});
			}
		}

		if (KeyToNewChunkData.Num() == 0)
		{
			return;
		}

		const FVoxelIntBox BoundsToReplace = FVoxelIntBox::FromPositions(KeyToNewChunkData.KeyArray()).Scale(ChunkSize);

		KeyToFarChunk = FVoxelSculptVolumeTreeBuilder::Build(
			{},
			BoundsToReplace,
			KeyToNewChunkData,
			0.f);

		ComputeBounds();
		return;
	}

	ensure(Version == FVersion::LatestVersion);

	Ar << KeyToFarChunk;

	checkVoxelSlow(KeyToFarChunk.AreKeySorted());

	if (Ar.IsLoading())
	{
		ComputeBounds();
	}
}

void FVoxelSculptVolumeData::GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const
{
	for (const auto& FarChunkIt : KeyToFarChunk)
	{
		FarChunkIt.Value.GatherObjects(OutObjects);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptVolumeData::Apply(
	IVoxelBulkLoader& Loader,
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& StampToQuery,
	const float Scale,
	const EVoxelStampBehavior Behavior,
	const EVoxelVolumeBlendMode BlendMode,
	const bool bApplyOnVoid,
	const EVoxelVolumeChunkQuality Quality,
	const FVoxelBulkHash& RootHash) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const int64 BulkDataTimestamp = FVoxelTaskScope::GetContext().BulkDataTimestamp;

	const FVoxelBox LocalBounds = StampToQuery.InverseTransform(Query.PositionBounds).Scale(1. / Scale);

	FVoxelIntBox NearBounds = FVoxelIntBox::FromFloatBox_WithPadding(LocalBounds).DivideBigger(ChunkSize);
	FVoxelIntBox MidBounds = NearBounds.DivideBigger(ChunkSize);
	FVoxelIntBox FarBounds = MidBounds.DivideBigger(ChunkSize);

	// Ensure we find max chunks to be able to interpolate if needed
	NearBounds.Max += FIntVector(1);
	MidBounds.Max += FIntVector(1);
	FarBounds.Max += FIntVector(1);

	struct FFarChunkData
	{
		TVoxelBulkPtr<FVoxelVolumeFarChunk> Chunk;
		FVoxelBulkHint Hint;
	};
	TVoxelChunkedArray<FFarChunkData> FarChunksToLoad;
	struct FMidChunk
	{
		TVoxelBulkPtr<FVoxelVolumeMidChunk> Chunk;
		FVoxelBulkHint Hint;
	};
	TVoxelChunkedArray<FMidChunk> MidChunksToLoad;
	struct FNearChunk
	{
		TVoxelBulkPtr<FVoxelVolumeNearChunk> Chunk;
		FVoxelBulkHint Hint;
	};
	TVoxelChunkedArray<FNearChunk> NearChunksToLoad;

	TVoxelChunkedArray<TPair<FIntVector, const FVoxelVolumeChunkData*>> KeyToFarChunkData;
	TVoxelChunkedArray<TPair<FIntVector, const FVoxelVolumeChunkData*>> KeyToMidChunkData;
	TVoxelChunkedArray<TPair<FIntVector, const FVoxelVolumeChunkData*>> KeyToNearChunkData;

	{
		VOXEL_SCOPE_COUNTER("Find chunks");

		FarBounds.Iterate([&](const FIntVector& FarChunkKey)
		{
			const TVoxelBulkPtr<FVoxelVolumeFarChunk>* FarChunkPtr = KeyToFarChunk.Find(FarChunkKey);
			if (!FarChunkPtr)
			{
				return;
			}

			if (!FarChunkPtr->IsLoaded(BulkDataTimestamp))
			{
				const TSharedRef<FVoxelVolumeFarChunkHint> Hint = MakeShared<FVoxelVolumeFarChunkHint>();
				Hint->RootHash = RootHash;
				Hint->FarChunkKey = FarChunkKey;

				FarChunksToLoad.Add({ *FarChunkPtr, FVoxelBulkHint(Hint) });
				return;
			}
			const FVoxelVolumeFarChunk& FarChunk = FarChunkPtr->Get();

			KeyToFarChunkData.Emplace(FarChunkKey, &FarChunk.ChunkData);

			const FVoxelIntBox LocalMidBounds =
				MidBounds
				.ShiftBy(-FarChunkKey * ChunkSize)
				.IntersectWith(FVoxelIntBox(0, ChunkSize));

			if (Quality < EVoxelVolumeChunkQuality::Mid)
			{
				return;
			}

			LocalMidBounds.Iterate([&](const FIntVector& LocalMidChunkKey)
			{
				const TVoxelBulkPtr<FVoxelVolumeMidChunk>* MidChunkPtr = FarChunk.KeyToMidChunk.Find(FVoxelVolumeChunkKey(LocalMidChunkKey));
				if (!MidChunkPtr)
				{
					return;
				}

				if (!MidChunkPtr->IsLoaded(BulkDataTimestamp))
				{
					const TSharedRef<FVoxelVolumeMidChunkHint> Hint = MakeShared<FVoxelVolumeMidChunkHint>();
					Hint->RootHash = RootHash;
					Hint->FarChunkKey = FarChunkKey;
					Hint->MidChunkKey = LocalMidChunkKey;

					MidChunksToLoad.Add({ *MidChunkPtr, FVoxelBulkHint(Hint) });
					return;
				}

				const FIntVector MidChunkKey = FarChunkKey * ChunkSize + LocalMidChunkKey;
				const FVoxelVolumeMidChunk& MidChunk = MidChunkPtr->Get();

				KeyToMidChunkData.Emplace(MidChunkKey, &MidChunk.ChunkData);

				if (Quality < EVoxelVolumeChunkQuality::Near)
				{
					return;
				}

				const FVoxelIntBox LocalNearBounds =
					NearBounds
					.ShiftBy(-MidChunkKey * ChunkSize)
					.IntersectWith(FVoxelIntBox(0, ChunkSize));

				LocalNearBounds.Iterate([&](const FIntVector& LocalNearChunkKey)
				{
					const TVoxelBulkPtr<FVoxelVolumeNearChunk>* NearChunkPtr = MidChunk.KeyToNearChunk.Find(FVoxelVolumeChunkKey(LocalNearChunkKey));
					if (!NearChunkPtr)
					{
						return;
					}

					if (!NearChunkPtr->IsLoaded(BulkDataTimestamp))
					{
						const TSharedRef<FVoxelVolumeNearChunkHint> Hint = MakeShared<FVoxelVolumeNearChunkHint>();
						Hint->RootHash = RootHash;
						Hint->FarChunkKey = FarChunkKey;
						Hint->MidChunkKey = LocalMidChunkKey;
						Hint->NearChunkKey = LocalNearChunkKey;

						NearChunksToLoad.Add({ *NearChunkPtr, FVoxelBulkHint(Hint) });
						return;
					}

					const FIntVector NearChunkKey = MidChunkKey * ChunkSize + LocalNearChunkKey;
					const FVoxelVolumeNearChunk& NearChunk = NearChunkPtr->Get();

					KeyToNearChunkData.Emplace(NearChunkKey, &NearChunk.ChunkData);
				});
			});
		});
	}

	if (FarChunksToLoad.Num() > 0 ||
		MidChunksToLoad.Num() > 0 ||
		NearChunksToLoad.Num() > 0)
	{
		VOXEL_SCOPE_COUNTER("Load chunks");

		const TSharedRef<FVoxelDependency> Dependency = FVoxelDependency::Create("Sculpt Chunk Loader");

		Query.AddDependency(*Dependency);

		FVoxelTaskScope Scope(*GVoxelGlobalTaskContext);

		TVoxelChunkedArray<FVoxelFuture> Futures;

		for (const FFarChunkData& ChunkData : FarChunksToLoad)
		{
			Futures.Add(ChunkData.Chunk.Load(Loader, ChunkData.Hint));
		}
		for (const FMidChunk& ChunkData : MidChunksToLoad)
		{
			Futures.Add(ChunkData.Chunk.Load(Loader, ChunkData.Hint));
		}
		for (const FNearChunk& ChunkData : NearChunksToLoad)
		{
			Futures.Add(ChunkData.Chunk.Load(Loader, ChunkData.Hint));
		}

		FVoxelFuture(Futures).Then_AsyncThread([=]
		{
			Dependency->Invalidate();
		});
	}

	EVoxelVolumeChunkQuality FinalQuality = Quality;
	if (NearChunksToLoad.Num() > 0)
	{
		FinalQuality = EVoxelVolumeChunkQuality::Mid;
	}
	if (MidChunksToLoad.Num() > 0)
	{
		FinalQuality = EVoxelVolumeChunkQuality::Far;
	}

	int32 SizeMultiplier;
	TVoxelChunkedArray<TPair<FIntVector, const FVoxelVolumeChunkData*>>* KeyToChunkData;

	switch (FinalQuality)
	{
	default: VOXEL_ASSUME(false);
	case EVoxelVolumeChunkQuality::Far:
	{
		SizeMultiplier = ChunkSize * ChunkSize;
		KeyToChunkData = &KeyToFarChunkData;
	}
	break;
	case EVoxelVolumeChunkQuality::Mid:
	{
		SizeMultiplier = ChunkSize;
		KeyToChunkData = &KeyToMidChunkData;
	}
	break;
	case EVoxelVolumeChunkQuality::Near:
	{
		SizeMultiplier = 1;
		KeyToChunkData = &KeyToNearChunkData;
	}
	break;
	}

	if (KeyToChunkData->Num() == 0)
	{
		return;
	}

	TVoxelMap<FIntVector, const FVoxelVolumeDistanceChunk*> KeyToDistanceChunk;
	TVoxelMap<FIntVector, const FVoxelVolumeSurfaceTypeChunk*> KeyToSurfaceTypeChunk;
	TVoxelMap<FVoxelMetadataRef, TVoxelMap<FIntVector, const FVoxelVolumeMetadataChunk*>> MetadataRefToKeyToMetadataChunk;
	{
		VOXEL_SCOPE_COUNTER("Build maps");

		KeyToDistanceChunk.Reserve(KeyToChunkData->Num());

		if (Query.bQuerySurfaceTypes)
		{
			KeyToSurfaceTypeChunk.Reserve(KeyToChunkData->Num());
		}

		for (const auto& It : *KeyToChunkData)
		{
			const FVoxelVolumeChunkData& ChunkData = *It.Value;

			if (EnumHasAllFlags(Behavior, EVoxelStampBehavior::AffectShape) &&
				ChunkData.DistanceChunk)
			{
				KeyToDistanceChunk.Add_EnsureNew(It.Key, ChunkData.DistanceChunk.Get());
			}
			if (EnumHasAllFlags(Behavior, EVoxelStampBehavior::AffectSurfaceType) &&
				Query.bQuerySurfaceTypes &&
				ChunkData.SurfaceTypeChunk)
			{
				KeyToSurfaceTypeChunk.Add_EnsureNew(It.Key, ChunkData.SurfaceTypeChunk.Get());
			}

			if (!EnumHasAllFlags(Behavior, EVoxelStampBehavior::AffectMetadata) ||
				Query.MetadatasToQuery.Num() == 0)
			{
				continue;
			}

			for (const auto& MetadataIt : ChunkData.MetadataChunks)
			{
				if (!Query.MetadatasToQuery.Contains(MetadataIt.MetadataRef))
				{
					continue;
				}

				TVoxelMap<FIntVector, const FVoxelVolumeMetadataChunk*>& KeyToMetadataChunk = MetadataRefToKeyToMetadataChunk.FindOrAdd(MetadataIt.MetadataRef);
				if (KeyToMetadataChunk.Num() == 0)
				{
					KeyToMetadataChunk.Reserve(KeyToChunkData->Num());
				}

				KeyToMetadataChunk.Add_EnsureNew(It.Key, MetadataIt.Chunk.Get());
			}
		}
	}

	FVoxelVolumeTransform SculptToQuery = StampToQuery;
	SculptToQuery.Scale *= Scale * SizeMultiplier;
	SculptToQuery.DistanceScale *= Scale * SizeMultiplier;

	// Offset LODs. We build them based on higher quality chunks, which means they are inherently offset by half of their chunk size
	const double Offset = FinalQuality != EVoxelVolumeChunkQuality::Near ? -0.5 : 0.;

	if (KeyToDistanceChunk.Num() > 0)
	{
		const TVoxelVolumeChunkTreeIterator<const FVoxelVolumeDistanceChunk> Iterator = TVoxelVolumeChunkTreeIterator<const FVoxelVolumeDistanceChunk>::Create(
			KeyToDistanceChunk,
			Query,
			SculptToQuery,
			Offset);

		FVoxelVolumeChunkSamplers::ProcessDistances(
			Iterator,
			[&](
				const int32 Index,
				float NewDistance)
			{
				NewDistance = SculptToQuery.TransformDistance(NewDistance);

				float& Distance = Query.IndirectDistances[Index];

				Distance = FVoxelVolumeChunkSamplers::BlendDistances(
					Distance,
					NewDistance,
					BlendMode,
					bApplyOnVoid);
			},
			[&](
				const int32 Index,
				FVoxelMovableDistances MovableDistances)
			{
				// AdditiveDistance will be NaN if we only removed
				// SubtractiveDistance will be NaN if we only added

				if (!FVoxelUtilities::IsNaN(MovableDistances.Additive))
				{
					MovableDistances.Additive = SculptToQuery.TransformDistance(MovableDistances.Additive);
				}
				if (!FVoxelUtilities::IsNaN(MovableDistances.Subtractive))
				{
					MovableDistances.Subtractive = SculptToQuery.TransformDistance(-MovableDistances.Subtractive);
				}

				float& Distance = Query.IndirectDistances[Index];

				Distance = FVoxelVolumeChunkSamplers::BlendDistances(
					Distance,
					MovableDistances.Additive,
					MovableDistances.Subtractive,
					BlendMode,
					bApplyOnVoid);
			},
			[&](const int32 Index)
			{
				if (BlendMode == EVoxelVolumeBlendMode::Intersect)
				{
					Query.IndirectDistances[Index] = FVoxelUtilities::NaNf();
				}
			});
	}

	if (KeyToSurfaceTypeChunk.Num() > 0)
	{
		FVoxelVolumeChunkSamplers::ProcessSurfaceTypes(
			TVoxelVolumeChunkTreeIterator<const FVoxelVolumeSurfaceTypeChunk>::Create(
				KeyToSurfaceTypeChunk,
				Query,
				SculptToQuery,
				Offset),
			[&](
			const int32 Index,
			const float Alpha,
			const FVoxelSurfaceTypeBlend& SurfaceType)
			{
				if (Alpha == 0)
				{
					return;
				}

				FVoxelSurfaceTypeBlend& SurfaceTypeRef = Query.IndirectSurfaceTypes[Index];

				FVoxelSurfaceTypeBlend::Lerp(
					SurfaceTypeRef,
					SurfaceTypeRef,
					SurfaceType,
					Alpha);
			});
	}

	for (const auto& MetadataIt : MetadataRefToKeyToMetadataChunk)
	{
		const FVoxelMetadataRef MetadataRef = MetadataIt.Key;

		const TVoxelVolumeChunkTreeIterator<const FVoxelVolumeMetadataChunk> Iterator = TVoxelVolumeChunkTreeIterator<const FVoxelVolumeMetadataChunk>::Create(
			MetadataIt.Value,
			Query,
			SculptToQuery,
			Offset);

		FVoxelVolumeMetadataChunk::SwitchType(MetadataRef, [&]<typename InnerType>()
		{
			TVoxelBufferType<InnerType>& Buffer = Query.IndirectMetadata.FindChecked<TVoxelBufferType<InnerType>>(MetadataRef);

			FVoxelVolumeChunkSamplers::ProcessMetadata<InnerType>(Iterator, [&](
				const int32 Index,
				const float Alpha,
				const InnerType Value)
				{
					Buffer.Set(Index, FMath::Lerp(Buffer[Index], Value, Alpha));
				});
		});
	}
}

TVoxelFuture<TVoxelMap<FIntVector, TSharedPtr<const FVoxelVolumeNearChunk>>> FVoxelSculptVolumeData::LoadNearChunks(
	const TSharedRef<IVoxelBulkLoader>& Loader,
	const FVoxelIntBox& NearBounds,
	const FVoxelBulkHash& RootHash) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(NearBounds.Count_double() < 1024 * 1024 * 1024))
	{
		return {};
	}

	const FVoxelIntBox MidBounds = NearBounds.DivideBigger(ChunkSize);
	const FVoxelIntBox FarBounds = MidBounds.DivideBigger(ChunkSize);

	struct FData
	{
		FVoxelCriticalSection CriticalSection;
		TVoxelMap<FIntVector, TSharedPtr<const FVoxelVolumeNearChunk>> KeyToNearChunk_RequiresLock;
	};

	const TSharedRef<FData> Data = MakeShared<FData>();
	Data->KeyToNearChunk_RequiresLock.Reserve(NearBounds.Count_int32());

	TVoxelChunkedArray<FVoxelFuture> FarFutures;

	FarBounds.Iterate([&](const FIntVector& FarChunkKey)
	{
		const TVoxelBulkPtr<FVoxelVolumeFarChunk>* FarChunkPtr = KeyToFarChunk.Find(FarChunkKey);
		if (!FarChunkPtr)
		{
			return;
		}

		const TSharedRef<FVoxelVolumeFarChunkHint> FarChunkHint = MakeShared<FVoxelVolumeFarChunkHint>();
		FarChunkHint->RootHash = RootHash;
		FarChunkHint->FarChunkKey = FarChunkKey;

		FarFutures.Add(FarChunkPtr->Load(*Loader, FVoxelBulkHint(FarChunkHint)).Then_AnyThread([=](const TSharedRef<const FVoxelVolumeFarChunk>& FarChunk)
		{
			VOXEL_FUNCTION_COUNTER();

			const FVoxelIntBox LocalMidBounds = MidBounds.IntersectWith(FVoxelIntBox(FarChunkKey).Scale(ChunkSize));

			TVoxelChunkedArray<FVoxelFuture> MidFutures;

			LocalMidBounds.Iterate([&](const FIntVector& MidChunkKey)
			{
				const TVoxelBulkPtr<FVoxelVolumeMidChunk>* MidChunkPtr = FarChunk->KeyToMidChunk.Find(FVoxelVolumeChunkKey(MidChunkKey - FarChunkKey * ChunkSize));
				if (!MidChunkPtr)
				{
					return;
				}

				const TSharedRef<FVoxelVolumeMidChunkHint> MidChunkHint = MakeShared<FVoxelVolumeMidChunkHint>();
				MidChunkHint->RootHash = RootHash;
				MidChunkHint->FarChunkKey = FarChunkKey;
				MidChunkHint->MidChunkKey = MidChunkKey;

				MidFutures.Add(MidChunkPtr->Load(*Loader, FVoxelBulkHint(MidChunkHint)).Then_AnyThread([=](const TSharedRef<const FVoxelVolumeMidChunk>& MidChunk)
				{
					VOXEL_FUNCTION_COUNTER();

					const FVoxelIntBox LocalNearBounds = NearBounds.IntersectWith(FVoxelIntBox(MidChunkKey).Scale(ChunkSize));

					TVoxelChunkedArray<FVoxelFuture> NearFutures;

					LocalNearBounds.Iterate([&](const FIntVector& NearChunkKey)
					{
						const TVoxelBulkPtr<FVoxelVolumeNearChunk>* NearChunkPtr = MidChunk->KeyToNearChunk.Find(FVoxelVolumeChunkKey(NearChunkKey - MidChunkKey * ChunkSize));
						if (!NearChunkPtr)
						{
							return;
						}

						const TSharedRef<FVoxelVolumeNearChunkHint> NearChunkHint = MakeShared<FVoxelVolumeNearChunkHint>();
						NearChunkHint->RootHash = RootHash;
						NearChunkHint->FarChunkKey = FarChunkKey;
						NearChunkHint->MidChunkKey = MidChunkKey;
						NearChunkHint->NearChunkKey = NearChunkKey;

						NearFutures.Add(NearChunkPtr->Load(*Loader, FVoxelBulkHint(NearChunkHint)).Then_AnyThread([=](const TSharedRef<const FVoxelVolumeNearChunk>& NearChunk)
						{
							VOXEL_FUNCTION_COUNTER();

							VOXEL_SCOPE_LOCK(Data->CriticalSection);
							Data->KeyToNearChunk_RequiresLock.Add_EnsureNew(NearChunkKey, NearChunk);
						}));
					});

					return FVoxelFuture(NearFutures);
				}));
			});

			return FVoxelFuture(MidFutures);
		}));
	});

	return FVoxelFuture(FarFutures).Then_AnyThread([=]
	{
		return Data->KeyToNearChunk_RequiresLock;
	});
}

TVoxelFuture<const FVoxelSculptVolumeData> FVoxelSculptVolumeData::ApplyModifier(
	const TSharedRef<IVoxelBulkLoader>& Loader,
	const FVoxelSculptVolumeContext& Context,
	const FVoxelBulkHash& RootHash,
	const TSharedRef<const IVoxelVolumeModifierRuntime>& Modifier) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelIntBox BoundsToSculpt =
		FVoxelIntBox::FromFloatBox_WithPadding(
			Modifier->GetBounds()
			.TransformBy(Context.SculptToWorld.Inverse())
			.Extend(2))
		.MakeMultipleOfBigger(ChunkSize)
		// Add a border of size 1 for jump flood
		.Extend(ChunkSize);

	if (!ensure(BoundsToSculpt.Count_double() < MAX_int32))
	{
		return SharedThis(this);
	}

	if (GVoxelShowVolumeSculptBounds)
	{
		FVoxelDebugDrawer()
			.Color(FLinearColor::Blue)
			.LifeTime(1.f)
			.DrawBox(BoundsToSculpt.ToVoxelBox(), Context.SculptToWorld);
	}

	const FVoxelIntBox ChunkKeyBounds = BoundsToSculpt.DivideExact(ChunkSize);

	return
		FVoxelVolumeSculptCanvas::CreateCanvas(Loader, SharedThis(this), Context.PreviousDistanceProvider, ChunkKeyBounds, RootHash)
		.Then_AsyncThread([=](const TSharedRef<FVoxelVolumeSculptCanvas>& Canvas)
		{
			VOXEL_FUNCTION_COUNTER();

			FVoxelVolumeSculptCanvasWriter CanvasWriter(
				*Canvas,
				Context.SculptToWorld);

			Modifier->Apply(CanvasWriter);

			return Canvas->Finalize(Context.bStoreMovableDistances, Context.MaxErrorPercentage);
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptVolumeData::ComputeBounds()
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelIntBox Bounds = FVoxelIntBox::InvertedInfinite;

	for (const auto& FarChunkIt : KeyToFarChunk)
	{
		const FIntVector FarChunkKey = FarChunkIt.Key;
		const FVoxelIntBox FarChunkBounds = FVoxelIntBox(FarChunkKey).Scale(ChunkSize * ChunkSize * ChunkSize);

		if (Bounds.Contains(FarChunkBounds))
		{
			continue;
		}

		if (!FarChunkIt.Value.IsLoaded())
		{
			Bounds += FarChunkBounds;
			continue;
		}

		for (const auto& MidChunkIt : FarChunkIt.Value->KeyToMidChunk)
		{
			const FIntVector MidChunkKey = FarChunkKey * ChunkSize + MidChunkIt.Key.ToVector();
			const FVoxelIntBox MidChunkBounds = FVoxelIntBox(MidChunkKey).Scale(ChunkSize * ChunkSize);

			if (Bounds.Contains(MidChunkBounds))
			{
				continue;
			}

			if (!MidChunkIt.Value.IsLoaded())
			{
				Bounds += MidChunkBounds;
				continue;
			}

			for (const auto& NearChunkIt : MidChunkIt.Value->KeyToNearChunk)
			{
				const FIntVector NearChunkKey = MidChunkKey * ChunkSize + NearChunkIt.Key.ToVector();
				const FVoxelIntBox NearChunkBounds = FVoxelIntBox(NearChunkKey).Scale(ChunkSize);

				Bounds += NearChunkBounds;
			}
		}
	}

	PrivateBounds = Bounds.IsValid() ? Bounds : FVoxelIntBox();
}