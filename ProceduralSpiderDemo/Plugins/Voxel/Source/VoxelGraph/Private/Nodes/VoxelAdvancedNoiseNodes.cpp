// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelAdvancedNoiseNodes.h"
#include "VoxelAdvancedNoiseNodesImpl.ispc.generated.h"
#include "VoxelBufferAccessor.h"

FORCEINLINE ispc::EOctaveType GetISPCNoise(const EVoxelAdvancedNoiseOctaveType Noise)
{
	switch (Noise)
	{
	default: ensure(false);

#define CASE(Name) case EVoxelAdvancedNoiseOctaveType::Name: return ispc::OctaveType_ ## Name;

	case EVoxelAdvancedNoiseOctaveType::Default:
	CASE(SmoothPerlin);
	CASE(BillowyPerlin);
	CASE(RidgedPerlin);

	CASE(SmoothCellular);
	CASE(BillowyCellular);
	CASE(RidgedCellular);

	CASE(SmoothSimplex);
	CASE(BillowySimplex);
	CASE(RidgedSimplex);

	CASE(SmoothValue);
	CASE(BillowyValue);
	CASE(RidgedValue);

#undef CASE
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_AdvancedNoise2D::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelVector2DBuffer> Positions = PositionPin.Get(Query);
	const TValue<FVoxelFloatBuffer> Amplitudes = AmplitudePin.Get(Query);
	const TValue<FVoxelFloatBuffer> FeatureScales = FeatureScalePin.Get(Query);
	const TValue<FVoxelFloatBuffer> Lacunarities = LacunarityPin.Get(Query);
	const TValue<FVoxelFloatBuffer> Gains = GainPin.Get(Query);
	const TValue<FVoxelFloatBuffer> CellularJitters = CellularJitterPin.Get(Query);
	const TValue<int32> NumOctaves = NumOctavesPin.Get(Query);
	const TValue<FVoxelSeed> Seed = SeedPin.Get(Query);
	const TValue<EVoxelAdvancedNoiseOctaveType> DefaultOctaveType = DefaultOctaveTypePin.Get(Query);
	const TVoxelArray<TValue<EVoxelAdvancedNoiseOctaveType>> OctaveTypes = OctaveTypePins.Get(Query);
	const TVoxelArray<TValue<FVoxelFloatBuffer>> OctaveStrengths = OctaveStrengthPins.Get(Query);

	VOXEL_GRAPH_WAIT(Positions, Amplitudes, FeatureScales, Lacunarities, Gains, CellularJitters, NumOctaves, Seed, DefaultOctaveType, OctaveTypes, OctaveStrengths)
	{
		const int32 Num = ComputeVoxelBuffersNum(Positions, Amplitudes, FeatureScales, Lacunarities, Gains, CellularJitters);
		const int32 SafeNumOctaves = FMath::Clamp(NumOctaves, 1, 255);

		TVoxelInlineArray<ispc::FOctave, 16> Octaves;
		Octaves.Reserve(SafeNumOctaves);

		for (int32 Index = 0; Index < SafeNumOctaves; Index++)
		{
			ispc::FOctave& Octave = Octaves.Emplace_GetRef(ispc::FOctave{});

			if (OctaveTypes.IsValidIndex(Index) &&
				OctaveTypes[Index] != EVoxelAdvancedNoiseOctaveType::Default)
			{
				Octave.Type = GetISPCNoise(OctaveTypes[Index]);
			}
			else
			{
				Octave.Type = GetISPCNoise(DefaultOctaveType);
			}

			if (OctaveStrengths.IsValidIndex(Index))
			{
				const FVoxelFloatBuffer& Strength = *OctaveStrengths[Index];

				if (Strength.IsConstant())
				{
					Octave.bStrengthIsConstant = true;
					Octave.StrengthConstant = Strength.GetConstant();
				}
				else
				{
					if (Strength.Num() != Num)
					{
						RaiseBufferError();
						return;
					}

					Octave.bStrengthIsConstant = false;
					Octave.StrengthArray = Strength.GetData();
				}
			}
			else
			{
				Octave.bStrengthIsConstant = true;
				Octave.StrengthConstant = 1.f;
				Octave.StrengthArray = nullptr;
			}
		}

		VOXEL_SCOPE_COUNTER_FORMAT("AdvancedNoise2D Num=%d", Num);
		FVoxelNodeStatScope StatScope(*this, Num);

		FVoxelFloatBuffer ReturnValue;
		ReturnValue.Allocate(Num);

		ispc::VoxelNode_AdvancedNoise2D(
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
			CellularJitters->GetData(),
			CellularJitters->IsConstant(),
			Octaves.GetData(),
			Octaves.Num(),
			Seed,
			ReturnValue.GetData(),
			Num);

		ValuePin.Set(Query, MoveTemp(ReturnValue));
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_AdvancedNoise3D::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelVectorBuffer> Positions = PositionPin.Get(Query);
	const TValue<FVoxelFloatBuffer> Amplitudes = AmplitudePin.Get(Query);
	const TValue<FVoxelFloatBuffer> FeatureScales = FeatureScalePin.Get(Query);
	const TValue<FVoxelFloatBuffer> Lacunarities = LacunarityPin.Get(Query);
	const TValue<FVoxelFloatBuffer> Gains = GainPin.Get(Query);
	const TValue<FVoxelFloatBuffer> CellularJitters = CellularJitterPin.Get(Query);
	const TValue<int32> NumOctaves = NumOctavesPin.Get(Query);
	const TValue<FVoxelSeed> Seed = SeedPin.Get(Query);
	const TValue<EVoxelAdvancedNoiseOctaveType> DefaultOctaveType = DefaultOctaveTypePin.Get(Query);
	const TVoxelArray<TValue<EVoxelAdvancedNoiseOctaveType>> OctaveTypes = OctaveTypePins.Get(Query);
	const TVoxelArray<TValue<FVoxelFloatBuffer>> OctaveStrengths = OctaveStrengthPins.Get(Query);

	VOXEL_GRAPH_WAIT(Positions, Amplitudes, FeatureScales, Lacunarities, Gains, CellularJitters, NumOctaves, Seed, DefaultOctaveType, OctaveTypes, OctaveStrengths)
	{
		const int32 Num = ComputeVoxelBuffersNum(Positions, Amplitudes, FeatureScales, Lacunarities, Gains, CellularJitters);
		const int32 SafeNumOctaves = FMath::Clamp(NumOctaves, 1, 255);

		TVoxelInlineArray<ispc::FOctave, 16> Octaves;
		Octaves.Reserve(SafeNumOctaves);

		for (int32 Index = 0; Index < SafeNumOctaves; Index++)
		{
			ispc::FOctave& Octave = Octaves.Emplace_GetRef(ispc::FOctave{});

			if (OctaveTypes.IsValidIndex(Index) &&
				OctaveTypes[Index] != EVoxelAdvancedNoiseOctaveType::Default)
			{
				Octave.Type = GetISPCNoise(OctaveTypes[Index]);
			}
			else
			{
				Octave.Type = GetISPCNoise(DefaultOctaveType);
			}

			if (OctaveStrengths.IsValidIndex(Index))
			{
				const FVoxelFloatBuffer& Strength = *OctaveStrengths[Index];

				if (Strength.IsConstant())
				{
					Octave.bStrengthIsConstant = true;
					Octave.StrengthConstant = Strength.GetConstant();
				}
				else
				{
					if (Strength.Num() != Num)
					{
						RaiseBufferError();
						return;
					}

					Octave.bStrengthIsConstant = false;
					Octave.StrengthArray = Strength.GetData();
				}
			}
			else
			{
				Octave.bStrengthIsConstant = true;
				Octave.StrengthConstant = 1.f;
				Octave.StrengthArray = nullptr;
			}
		}

		VOXEL_SCOPE_COUNTER_FORMAT("AdvancedNoise3D Num=%d", Num);
		FVoxelNodeStatScope StatScope(*this, Num);

		FVoxelFloatBuffer ReturnValue;
		ReturnValue.Allocate(Num);

		ispc::VoxelNode_AdvancedNoise3D(
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
			CellularJitters->GetData(),
			CellularJitters->IsConstant(),
			Octaves.GetData(),
			Octaves.Num(),
			Seed,
			ReturnValue.GetData(),
			Num);

		ValuePin.Set(Query, MoveTemp(ReturnValue));
	};
}