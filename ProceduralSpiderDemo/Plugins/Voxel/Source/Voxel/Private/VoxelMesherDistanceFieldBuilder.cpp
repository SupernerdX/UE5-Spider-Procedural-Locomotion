// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMesherDistanceFieldBuilder.h"
#include "VoxelCellGenerator.h"
#include "VoxelDistanceFieldWrapper.h"
#include "VoxelMesherDistanceFieldBuilderImpl.ispc.generated.h"

TSharedPtr<FDistanceFieldVolumeData> FVoxelMesherDistanceFieldBuilder::Build(
	const int32 ChunkLOD,
	const FInt64Vector& ChunkOffset,
	const int32 VoxelSize,
	const int32 ChunkSize,
	const float MeshDistanceFieldBias,
	const int32 MeshDistanceFieldBricksPerChunk,
	const int32 DataSize,
	FVoxelCellGeneratorOutput& CellGeneratorOutput)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelDistanceFieldWrapper Wrapper(FVoxelBox(0, ChunkSize).ToFBox());
	Wrapper.SetSize(FIntVector(ChunkSize / 32 * MeshDistanceFieldBricksPerChunk));

	FVoxelDistanceFieldWrapper::FMip& Mip = Wrapper.Mips[0];
	const int32 NumBricksPerChunk = Mip.GetIndirectionSize().GetMax();
	const int32 NumQueriesPerChunk = NumBricksPerChunk * DistanceField::UniqueDataBrickSize;
	// 2 * MeshDistanceFieldObjectBorder, additional padding at the end to overlap chunks
	const int32 NumQueriesInChunk = NumQueriesPerChunk - 2;
	const float TexelSize = ChunkSize / float(NumQueriesInChunk);

	const float CellSize = float(1 << ChunkLOD) * VoxelSize;

	if (CellGeneratorOutput.VolumeData.IsSet())
	{
		VOXEL_SCOPE_COUNTER("Generate Volume Distance Fields");

		FVoxelCellGeneratorVolumeOutput& VolumeData = CellGeneratorOutput.VolumeData.GetValue();
		const FIntVector Size = VolumeData.Bounds.Size();

		const FVoxelIntBox BrickBoundsA(0, NumBricksPerChunk);
		BrickBoundsA.Iterate([&](const FIntVector& BrickPositionA)
		{
			const TSharedRef<FVoxelDistanceFieldWrapper::FBrick> Brick = Mip.FindOrAddBrick(BrickPositionA);

			ispc::VoxelMesherDistanceFieldBuilder_ComputeVolumeDistanceFields(
				BrickPositionA.X,
				BrickPositionA.Y,
				BrickPositionA.Z,
				Size.X,
				Size.Y,
				Size.Z,
				VolumeData.Bounds.Min.X,
				VolumeData.Bounds.Min.Y,
				VolumeData.Bounds.Min.Z,
				CellSize,
				TexelSize,
				DataSize,
				VolumeData.Distances.GetData(),
				MeshDistanceFieldBias,
				Mip.GetLocalToVolumeScale(),
				Mip.GetDistanceFieldToVolumeScaleBias().X,
				Mip.GetDistanceFieldToVolumeScaleBias().Y,
				DistanceField::BrickSize,
				DistanceField::UniqueDataBrickSize,
				Brick->GetData());
		});
	}
	if (CellGeneratorOutput.HeightData.IsSet())
	{
		VOXEL_SCOPE_COUNTER("Generate Height Distance Fields");

		FVoxelCellGeneratorHeightOutput& HeightData = CellGeneratorOutput.HeightData.GetValue();
		const FVoxelIntBox2D BrickBoundsA(0, FIntPoint(NumBricksPerChunk));

		const FIntPoint Size = HeightData.Bounds.Size();

		const float StartZ = (FVector(ChunkOffset - (1 << ChunkLOD)) * VoxelSize).Z;

		BrickBoundsA.Iterate([&](const FIntPoint& BrickPositionA)
		{
			TVoxelArray<TSharedRef<FVoxelDistanceFieldWrapper::FBrick>> Bricks;
			TVoxelArray<uint8*> BricksRawData;
			for (int32 Z = 0; Z < NumBricksPerChunk; Z++)
			{
				TSharedRef<FVoxelDistanceFieldWrapper::FBrick> Brick = Mip.FindOrAddBrick(FIntVector(BrickPositionA.X, BrickPositionA.Y, Z));
				Bricks.Add(Brick);
				BricksRawData.Add(Brick->GetData());
			}

			ispc::VoxelMesherDistanceFieldBuilder_ComputeHeightDistanceFields(
				BrickPositionA.X,
				BrickPositionA.Y,
				Size.X,
				Size.Y,
				HeightData.Bounds.Min.X,
				HeightData.Bounds.Min.Y,
				CellSize,
				TexelSize,
				DataSize,
				HeightData.Heights.GetData(),
				MeshDistanceFieldBias,
				Mip.GetLocalToVolumeScale(),
				Mip.GetDistanceFieldToVolumeScaleBias().X,
				Mip.GetDistanceFieldToVolumeScaleBias().Y,
				NumBricksPerChunk,
				StartZ,
				DistanceField::BrickSize,
				DistanceField::UniqueDataBrickSize,
				BricksRawData.GetData());
		});
	}

	for (TSharedPtr<FVoxelDistanceFieldWrapper::FBrick>& Brick : Mip.GetBricks())
	{
		if (!Brick)
		{
			continue;
		}

		const bool bHasValue = INLINE_LAMBDA
		{
			const uint8 UniqueValue = (*Brick)[0];
			for (const uint8 Value : *Brick)
			{
				if (UniqueValue != Value)
				{
					return true;
				}
			}

			return false;
		};

		if (!bHasValue)
		{
			Brick = nullptr;
		}
	}

	Wrapper.Mips[1] = Wrapper.Mips[0];
	Wrapper.Mips[2] = Wrapper.Mips[0];
	return Wrapper.Build();
}