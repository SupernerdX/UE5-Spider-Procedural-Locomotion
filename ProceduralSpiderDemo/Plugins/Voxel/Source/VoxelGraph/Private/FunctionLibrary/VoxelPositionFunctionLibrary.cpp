// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "FunctionLibrary/VoxelPositionFunctionLibrary.h"

FVoxelVector2DBuffer UVoxelPositionFunctionLibrary::GetPosition2D(const EVoxelPositionSpace Space) const
{
	const FVoxelGraphParameters::FPosition2D* PositionParameter = FVoxelGraphParameters::FPosition2D::Find(Query);
	if (!PositionParameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot query positions here", this);
		return DefaultBuffer;
	}

	switch (Space)
	{
	default: ensure(false);
	case EVoxelPositionSpace::LocalSpace: return PositionParameter->GetLocalPosition_Float(Query);
	case EVoxelPositionSpace::WorldSpace: return PositionParameter->GetWorldPosition_Float(Query);
	}
}

FVoxelVectorBuffer UVoxelPositionFunctionLibrary::GetPosition3D(
	const EVoxelPositionSpace Space,
	const bool bFallbackTo2D) const
{
	const FVoxelGraphParameters::FPosition3D* PositionParameter = Query->FindParameter<FVoxelGraphParameters::FPosition3D>();
	if (!PositionParameter)
	{
		if (Query->FindParameter<FVoxelGraphParameters::FPosition2D>())
		{
			if (bFallbackTo2D)
			{
				const FVoxelVector2DBuffer Position2D = GetPosition2D(Space);

				FVoxelVectorBuffer Result;
				Result.X = Position2D.X;
				Result.Y = Position2D.Y;
				Result.Z = 0.f;
				return Result;
			}

			VOXEL_MESSAGE(Error, "{0}: Cannot query 3D positions here, use GetPosition2D", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: Cannot query positions here", this);
		}
		return DefaultBuffer;
	}

	switch (Space)
	{
	default: ensure(false);
	case EVoxelPositionSpace::LocalSpace: return PositionParameter->GetLocalPosition_Float(Query);
	case EVoxelPositionSpace::WorldSpace: return PositionParameter->GetWorldPosition_Float(Query);
	}
}

FVoxelDoubleVector2DBuffer UVoxelPositionFunctionLibrary::GetPosition2D_Double(const EVoxelPositionSpace Space) const
{
	const FVoxelGraphParameters::FPosition2D* PositionParameter = FVoxelGraphParameters::FPosition2D::Find(Query);
	if (!PositionParameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot query positions here", this);
		return DefaultBuffer;
	}

	switch (Space)
	{
	default: ensure(false);
	case EVoxelPositionSpace::LocalSpace: return PositionParameter->GetLocalPosition_Double(Query);
	case EVoxelPositionSpace::WorldSpace: return PositionParameter->GetWorldPosition_Double(Query);
	}
}

FVoxelDoubleVectorBuffer UVoxelPositionFunctionLibrary::GetPosition3D_Double(
	const EVoxelPositionSpace Space,
	const bool bFallbackTo2D) const
{
	const FVoxelGraphParameters::FPosition3D* PositionParameter = Query->FindParameter<FVoxelGraphParameters::FPosition3D>();
	if (!PositionParameter)
	{
		if (Query->FindParameter<FVoxelGraphParameters::FPosition2D>())
		{
			if (bFallbackTo2D)
			{
				const FVoxelDoubleVector2DBuffer Position2D = GetPosition2D_Double(Space);

				FVoxelDoubleVectorBuffer Result;
				Result.X = Position2D.X;
				Result.Y = Position2D.Y;
				Result.Z = 0.f;
				return Result;
			}

			VOXEL_MESSAGE(Error, "{0}: Cannot query 3D positions here, use GetPosition2D", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: Cannot query positions here", this);
		}
		return DefaultBuffer;
	}

	switch (Space)
	{
	default: ensure(false);
	case EVoxelPositionSpace::LocalSpace: return PositionParameter->GetLocalPosition_Double(Query);
	case EVoxelPositionSpace::WorldSpace: return PositionParameter->GetWorldPosition_Double(Query);
	}
}