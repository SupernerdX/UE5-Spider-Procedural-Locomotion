// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelPreviewDataNode_SmartSurfaceTypePreviewData.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelGraphPositionParameter.h"
#include "Utilities/VoxelBufferMathUtilities.h"
#include "Utilities/VoxelBufferGradientUtilities.h"
#include "Surface/VoxelSmartSurfaceFunctionLibrary.h"

void FVoxelPreviewDataNode_SmartSurfaceTypePreviewData::Query_WithPosition(const FVoxelGraphQueryImpl& Query, FVoxelGraphQueryImpl& OutQuery) const
{
	TVoxelMap<FVoxelMetadataRef, TPinRef_Input<FVoxelBuffer>> Metadata;
	for (const auto& Pin : MetadataPins)
	{
		Metadata.Add_EnsureNew(Pin.MetadataRef, Pin.PinRef);
	}

	OutQuery.AddParameter<FVoxelGraphParameters::FSmartSurfaceTypePreview>(
		Query,
		DistancePin,
		SurfaceTypePin,
		MoveTemp(Metadata));

	const FVoxelVectorBuffer Normals = INLINE_LAMBDA -> FVoxelVectorBuffer
	{
		FVoxelGraphCallstack* Callstack = nullptr;
	#if WITH_EDITOR
		Callstack = &Query.Context.Callstacks_EditorOnly.Emplace_GetRef();
		Callstack->Node = this;
	#endif

		const float Granularity = NormalGranularityPin.GetSynchronous(Query);

		FVoxelGraphQueryImpl& NewQuery = Query.CloneParameters();
		{
			FVoxelVectorBuffer QueryPositions = Query.FindParameter<FVoxelGraphParameters::FPosition3D>()->GetLocalPosition_Float(FVoxelGraphQuery(Query, Callstack));
			QueryPositions.ExpandConstants();

			const FVoxelVectorBuffer NewPositions = FVoxelBufferGradientUtilities::SplitPositions3D(QueryPositions, Granularity);

			// Ensure Position2D resolves to our new Position3D downstream
			NewQuery.RemoveParameter<FVoxelGraphParameters::FPosition2D>();
			NewQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetLocalPosition(NewPositions);
		}

		const TSharedRef<const FVoxelFloatBuffer> Distances = DistancePin.GetSynchronous(NewQuery);
		if (Distances->IsConstant())
		{
			return FVoxelVectorBuffer(FVector3f::ZeroVector);
		}

		const FVoxelVectorBuffer QueryPositions = Query.FindParameter<FVoxelGraphParameters::FPosition3D>()->GetLocalPosition_Float(FVoxelGraphQuery(Query, Callstack));

		if (!ensureVoxelSlow(Distances->Num() == 6 * QueryPositions.Num()))
		{
			VOXEL_MESSAGE(Error, "Input size mismatch");

			return FVoxelVectorBuffer(FVector3f::ZeroVector);
		}

		const FVoxelVectorBuffer Result = FVoxelBufferGradientUtilities::CollapseGradient3D(*Distances, QueryPositions.Num(), Granularity);

		return FVoxelBufferMathUtilities::Normalize(Result);
	};

	OutQuery.AddParameter<FVoxelGraphParameters::FSmartSurface>().Normals = Normals;
}