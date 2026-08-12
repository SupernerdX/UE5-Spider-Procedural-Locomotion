// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_RandomSelect.h"

void FVoxelNode_RandomSelect::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelBuffer> Values = ValuesPin.Get<FVoxelBuffer>(Query);
	const TValue<FVoxelFloatBuffer> Weights = WeightsPin.Get(Query);

	if (AreTemplatePinsBuffers())
	{
		const TValue<FVoxelSeedBuffer> Seeds = SeedPin.GetBuffer(Query);

		VOXEL_GRAPH_WAIT(Seeds, Values, Weights)
		{
			const int32 NumValues = Values->Num_Slow();
			if (NumValues == 0)
			{
				// Make sure to return an empty buffer
				ResultPin.Set(Query, FVoxelRuntimePinValue::Make(Values));
				IndexPin.Set(Query, FVoxelInt32Buffer());
				return;
			}

			FVoxelNodeStatScope StatScope(*this, Seeds->Num());

			FVoxelInt32Buffer Indices;
			Indices.Allocate(Seeds->Num());

			const int32 Random = STATIC_HASH("RandomSelect");

			if (Weights->Num() > 0)
			{
				if (Weights->Num() != NumValues)
				{
					VOXEL_MESSAGE(Error, "Values.Num={0} but Weights.Num={1}", NumValues, Weights->Num());
					return;
				}

				TVoxelArray<double> RunningSum;
				FVoxelUtilities::SetNumFast(RunningSum, NumValues);

				double TotalSum = 0.;
				for (int32 Index = 0; Index < NumValues; Index++)
				{
					const float Weight = FMath::Max((*Weights)[Index], 0.f);
					TotalSum += Weight;

					if (Index == 0)
					{
						RunningSum[Index] = Weight;
					}
					else
					{
						RunningSum[Index] = RunningSum[Index - 1] + Weight;
					}
				}

				for (int32 Index = 0; Index < Seeds->Num(); Index++)
				{
					const FVoxelSeed Seed = (*Seeds)[Index];
					const uint32 LocalSeed = FVoxelUtilities::MurmurHash(uint32(Seed), Random);
					const double Value = TotalSum * FVoxelUtilities::GetFraction(LocalSeed);

					for (int32 ValueIndex = 0; ValueIndex < NumValues; ValueIndex++)
					{
						if (Value <= RunningSum[ValueIndex])
						{
							Indices.Set(Index, ValueIndex);
							goto End;
						}
					}

					ensureVoxelSlow(false);
					Indices.Set(Index, 0);

					End:
						;
				}
			}
			else
			{
				for (int32 Index = 0; Index < Seeds->Num(); Index++)
				{
					const FVoxelSeed Seed = (*Seeds)[Index];
					const uint32 LocalSeed = FVoxelUtilities::MurmurHash(uint32(Seed), Random);
					Indices.Set(Index, FVoxelUtilities::RandRange(LocalSeed, 0, NumValues - 1));
				}
			}

			const TSharedRef<const FVoxelBuffer> Buffer = Values->Gather(Indices.View());

			ResultPin.Set(Query, FVoxelRuntimePinValue::Make(Buffer));
			IndexPin.Set(Query, MoveTemp(Indices));
		};
	}
	else
	{
		const TValue<FVoxelSeed> Seed = SeedPin.GetUniform(Query);

		VOXEL_GRAPH_WAIT(Seed, Values, Weights)
		{
			const int32 NumValues = Values->Num_Slow();
			if (NumValues == 0)
			{
				return;
			}

			FVoxelNodeStatScope StatScope(*this, 1);

			int32 ResultIndex = -1;

			const int32 Random = STATIC_HASH("RandomSelect");

			if (Weights->Num() > 0)
			{
				if (Weights->Num() != NumValues)
				{
					VOXEL_MESSAGE(Error, "Values.Num={0} but Weights.Num={1}", NumValues, Weights->Num());
					return;
				}

				TVoxelArray<double> RunningSum;
				FVoxelUtilities::SetNumFast(RunningSum, NumValues);

				double TotalSum = 0.;
				for (int32 Index = 0; Index < NumValues; Index++)
				{
					const float Weight = FMath::Max((*Weights)[Index], 0.f);
					TotalSum += Weight;

					if (Index == 0)
					{
						RunningSum[Index] = Weight;
					}
					else
					{
						RunningSum[Index] = RunningSum[Index - 1] + Weight;
					}
				}

				const uint32 LocalSeed = FVoxelUtilities::MurmurHash(uint32(Seed), Random);
				const double Value = TotalSum * FVoxelUtilities::GetFraction(LocalSeed);

				for (int32 ValueIndex = 0; ValueIndex < NumValues; ValueIndex++)
				{
					if (Value <= RunningSum[ValueIndex])
					{
						ResultIndex = ValueIndex;
						goto End;
					}
				}

				ensureVoxelSlow(false);
				ResultIndex = 0;

				End:
					;
			}
			else
			{
				const uint32 LocalSeed = FVoxelUtilities::MurmurHash(uint32(Seed), Random);
				ResultIndex = FVoxelUtilities::RandRange(LocalSeed, 0, NumValues - 1);
			}

			ResultPin.Set(Query, Values->GetGeneric(ResultIndex));
			IndexPin.Set(Query, ResultIndex);
		};
	}
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_RandomSelect::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == SeedPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add(FVoxelPinType::Make<FVoxelSeed>());
		OutTypes.Add(FVoxelPinType::Make<FVoxelSeedBuffer>());
		return OutTypes;
	}
	else if (Pin == ValuesPin)
	{
		return FVoxelPinTypeSet::AllBufferArrays();
	}
	else if (Pin == ResultPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add(FVoxelPinTypeSet::AllUniforms());
		OutTypes.Add(FVoxelPinTypeSet::AllBuffers());
		return OutTypes;
	}
	else if (Pin == IndexPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add(FVoxelPinType::Make<int32>());
		OutTypes.Add(FVoxelPinType::Make<FVoxelInt32Buffer>());
		return OutTypes;
	}
	else
	{
		return FVoxelPinTypeSet::AllBuffers();
	}
}

void FVoxelNode_RandomSelect::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Pin.SetType(NewType);

	const auto SwitchPinType = [](FVoxelPin& TargetPin, const bool bBuffer)
	{
		if (bBuffer)
		{
			TargetPin.SetType(TargetPin.GetType().GetBufferType());
		}
		else
		{
			TargetPin.SetType(TargetPin.GetType().GetInnerType());
		}
	};

	if (Pin == SeedPin)
	{
		SwitchPinType(GetPin(ResultPin), NewType.IsBuffer());
		SwitchPinType(GetPin(IndexPin), NewType.IsBuffer());
	}
	else if (Pin == ValuesPin)
	{
		if (GetPin(SeedPin).GetType().IsBuffer())
		{
			GetPin(ResultPin).SetType(NewType.WithBufferArray(false));
		}
		else
		{
			GetPin(ResultPin).SetType(NewType.GetInnerType());
		}
	}
	else if (Pin == ResultPin)
	{
		SwitchPinType(GetPin(SeedPin), NewType.IsBuffer());
		SwitchPinType(GetPin(IndexPin), NewType.IsBuffer());

		GetPin(ValuesPin).SetType(NewType.GetBufferType().WithBufferArray(true));
	}
	else if (Pin == IndexPin)
	{
		SwitchPinType(GetPin(SeedPin), NewType.IsBuffer());
		SwitchPinType(GetPin(ResultPin), NewType.IsBuffer());
	}
}
#endif