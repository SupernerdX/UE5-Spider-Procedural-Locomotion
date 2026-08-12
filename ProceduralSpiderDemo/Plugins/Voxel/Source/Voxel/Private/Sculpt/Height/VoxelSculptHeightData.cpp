// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelSculptHeightData.h"
#include "Sculpt/Height/VoxelHeightChunkSamplers.h"
#include "Sculpt/Height/VoxelHeightChunks.h"
#include "Sculpt/Height/VoxelHeightModifier.h"
#include "Sculpt/Height/VoxelHeightSculptCanvas.h"
#include "Sculpt/Height/VoxelSculptHeightContext.h"
#include "Sculpt/Height/VoxelSculptHeightLegacyData.h"
#include "Sculpt/Height/VoxelSculptHeightTreeBuilder.h"
#include "Sculpt/Height/VoxelHeightSculptCanvasWriter.h"
#include "VoxelDependency.h"
#include "VoxelHeightChunkTreeIterator.h"
#include "VoxelStampQuery.h"
#include "VoxelStampTransform.h"
#include "VoxelTaskContext.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelShowHeightSculptBounds, false,
	"voxel.height.ShowSculptBounds",
	"Show bounds being sculpted");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelSculptHeightData> FVoxelSculptHeightData::Create(TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>> KeyToFarChunk)
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(KeyToFarChunk.AreKeySorted());

	const TSharedRef<FVoxelSculptHeightData> Result = MakeShared<FVoxelSculptHeightData>();
	Result->KeyToFarChunk = MoveTemp(KeyToFarChunk);
	Result->ComputeBounds();
	return Result;
}

