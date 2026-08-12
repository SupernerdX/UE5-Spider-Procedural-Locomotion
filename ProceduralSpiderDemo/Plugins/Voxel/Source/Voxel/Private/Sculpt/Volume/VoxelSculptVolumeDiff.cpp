// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSculptVolumeDiff.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"

TSharedRef<const FVoxelSculptVolumeDiff> FVoxelSculptVolumeDiff::Create(
	const TVoxelBulkRef<FVoxelSculptVolumeData>& OldData,
	const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelSculptVolumeDiff> Result = MakeShared<FVoxelSculptVolumeDiff>();
	Result->OldHash = OldData.GetHash();
	Result->NewData = NewData;

	FVoxelUtilities::IterateSortedMaps(
		OldData->GetKeyToFarChunk(),
		NewData->GetKeyToFarChunk(),
		[&](
			const FIntVector& Key,
			const TVoxelBulkPtr<FVoxelVolumeFarChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelVolumeFarChunk>* ChunkBPtr)
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

TVoxelBulkPtr<FVoxelSculptVolumeData> FVoxelSculptVolumeDiff::TryApply(
	const TVoxelBulkRef<FVoxelSculptVolumeData>& OldData,
	const float MaxErrorPercentage) const
{
	VOXEL_FUNCTION_COUNTER();

	if (OldData.GetHash() == OldHash)
	{
		return NewData;
	}

	TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>> NewKeyToFarChunk = OldData->GetKeyToFarChunk();

	for (const FFarChunkDiff& FarChunkDiff : FarChunkDiffs)
	{
		const TVoxelOptional<TVoxelBulkPtr<FVoxelVolumeFarChunk>> NewFarChunk = TryApply(
			FarChunkDiff,
			NewKeyToFarChunk.FindRef(FarChunkDiff.ChunkKey),
			MaxErrorPercentage);

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

	return TVoxelBulkPtr<FVoxelSculptVolumeData>(FVoxelSculptVolumeData::Create(MoveTemp(NewKeyToFarChunk)));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelOptional<FVoxelSculptVolumeDiff::FFarChunkDiff> FVoxelSculptVolumeDiff::Create(
	const FIntVector& ChunkKey,
	const TVoxelBulkPtr<FVoxelVolumeFarChunk> OldFarChunk,
	const TVoxelBulkPtr<FVoxelVolumeFarChunk> NewFarChunk)
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
			const FVoxelVolumeChunkKey& Key,
			const TVoxelBulkPtr<FVoxelVolumeMidChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelVolumeMidChunk>* ChunkBPtr)
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

TVoxelOptional<FVoxelSculptVolumeDiff::FMidChunkDiff> FVoxelSculptVolumeDiff::Create(
	const FVoxelVolumeChunkKey ChunkKey,
	const TVoxelBulkPtr<FVoxelVolumeMidChunk> OldMidChunk,
	const TVoxelBulkPtr<FVoxelVolumeMidChunk> NewMidChunk)
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
			const FVoxelVolumeChunkKey& Key,
			const TVoxelBulkPtr<FVoxelVolumeNearChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelVolumeNearChunk>* ChunkBPtr)
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

TVoxelOptional<FVoxelSculptVolumeDiff::FNearChunkDiff> FVoxelSculptVolumeDiff::Create(
	const FVoxelVolumeChunkKey ChunkKey,
	const TVoxelBulkPtr<FVoxelVolumeNearChunk> OldNearChunk,
	const TVoxelBulkPtr<FVoxelVolumeNearChunk> NewNearChunk)
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

TVoxelOptional<TVoxelBulkPtr<FVoxelVolumeFarChunk>> FVoxelSculptVolumeDiff::TryApply(
	const FFarChunkDiff& FarChunkDiff,
	const TVoxelBulkPtr<FVoxelVolumeFarChunk>& OldFarChunk,
	const float MaxErrorPercentage)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelBulkHash OldHash = OldFarChunk.GetHashOrNull();

	if (OldHash == FarChunkDiff.OldHash)
	{
		return FarChunkDiff.NewChunk;
	}

	TVoxelMap<FVoxelVolumeChunkKey, TVoxelBulkPtr<FVoxelVolumeMidChunk>> NewKeyToMidChunk;
	if (OldFarChunk)
	{
		NewKeyToMidChunk = OldFarChunk->KeyToMidChunk;
	}

	for (const FMidChunkDiff& MidChunkDiff : FarChunkDiff.MidChunkDiffs)
	{
		const TVoxelOptional<TVoxelBulkPtr<FVoxelVolumeMidChunk>> NewMidChunk = TryApply(
			MidChunkDiff,
			NewKeyToMidChunk.FindRef(MidChunkDiff.ChunkKey),
			MaxErrorPercentage);

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

	return FVoxelVolumeFarChunk::Create(MoveTemp(NewKeyToMidChunk), MaxErrorPercentage);
}

TVoxelOptional<TVoxelBulkPtr<FVoxelVolumeMidChunk>> FVoxelSculptVolumeDiff::TryApply(
	const FMidChunkDiff& MidChunkDiff,
	const TVoxelBulkPtr<FVoxelVolumeMidChunk>& OldMidChunk,
	const float MaxErrorPercentage)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelBulkHash OldHash = OldMidChunk.GetHashOrNull();

	if (OldHash == MidChunkDiff.OldHash)
	{
		return MidChunkDiff.NewChunk;
	}

	TVoxelMap<FVoxelVolumeChunkKey, TVoxelBulkPtr<FVoxelVolumeNearChunk>> NewKeyToNearChunk;
	if (OldMidChunk)
	{
		NewKeyToNearChunk = OldMidChunk->KeyToNearChunk;
	}

	for (const FNearChunkDiff& NearChunkDiff : MidChunkDiff.NearChunkDiffs)
	{
		const TVoxelOptional<TVoxelBulkPtr<FVoxelVolumeNearChunk>> NewNearChunk = TryApply(
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

	return FVoxelVolumeMidChunk::Create(MoveTemp(NewKeyToNearChunk), MaxErrorPercentage);
}

TVoxelOptional<TVoxelBulkPtr<FVoxelVolumeNearChunk>> FVoxelSculptVolumeDiff::TryApply(
	const FNearChunkDiff& NearChunkDiff,
	const TVoxelBulkPtr<FVoxelVolumeNearChunk>& OldNearChunk)
{
	const FVoxelBulkHash OldHash = OldNearChunk.GetHashOrNull();

	if (OldHash == NearChunkDiff.OldHash)
	{
		return NearChunkDiff.NewChunk;
	}

	return {};
}