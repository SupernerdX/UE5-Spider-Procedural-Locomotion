// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelNode_BreakSurfaceTypeBlend.h"
#include "VoxelGraphMigration.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	REGISTER_VOXEL_NODE_MIGRATION("UnpackMaterial", FVoxelNode_BreakSurfaceTypeBlend);
}

FVoxelNode_BreakSurfaceTypeBlend::FVoxelNode_BreakSurfaceTypeBlend()
{
	FixupLayerPins();
}

void FVoxelNode_BreakSurfaceTypeBlend::Initialize(FInitializer& Initializer)
{
	VOXEL_FUNCTION_COUNTER();

	for (FLayerPin& LayerPin : LayerPins)
	{
		Initializer.InitializePinRef(LayerPin.Type);
		Initializer.InitializePinRef(LayerPin.Weight);
	}
}

void FVoxelNode_BreakSurfaceTypeBlend::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelSurfaceTypeBlendBuffer> SurfaceTypeBlends = SurfaceTypeBlendPin.Get(Query);

	VOXEL_GRAPH_WAIT(SurfaceTypeBlends)
	{
		struct FLayer
		{
			TSharedRef<FVoxelSurfaceTypeBuffer> Types = MakeShared<FVoxelSurfaceTypeBuffer>();
			TSharedRef<FVoxelFloatBuffer> Weights = MakeShared<FVoxelFloatBuffer>();
		};

		TVoxelInlineArray<FLayer, FVoxelSurfaceTypeBlend::MaxLayers> Layers;
		Layers.Reserve(LayerPins.Num());

		for (int32 Index = 0; Index < LayerPins.Num(); Index++)
		{
			const FLayer& Layer = Layers.Emplace_GetRef_EnsureNoGrow();
			Layer.Types->Allocate(SurfaceTypeBlends->Num());
			Layer.Weights->Allocate(SurfaceTypeBlends->Num());
		}

		for (int32 Index = 0; Index < SurfaceTypeBlends->Num(); Index++)
		{
			FVoxelSurfaceTypeBlendBase SurfaceTypeBlend = (*SurfaceTypeBlends)[Index].AsBase();
			SurfaceTypeBlend.SortByWeight();

			for (int32 LayerIndex = 0; LayerIndex < LayerPins.Num(); LayerIndex++)
			{
				const FLayer& Layer = Layers[LayerIndex];

				if (LayerIndex < SurfaceTypeBlend.NumLayers)
				{
					const FVoxelSurfaceTypeBlendLayer& SurfaceLayer = SurfaceTypeBlend.Layers[LayerIndex];

					Layer.Types->Set(Index, SurfaceLayer.Type);
					Layer.Weights->Set(Index, SurfaceLayer.Weight.ToFloat());
				}
				else
				{
					Layer.Types->Set(Index, {});
					Layer.Weights->Set(Index, 0.f);
				}
			}
		}

		for (int32 Index = 0; Index < LayerPins.Num(); Index++)
		{
			const FLayer& Layer = Layers[Index];
			const FLayerPin& LayerPin = LayerPins[Index];

			LayerPin.Type.Set(Query, Layer.Types);
			LayerPin.Weight.Set(Query, Layer.Weights);
		}
	};
}

void FVoxelNode_BreakSurfaceTypeBlend::PostSerialize()
{
	FixupLayerPins();

	Super::PostSerialize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_BreakSurfaceTypeBlend::FixupLayerPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (const FLayerPin& Layer : LayerPins)
	{
		RemovePin(Layer.Type.GetName());
		RemovePin(Layer.Weight.GetName());
	}
	LayerPins.Reset();

	for (int32 Index = 0; Index < NumLayerPins; Index++)
	{
		const FString Category = "Layer " + FString::FromInt(Index);

		LayerPins.Add(FLayerPin
		{
			CreateOutputPin<FVoxelSurfaceTypeBuffer>(
				FName("Type", Index + 1),
				VOXEL_PIN_METADATA(
					FVoxelSurfaceTypeBlendBuffer,
					nullptr,
					DisplayName("Type"),
					Tooltip("Surface type of this layer"),
					Category(Category))),

			CreateOutputPin<FVoxelFloatBuffer>(
				FName("Weight", Index + 1),
				VOXEL_PIN_METADATA(
					FVoxelFloatBuffer,
					nullptr,
					DisplayName("Weight"),
					Tooltip("Weight of this layer"),
					Category(Category)))
		});
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString FVoxelNode_BreakSurfaceTypeBlend::FDefinition::GetAddPinLabel() const
{
	return "Add Layer";
}

FString FVoxelNode_BreakSurfaceTypeBlend::FDefinition::GetAddPinTooltip() const
{
	return "Adds an additional layer pin";
}

bool FVoxelNode_BreakSurfaceTypeBlend::FDefinition::CanAddInputPin() const
{
	return Node.NumLayerPins < FVoxelSurfaceTypeBlend::MaxLayers;
}

void FVoxelNode_BreakSurfaceTypeBlend::FDefinition::AddInputPin()
{
	Node.NumLayerPins++;
	Node.FixupLayerPins();
}

bool FVoxelNode_BreakSurfaceTypeBlend::FDefinition::CanRemoveInputPin() const
{
	return Node.NumLayerPins > 1;
}

void FVoxelNode_BreakSurfaceTypeBlend::FDefinition::RemoveInputPin()
{
	Node.NumLayerPins--;
	Node.FixupLayerPins();
}
#endif