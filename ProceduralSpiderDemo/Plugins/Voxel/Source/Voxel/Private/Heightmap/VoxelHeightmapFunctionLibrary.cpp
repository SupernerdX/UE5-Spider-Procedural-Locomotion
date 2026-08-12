// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Heightmap/VoxelHeightmapFunctionLibrary.h"
#include "Heightmap/VoxelHeightmap_Height.h"

void FVoxelHeightmapRefPinType::Convert(
	const bool bSetObject,
	TVoxelObjectPtr<UVoxelHeightmap>& OutObject,
	UVoxelHeightmap& InObject,
	FVoxelHeightmapRef& Struct)
{
	if (bSetObject)
	{
		OutObject = Struct.Object;
	}
	else
	{
		const UVoxelHeightmap_Height* Height = InObject.Height;
		if (!ensure(Height))
		{
			return;
		}

		Struct.Object = InObject;
		Struct.ScaleXY = InObject.ScaleXY;
		Struct.DefaultSurfaceType = FVoxelSurfaceType(InObject.DefaultSurfaceType);
		Struct.ScaleZ = Height->ScaleZ;
		Struct.OffsetZ = Height->OffsetZ;
		Struct.HeightData = Height->GetData();

		if (!Struct.HeightData)
		{
			return;
		}

		for (int32 Index = 0; Index < InObject.Weights.Num(); Index++)
		{
			const UVoxelHeightmap_Weight* Weight = InObject.Weights[Index];
			if (!ensure(Weight))
			{
				continue;
			}

			const TSharedPtr<const FVoxelHeightmap_WeightData> WeightData = Weight->GetData();
			if (!WeightData)
			{
				continue;
			}

			Struct.Weightmaps.Add(FVoxelHeightmapUtilities::FWeightmap
			{
				FVoxelSurfaceType(Weight->SurfaceType),
				Weight->Type,
				Weight->Strength,
				FVoxelFloatMetadataRef(Weight->Metadata),
				Weight->bUseBicubic,
				WeightData
			});
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntPoint UVoxelHeightmapFunctionLibrary::GetHeightmapSize(const FVoxelHeightmapRef& Heightmap) const
{
	if (!Heightmap.HeightData)
	{
		VOXEL_MESSAGE(Error, "{0}: Heightmap is null", this);
		return FIntPoint(ForceInit);
	}

	return FIntPoint(
		Heightmap.HeightData->SizeX,
		Heightmap.HeightData->SizeY);
}