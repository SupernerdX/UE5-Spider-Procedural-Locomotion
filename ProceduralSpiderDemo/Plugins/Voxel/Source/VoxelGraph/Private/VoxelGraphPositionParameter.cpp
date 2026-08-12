// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphPositionParameter.h"
#include "VoxelBufferSplitter.h"
#include "Utilities/VoxelBufferTransformUtilities.h"
#include "Utilities/VoxelBufferConversionUtilities.h"

const FVoxelGraphParameters::FPosition2D* FVoxelGraphParameters::FPosition2D::Find(const FVoxelGraphQuery& Query)
{
	if (const FPosition2D* Result = Query->FindParameter<FPosition2D>())
	{
		return Result;
	}

	if (const FPosition3D* Result = Query->FindParameter<FPosition3D>())
	{
		return &Result->GetPosition2D();
	}

	return nullptr;
}

int32 FVoxelGraphParameters::FPosition2D::Num() const
{
	if (LocalPosition_Float)
	{
		return LocalPosition_Float->Num();
	}
	if (WorldPosition_Float)
	{
		return WorldPosition_Float->Num();
	}
	if (LocalPosition_Double)
	{
		return LocalPosition_Double->Num();
	}
	if (WorldPosition_Double)
	{
		return WorldPosition_Double->Num();
	}

	ensure(false);
	return 0;
}

void FVoxelGraphParameters::FPosition2D::Split(
	const FVoxelBufferSplitter& Splitter,
	const TConstVoxelArrayView<FPosition2D*> OutResult) const
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(Splitter.NumOutputs() == OutResult.Num());

	if (LocalPosition_Float)
	{
		TVoxelInlineArray<FVoxelVector2DBuffer*, 8> Buffers;
		FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Buffers[Index] = &OutResult[Index]->LocalPosition_Float.Emplace();
		}

		LocalPosition_Float->Split(Splitter, Buffers);
	}

	if (WorldPosition_Float)
	{
		TVoxelInlineArray<FVoxelVector2DBuffer*, 8> Buffers;
		FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Buffers[Index] = &OutResult[Index]->WorldPosition_Float.Emplace();
		}

		WorldPosition_Float->Split(Splitter, Buffers);
	}

	if (LocalPosition_Double)
	{
		TVoxelInlineArray<FVoxelDoubleVector2DBuffer*, 8> Buffers;
		FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Buffers[Index] = &OutResult[Index]->LocalPosition_Double.Emplace();
		}

		LocalPosition_Double->Split(Splitter, Buffers);
	}

	if (WorldPosition_Double)
	{
		TVoxelInlineArray<FVoxelDoubleVector2DBuffer*, 8> Buffers;
		FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Buffers[Index] = &OutResult[Index]->WorldPosition_Double.Emplace();
		}

		WorldPosition_Double->Split(Splitter, Buffers);
	}
}

void FVoxelGraphParameters::FPosition2D::SetLocalPosition(const FVoxelVector2DBuffer& LocalPosition)
{
	LocalPosition_Float = LocalPosition;
}

void FVoxelGraphParameters::FPosition2D::SetLocalPosition(const FVoxelDoubleVector2DBuffer& LocalPosition)
{
	LocalPosition_Double = LocalPosition;
}

void FVoxelGraphParameters::FPosition2D::SetWorldPosition(const FVoxelVector2DBuffer& WorldPosition)
{
	WorldPosition_Float = WorldPosition;
}

void FVoxelGraphParameters::FPosition2D::SetWorldPosition(const FVoxelDoubleVector2DBuffer& WorldPosition)
{
	WorldPosition_Double = WorldPosition;
}

FVoxelVector2DBuffer FVoxelGraphParameters::FPosition2D::GetLocalPosition_Float(const FVoxelGraphQuery& Query) const
{
	if (!LocalPosition_Float)
	{
		VOXEL_FUNCTION_COUNTER();

		if (LocalPosition_Double)
		{
			LocalPosition_Float = FVoxelBufferConversionUtilities::DoubleToFloat(LocalPosition_Double.GetValue());
		}
		else
		{
			LocalPosition_Float = FVoxelBufferTransformUtilities::ApplyInverseTransform(
				GetWorldPosition_Float(Query),
				Query->Context.Environment.LocalToWorld2D);
		}
	}

	return *LocalPosition_Float;
}

