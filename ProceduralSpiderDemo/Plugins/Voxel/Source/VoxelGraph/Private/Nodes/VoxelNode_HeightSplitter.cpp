// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_HeightSplitter.h"
#include "VoxelBufferAccessor.h"
#include "VoxelHeightSplitterNodeImpl.ispc.generated.h"

FVoxelNode_HeightSplitter::FVoxelNode_HeightSplitter()
{
	FixupLayerPins();
}

void FVoxelNode_HeightSplitter::Initialize(FInitializer& Initializer)
{
	VOXEL_FUNCTION_COUNTER();

	for (FLayerPin& LayerPin : LayerPins)
	{
		Initializer.InitializePinRef(LayerPin.Height);
		Initializer.InitializePinRef(LayerPin.Falloff);
	}

	for (TPinRef_Output<FVoxelFloatBuffer>& Pin : ResultPins)
	{
		Initializer.InitializePinRef(Pin);
	}
}

void FVoxelNode_HeightSplitter::Compute(const FVoxelGraphQuery Query) const
{
	if (!ensure(LayerPins.Num() > 0) ||
		!ensure(LayerPins.Num() + 1 == ResultPins.Num()))
	{
		return;
	}

	const TValue<FVoxelFloatBuffer> HeightBuffer = HeightPin.Get(Query);

	TVoxelArray<TValue<FVoxelFloatBuffer>> HeightSplits;
	TVoxelArray<TValue<FVoxelFloatBuffer>> FalloffSplits;
	HeightSplits.Reserve(LayerPins.Num());
	FalloffSplits.Reserve(LayerPins.Num());

	for (const FLayerPin& LayerPin : LayerPins)
	{
		HeightSplits.Add(LayerPin.Height.Get(Query));
		FalloffSplits.Add(LayerPin.Falloff.Get(Query));
	}

	VOXEL_GRAPH_WAIT(HeightBuffer, HeightSplits, FalloffSplits)
	{
		// First layer
		{
			const int32 Num = ComputeVoxelBuffersNum(HeightBuffer, HeightSplits[0], FalloffSplits[0]);

			FVoxelFloatBuffer Result;
			Result.Allocate(Num);

			ispc::VoxelNode_HeightSplitter_FirstLayer(
				HeightBuffer->GetData(),
				HeightBuffer->IsConstant(),
				HeightSplits[0]->GetData(),
				HeightSplits[0]->IsConstant(),
				FalloffSplits[0]->GetData(),
				FalloffSplits[0]->IsConstant(),
				Num,
				Result.GetData());

			ResultPins[0].Set(Query, MoveTemp(Result));
		}

		// Middle layers
		for (int32 Index = 1; Index < NumLayerPins; Index++)
		{
			const int32 Num = ComputeVoxelBuffersNum(HeightBuffer, HeightSplits[Index - 1], FalloffSplits[Index - 1], HeightSplits[Index], FalloffSplits[Index]);

			FVoxelFloatBuffer Result;
			Result.Allocate(Num);

			ispc::VoxelNode_HeightSplitter_MiddleLayer(
				HeightBuffer->GetData(),
				HeightBuffer->IsConstant(),
				HeightSplits[Index - 1]->GetData(),
				HeightSplits[Index - 1]->IsConstant(),
				FalloffSplits[Index - 1]->GetData(),
				FalloffSplits[Index - 1]->IsConstant(),
				HeightSplits[Index]->GetData(),
				HeightSplits[Index]->IsConstant(),
				FalloffSplits[Index]->GetData(),
				FalloffSplits[Index]->IsConstant(),
				Num,
				Result.GetData());

			ResultPins[Index].Set(Query, MoveTemp(Result));
		}

		// Last layer
		{
			const int32 Num = ComputeVoxelBuffersNum(HeightBuffer, HeightSplits[NumLayerPins - 1], FalloffSplits[NumLayerPins - 1]);

			FVoxelFloatBuffer Result;
			Result.Allocate(Num);

			ispc::VoxelNode_HeightSplitter_LastLayer(
				HeightBuffer->GetData(),
				HeightBuffer->IsConstant(),
				HeightSplits[NumLayerPins - 1]->GetData(),
				HeightSplits[NumLayerPins - 1]->IsConstant(),
				FalloffSplits[NumLayerPins - 1]->GetData(),
				FalloffSplits[NumLayerPins - 1]->IsConstant(),
				Num,
				Result.GetData());

			ResultPins[NumLayerPins].Set(Query, MoveTemp(Result));
		}
	};
}

void FVoxelNode_HeightSplitter::PostSerialize()
{
	Super::PostSerialize();

	FixupLayerPins();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_HeightSplitter::FixupLayerPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (const FLayerPin& Layer : LayerPins)
	{
		RemovePin(Layer.Height.GetName());
		RemovePin(Layer.Falloff.GetName());
	}
	LayerPins.Reset();

	for (const TPinRef_Output<FVoxelFloatBuffer>& ResultPin : ResultPins)
	{
		RemovePin(ResultPin.GetName());
	}
	ResultPins.Reset();

	ResultPins.Add(CreateOutputPin<FVoxelFloatBuffer>(
		"Strength_0",
		VOXEL_PIN_METADATA(
			FVoxelFloatBuffer,
			nullptr,
			DisplayName("Strength 1"),
			Tooltip("Strength of this layer"))));

	for (int32 Index = 0; Index < NumLayerPins; Index++)
	{
		const FString Category = "Layer " + FString::FromInt(Index);

		LayerPins.Add(FLayerPin
		{
			CreateInputPin<FVoxelFloatBuffer>(
				FName("Height", Index + 1),
				VOXEL_PIN_METADATA(
					FVoxelFloatBuffer,
					Index * 1000.f,
					DisplayName("Height"),
					Tooltip("Height of this layer"),
					Category(Category))),

			CreateInputPin<FVoxelFloatBuffer>(
				FName("Falloff", Index + 1),
				VOXEL_PIN_METADATA(
					FVoxelFloatBuffer,
					100.f,
					DisplayName("Falloff"),
					Tooltip("Falloff of this layer"),
					Category(Category)))
		});

		ResultPins.Add(CreateOutputPin<FVoxelFloatBuffer>(
			FName("Strength", Index + 2),
			VOXEL_PIN_METADATA(
				FVoxelFloatBuffer,
				nullptr,
				DisplayName("Strength " + LexToString(Index + 2)),
				Tooltip("Strength of this layer"))));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString FVoxelNode_HeightSplitter::FDefinition::GetAddPinLabel() const
{
	return "Add Layer";
}

FString FVoxelNode_HeightSplitter::FDefinition::GetAddPinTooltip() const
{
	return "Adds an additional layer pin";
}

void FVoxelNode_HeightSplitter::FDefinition::AddInputPin()
{
	Node.NumLayerPins++;
	Node.FixupLayerPins();
}

bool FVoxelNode_HeightSplitter::FDefinition::CanRemoveInputPin() const
{
	return Node.NumLayerPins > 1;
}

void FVoxelNode_HeightSplitter::FDefinition::RemoveInputPin()
{
	Node.NumLayerPins--;
	Node.FixupLayerPins();
}
#endif