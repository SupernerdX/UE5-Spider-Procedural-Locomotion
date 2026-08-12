// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Heightmap/VoxelNode_SampleHeightmap.h"
#include "Heightmap/VoxelHeightmap_HeightData.h"
#include "Surface/VoxelSurfaceTypeBlendBuilder.h"
#include "VoxelNode_SampleHeightmapImpl.ispc.generated.h"

void FVoxelNode_SampleHeightmap::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelHeightmapRef> Heightmap = HeightmapPin.Get(Query);
	const TValue<FVoxelVector2DBuffer> Position = PositionPin.Get(Query);
	const TValue<bool> bUseBicubic = UseBicubicPin.Get(Query);

	VOXEL_GRAPH_WAIT(Position, Heightmap, bUseBicubic)
	{
		if (!Heightmap->HeightData)
		{
			VOXEL_MESSAGE(Error, "{0}: heightmap is not assigned", this);
			return;
		}

		FVoxelVector2DBuffer LocalPosition = *Position;
		LocalPosition.ExpandConstants();

		const int32 Num = LocalPosition.Num();

		FVoxelFloatBuffer Result;
		if (HeightPin.ShouldCompute())
		{
			Result.Allocate(LocalPosition.Num());
		}

		FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
		if (SurfaceTypePin.ShouldCompute())
		{
			SurfaceTypes.AllocateZeroed(Num);
		}

		TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
		MetadataToBuffer.Reserve(MetadataPins.Num());

		for (const FMetadataPin& MetadataPin : MetadataPins)
		{
			if (MetadataPin.PinRef.ShouldCompute())
			{
				MetadataToBuffer.Add_EnsureNew(
					MetadataPin.MetadataRef,
					MetadataPin.MetadataRef.MakeDefaultBuffer(Num));
			}
		}

		float ScaleZ;
		float OffsetZ;
		FVoxelUtilities::CombineScaleAndOffset(
			Heightmap->HeightData->InternalScaleZ,
			Heightmap->HeightData->InternalOffsetZ,
			Heightmap->ScaleZ,
			Heightmap->OffsetZ,
			ScaleZ,
			OffsetZ);

		if (HeightPin.ShouldCompute())
		{
			if (Heightmap->HeightData->bIsUINT16)
			{
				ispc::VoxelNode_SampleHeightmap_uint16(
					LocalPosition.X.GetData(),
					LocalPosition.Y.GetData(),
					Result.GetData(),
					LocalPosition.Num(),
					bUseBicubic,
					ScaleZ / MAX_uint16,
					OffsetZ,
					Heightmap->ScaleXY,
					Heightmap->HeightData->SizeX,
					Heightmap->HeightData->SizeY,
					Heightmap->HeightData->RawData.View<uint16>().GetData());
			}
			else
			{
				ispc::VoxelNode_SampleHeightmap_float(
					LocalPosition.X.GetData(),
					LocalPosition.Y.GetData(),
					Result.GetData(),
					LocalPosition.Num(),
					bUseBicubic,
					ScaleZ,
					OffsetZ,
					Heightmap->ScaleXY,
					Heightmap->HeightData->SizeX,
					Heightmap->HeightData->SizeY,
					Heightmap->HeightData->RawData.View<float>().GetData());
			}
		}

		TVoxelInlineArray<FVoxelHeightmapUtilities::FWeightData, 16> WeightDatas =
			FVoxelHeightmapUtilities::BuildWeightmaps(
				Heightmap->Weightmaps,
				MetadataToBuffer.KeyArray(),
				Num,
				SurfaceTypePin.ShouldCompute(),
				MetadataToBuffer.Num() > 0,
				[&](const FVoxelHeightmapUtilities::FWeightmap& Weightmap, FVoxelHeightmapUtilities::FWeightData& WeightData)
				{
					const FVector2f HeightmapToWeightmap = FVector2f(
						Weightmap.Data->SizeX / float(Heightmap->HeightData->SizeX),
						Weightmap.Data->SizeY / float(Heightmap->HeightData->SizeY));

					ispc::VoxelNode_SampleHeightmap_SampleWeightmap(
						LocalPosition.X.GetData(),
						LocalPosition.Y.GetData(),
						WeightData.Weights.GetData(),
						Num,
						Weightmap.Strength,
						Weightmap.bUseBicubic,
						Heightmap->ScaleXY,
						GetISPCValue(HeightmapToWeightmap),
						Weightmap.Data->SizeX,
						Weightmap.Data->SizeY,
						Weightmap.Data->Weights.GetData());
				});

		if (SurfaceTypePin.ShouldCompute())
		{
			FVoxelHeightmapUtilities::ComputeSurfaceTypes(
				WeightDatas,
				Heightmap->DefaultSurfaceType,
				FVoxelFloatBuffer(1.f),
				Num,
				[&](const int32 Index, const FVoxelSurfaceTypeBlend& SurfaceType, float Alpha)
				{
					SurfaceTypes.Set(Index, SurfaceType);
				});
		}

		if (MetadataToBuffer.Num() > 0)
		{
			VOXEL_SCOPE_COUNTER("Metadata");

			for (const FVoxelHeightmapUtilities::FWeightData& WeightData : WeightDatas)
			{
				FVoxelFloatBuffer* FloatBuffer = MetadataToBuffer.FindRef(WeightData.Weightmap->MetadataRef)->As<FVoxelFloatBuffer>();
				if (!FloatBuffer)
				{
					continue;
				}

				VOXEL_SCOPE_COUNTER_FNAME(WeightData.Weightmap->MetadataRef.GetFName());

				FloatBuffer->CopyFrom(WeightData.Weights, 0, 0, Num);
			}
		}

		if (HeightPin.ShouldCompute())
		{
			HeightPin.Set(Query, Result);
		}

		if (SurfaceTypePin.ShouldCompute())
		{
			SurfaceTypePin.Set(Query, SurfaceTypes);
		}

		for (const FMetadataPin& MetadataPin : MetadataPins)
		{
			if (MetadataPin.PinRef.ShouldCompute())
			{
				MetadataPin.PinRef.Set(Query, FVoxelRuntimePinValue::Make(MetadataToBuffer[MetadataPin.MetadataRef]));
			}
		}
	};
}