FVoxelVector2DBuffer FVoxelGraphParameters::FPosition2D::GetWorldPosition_Float(const FVoxelGraphQuery& Query) const
{
	if (!WorldPosition_Float)
	{
		VOXEL_FUNCTION_COUNTER();

		if (WorldPosition_Double)
		{
			WorldPosition_Float = FVoxelBufferConversionUtilities::DoubleToFloat(WorldPosition_Double.GetValue());
		}
		else
		{
			WorldPosition_Float = FVoxelBufferTransformUtilities::ApplyTransform(
				GetLocalPosition_Float(Query),
				Query->Context.Environment.LocalToWorld2D);
		}
	}

	return *WorldPosition_Float;
}

FVoxelDoubleVector2DBuffer FVoxelGraphParameters::FPosition2D::GetLocalPosition_Double(const FVoxelGraphQuery& Query) const
{
	if (!LocalPosition_Double)
	{
		VOXEL_FUNCTION_COUNTER();

		if (LocalPosition_Float &&
			!WorldPosition_Double)
		{
			LocalPosition_Double = FVoxelBufferConversionUtilities::FloatToDouble(LocalPosition_Float.GetValue());
		}
		else
		{
			LocalPosition_Double = FVoxelBufferTransformUtilities::ApplyInverseTransform(
				GetWorldPosition_Double(Query),
				Query->Context.Environment.LocalToWorld2D);
		}
	}

	return *LocalPosition_Double;
}

FVoxelDoubleVector2DBuffer FVoxelGraphParameters::FPosition2D::GetWorldPosition_Double(const FVoxelGraphQuery& Query) const
{
	if (!WorldPosition_Double)
	{
		VOXEL_FUNCTION_COUNTER();

		if (WorldPosition_Float &&
			!LocalPosition_Double)
		{
			WorldPosition_Double = FVoxelBufferConversionUtilities::FloatToDouble(WorldPosition_Float.GetValue());
		}
		else
		{
			WorldPosition_Double = FVoxelBufferTransformUtilities::ApplyTransform(
				GetLocalPosition_Double(Query),
				Query->Context.Environment.LocalToWorld2D);
		}
	}

	return *WorldPosition_Double;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 FVoxelGraphParameters::FPosition3D::Num() const
{
	if (LocalPosition_Float)
	{
		return LocalPosition_Float->Num();
	}
	if (WorldPosition_Float)
	{
		return WorldPosition_Float->Num();
	}
	if (LocalPosition_Double)
	{
		return LocalPosition_Double->Num();
	}
	if (WorldPosition_Double)
	{
		return WorldPosition_Double->Num();
	}

	ensure(false);
	return 0;
}

void FVoxelGraphParameters::FPosition3D::Split(
	const FVoxelBufferSplitter& Splitter,
	const TConstVoxelArrayView<FPosition3D*> OutResult) const
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(Splitter.NumOutputs() == OutResult.Num());

	if (Position2D)
	{
		TVoxelInlineArray<FPosition2D*, 8> Position2Ds;
		FVoxelUtilities::SetNumZeroed(Position2Ds, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Position2Ds[Index] = &OutResult[Index]->Position2D.Emplace();
		}

		Position2D->Split(Splitter, Position2Ds);
	}

	if (LocalPosition_Float)
	{
		TVoxelInlineArray<FVoxelVectorBuffer*, 8> Buffers;
		FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Buffers[Index] = &OutResult[Index]->LocalPosition_Float.Emplace();
		}

		LocalPosition_Float->Split(Splitter, Buffers);
	}

	if (WorldPosition_Float)
	{
		TVoxelInlineArray<FVoxelVectorBuffer*, 8> Buffers;
		FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Buffers[Index] = &OutResult[Index]->WorldPosition_Float.Emplace();
		}

		WorldPosition_Float->Split(Splitter, Buffers);
	}

	if (LocalPosition_Double)
	{
		TVoxelInlineArray<FVoxelDoubleVectorBuffer*, 8> Buffers;
		FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Buffers[Index] = &OutResult[Index]->LocalPosition_Double.Emplace();
		}

		LocalPosition_Double->Split(Splitter, Buffers);
	}

	if (WorldPosition_Double)
	{
		TVoxelInlineArray<FVoxelDoubleVectorBuffer*, 8> Buffers;
		FVoxelUtilities::SetNumZeroed(Buffers, Splitter.NumOutputs());

		for (const int32 Index : Splitter.GetValidOutputs())
		{
			Buffers[Index] = &OutResult[Index]->WorldPosition_Double.Emplace();
		}

		WorldPosition_Double->Split(Splitter, Buffers);
	}
}

