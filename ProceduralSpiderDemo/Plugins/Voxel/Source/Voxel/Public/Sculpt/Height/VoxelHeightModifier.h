// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkData.h"
#include "VoxelHeightModifier.generated.h"

class UVoxelMetadata;
class FVoxelHeightSculptCanvasWriter;

class VOXEL_API IVoxelHeightModifierRuntime
{
public:
	IVoxelHeightModifierRuntime() = default;
	virtual ~IVoxelHeightModifierRuntime() = default;

	virtual FVoxelBox2D GetBounds() const = 0;
	virtual void Apply(FVoxelHeightSculptCanvasWriter& Writer) const = 0;
};

USTRUCT()
struct VOXEL_API FVoxelHeightModifier : public FVoxelVirtualStruct
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	virtual TSharedPtr<IVoxelHeightModifierRuntime> GetRuntime() const VOXEL_PURE_VIRTUAL({});
};

USTRUCT()
struct VOXEL_API FVoxelHeightSerializedModifier final : public FVoxelBulkData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	TSharedPtr<FVoxelHeightModifier> Modifier;

	FVoxelHeightSerializedModifier() = default;
	explicit FVoxelHeightSerializedModifier(const TSharedRef<FVoxelHeightModifier>& Modifier)
		: Modifier(Modifier)
	{
	}

public:
	//~ Begin FVoxelBulkData Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelBulkData Interface
};