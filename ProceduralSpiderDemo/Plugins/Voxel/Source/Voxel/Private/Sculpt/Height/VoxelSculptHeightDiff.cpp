// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSculptHeightDiff.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"

TSharedRef<const FVoxelSculptHeightDiff> FVoxelSculptHeightDiff::Create(
	const TVoxelBulkRef<FVoxelSculptHeightData>& OldData,
	const TVoxelBulkRef<FVoxelSculptHeightData>& NewData)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelSculptHeightDiff> Result = MakeShared<FVoxelSculptHeightDiff>();
	Result->OldHash = OldData.GetHash();
	Result->NewData = NewData;

	FVoxelUtilities::IterateSortedMaps(
		OldData->GetKeyToFarChunk(),
		NewData->GetKeyToFarChunk(),
		[&](
			const FIntPoint& Key,
			const TVoxelBulkPtr<FVoxelHeightFarChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelHeightFarChunk>* ChunkBPtr)
		{
			TVoxelOptional<FFarChunkDiff> FarChunkDiff = Create(
				Key,
				ChunkAPtr ? *ChunkAPtr : nullptr,
				ChunkBPtr ? *ChunkBPtr : nullptr);

			if (!FarChunkDiff)
			{
				return;
			}

			Result->FarChunkDiffs.Add(MoveTemp(*FarChunkDiff));
		});

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelBulkPtr<FVoxelSculptHeightData> FVoxelSculptHeightDiff::TryApply(
	const TVoxelBulkRef<FVoxelSculptHeightData>& OldData,
	const float TargetPrecision) const
{
	VOXEL_FUNCTION_COUNTER();

	if (OldData.GetHash() == OldHash)
	{
		return NewData;
	}

	TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>> NewKeyToFarChunk = OldData->GetKeyToFarChunk();

	for (const FFarChunkDiff& FarChunkDiff : FarChunkDiffs)
	{
		const TVoxelOptional<TVoxelBulkPtr<FVoxelHeightFarChunk>> NewFarChunk = TryApply(
			FarChunkDiff,
			NewKeyToFarChunk.FindRef(FarChunkDiff.ChunkKey),
			TargetPrecision);

		if (!NewFarChunk)
		{
			return {};
		}

		NewKeyToFarChunk.FindOrAdd(FarChunkDiff.ChunkKey) = NewFarChunk.GetValue();
	}

	for (auto It = NewKeyToFarChunk.CreateIterator(); It; ++It)
	{
		if (!It.Value())
		{
			It.RemoveCurrent();
		}
	}

	NewKeyToFarChunk.KeySort();

	return TVoxelBulkPtr<FVoxelSculptHeightData>(FVoxelSculptHeightData::Create(MoveTemp(NewKeyToFarChunk)));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelOptional<FVoxelSculptHeightDiff::FFarChunkDiff> FVoxelSculptHeightDiff::Create(
	const FIntPoint& ChunkKey,
	const TVoxelBulkPtr<FVoxelHeightFarChunk> OldFarChunk,
	const TVoxelBulkPtr<FVoxelHeightFarChunk> NewFarChunk)
{
	if (OldFarChunk &&
		!NewFarChunk)
	{
		return FFarChunkDiff
		{
			ChunkKey,
			OldFarChunk.GetHash(),
			{},
			{}
		};
	}

	if (NewFarChunk &&
		!OldFarChunk)
	{
		return FFarChunkDiff
		{
			ChunkKey,
			FVoxelBulkHash(),
			NewFarChunk,
			{}
		};
	}

	checkVoxelSlow(OldFarChunk && NewFarChunk);

	if (OldFarChunk.GetHash() == NewFarChunk.GetHash())
	{
		return {};
	}

	if (!ensure(OldFarChunk.IsLoaded()) ||
		!ensure(NewFarChunk.IsLoaded()))
	{
		return {};
	}

	FFarChunkDiff FarChunk;
	FarChunk.ChunkKey = ChunkKey;
	FarChunk.OldHash = OldFarChunk.GetHash();
	FarChunk.NewChunk = NewFarChunk;

	FVoxelUtilities::IterateSortedMaps(
		OldFarChunk->KeyToMidChunk,
		NewFarChunk->KeyToMidChunk,
		[&](
			const FVoxelHeightChunkKey& Key,
			const TVoxelBulkPtr<FVoxelHeightMidChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelHeightMidChunk>* ChunkBPtr)
		{
			TVoxelOptional<FMidChunkDiff> MidChunkDiff = Create(
				Key,
				ChunkAPtr ? *ChunkAPtr : nullptr,
				ChunkBPtr ? *ChunkBPtr : nullptr);

			if (!MidChunkDiff)
			{
				return;
			}

			FarChunk.MidChunkDiffs.Add(MoveTemp(*MidChunkDiff));
		});

	return FarChunk;
}

TVoxelOptional<FVoxelSculptHeightDiff::FMidChunkDiff> FVoxelSculptHeightDiff::Create(
	const FVoxelHeightChunkKey ChunkKey,
	const TVoxelBulkPtr<FVoxelHeightMidChunk> OldMidChunk,
	const TVoxelBulkPtr<FVoxelHeightMidChunk> NewMidChunk)
{
	if (OldMidChunk &&
		!NewMidChunk)
	{
		return FMidChunkDiff
		{
			ChunkKey,
			OldMidChunk.GetHash(),
			{},
			{}
		};
	}

	if (NewMidChunk &&
		!OldMidChunk)
	{
		return FMidChunkDiff
		{
			ChunkKey,
			FVoxelBulkHash(),
			NewMidChunk,
			{}
		};
	}

	checkVoxelSlow(OldMidChunk && NewMidChunk);

	if (OldMidChunk.GetHash() == NewMidChunk.GetHash())
	{
		return {};
	}

	if (!ensure(OldMidChunk.IsLoaded()) ||
		!ensure(NewMidChunk.IsLoaded()))
	{
		return {};
	}

	FMidChunkDiff MidChunk;
	MidChunk.ChunkKey = ChunkKey;
	MidChunk.OldHash = OldMidChunk.GetHash();
	MidChunk.NewChunk = NewMidChunk;

	FVoxelUtilities::IterateSortedMaps(
		OldMidChunk->KeyToNearChunk,
		NewMidChunk->KeyToNearChunk,
		[&](
			const FVoxelHeightChunkKey& Key,
			const TVoxelBulkPtr<FVoxelHeightNearChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelHeightNearChunk>* ChunkBPtr)
		{
			TVoxelOptional<FNearChunkDiff> NearChunkDiff = Create(
				Key,
				ChunkAPtr ? *ChunkAPtr : nullptr,
				ChunkBPtr ? *ChunkBPtr : nullptr);

			if (!NearChunkDiff)
			{
				return;
			}

			MidChunk.NearChunkDiffs.Add(MoveTemp(*NearChunkDiff));
		});

	return MidChunk;
}

TVoxelOptional<FVoxelSculptHeightDiff::FNearChunkDiff> FVoxelSculptHeightDiff::Create(
	const FVoxelHeightChunkKey ChunkKey,
	const TVoxelBulkPtr<FVoxelHeightNearChunk> OldNearChunk,
	const TVoxelBulkPtr<FVoxelHeightNearChunk> NewNearChunk)
{
	if (OldNearChunk &&
		NewNearChunk &&
		OldNearChunk.GetHash() == NewNearChunk.GetHash())
	{
		return {};
	}

	return FNearChunkDiff
	{
		ChunkKey,
		OldNearChunk.GetHashOrNull(),
		NewNearChunk
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelOptional<TVoxelBulkPtr<FVoxelHeightFarChunk>> FVoxelSculptHeightDiff::TryApply(
	const FFarChunkDiff& FarChunkDiff,
	const TVoxelBulkPtr<FVoxelHeightFarChunk>& OldFarChunk,
	const float TargetPrecision)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelBulkHash OldHash = OldFarChunk.GetHashOrNull();

	if (OldHash == FarChunkDiff.OldHash)
	{
		return FarChunkDiff.NewChunk;
	}

	TVoxelMap<FVoxelHeightChunkKey, TVoxelBulkPtr<FVoxelHeightMidChunk>> NewKeyToMidChunk;
	if (OldFarChunk)
	{
		NewKeyToMidChunk = OldFarChunk->KeyToMidChunk;
	}

	for (const FMidChunkDiff& MidChunkDiff : FarChunkDiff.MidChunkDiffs)
	{
		const TVoxelOptional<TVoxelBulkPtr<FVoxelHeightMidChunk>> NewMidChunk = TryApply(
			MidChunkDiff,
			NewKeyToMidChunk.FindRef(MidChunkDiff.ChunkKey),
			TargetPrecision);

		if (!NewMidChunk)
		{
			return {};
		}

		NewKeyToMidChunk.FindOrAdd(MidChunkDiff.ChunkKey) = NewMidChunk.GetValue();
	}

	for (auto It = NewKeyToMidChunk.CreateIterator(); It; ++It)
	{
		if (!It.Value())
		{
			It.RemoveCurrent();
		}
	}

	NewKeyToMidChunk.KeySort();

	return FVoxelHeightFarChunk::Create(MoveTemp(NewKeyToMidChunk), TargetPrecision);
}

TVoxelOptional<TVoxelBulkPtr<FVoxelHeightMidChunk>> FVoxelSculptHeightDiff::TryApply(
	const FMidChunkDiff& MidChunkDiff,
	const TVoxelBulkPtr<FVoxelHeightMidChunk>& OldMidChunk,
	const float TargetPrecision)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelBulkHash OldHash = OldMidChunk.GetHashOrNull();

	if (OldHash == MidChunkDiff.OldHash)
	{
		return MidChunkDiff.NewChunk;
	}

	TVoxelMap<FVoxelHeightChunkKey, TVoxelBulkPtr<FVoxelHeightNearChunk>> NewKeyToNearChunk;
	if (OldMidChunk)
	{
		NewKeyToNearChunk = OldMidChunk->KeyToNearChunk;
	}

	for (const FNearChunkDiff& NearChunkDiff : MidChunkDiff.NearChunkDiffs)
	{
		const TVoxelOptional<TVoxelBulkPtr<FVoxelHeightNearChunk>> NewNearChunk = TryApply(
			NearChunkDiff,
			NewKeyToNearChunk.FindRef(NearChunkDiff.ChunkKey));

		if (!NewNearChunk)
		{
			return {};
		}

		NewKeyToNearChunk.FindOrAdd(NearChunkDiff.ChunkKey) = NewNearChunk.GetValue();
	}

	for (auto It = NewKeyToNearChunk.CreateIterator(); It; ++It)
	{
		if (!It.Value())
		{
			It.RemoveCurrent();
		}
	}

	NewKeyToNearChunk.KeySort();

	return FVoxelHeightMidChunk::Create(MoveTemp(NewKeyToNearChunk), TargetPrecision);
}

TVoxelOptional<TVoxelBulkPtr<FVoxelHeightNearChunk>> FVoxelSculptHeightDiff::TryApply(
	const FNearChunkDiff& NearChunkDiff,
	const TVoxelBulkPtr<FVoxelHeightNearChunk>& OldNearChunk)
{
	const FVoxelBulkHash OldHash = OldNearChunk.GetHashOrNull();

	if (OldHash == NearChunkDiff.OldHash)
	{
		return NearChunkDiff.NewChunk;
	}

	return {};
}