void FVoxelGraphParameters::FPosition3D::SetLocalPosition(const FVoxelVectorBuffer& LocalPosition)
{
	LocalPosition_Float = LocalPosition;
}

void FVoxelGraphParameters::FPosition3D::SetLocalPosition(const FVoxelDoubleVectorBuffer& LocalPosition)
{
	LocalPosition_Double = LocalPosition;
}

void FVoxelGraphParameters::FPosition3D::SetWorldPosition(const FVoxelVectorBuffer& WorldPosition)
{
	WorldPosition_Float = WorldPosition;
}

void FVoxelGraphParameters::FPosition3D::SetWorldPosition(const FVoxelDoubleVectorBuffer& WorldPosition)
{
	WorldPosition_Double = WorldPosition;
}

const FVoxelGraphParameters::FPosition2D& FVoxelGraphParameters::FPosition3D::GetPosition2D() const
{
	if (!Position2D)
	{
		Position2D.Emplace();

		if (LocalPosition_Double)
		{
			FVoxelDoubleVector2DBuffer LocalPosition;
			LocalPosition.X = LocalPosition_Double->X;
			LocalPosition.Y = LocalPosition_Double->Y;
			Position2D->SetLocalPosition(LocalPosition);
		}
		else
		{
			FVoxelVector2DBuffer LocalPosition;
			LocalPosition.X = LocalPosition_Float->X;
			LocalPosition.Y = LocalPosition_Float->Y;
			Position2D->SetLocalPosition(LocalPosition);
		}
	}

	return *Position2D;
}

FVoxelVectorBuffer FVoxelGraphParameters::FPosition3D::GetLocalPosition_Float(const FVoxelGraphQuery& Query) const
{
	if (!LocalPosition_Float)
	{
		VOXEL_FUNCTION_COUNTER();

		if (LocalPosition_Double)
		{
			LocalPosition_Float = FVoxelBufferConversionUtilities::DoubleToFloat(LocalPosition_Double.GetValue());
		}
		else
		{
			LocalPosition_Float = FVoxelBufferTransformUtilities::ApplyInverseTransform(
				GetWorldPosition_Float(Query),
				Query->Context.Environment.LocalToWorld);
		}
	}

	return *LocalPosition_Float;
}

FVoxelVectorBuffer FVoxelGraphParameters::FPosition3D::GetWorldPosition_Float(const FVoxelGraphQuery& Query) const
{
	if (!WorldPosition_Float)
	{
		VOXEL_FUNCTION_COUNTER();

		if (WorldPosition_Double)
		{
			WorldPosition_Float = FVoxelBufferConversionUtilities::DoubleToFloat(WorldPosition_Double.GetValue());
		}
		else
		{
			WorldPosition_Float = FVoxelBufferTransformUtilities::ApplyTransform(
				GetLocalPosition_Float(Query),
				Query->Context.Environment.LocalToWorld);
		}
	}

	return *WorldPosition_Float;
}

FVoxelDoubleVectorBuffer FVoxelGraphParameters::FPosition3D::GetLocalPosition_Double(const FVoxelGraphQuery& Query) const
{
	if (!LocalPosition_Double)
	{
		VOXEL_FUNCTION_COUNTER();

		if (LocalPosition_Float &&
			!WorldPosition_Double)
		{
			LocalPosition_Double = FVoxelBufferConversionUtilities::FloatToDouble(LocalPosition_Float.GetValue());
		}
		else
		{
			LocalPosition_Double = FVoxelBufferTransformUtilities::ApplyInverseTransform(
				GetWorldPosition_Double(Query),
				Query->Context.Environment.LocalToWorld);
		}
	}

	return *LocalPosition_Double;
}

FVoxelDoubleVectorBuffer FVoxelGraphParameters::FPosition3D::GetWorldPosition_Double(const FVoxelGraphQuery& Query) const
{
	if (!WorldPosition_Double)
	{
		VOXEL_FUNCTION_COUNTER();

		if (WorldPosition_Float &&
			!LocalPosition_Double)
		{
			WorldPosition_Double = FVoxelBufferConversionUtilities::FloatToDouble(WorldPosition_Float.GetValue());
		}
		else
		{
			WorldPosition_Double = FVoxelBufferTransformUtilities::ApplyTransform(
				GetLocalPosition_Double(Query),
				Query->Context.Environment.LocalToWorld);
		}
	}

	return *WorldPosition_Double;
}