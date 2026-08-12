// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeSculptCanvasWriter.h"
#include "Sculpt/Volume/VoxelVolumeSculptCanvas.h"

FVoxelVolumeSculptCanvasWriter::FVoxelVolumeSculptCanvasWriter(
	FVoxelVolumeSculptCanvas& Canvas,
	const FMatrix& SculptToWorld)
	: Size(Canvas.Size)
	, DistanceScale(SculptToWorld.GetScaleVector().GetAbsMax())
	, IndexToWorld(FTranslationMatrix(FVector(Canvas.ChunkKeyOffset) * Canvas.ChunkSize) * SculptToWorld)
	// Only write in the inner part that's fully jump flooded
	, Indices(Canvas.ChunkSize, Canvas.Size - Canvas.ChunkSize)
	, Canvas(Canvas)
{
}

TVoxelArrayView<float> FVoxelVolumeSculptCanvasWriter::GetUnscaledDistances()
{
	Canvas.bWriteDistances = true;
	return Canvas.Distances;
}

FVoxelVolumeSculptCanvasWriter::FSurfaceTypes FVoxelVolumeSculptCanvasWriter::GetSurfaceTypes()
{
	VOXEL_FUNCTION_COUNTER();

	Canvas.bWriteSurfaceTypes = true;

	if (!Canvas.SurfaceTypes)
	{
		Canvas.SurfaceTypes.Emplace();

		FVoxelUtilities::SetNumZeroed(Canvas.SurfaceTypes->Alphas, Canvas.NumVoxels);
		FVoxelUtilities::SetNum(Canvas.SurfaceTypes->Blends, Canvas.NumVoxels);
	}

	return FSurfaceTypes
	{
		Canvas.SurfaceTypes->Alphas,
		Canvas.SurfaceTypes->Blends
	};
}

FVoxelVolumeSculptCanvasWriter::FMetadata FVoxelVolumeSculptCanvasWriter::GetMetadata(const FVoxelMetadataRef& MetadataRef)
{
	VOXEL_FUNCTION_COUNTER();

	Canvas.MetadataRefsToWrite.AddUnique(MetadataRef);

	FVoxelVolumeSculptCanvas::FMetadata* Metadata = Canvas.MetadataRefToMetadata.Find(MetadataRef);
	if (!Metadata)
	{
		Metadata = &Canvas.MetadataRefToMetadata.Add_EnsureNew(MetadataRef);
		FVoxelUtilities::SetNumZeroed(Metadata->Alphas, Canvas.NumVoxels);
		Metadata->Buffer = MetadataRef.MakeDefaultBuffer(Canvas.NumVoxels);
	}

	return FMetadata
	{
		Metadata->Alphas,
		Metadata->Buffer.Get()
	};
}