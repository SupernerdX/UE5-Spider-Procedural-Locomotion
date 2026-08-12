// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampIndex.h"

struct FVoxelStamp;
struct FVoxelStampRuntime;

class VOXEL_API FVoxelStampRefInner : public TSharedFromThis<FVoxelStampRefInner>
{
public:
	void Load(FArchive& Ar);
	void Save(FArchive& Ar) const;

	void NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);

	FVoxelStampRefInner() = default;
	~FVoxelStampRefInner();

private:
	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion
	);

	TSharedPtr<FVoxelStamp> Stamp;
	FVoxelStampIndex Index;

#if WITH_EDITOR
	FSimpleMulticastDelegate OnRefreshDetails_Editor;
	TVoxelMap<UScriptStruct*, TSharedPtr<FVoxelStamp>> StructToStamp_Editor;
#endif

	friend struct FVoxelStampRef;
};