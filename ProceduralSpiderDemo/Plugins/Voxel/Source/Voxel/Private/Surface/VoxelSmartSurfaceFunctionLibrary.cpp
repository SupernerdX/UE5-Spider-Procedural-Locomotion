// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSmartSurfaceFunctionLibrary.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"
#include "VoxelBufferSplitter.h"
#include "VoxelQuery.h"
#include "Graphs/VoxelStampGraphParameters.h"

void FVoxelGraphParameters::FSmartSurface::Split(
	const FVoxelBufferSplitter& Splitter,
	const TConstVoxelArrayView<FSmartSurface*> OutResult) const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelInlineArray<FVoxelVectorBuffer*, 8> Buffers;
	FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

	for (const int32 Index : Splitter.GetValidOutputs())
	{
		Buffers[Index] = &OutResult[Index]->Normals;
	}

	Normals.Split(Splitter, Buffers);
}

FVoxelVectorBuffer UVoxelSmartSurfaceFunctionLibrary::GetVertexNormal() const
{
	const FVoxelGraphParameters::FSmartSurface* Parameter = Query->FindParameter<FVoxelGraphParameters::FSmartSurface>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Smart Surface Type Preview Data node in Editor Graph found", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No vertex data", this);
		}
		return FVector3f::UpVector;
	}

	return Parameter->Normals;
}

FVoxelSurfaceTypeBlendBuffer UVoxelSmartSurfaceFunctionLibrary::ResolveSmartSurfaces(
	const FVoxelSurfaceTypeBlendBuffer& SurfaceTypes,
	const FVoxelDoubleVectorBuffer& Position,
	const FVoxelVectorBuffer& Normal,
	const int32 LOD,
	const FVoxelWeakStackLayer& Layer) const
{
	VOXEL_FUNCTION_COUNTER();

	const int32 Num = ComputeVoxelBuffersNum_Return(SurfaceTypes, Position, Normal);

	const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot resolve smart surface types, no query", this);
		return SurfaceTypes;
	}

	FVoxelSurfaceTypeBlendBuffer Result = SurfaceTypes.MakeDeepCopy();
	Result.ExpandConstantIfNeeded(Num);

	FVoxelSmartSurfaceTypeResolver Resolver(
		LOD,
		Layer,
		Parameter->Query.Layers,
		Parameter->Query.SurfaceTypeTable,
		Query->Context.DependencyCollector,
		Position,
		Normal,
		Result.View());

	Resolver.Resolve();

	return Result;
}