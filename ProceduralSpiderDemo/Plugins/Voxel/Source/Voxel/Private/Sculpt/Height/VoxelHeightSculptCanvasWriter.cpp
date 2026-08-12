// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelHeightSculptCanvasWriter.h"
#include "Sculpt/Height/VoxelHeightSculptCanvas.h"

FVoxelHeightSculptCanvasWriter::FVoxelHeightSculptCanvasWriter(
	FVoxelHeightSculptCanvas& Canvas,
	const FTransform2d& SculptToWorld,
	const float ScaleZ,
	const float OffsetZ)
	: Size(Canvas.Size)
	, IndexToWorld(FTransform2d(FVector2D(Canvas.ChunkKeyOffset * Canvas.ChunkSize)) * SculptToWorld)
	, ScaleZ(ScaleZ)
	, OffsetZ(OffsetZ)
	// Indices is in relative space (0..Size) for iteration
	, Indices(Canvas.ChunkSize, Canvas.Size - Canvas.ChunkSize)
	, Canvas(Canvas)
{
	VOXEL_FUNCTION_COUNTER();
}

TVoxelArrayView<float> FVoxelHeightSculptCanvasWriter::GetUnscaledHeights()
{
	Canvas.bWriteHeights = true;
	return Canvas.Heights;
}

FVoxelHeightSculptCanvasWriter::FSurfaceTypes FVoxelHeightSculptCanvasWriter::GetSurfaceTypes()
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

FVoxelHeightSculptCanvasWriter::FMetadata FVoxelHeightSculptCanvasWriter::GetMetadata(const FVoxelMetadataRef& MetadataRef)
{
	VOXEL_FUNCTION_COUNTER();

	Canvas.MetadataRefsToWrite.AddUnique(MetadataRef);

	FVoxelHeightSculptCanvas::FMetadata* Metadata = Canvas.MetadataRefToMetadata.Find(MetadataRef);
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
