// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNoiseNodes.h"

void FVoxelNode_MakeSeeds::Initialize(FInitializer& Initializer)
{
	for (FPinRef_Output& PinRef : ResultPins)
	{
		Initializer.InitializePinRef(PinRef);
	}
}

void FVoxelNode_MakeSeeds::Compute(const FVoxelGraphQuery Query) const
{
	if (AreTemplatePinsBuffers())
	{
		const TValue<FVoxelSeedBuffer> Seeds = SeedPin.GetBuffer(Query);

		VOXEL_GRAPH_WAIT(Seeds)
		{
			for (int32 PinIndex = 0; PinIndex < ResultPins.Num(); PinIndex++)
			{
				FVoxelSeedBuffer NewSeeds;
				NewSeeds.Allocate(Seeds->Num());

				for (int32 SeedIndex = 0; SeedIndex < Seeds->Num(); SeedIndex++)
				{
					NewSeeds.Set(SeedIndex, FVoxelUtilities::MurmurHash((*Seeds)[SeedIndex], PinIndex));
				}

				ResultPins[PinIndex].Set(Query, FVoxelRuntimePinValue::Make(MoveTemp(NewSeeds)));
			}
		};
	}
	else
	{
		const TValue<FVoxelSeed> Seed = SeedPin.GetUniform(Query);

		VOXEL_GRAPH_WAIT(Seed)
		{
			for (int32 PinIndex = 0; PinIndex < ResultPins.Num(); PinIndex++)
			{
				const FVoxelSeed NewSeed = FVoxelUtilities::MurmurHash(Seed, PinIndex);
				ResultPins[PinIndex].Set(Query, FVoxelRuntimePinValue::Make(NewSeed));
			}
		};
	}
}

void FVoxelNode_MakeSeeds::PostSerialize()
{
	FixupSeedPins();

	Super::PostSerialize();

	FixupSeedPins();
}

void FVoxelNode_MakeSeeds::FixupSeedPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (const FPinRef& ResultPin : ResultPins)
	{
		RemovePin(ResultPin.GetName());
	}

	ResultPins.Reset();

	const bool bIsBuffer = AreTemplatePinsBuffers();
	for (int32 Index = 0; Index < NumNewSeeds; Index++)
	{
		if (bIsBuffer)
		{
			ResultPins.Add(
				CreateOutputPin<FVoxelSeedBuffer>(
					FName("Seed", Index + 1),
					VOXEL_PIN_METADATA(
						FVoxelSeedBuffer,
						nullptr,
						DisplayName("Seed " + LexToString(Index + 1))),
					EVoxelPinFlags::TemplatePin)
			);
		}
		else
		{
			ResultPins.Add(
				CreateOutputPin<FVoxelSeed>(
					FName("Seed", Index + 1),
					VOXEL_PIN_METADATA(
						FVoxelSeed,
						nullptr,
						DisplayName("Seed " + LexToString(Index + 1))),
					EVoxelPinFlags::TemplatePin)
			);
		}
	}
}