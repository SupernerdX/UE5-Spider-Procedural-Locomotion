// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelDomainWarpNodes.h"
#include "VoxelBufferAccessor.h"
#include "VoxelDomainWarpNodesImpl.ispc.generated.h"

void FVoxelNode_DomainWarp2D::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelVector2DBuffer> Positions = PositionPin.Get(Query);
	const TValue<FVoxelFloatBuffer> Amplitudes = AmplitudePin.Get(Query);
	const TValue<FVoxelFloatBuffer> FeatureScales = FeatureScalePin.Get(Query);
	const TValue<FVoxelFloatBuffer> Lacunarities = LacunarityPin.Get(Query);
	const TValue<FVoxelFloatBuffer> Gains = GainPin.Get(Query);
	const TValue<FVoxelFloatBuffer> WeightedStrength = WeightedStrengthPin.Get(Query);
	const TValue<int32> NumOctaves = NumOctavesPin.Get(Query);
	const TValue<FVoxelSeed> Seed = SeedPin.Get(Query);

	VOXEL_GRAPH_WAIT(Positions, Amplitudes, FeatureScales, Lacunarities, Gains, WeightedStrength, NumOctaves, Seed)
	{
		const int32 Num = ComputeVoxelBuffersNum(Positions, Amplitudes, FeatureScales, Lacunarities, Gains, WeightedStrength);
		const int32 SafeNumOctaves = FMath::Clamp(NumOctaves, 1, 255);

		VOXEL_SCOPE_COUNTER_FORMAT("DomainWarp2D Num=%d", Num);
		FVoxelNodeStatScope StatScope(*this, Num);

		FVoxelVector2DBuffer ReturnValue;
		ReturnValue.Allocate(Num);

		const bool bConstantFractalBounding = Gains->IsConstant();
		float FractalBounding = 0.f;
		if (bConstantFractalBounding)
		{
			const float Gain = Gains->GetConstant();
			float AmpFractal = 1.f;
			float Amp = Gain;
			for (int32 OctaveIndex = 1; OctaveIndex < NumOctaves; OctaveIndex++)
			{
				AmpFractal += Amp;
				Amp *= Gain;
			}
			FractalBounding = 1.f / AmpFractal;
		}

		ispc::VoxelNode_DomainWarp2D(
			Positions->X.GetData(),
			Positions->X.IsConstant(),
			Positions->Y.GetData(),
			Positions->Y.IsConstant(),
			Amplitudes->GetData(),
			Amplitudes->IsConstant(),
			FeatureScales->GetData(),
			FeatureScales->IsConstant(),
			Lacunarities->GetData(),
			Lacunarities->IsConstant(),
			Gains->GetData(),
			Gains->IsConstant(),
			WeightedStrength->GetData(),
			WeightedStrength->IsConstant(),
			FractalBounding,
			bConstantFractalBounding,
			SafeNumOctaves,
			Seed,
			ReturnValue.X.GetData(),
			ReturnValue.Y.GetData(),
			Num);

		ValuePin.Set(Query, MoveTemp(ReturnValue));
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_DomainWarp3D::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelVectorBuffer> Positions = PositionPin.Get(Query);
	const TValue<FVoxelFloatBuffer> Amplitudes = AmplitudePin.Get(Query);
	const TValue<FVoxelFloatBuffer> FeatureScales = FeatureScalePin.Get(Query);
	const TValue<FVoxelFloatBuffer> Lacunarities = LacunarityPin.Get(Query);
	const TValue<FVoxelFloatBuffer> Gains = GainPin.Get(Query);
	const TValue<FVoxelFloatBuffer> WeightedStrength = WeightedStrengthPin.Get(Query);
	const TValue<int32> NumOctaves = NumOctavesPin.Get(Query);
	const TValue<FVoxelSeed> Seed = SeedPin.Get(Query);

	VOXEL_GRAPH_WAIT(Positions, Amplitudes, FeatureScales, Lacunarities, Gains, WeightedStrength, NumOctaves, Seed)
	{
		const int32 Num = ComputeVoxelBuffersNum(Positions, Amplitudes, FeatureScales, Lacunarities, Gains, WeightedStrength);
		const int32 SafeNumOctaves = FMath::Clamp(NumOctaves, 1, 255);

		VOXEL_SCOPE_COUNTER_FORMAT("DomainWarp3D Num=%d", Num);
		FVoxelNodeStatScope StatScope(*this, Num);

		FVoxelVectorBuffer ReturnValue;
		ReturnValue.Allocate(Num);

		const bool bConstantFractalBounding = Gains->IsConstant();
		float FractalBounding = 0.f;
		if (bConstantFractalBounding)
		{
			const float Gain = Gains->GetConstant();
			float AmpFractal = 1.f;
			float Amp = Gain;
			for (int32 OctaveIndex = 1; OctaveIndex < NumOctaves; OctaveIndex++)
			{
				AmpFractal += Amp;
				Amp *= Gain;
			}
			FractalBounding = 1.f / AmpFractal;
		}

		ispc::VoxelNode_DomainWarp3D(
			Positions->X.GetData(),
			Positions->X.IsConstant(),
			Positions->Y.GetData(),
			Positions->Y.IsConstant(),
			Positions->Z.GetData(),
			Positions->Z.IsConstant(),
			Amplitudes->GetData(),
			Amplitudes->IsConstant(),
			FeatureScales->GetData(),
			FeatureScales->IsConstant(),
			Lacunarities->GetData(),
			Lacunarities->IsConstant(),
			Gains->GetData(),
			Gains->IsConstant(),
			WeightedStrength->GetData(),
			WeightedStrength->IsConstant(),
			FractalBounding,
			bConstantFractalBounding,
			SafeNumOctaves,
			Seed,
			ReturnValue.X.GetData(),
			ReturnValue.Y.GetData(),
			ReturnValue.Z.GetData(),
			Num);

		ValuePin.Set(Query, MoveTemp(ReturnValue));
	};
}