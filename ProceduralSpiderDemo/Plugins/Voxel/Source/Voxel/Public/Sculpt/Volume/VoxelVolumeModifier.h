// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkData.h"
#include "VoxelVolumeModifier.generated.h"

class FVoxelVolumeSculptCanvasWriter;

class VOXEL_API IVoxelVolumeModifierRuntime
{
public:
	IVoxelVolumeModifierRuntime() = default;
	virtual ~IVoxelVolumeModifierRuntime() = default;

	virtual FVoxelBox GetBounds() const = 0;
	virtual void Apply(FVoxelVolumeSculptCanvasWriter& Writer) const = 0;
};

USTRUCT()
struct VOXEL_API FVoxelVolumeModifier : public FVoxelVirtualStruct
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	virtual TSharedPtr<IVoxelVolumeModifierRuntime> GetRuntime() const VOXEL_PURE_VIRTUAL({});
};

USTRUCT()
struct VOXEL_API FVoxelVolumeSerializedModifier final : public FVoxelBulkData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	TSharedPtr<FVoxelVolumeModifier> Modifier;

	FVoxelVolumeSerializedModifier() = default;
	explicit FVoxelVolumeSerializedModifier(const TSharedRef<FVoxelVolumeModifier>& Modifier)
		: Modifier(Modifier)
	{
	}

public:
	//~ Begin FVoxelBulkData Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelBulkData Interface
};