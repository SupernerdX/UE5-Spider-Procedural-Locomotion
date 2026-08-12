// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeSurfaceTypeChunk.h"
#include "Sculpt/VoxelSculptVersion.h"
#include "Surface/VoxelSurfaceTypeBlendBuilder.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelVolumeSurfaceTypeChunk);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelVolumeSurfaceType_Memory);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVolumeSurfaceTypeChunk::FVoxelVolumeSurfaceTypeChunk()
{
	UpdateStats();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeSurfaceTypeChunk::GetSurfaceType(
	const int32 Index,
	float& OutAlpha,
	FVoxelSurfaceTypeBlend& OutSurfaceType) const
{
	FVoxelSurfaceTypeBlendBuilder Builder;

	for (const FLayer& Layer : Layers)
	{
		const uint8 Weight = Layer.Weights[Index];
		if (Weight == 0)
		{
			continue;
		}

		Builder.AddLayer_CheckNew
		(
			UsedSurfaceTypes[Layer.Types[Index]],
			Weight
		);
	}

	OutAlpha = FVoxelUtilities::UINT8ToFloat(Alphas[Index]);
	Builder.Build(OutSurfaceType);
}

void FVoxelVolumeSurfaceTypeChunk::GetAverageSurfaceType(
	float& OutAlpha,
	FVoxelSurfaceTypeBlend& OutSurfaceType) const
{
	VOXEL_FUNCTION_COUNTER();

	{
		int32 AlphaSum = 0;
		for (const uint8 Alpha : Alphas)
		{
			AlphaSum += Alpha;
		}
		OutAlpha = AlphaSum / (255.f * ChunkCount);

		checkVoxelSlow(0.f <= OutAlpha && OutAlpha <= 1.f);
	}

	TVoxelFixedArray<float, 256> Weights;
	Weights.SetNumZeroed(UsedSurfaceTypes.Num());

	for (int32 Index = 0; Index < ChunkCount; Index++)
	{
		int32 WeightSum = 0;
		for (const FLayer& Layer : Layers)
		{
			WeightSum += Layer.Weights[Index];
		}

		// Bias the weights by the alphas
		const float Multiplier = Alphas[Index] / float(WeightSum);

		for (const FLayer& Layer : Layers)
		{
			const uint8 Weight = Layer.Weights[Index];
			if (Weight == 0)
			{
				continue;
			}

			Weights[Layer.Types[Index]] += Weight * Multiplier;
		}
	}

	FVoxelSurfaceTypeBlendBuilder Builder;
	for (int32 Index = 0; Index < UsedSurfaceTypes.Num(); Index++)
	{
		Builder.AddLayer_CheckNew(
			UsedSurfaceTypes[Index],
			Weights[Index]);
	}

	Builder.Build(OutSurfaceType);
}

TVoxelRefCountPtr<FVoxelVolumeSurfaceTypeChunk> FVoxelVolumeSurfaceTypeChunk::Create(
	const TConstVoxelArrayView<float> Alphas,
	const TConstVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypes,
	const FIntVector& Offset,
	const FIntVector& Size)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelRefCountPtr<FVoxelVolumeSurfaceTypeChunk> Chunk;

	{
		VOXEL_SCOPE_COUNTER("UsedSurfaceTypes");

		int32 NumLayers = 0;

		TVoxelArray<FVoxelSurfaceType> UsedSurfaceTypes;
		UsedSurfaceTypes.Reserve(16);

		for (int32 IndexZ = 0; IndexZ < ChunkSize; IndexZ++)
		{
			for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
			{
				for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
				{
					const int32 IndexInSculpt = FVoxelUtilities::Get3DIndex<int32>(
						Size,
						Offset.X + IndexX,
						Offset.Y + IndexY,
						Offset.Z + IndexZ);

					const FVoxelSurfaceTypeBlend& SurfaceType = SurfaceTypes[IndexInSculpt];

					NumLayers = FMath::Max(NumLayers, SurfaceType.GetLayers().Num());

					for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceType.GetLayers())
					{
						UsedSurfaceTypes.AddUnique(Layer.Type);
					}
				}
			}
		}

		if (NumLayers == 0)
		{
			return nullptr;
		}

		Chunk = new FVoxelVolumeSurfaceTypeChunk();
		Chunk->Layers.SetNum(NumLayers);
		Chunk->UsedSurfaceTypes = MoveTemp(UsedSurfaceTypes);
	}

	FVoxelUtilities::Memzero_Stats(Chunk->Layers);

	for (int32 IndexZ = 0; IndexZ < ChunkSize; IndexZ++)
	{
		for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
		{
			for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
			{
				const int32 IndexInSculpt = FVoxelUtilities::Get3DIndex<int32>(
					Size,
					Offset.X + IndexX,
					Offset.Y + IndexY,
					Offset.Z + IndexZ);

				const int32 IndexInChunk = FVoxelUtilities::Get3DIndex<int32>(
					ChunkSize,
					IndexX,
					IndexY,
					IndexZ);

				Chunk->Alphas[IndexInChunk] = FVoxelUtilities::FloatToUINT8(Alphas[IndexInSculpt]);

				const FVoxelSurfaceTypeBlend& SurfaceType = SurfaceTypes[IndexInSculpt];

				float MaxWeight = 0.f;
				for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceType.GetLayers())
				{
					MaxWeight = FMath::Max(MaxWeight, Layer.Weight.ToFloat());
				}

				int32 LayerIndex = 0;
				for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceType.GetLayers())
				{
					FLayer& ChunkLayer = Chunk->Layers[LayerIndex++];

					const int32 TypeIndex = Chunk->UsedSurfaceTypes.Find(Layer.Type);
					checkVoxelSlow(FVoxelUtilities::IsValidUINT8(TypeIndex));

					ChunkLayer.Types[IndexInChunk] = uint8(TypeIndex);
					ChunkLayer.Weights[IndexInChunk] = FVoxelUtilities::FloatToUINT8(Layer.Weight.ToFloat() / MaxWeight);
				}
			}
		}
	}

	return Chunk;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeSurfaceTypeChunk::Serialize(
	FArchive& Ar,
	const int32 Version)
{
	VOXEL_FUNCTION_COUNTER();

	if (Version < FVoxelSculptVersion::MergeVersions)
	{
		using FVersion = DECLARE_VOXEL_VERSION
		(
			FirstVersion
		);

		int32 LegacyVersion = FVersion::LatestVersion;
		Ar << LegacyVersion;
	}

	Ar << Alphas;
	Ar << Layers;
	Ar << UsedSurfaceTypes;
}

int64 FVoxelVolumeSurfaceTypeChunk::GetAllocatedSize() const
{
	int64 AllocatedSize = 0;
	AllocatedSize += Alphas.GetAllocatedSize();
	AllocatedSize += Layers.GetAllocatedSize();
	AllocatedSize += UsedSurfaceTypes.GetAllocatedSize();
	return AllocatedSize;
}