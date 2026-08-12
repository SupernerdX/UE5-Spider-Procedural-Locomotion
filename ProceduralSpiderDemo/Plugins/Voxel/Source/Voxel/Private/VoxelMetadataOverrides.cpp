// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMetadataOverrides.h"
#include "VoxelMetadata.h"
#include "VoxelStampQuery.h"
#include "VoxelCompiledGraph.h"
#include "Graphs/VoxelOutputNode_MetadataBase.h"

void FVoxelMetadataOverrides::Fixup()
{
	VOXEL_FUNCTION_COUNTER();
	checkUObjectAccess();

	for (FVoxelMetadataOverride& Override : Overrides)
	{
		if (!Override.Metadata)
		{
			continue;
		}

		Override.Value.Fixup(Override.Metadata->GetInnerType().GetExposedType());
	}
}

TSharedRef<FVoxelRuntimeMetadataOverrides> FVoxelMetadataOverrides::CreateRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	ConstCast(*this).Fixup();

	const TSharedRef<FVoxelRuntimeMetadataOverrides> Result = MakeShared<FVoxelRuntimeMetadataOverrides>();

	TVoxelMap<FVoxelMetadataRef, FVoxelRuntimeMetadataOverrides::FMetadataValue>& MetadataToValue = Result->MetadataToValue;
	MetadataToValue.Reserve(Overrides.Num());

	for (const FVoxelMetadataOverride& Override : Overrides)
	{
		if (!Override.Metadata ||
			!ensureVoxelSlow(Override.Value.IsValid()))
		{
			continue;
		}

		FVoxelRuntimeMetadataOverrides::FMetadataValue& Value = MetadataToValue.FindOrAdd(FVoxelMetadataRef(Override.Metadata));
		Value.PinIndex = -1;
		Value.Constant = FVoxelBuffer::MakeConstant(FVoxelPinType::MakeRuntimeValue(
			Override.Metadata->GetInnerType(),
			FVoxelPinValue(Override.Value),
			{}));
	}

	return Result;
}

TSharedRef<FVoxelRuntimeMetadataOverrides> FVoxelMetadataOverrides::CreateRuntime(
	const FVoxelGraphQueryImpl& GraphQuery,
	const TVoxelNodeEvaluator<FVoxelOutputNode_MetadataBase>& Evaluator) const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelRuntimeMetadataOverrides> Result = MakeShared<FVoxelRuntimeMetadataOverrides>();

	TVoxelMap<FVoxelMetadataRef, FVoxelRuntimeMetadataOverrides::FMetadataValue>& MetadataToValue = Result->MetadataToValue;
	MetadataToValue.Reserve(Evaluator->MetadataPins.Num() + Overrides.Num());

	for (int32 PinIndex = 0; PinIndex < Evaluator->MetadataPins.Num(); PinIndex++)
	{
		const FVoxelOutputNode_MetadataBase::FMetadataPin& MetadataPin = Evaluator->MetadataPins[PinIndex];

		const FVoxelMetadataRef Metadata = MetadataPin.Metadata.GetSynchronous(GraphQuery);
		if (!Metadata.IsValid())
		{
			continue;
		}

		if (MetadataPin.Value.GetType_RuntimeOnly() != Metadata.GetInnerType().GetBufferType())
		{
			VOXEL_MESSAGE(Error, "{0}: Mismatched pin type for Metadata {1}: Pin type is {2}, metadata type is {3}",
				GraphQuery.CompiledGraph.Graph,
				Metadata.GetMetadata(),
				MetadataPin.Value.GetInnerType_RuntimeOnly().ToString(),
				Metadata.GetInnerType().ToString());

			continue;
		}

		if (MetadataToValue.Contains(Metadata))
		{
			VOXEL_MESSAGE(Error, "{0}: Metadata {1} is outputted multiple times", GraphQuery.CompiledGraph.Graph, Metadata.GetMetadata());
			continue;
		}

		MetadataToValue.Add_EnsureNew(Metadata).PinIndex = PinIndex;
	}

	for (const FVoxelMetadataOverride& Override : Overrides)
	{
		if (!Override.Metadata ||
			!ensureVoxelSlow(Override.Value.IsValid()))
		{
			continue;
		}

		FVoxelRuntimeMetadataOverrides::FMetadataValue& Value = MetadataToValue.FindOrAdd(FVoxelMetadataRef(Override.Metadata));
		Value.PinIndex = -1;
		Value.Constant = FVoxelBuffer::MakeConstant(FVoxelPinType::MakeRuntimeValue(
			Override.Metadata->GetInnerType(),
			FVoxelPinValue(Override.Value),
			{}));
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelRuntimeMetadataOverrides::FMetadataValue::operator==(const FMetadataValue& Other) const
{
	if (PinIndex != Other.PinIndex ||
		Constant.IsValid() != Other.Constant.IsValid())
	{
		return false;
	}

	if (!Constant.IsValid())
	{
		return true;
	}

	if (Constant->GetStruct() != Other.Constant->GetStruct())
	{
		return false;
	}

	return Constant->Equal(*Other.Constant);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelRuntimeMetadataOverrides::ShouldCompute(const FVoxelStampSparseQuery& Query) const
{
	for (const auto& It : MetadataToValue)
	{
		if (Query.MetadatasToQuery.Contains(It.Key))
		{
			return true;
		}
	}

	return false;
}

bool FVoxelRuntimeMetadataOverrides::operator==(const FVoxelRuntimeMetadataOverrides& Other) const
{
	return MetadataToValue.OrderIndependentEqual(Other.MetadataToValue);
}