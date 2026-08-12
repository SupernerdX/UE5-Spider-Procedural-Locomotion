// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelGraphVolumeModifier.h"
#include "Sculpt/Volume/VoxelVolumeSculptGraph.h"
#include "Sculpt/Volume/VoxelVolumeSculptCanvasWriter.h"
#include "Sculpt/VoxelSculptGraphFunctionLibrary.h"
#include "Sculpt/VoxelOutputNode_OutputSculptDistance.h"
#include "VoxelGraphQuery.h"
#include "VoxelGraphContext.h"
#include "VoxelGraphPositionParameter.h"
#include "Buffer/VoxelDoubleBuffers.h"

TSharedPtr<IVoxelVolumeModifierRuntime> FVoxelGraphVolumeModifier::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!Graph.Graph)
	{
		return {};
	}

	const TSharedRef<FVoxelGraphEnvironment> Environment = FVoxelGraphEnvironment::Create(
		{},
		*Graph.Graph,
		Graph.ParameterOverrides,
		Transform,
		FVoxelDependencyCollector::Null);

	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptDistance> Evaluator = FVoxelNodeEvaluator::Create<FVoxelOutputNode_OutputSculptDistance>(Environment);
	if (!Evaluator)
	{
		return {};
	}

	return MakeShared<FVoxelGraphVolumeModifierRuntime>(
		Transform,
		Radius,
		Evaluator);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox FVoxelGraphVolumeModifierRuntime::GetBounds() const
{
	return FVoxelBox(Transform.GetLocation() - Radius, Transform.GetLocation() + Radius);
}

void FVoxelGraphVolumeModifierRuntime::Apply(FVoxelVolumeSculptCanvasWriter& Writer) const
{
	VOXEL_FUNCTION_COUNTER();

	const FIntVector Size = Writer.Size;
	const FMatrix IndexToWorld = Writer.IndexToWorld;
	const FVoxelIntBox Indices = Writer.Indices;
	const TVoxelArrayView<float> UnscaledDistances = Writer.GetUnscaledDistances();

	const float RadiusSquared = FMath::Square(Radius);
	const FVector Center = Transform.GetLocation();

	FVoxelInt32Buffer Indirection;
	FVoxelDoubleVectorBuffer Positions;
	FVoxelFloatBuffer PreviousDistances;
	{
		VOXEL_SCOPE_COUNTER("Write positions");

		Indirection.Allocate(Indices.Count_int32());
		Positions.Allocate(Indices.Count_int32());
		PreviousDistances.Allocate(Indices.Count_int32());

		int32 WriteIndex = 0;
		for (int32 IndexZ = Indices.Min.Z; IndexZ < Indices.Max.Z; IndexZ++)
		{
			for (int32 IndexY = Indices.Min.Y; IndexY < Indices.Max.Y; IndexY++)
			{
				for (int32 IndexX = Indices.Min.X; IndexX < Indices.Max.X; IndexX++)
				{
					const FVector Position = IndexToWorld.TransformPosition(FVector(IndexX, IndexY, IndexZ));
					if (FVector::DistSquared(Position, Center) > RadiusSquared)
					{
						continue;
					}

					const int32 ReadIndex = FVoxelUtilities::Get3DIndex<int32>(Size, IndexX, IndexY, IndexZ);
					const float Distance = UnscaledDistances[ReadIndex] * Writer.DistanceScale;

					Indirection.Set(WriteIndex, ReadIndex);
					Positions.Set(WriteIndex, Position);
					PreviousDistances.Set(WriteIndex, Distance);

					WriteIndex++;
				}
		}
		}

		Indirection.ShrinkTo(WriteIndex);
		Positions.ShrinkTo(WriteIndex);
		PreviousDistances.ShrinkTo(WriteIndex);
	}

	FVoxelGraphContext Context = Evaluator.MakeContext(FVoxelDependencyCollector::Null);

	FVoxelGraphQueryImpl& Query = Context.MakeQuery();
	Query.AddParameter<FVoxelGraphParameters::FPosition3D>().SetWorldPosition(Positions);

	{
		FVoxelGraphParameters::FVolumeSculpt& Parameter = Query.AddParameter<FVoxelGraphParameters::FVolumeSculpt>();
		Parameter.PreviousDistances = PreviousDistances;
	}

	const TSharedRef<const FVoxelFloatBuffer> NewDistances = Evaluator->DistancePin.GetSynchronous(Query);

	VOXEL_SCOPE_COUNTER("Write Distances");

	for (int32 Index = 0; Index < Indirection.Num(); Index++)
	{
		UnscaledDistances[Indirection[Index]] = (*NewDistances)[Index] / Writer.DistanceScale;
	}
}