void FVoxelSculptHeightData::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion = 3
	);

	const int64 Offset = Ar.Tell();

	int32 Version = FVersion::LatestVersion;
	Ar << Version;

	if (Version < FVersion::FirstVersion)
	{
		check(bLegacyHeightChunksRelative.IsSet());

		Ar.Seek(Offset);

		TVoxelMap<FIntPoint, TVoxelRefCountPtr<FVoxelHeightHeightChunk>> KeyToDistanceChunk;
		TVoxelMap<FIntPoint, TVoxelRefCountPtr<FVoxelHeightSurfaceTypeChunk>> KeyToSurfaceChunk;
		TVoxelMap<FVoxelMetadataRef, TVoxelMap<FIntPoint, TVoxelRefCountPtr<FVoxelHeightMetadataChunk>>> MetadataToKeyToMetadataChunk;
		FVoxelSculptHeightLegacyData::SerializeLegacyData(
			Ar,
			bLegacyHeightChunksRelative.Get(false),
			KeyToDistanceChunk,
			KeyToSurfaceChunk,
			MetadataToKeyToMetadataChunk);

		TVoxelMap<FIntPoint, FVoxelHeightChunkData> KeyToNewChunkData;
		for (auto& It : KeyToDistanceChunk)
		{
			KeyToNewChunkData.FindOrAdd(It.Key).HeightChunk = It.Value;
		}
		for (auto& It : KeyToSurfaceChunk)
		{
			KeyToNewChunkData.FindOrAdd(It.Key).SurfaceTypeChunk = It.Value;
		}
		for (const auto& MetadataIt : MetadataToKeyToMetadataChunk)
		{
			for (const auto& It : MetadataIt.Value)
			{
				KeyToNewChunkData.FindOrAdd(It.Key).MetadataChunks.Add(FVoxelHeightChunkData::FMetadataChunk
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

		const FVoxelIntBox2D BoundsToReplace = FVoxelIntBox2D::FromPositions(KeyToNewChunkData.KeyArray()).Scale(ChunkSize);

		KeyToFarChunk = FVoxelSculptHeightTreeBuilder::Build(
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

void FVoxelSculptHeightData::GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const
{
	for (const auto& FarChunkIt : KeyToFarChunk)
	{
		FarChunkIt.Value.GatherObjects(OutObjects);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptHeightData::Apply(
	IVoxelBulkLoader& Loader,
	const FVoxelHeightSparseQuery& Query,
	const FVoxelHeightTransform& StampToQuery,
	const float ScaleXY,
	const EVoxelStampBehavior Behavior,
	const EVoxelHeightBlendMode BlendMode,
	const bool bApplyOnVoid,
	const EVoxelHeightChunkQuality Quality,
	const FVoxelBulkHash& RootHash) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const int64 BulkDataTimestamp = FVoxelTaskScope::GetContext().BulkDataTimestamp;

	const FVoxelBox2D LocalBounds = StampToQuery.InverseTransform(Query.PositionBounds).Scale(1. / ScaleXY);

	FVoxelIntBox2D NearBounds = FVoxelIntBox2D::FromFloatBox_WithPadding(LocalBounds).DivideBigger(ChunkSize);
	FVoxelIntBox2D MidBounds = NearBounds.DivideBigger(ChunkSize);
	FVoxelIntBox2D FarBounds = MidBounds.DivideBigger(ChunkSize);

	// Ensure we find max chunks to be able to interpolate if needed
	NearBounds.Max += FIntPoint(1);
	MidBounds.Max += FIntPoint(1);
	FarBounds.Max += FIntPoint(1);

	struct FFarChunkData
	{
		TVoxelBulkPtr<FVoxelHeightFarChunk> Chunk;
		FVoxelBulkHint Hint;
	};
	TVoxelChunkedArray<FFarChunkData> FarChunksToLoad;
	struct FMidChunk
	{
		TVoxelBulkPtr<FVoxelHeightMidChunk> Chunk;
		FVoxelBulkHint Hint;
	};
	TVoxelChunkedArray<FMidChunk> MidChunksToLoad;
	struct FNearChunk
	{
		TVoxelBulkPtr<FVoxelHeightNearChunk> Chunk;
		FVoxelBulkHint Hint;
	};
	TVoxelChunkedArray<FNearChunk> NearChunksToLoad;

	TVoxelChunkedArray<TPair<FIntPoint, const FVoxelHeightChunkData*>> KeyToFarChunkData;
	TVoxelChunkedArray<TPair<FIntPoint, const FVoxelHeightChunkData*>> KeyToMidChunkData;
	TVoxelChunkedArray<TPair<FIntPoint, const FVoxelHeightChunkData*>> KeyToNearChunkData;

	{
		VOXEL_SCOPE_COUNTER("Find chunks");

		FarBounds.Iterate([&](const FIntPoint& FarChunkKey)
		{
			const TVoxelBulkPtr<FVoxelHeightFarChunk>* FarChunkPtr = KeyToFarChunk.Find(FarChunkKey);
			if (!FarChunkPtr)
			{
				return;
			}

			if (!FarChunkPtr->IsLoaded(BulkDataTimestamp))
			{
				const TSharedRef<FVoxelHeightFarChunkHint> Hint = MakeShared<FVoxelHeightFarChunkHint>();
				Hint->RootHash = RootHash;
				Hint->FarChunkKey = FarChunkKey;

				FarChunksToLoad.Add({ *FarChunkPtr, FVoxelBulkHint(Hint) });
				return;
			}
			const FVoxelHeightFarChunk& FarChunk = FarChunkPtr->Get();

			KeyToFarChunkData.Emplace(FarChunkKey, &FarChunk.ChunkData);

			const FVoxelIntBox2D LocalMidBounds =
				MidBounds
				.ShiftBy(-FarChunkKey * ChunkSize)
				.IntersectWith(FVoxelIntBox2D(0, ChunkSize));

			if (Quality < EVoxelHeightChunkQuality::Mid)
			{
				return;
			}

			LocalMidBounds.Iterate([&](const FIntPoint& LocalMidChunkKey)
			{
				const TVoxelBulkPtr<FVoxelHeightMidChunk>* MidChunkPtr = FarChunk.KeyToMidChunk.Find(FVoxelHeightChunkKey(LocalMidChunkKey));
				if (!MidChunkPtr)
				{
					return;
				}

				if (!MidChunkPtr->IsLoaded(BulkDataTimestamp))
				{
					const TSharedRef<FVoxelHeightMidChunkHint> Hint = MakeShared<FVoxelHeightMidChunkHint>();
					Hint->RootHash = RootHash;
					Hint->FarChunkKey = FarChunkKey;
					Hint->MidChunkKey = LocalMidChunkKey;

					MidChunksToLoad.Add({ *MidChunkPtr, FVoxelBulkHint(Hint) });
					return;
				}

				const FIntPoint MidChunkKey = FarChunkKey * ChunkSize + LocalMidChunkKey;
				const FVoxelHeightMidChunk& MidChunk = MidChunkPtr->Get();

				KeyToMidChunkData.Emplace(MidChunkKey, &MidChunk.ChunkData);

				if (Quality < EVoxelHeightChunkQuality::Near)
				{
					return;
				}

				const FVoxelIntBox2D LocalNearBounds =
					NearBounds
					.ShiftBy(-MidChunkKey * ChunkSize)
					.IntersectWith(FVoxelIntBox2D(0, ChunkSize));

				LocalNearBounds.Iterate([&](const FIntPoint& LocalNearChunkKey)
				{
					const TVoxelBulkPtr<FVoxelHeightNearChunk>* NearChunkPtr = MidChunk.KeyToNearChunk.Find(FVoxelHeightChunkKey(LocalNearChunkKey));
					if (!NearChunkPtr)
					{
						return;
					}

					if (!NearChunkPtr->IsLoaded(BulkDataTimestamp))
					{
						const TSharedRef<FVoxelHeightNearChunkHint> Hint = MakeShared<FVoxelHeightNearChunkHint>();
						Hint->RootHash = RootHash;
						Hint->FarChunkKey = FarChunkKey;
						Hint->MidChunkKey = LocalMidChunkKey;
						Hint->NearChunkKey = LocalNearChunkKey;

						NearChunksToLoad.Add({ *NearChunkPtr, FVoxelBulkHint(Hint) });
						return;
					}

					const FIntPoint NearChunkKey = MidChunkKey * ChunkSize + LocalNearChunkKey;
					const FVoxelHeightNearChunk& NearChunk = NearChunkPtr->Get();

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

	EVoxelHeightChunkQuality FinalQuality = Quality;
	if (NearChunksToLoad.Num() > 0)
	{
		FinalQuality = EVoxelHeightChunkQuality::Mid;
	}
	if (MidChunksToLoad.Num() > 0)
	{
		FinalQuality = EVoxelHeightChunkQuality::Far;
	}

	int32 SizeMultiplier;
	TVoxelChunkedArray<TPair<FIntPoint, const FVoxelHeightChunkData*>>* KeyToChunkData;

	switch (FinalQuality)
	{
	default: VOXEL_ASSUME(false);
	case EVoxelHeightChunkQuality::Far:
	{
		SizeMultiplier = ChunkSize * ChunkSize;
		KeyToChunkData = &KeyToFarChunkData;
	}
	break;
	case EVoxelHeightChunkQuality::Mid:
	{
		SizeMultiplier = ChunkSize;
		KeyToChunkData = &KeyToMidChunkData;
	}
	break;
	case EVoxelHeightChunkQuality::Near:
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

	TVoxelMap<FIntPoint, const FVoxelHeightHeightChunk*> KeyToHeightChunk;
	TVoxelMap<FIntPoint, const FVoxelHeightSurfaceTypeChunk*> KeyToSurfaceTypeChunk;
	TVoxelMap<FVoxelMetadataRef, TVoxelMap<FIntPoint, const FVoxelHeightMetadataChunk*>> MetadataRefToKeyToMetadataChunk;
	{
		VOXEL_SCOPE_COUNTER("Build maps");

		KeyToHeightChunk.Reserve(KeyToChunkData->Num());

		if (Query.bQuerySurfaceTypes)
		{
			KeyToSurfaceTypeChunk.Reserve(KeyToChunkData->Num());
		}

		for (const auto& It : *KeyToChunkData)
		{
			const FVoxelHeightChunkData& ChunkData = *It.Value;

			if (EnumHasAllFlags(Behavior, EVoxelStampBehavior::AffectShape) &&
				ChunkData.HeightChunk)
			{
				KeyToHeightChunk.Add_EnsureNew(It.Key, ChunkData.HeightChunk.Get());
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

				TVoxelMap<FIntPoint, const FVoxelHeightMetadataChunk*>& KeyToMetadataChunk = MetadataRefToKeyToMetadataChunk.FindOrAdd(MetadataIt.MetadataRef);
				if (KeyToMetadataChunk.Num() == 0)
				{
					KeyToMetadataChunk.Reserve(KeyToChunkData->Num());
				}

				KeyToMetadataChunk.Add_EnsureNew(It.Key, MetadataIt.Chunk.Get());
			}
		}
	}

	FVoxelHeightTransform SculptToQuery = StampToQuery;
	SculptToQuery.Scale *= ScaleXY * SizeMultiplier;
	SculptToQuery.ScaleZ *= SizeMultiplier;

	// Offset LODs. We build them based on higher quality chunks, which means they are inherently offset by half of their chunk size
	const double Offset = FinalQuality != EVoxelHeightChunkQuality::Near ? -0.5 : 0.;

	if (KeyToHeightChunk.Num() > 0)
	{
		const TVoxelHeightChunkTreeIterator<const FVoxelHeightHeightChunk> Iterator = TVoxelHeightChunkTreeIterator<const FVoxelHeightHeightChunk>::Create(
			KeyToHeightChunk,
			Query,
			SculptToQuery,
			Offset);

		FVoxelHeightChunkSamplers::ProcessHeights(
			Iterator,
			[&](
				const int32 Index,
				float NewHeight)
			{
				if (FVoxelUtilities::IsNaN(NewHeight))
				{
					return;
				}

				NewHeight = SculptToQuery.TransformHeight(NewHeight, Query.Positions[Index]);

				const int32 IndirectIndex = Query.GetIndirectIndex(Index);
				float& Height = Query.IndirectHeights[IndirectIndex];

				Height = FVoxelHeightChunkSamplers::BlendHeights(
					Height,
					NewHeight,
					BlendMode,
					bApplyOnVoid);
			},
			[&](
				const int32 Index,
				float NewHeight)
			{
				if (FVoxelUtilities::IsNaN(NewHeight))
				{
					return;
				}

				NewHeight = SculptToQuery.TransformHeight(NewHeight, Query.Positions[Index]);

				const int32 IndirectIndex = Query.GetIndirectIndex(Index);
				float& Height = Query.IndirectHeights[IndirectIndex];

				if (!FVoxelUtilities::IsNaN(Height))
				{
					NewHeight += Height;
				}

				Height = FVoxelHeightChunkSamplers::BlendHeights(
					Height,
					NewHeight,
					BlendMode,
					bApplyOnVoid);
			},
			[&](const int32 Index)
			{
			});
	}

	if (KeyToSurfaceTypeChunk.Num() > 0)
	{
		FVoxelHeightChunkSamplers::ProcessSurfaceTypes(
			TVoxelHeightChunkTreeIterator<const FVoxelHeightSurfaceTypeChunk>::Create(
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

				const int32 IndirectIndex = Query.GetIndirectIndex(Index);
				FVoxelSurfaceTypeBlend& SurfaceTypeRef = Query.IndirectSurfaceTypes[IndirectIndex];

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

		const TVoxelHeightChunkTreeIterator<const FVoxelHeightMetadataChunk> Iterator = TVoxelHeightChunkTreeIterator<const FVoxelHeightMetadataChunk>::Create(
			MetadataIt.Value,
			Query,
			SculptToQuery,
			Offset);

		FVoxelHeightMetadataChunk::SwitchType(MetadataRef, [&]<typename InnerType>()
		{
			TVoxelBufferType<InnerType>& Buffer = Query.IndirectMetadata.FindChecked<TVoxelBufferType<InnerType>>(MetadataRef);

			FVoxelHeightChunkSamplers::ProcessMetadata<InnerType>(Iterator, [&](
				const int32 Index,
				const float Alpha,
				const InnerType Value)
				{
					const int32 IndirectIndex = Query.GetIndirectIndex(Index);
					Buffer.Set(IndirectIndex, FMath::Lerp(Buffer[IndirectIndex], Value, Alpha));
				});
		});
	}
}

TVoxelFuture<TVoxelMap<FIntPoint, TSharedPtr<const FVoxelHeightNearChunk>>> FVoxelSculptHeightData::LoadNearChunks(
	const TSharedRef<IVoxelBulkLoader>& Loader,
	const FVoxelIntBox2D& NearBounds,
	const FVoxelBulkHash& RootHash) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(NearBounds.Count_double() < 1024 * 1024 * 1024))
	{
		return {};
	}

	const FVoxelIntBox2D MidBounds = NearBounds.DivideBigger(ChunkSize);
	const FVoxelIntBox2D FarBounds = MidBounds.DivideBigger(ChunkSize);

	struct FData
	{
		FVoxelCriticalSection CriticalSection;
		TVoxelMap<FIntPoint, TSharedPtr<const FVoxelHeightNearChunk>> KeyToNearChunk_RequiresLock;
	};

	const TSharedRef<FData> Data = MakeShared<FData>();
	Data->KeyToNearChunk_RequiresLock.Reserve(NearBounds.Count_int32());

	TVoxelChunkedArray<FVoxelFuture> FarFutures;

	FarBounds.Iterate([&](const FIntPoint& FarChunkKey)
	{
		const TVoxelBulkPtr<FVoxelHeightFarChunk>* FarChunkPtr = KeyToFarChunk.Find(FarChunkKey);
		if (!FarChunkPtr)
		{
			return;
		}

		const TSharedRef<FVoxelHeightFarChunkHint> FarChunkHint = MakeShared<FVoxelHeightFarChunkHint>();
		FarChunkHint->RootHash = RootHash;
		FarChunkHint->FarChunkKey = FarChunkKey;

		FarFutures.Add(FarChunkPtr->Load(*Loader, FVoxelBulkHint(FarChunkHint)).Then_AnyThread([=](const TSharedRef<const FVoxelHeightFarChunk>& FarChunk)
		{
			VOXEL_FUNCTION_COUNTER();

			const FVoxelIntBox2D LocalMidBounds = MidBounds.IntersectWith(FVoxelIntBox2D(FarChunkKey).Scale(ChunkSize));

			TVoxelChunkedArray<FVoxelFuture> MidFutures;

			LocalMidBounds.Iterate([&](const FIntPoint& MidChunkKey)
			{
				const TVoxelBulkPtr<FVoxelHeightMidChunk>* MidChunkPtr = FarChunk->KeyToMidChunk.Find(FVoxelHeightChunkKey(MidChunkKey - FarChunkKey * ChunkSize));
				if (!MidChunkPtr)
				{
					return;
				}

				const TSharedRef<FVoxelHeightMidChunkHint> MidChunkHint = MakeShared<FVoxelHeightMidChunkHint>();
				MidChunkHint->RootHash = RootHash;
				MidChunkHint->FarChunkKey = FarChunkKey;
				MidChunkHint->MidChunkKey = MidChunkKey;

				MidFutures.Add(MidChunkPtr->Load(*Loader, FVoxelBulkHint(MidChunkHint)).Then_AnyThread([=](const TSharedRef<const FVoxelHeightMidChunk>& MidChunk)
				{
					VOXEL_FUNCTION_COUNTER();

					const FVoxelIntBox2D LocalNearBounds = NearBounds.IntersectWith(FVoxelIntBox2D(MidChunkKey).Scale(ChunkSize));

					TVoxelChunkedArray<FVoxelFuture> NearFutures;

					LocalNearBounds.Iterate([&](const FIntPoint& NearChunkKey)
					{
						const TVoxelBulkPtr<FVoxelHeightNearChunk>* NearChunkPtr = MidChunk->KeyToNearChunk.Find(FVoxelHeightChunkKey(NearChunkKey - MidChunkKey * ChunkSize));
						if (!NearChunkPtr)
						{
							return;
						}

						const TSharedRef<FVoxelHeightNearChunkHint> NearChunkHint = MakeShared<FVoxelHeightNearChunkHint>();
						NearChunkHint->RootHash = RootHash;
						NearChunkHint->FarChunkKey = FarChunkKey;
						NearChunkHint->MidChunkKey = MidChunkKey;
						NearChunkHint->NearChunkKey = NearChunkKey;

						NearFutures.Add(NearChunkPtr->Load(*Loader, FVoxelBulkHint(NearChunkHint)).Then_AnyThread([=](const TSharedRef<const FVoxelHeightNearChunk>& NearChunk)
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

TVoxelFuture<const FVoxelSculptHeightData> FVoxelSculptHeightData::ApplyModifier(
	const TSharedRef<IVoxelBulkLoader>& Loader,
	const FVoxelSculptHeightContext& Context,
	const FVoxelBulkHash& RootHash,
	const TSharedRef<const IVoxelHeightModifierRuntime>& Modifier) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelIntBox2D BoundsToSculpt =
		FVoxelIntBox2D::FromFloatBox_WithPadding(
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

	if (GVoxelShowHeightSculptBounds)
	{
		FVoxelDebugDrawer()
			.Color(FLinearColor::Blue)
			.LifeTime(1.f)
			.DrawBox(BoundsToSculpt.ToVoxelBox2D().ToBox3D_Infinite(), Context.SculptToWorld.To3DMatrix());
	}

	const FVoxelIntBox2D ChunkKeyBounds = BoundsToSculpt.DivideExact(ChunkSize);

	return
		FVoxelHeightSculptCanvas::CreateCanvas(Loader, SharedThis(this), Context, ChunkKeyBounds, RootHash)
		.Then_AsyncThread([=](const TSharedRef<FVoxelHeightSculptCanvas>& Canvas)
		{
			VOXEL_FUNCTION_COUNTER();

			FVoxelHeightSculptCanvasWriter CanvasWriter(
				*Canvas,
				Context.SculptToWorld,
				Context.ScaleZ,
				Context.OffsetZ);

			Modifier->Apply(CanvasWriter);

			return Canvas->Finalize(Context.TargetPrecision);
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptHeightData::ComputeBounds()
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelIntBox2D Bounds = FVoxelIntBox2D::InvertedInfinite;
	double HeightMin = -1.f;
	double HeightMax = 1.f;

	const auto UpdateRange = [&](const FVoxelHeightChunkData& ChunkData)
	{
		const FDoubleInterval Range = ChunkData.GetRange();
		if (!Range.IsValid())
		{
			return;
		}

		HeightMin = FMath::Min(Range.Min, HeightMin);
		HeightMax = FMath::Max(Range.Max, HeightMax);
	};

	for (const auto& FarChunkIt : KeyToFarChunk)
	{
		const FIntPoint FarChunkKey = FarChunkIt.Key;
		const FVoxelIntBox2D FarChunkBounds = FVoxelIntBox2D(FarChunkKey).Scale(ChunkSize * ChunkSize * ChunkSize);

		if (FarChunkIt.Value.IsLoaded())
		{
			UpdateRange(FarChunkIt.Value->ChunkData);
		}

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
			const FIntPoint MidChunkKey = FarChunkKey * ChunkSize + MidChunkIt.Key.ToPoint();
			const FVoxelIntBox2D MidChunkBounds = FVoxelIntBox2D(MidChunkKey).Scale(ChunkSize * ChunkSize);

			if (MidChunkIt.Value.IsLoaded())
			{
				UpdateRange(MidChunkIt.Value->ChunkData);
			}

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
				const FIntPoint NearChunkKey = MidChunkKey * ChunkSize + NearChunkIt.Key.ToPoint();
				const FVoxelIntBox2D NearChunkBounds = FVoxelIntBox2D(NearChunkKey).Scale(ChunkSize);

				if (NearChunkIt.Value.IsLoaded())
				{
					UpdateRange(NearChunkIt.Value->ChunkData);
				}

				Bounds += NearChunkBounds;
			}
		}
	}

	if (!Bounds.IsValid())
	{
		PrivateBounds = FVoxelBox();
		return;
	}

	PrivateBounds = Bounds.ToVoxelBox2D().ToBox3D(HeightMin, HeightMax);
}