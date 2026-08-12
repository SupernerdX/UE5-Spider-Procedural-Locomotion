// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelGraphStamp.h"
#include "VoxelQuery.h"
#include "VoxelStampRef.h"
#include "VoxelHeightStampRef.h"
#include "VoxelVolumeStampRef.h"

EVoxelPinValueOpsUsage FVoxelPinValueOps_FVoxelHeightGraphStampWrapper::GetUsage() const
{
	return
		EVoxelPinValueOpsUsage::GetExposedType |
		EVoxelPinValueOpsUsage::MakeRuntimeValue |
		EVoxelPinValueOpsUsage::HasPinDefaultValue |
		EVoxelPinValueOpsUsage::CustomMetaData;
}

FVoxelPinType FVoxelPinValueOps_FVoxelHeightGraphStampWrapper::GetExposedType() const
{
	return FVoxelPinType::Make<FVoxelHeightInstancedStampRef>();
}

FVoxelRuntimePinValue FVoxelPinValueOps_FVoxelHeightGraphStampWrapper::MakeRuntimeValue(
	const FVoxelPinValue& Value,
	const FVoxelPinType::FRuntimeValueContext& Context) const
{
	if (!ensure(Value.Is<FVoxelHeightInstancedStampRef>()))
	{
		return {};
	}

	const TSharedPtr<FVoxelStampRuntime> Runtime = FVoxelStampRuntime::Create(
		{},
		Value.Get<FVoxelHeightInstancedStampRef>(),
		{});

	const TSharedRef<FVoxelHeightGraphStampWrapper> GraphStamp = MakeShared<FVoxelHeightGraphStampWrapper>();
	GraphStamp->Stamp = CastStructEnsured<FVoxelHeightStampRuntime>(Runtime);
	return FVoxelRuntimePinValue::Make(GraphStamp);
}

bool FVoxelPinValueOps_FVoxelHeightGraphStampWrapper::HasPinDefaultValue() const
{
	// Storing the stamp in a string is messy
	return false;
}

#if WITH_EDITOR
TMap<FName, FString> FVoxelPinValueOps_FVoxelHeightGraphStampWrapper::GetMetaData() const
{
	return TMap<FName, FString>
	{
		{ "HideStampData", "" }
	};
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EVoxelPinValueOpsUsage FVoxelPinValueOps_FVoxelVolumeGraphStampWrapper::GetUsage() const
{
	return
		EVoxelPinValueOpsUsage::GetExposedType |
		EVoxelPinValueOpsUsage::MakeRuntimeValue |
		EVoxelPinValueOpsUsage::HasPinDefaultValue |
		EVoxelPinValueOpsUsage::CustomMetaData;
}

FVoxelPinType FVoxelPinValueOps_FVoxelVolumeGraphStampWrapper::GetExposedType() const
{
	return FVoxelPinType::Make<FVoxelVolumeInstancedStampRef>();
}

FVoxelRuntimePinValue FVoxelPinValueOps_FVoxelVolumeGraphStampWrapper::MakeRuntimeValue(
	const FVoxelPinValue& Value,
	const FVoxelPinType::FRuntimeValueContext& Context) const
{
	if (!ensure(Value.Is<FVoxelVolumeInstancedStampRef>()))
	{
		return {};
	}

	const TSharedPtr<FVoxelStampRuntime> Runtime = FVoxelStampRuntime::Create(
		{},
		Value.Get<FVoxelVolumeInstancedStampRef>(),
		{});

	const TSharedRef<FVoxelVolumeGraphStampWrapper> GraphStamp = MakeShared<FVoxelVolumeGraphStampWrapper>();
	GraphStamp->Stamp = CastStructEnsured<FVoxelVolumeStampRuntime>(Runtime);
	return FVoxelRuntimePinValue::Make(GraphStamp);
}

bool FVoxelPinValueOps_FVoxelVolumeGraphStampWrapper::HasPinDefaultValue() const
{
	// Storing the stamp in a string is messy
	return false;
}

#if WITH_EDITOR
TMap<FName, FString> FVoxelPinValueOps_FVoxelVolumeGraphStampWrapper::GetMetaData() const
{
	return TMap<FName, FString>
	{
		{ "HideStampData", "" }
	};
}
#endif