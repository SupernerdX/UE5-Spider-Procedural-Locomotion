// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/VoxelSculptGraphFunctionLibrary.h"
#include "VoxelBufferSplitter.h"

void FVoxelGraphParameters::FHeightSculpt::Split(
	const FVoxelBufferSplitter& Splitter,
	const TConstVoxelArrayView<FHeightSculpt*> OutResult) const
{
	VOXEL_FUNCTION_COUNTER();

	{
		TVoxelInlineArray<FVoxelFloatBuffer*, 8> Buffers;
		FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Buffers[Index] = &OutResult[Index]->PreviousHeights;
		}

		PreviousHeights.Split(Splitter, Buffers);
	}

	{
		TVoxelInlineArray<FVoxelBoolBuffer*, 8> Buffers;
		FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Buffers[Index] = &OutResult[Index]->IsValid;
		}

		IsValid.Split(Splitter, Buffers);
	}
}

void FVoxelGraphParameters::FVolumeSculpt::Split(
	const FVoxelBufferSplitter& Splitter,
	const TConstVoxelArrayView<FVolumeSculpt*> OutResult) const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelInlineArray<FVoxelFloatBuffer*, 8> Buffers;
	FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

	for (const int32 Index : Splitter.GetValidOutputs())
	{
		Buffers[Index] = &OutResult[Index]->PreviousDistances;
	}

	PreviousDistances.Split(Splitter, Buffers);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewDataNode_HeightSculptPreviewData::Query_WithPosition(const FVoxelGraphQueryImpl& Query, FVoxelGraphQueryImpl& OutQuery) const
{
	FVoxelGraphParameters::FHeightSculpt& HeightSculpt = OutQuery.AddParameter<FVoxelGraphParameters::FHeightSculpt>();
	HeightSculpt.PreviousHeights = *HeightPin.GetSynchronous(Query);
	HeightSculpt.IsValid = *IsValidPin.GetSynchronous(Query);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewDataNode_VolumeSculptPreviewData::Query_WithPosition(const FVoxelGraphQueryImpl& Query, FVoxelGraphQueryImpl& OutQuery) const
{
	FVoxelGraphParameters::FVolumeSculpt& VolumeSculpt = OutQuery.AddParameter<FVoxelGraphParameters::FVolumeSculpt>();
	VolumeSculpt.PreviousDistances = *DistancePin.GetSynchronous(Query);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSculptGraphFunctionLibrary::GetPreviousHeight(
	FVoxelFloatBuffer& Height,
	FVoxelBoolBuffer& IsValid) const
{
	const FVoxelGraphParameters::FHeightSculpt* Parameter = Query->FindParameter<FVoxelGraphParameters::FHeightSculpt>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Height Sculpt Preview Data in Editor Graph found", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No height sculpt data", this);
		}
		return;
	}

	Height = Parameter->PreviousHeights;
	IsValid = Parameter->IsValid;
}

FVoxelFloatBuffer UVoxelSculptGraphFunctionLibrary::GetPreviousDistance() const
{
	const FVoxelGraphParameters::FVolumeSculpt* Parameter = Query->FindParameter<FVoxelGraphParameters::FVolumeSculpt>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Volume Sculpt Preview Data in Editor Graph found", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No volume sculpt data", this);
		}
		return DefaultBuffer;
	}

	return Parameter->PreviousDistances;
}