// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValueOps.h"
#include "Materials/MaterialInterface.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "Surface/VoxelSurfaceTypeBlend.h"
#include "VoxelPinValueOps_FVoxelSurfaceTypeBlend.generated.h"

USTRUCT()
struct FVoxelPinValueOps_FVoxelSurfaceTypeBlend : public FVoxelPinValueOps
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	//~ Begin FVoxelPinValueOps Interface
	virtual EVoxelPinValueOpsUsage GetUsage() const override
	{
		return
			EVoxelPinValueOpsUsage::ToDebugString |
			EVoxelPinValueOpsUsage::MigrateParameter;
	}
	virtual FString ToDebugString(const FVoxelRuntimePinValue& Value) const override
	{
		const TVoxelArray<FVoxelSurfaceTypeBlendLayer> Layers = Value.Get<FVoxelSurfaceTypeBlend>().GetLayersSortedByWeight();
		if (Layers.Num() == 0)
		{
			return "null";
		}

		FString Result;
		for (const FVoxelSurfaceTypeBlendLayer& Layer : Layers)
		{
			Result += Layer.GetSurfaceName() + " -> " + Layer.GetWeightString() + "\n";
		}
		Result.RemoveFromEnd("\n");
		return Result;
	}
	virtual void MigrateParameter(FVoxelPinValue& Value) const override
	{
		if (!Value.CanBeCastedTo<UObject>())
		{
			return;
		}

		UObject* Object = Value.Get<UObject>();
		if (!Object)
		{
			Value = FVoxelPinValue::Make<UVoxelSurfaceTypeInterface>(nullptr);
			return;
		}

		UMaterialInterface* Material = Cast<UMaterialInterface>(Object);
		if (!Material)
		{
			return;
		}

#if WITH_EDITOR
		Value = FVoxelPinValue::Make(UVoxelSurfaceTypeInterface::Migrate(Material));
#endif
	}
	//~ End FVoxelPinValueOps Interface
};