// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelPaintVolumeModifier.h"
#include "Sculpt/Volume/VoxelVolumeSculptCanvasWriter.h"
#include "Surface/VoxelSurfaceTypeBlend.h"
#include "Buffer/VoxelBaseBuffers.h"

TSharedPtr<IVoxelVolumeModifierRuntime> FVoxelPaintVolumeModifier::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	return MakeShared<FVoxelPaintVolumeModifierRuntime>(
		Center,
		Radius,
		Strength,
		Mode,
		FVoxelSurfaceType(SurfaceTypeToPaint),
		MetadatasToPaint.CreateRuntime(),
		Brush.GetRuntimeBrush());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox FVoxelPaintVolumeModifierRuntime::GetBounds() const
{
	return FVoxelBox(Center - Radius, Center + Radius);
}

void FVoxelPaintVolumeModifierRuntime::Apply(FVoxelVolumeSculptCanvasWriter& Writer) const
{
	VOXEL_FUNCTION_COUNTER();

	const FIntVector Size = Writer.Size;
	const FMatrix IndexToWorld = Writer.IndexToWorld;
	const FVoxelIntBox Indices = Writer.Indices;

	TVoxelOptional<FVoxelVolumeSculptCanvasWriter::FSurfaceTypes> SurfaceTypes;
	if (!SurfaceType.IsNull())
	{
		SurfaceTypes = Writer.GetSurfaceTypes();
	}

	TVoxelArray<FVoxelVolumeSculptCanvasWriter::FMetadata> Metadatas;
	for (const auto& It : MetadataOverrides->MetadataToValue)
	{
		Metadatas.Add(Writer.GetMetadata(It.Key));
	}

	FVoxelFloatBuffer Alphas;
	FVoxelInt32Buffer Indirection;

	Alphas.Allocate(Indices.Count_int32());
	Indirection.Allocate(Indices.Count_int32());

	FVoxelCounter32 AtomicWriteIndex;

	Voxel::ParallelFor(Indices.GetZ(), [&](const int32 IndexZ)
	{
		for (int32 IndexY = Indices.Min.Y; IndexY < Indices.Max.Y; IndexY++)
		{
			for (int32 IndexX = Indices.Min.X; IndexX < Indices.Max.X; IndexX++)
			{
				const int32 Index = FVoxelUtilities::Get3DIndex<int32>(Size, IndexX, IndexY, IndexZ);
				const FVector Position = IndexToWorld.TransformPosition(FVector(IndexX, IndexY, IndexZ));

				const double DistanceToCenter = FVector::Distance(Position, Center);

				const float FalloffMultiplier = Brush->GetFalloff(DistanceToCenter, Radius);
				const float MaskStrength = Brush->GetMaskStrength(Center, Position, Radius);

				float Alpha = FalloffMultiplier * Strength * MaskStrength;
				Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

				if (Alpha == 0.f)
				{
					continue;
				}

				if (Mode == EVoxelSculptMode::Remove)
				{
					if (SurfaceTypes)
					{
						float& SurfaceTypeAlpha = SurfaceTypes->Alphas[Index];

						SurfaceTypeAlpha = FMath::Lerp(
							SurfaceTypeAlpha,
							0.f,
							Alpha);
					}

					for (const FVoxelVolumeSculptCanvasWriter::FMetadata& Metadata : Metadatas)
					{
						float& MetadataAlpha = Metadata.Alphas[Index];

						MetadataAlpha = FMath::Lerp(
							MetadataAlpha,
							0.f,
							Alpha);
					}

					continue;
				}
				checkVoxelSlow(Mode == EVoxelSculptMode::Add);

				if (SurfaceTypes)
				{
					float& SurfaceTypeAlpha = SurfaceTypes->Alphas[Index];

					SurfaceTypeAlpha = FMath::Lerp(
						SurfaceTypeAlpha,
						1.f,
						Alpha);
				}

				for (const FVoxelVolumeSculptCanvasWriter::FMetadata& Metadata : Metadatas)
				{
					float& MetadataAlpha = Metadata.Alphas[Index];

					MetadataAlpha = FMath::Lerp(
						MetadataAlpha,
						1.f,
						Alpha);
				}

				const int32 WriteIndex = AtomicWriteIndex.Increment_ReturnOld();

				Alphas.Set(WriteIndex, Alpha);
				Indirection.Set(WriteIndex, Index);
			}
		}
	});

	const int32 Num = AtomicWriteIndex.Get();

	if (Num == 0)
	{
		return;
	}

	Alphas.ShrinkTo(Num);
	Indirection.ShrinkTo(Num);

	if (SurfaceTypes)
	{
		VOXEL_SCOPE_COUNTER_NUM("SurfaceTypes", Num);

		Voxel::ParallelFor(Num, [&](const int32 Index)
		{
			FVoxelSurfaceTypeBlend& SurfaceTypeBlend = SurfaceTypes->Blends[Indirection[Index]];

			FVoxelSurfaceTypeBlend::Lerp(
				SurfaceTypeBlend,
				SurfaceTypeBlend,
				SurfaceType,
				Alphas[Index]);
		});
	}

	for (const auto& It : MetadataOverrides->MetadataToValue)
	{
		VOXEL_SCOPE_COUNTER_FORMAT("Metadata %s Num=%d", *It.Key.GetName(), Num);

		It.Key.IndirectBlend(
			Indirection.View(),
			*It.Value.Constant,
			Alphas,
			*Writer.GetMetadata(It.Key).Buffer);
	}
}