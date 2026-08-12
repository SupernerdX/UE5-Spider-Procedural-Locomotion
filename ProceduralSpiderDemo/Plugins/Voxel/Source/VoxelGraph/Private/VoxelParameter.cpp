// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelParameter.h"
#include "VoxelPinValueOps.h"
#include "FunctionLibrary/VoxelCurveFunctionLibrary.h"

void FVoxelParameter::Fixup()
{
	if (!Type.IsValid())
	{
		Type = FVoxelPinType::Make<float>();
	}

	// TODO REMOVE MIGRATION
	if (Type.Is<FVoxelCurve>())
	{
		Type = FVoxelPinType::Make<FVoxelRuntimeCurveRef>();
	}
}

#if WITH_EDITOR
TMap<FName, FString> FVoxelParameter::GetMetaData() const
{
	TMap<FName, FString> Result = MetaData;

	if (const TSharedPtr<FVoxelPinValueOps> Ops = FVoxelPinValueOps::Find(Type, EVoxelPinValueOpsUsage::CustomMetaData))
	{
		Result.Append(Ops->GetMetaData());
	}

	return Result;
}
#endif