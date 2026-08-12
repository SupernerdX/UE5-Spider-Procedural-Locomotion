// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStackLayer.h"
#include "VoxelLayer.h"
#include "VoxelLayerStack.h"

FVoxelStackLayer::FVoxelStackLayer()
{
	Stack = UVoxelLayerStack::Default();
	Layer = UVoxelVolumeLayer::Default();
}

bool FVoxelStackLayer::IsValid() const
{
	return !FVoxelWeakStackLayer(*this).Layer.IsExplicitlyNull();
}

EVoxelLayerType FVoxelStackLayer::GetType() const
{
	return FVoxelWeakStackLayer(*this).Type;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStackHeightLayer::FVoxelStackHeightLayer()
{
	Stack = UVoxelLayerStack::Default();
	Layer = UVoxelHeightLayer::Default();
}

FVoxelStackHeightLayer::operator FVoxelStackLayer() const
{
	return FVoxelStackLayer
	{
		Stack,
		Layer
	};
}

bool FVoxelStackHeightLayer::IsValid() const
{
	return FVoxelStackLayer(Stack, Layer).IsValid();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStackVolumeLayer::FVoxelStackVolumeLayer()
{
	Stack = UVoxelLayerStack::Default();
	Layer = UVoxelVolumeLayer::Default();
}

FVoxelStackVolumeLayer::operator FVoxelStackLayer() const
{
	return FVoxelStackLayer
	{
		Stack,
		Layer
	};
}

bool FVoxelStackVolumeLayer::IsValid() const
{
	return FVoxelStackLayer(Stack, Layer).IsValid();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelWeakStackLayer::FVoxelWeakStackLayer(const FVoxelStackLayer& StackLayer)
{
	checkUObjectAccess();

	UVoxelLayerStack* NewStack = StackLayer.Stack;

	UVoxelLayer* NewLayer = INLINE_LAMBDA -> UVoxelLayer*
	{
		if (!NewStack)
		{
			return nullptr;
		}

		if (StackLayer.Layer)
		{
			if (UVoxelHeightLayer* HeightLayer = Cast<UVoxelHeightLayer>(StackLayer.Layer))
			{
				if (NewStack->HeightLayers.Contains(HeightLayer))
				{
					return HeightLayer;
				}

				return nullptr;
			}

			if (UVoxelVolumeLayer* VolumeLayer = Cast<UVoxelVolumeLayer>(StackLayer.Layer))
			{
				if (NewStack->VolumeLayers.Contains(VolumeLayer))
				{
					return VolumeLayer;
				}

				return nullptr;
			}

			checkVoxelSlow(false);
			return nullptr;
		}

		if (NewStack->VolumeLayers.Num() > 0)
		{
			return NewStack->VolumeLayers.Last();
		}
		else if (NewStack->HeightLayers.Num() > 0)
		{
			return NewStack->HeightLayers.Last();
		}

		return nullptr;
	};

	Type =
		NewLayer && NewLayer->IsA<UVoxelVolumeLayer>()
		? EVoxelLayerType::Volume
		: EVoxelLayerType::Height;

	Stack = NewStack;
	Layer = NewLayer;
}

FVoxelWeakStackLayer::FVoxelWeakStackLayer(const FVoxelStackHeightLayer& StackLayer)
	: FVoxelWeakStackLayer(ReinterpretCastRef<FVoxelStackLayer>(StackLayer))
{
}

FVoxelWeakStackLayer::FVoxelWeakStackLayer(const FVoxelStackVolumeLayer& StackLayer)
	: FVoxelWeakStackLayer(ReinterpretCastRef<FVoxelStackLayer>(StackLayer))
{
}

FString FVoxelWeakStackLayer::ToString() const
{
	ensure(IsInGameThread());

	FString Result;

	if (const UVoxelLayerStack* StackObject = Stack.Resolve())
	{
		Result += StackObject->GetName();
	}
	else
	{
		Result = "Invalid";
	}

	Result += ".";

	if (const UVoxelLayer* LayerObject = Layer.Resolve())
	{
		Result += LayerObject->GetName();
	}
	else
	{
		Result = "Invalid";
	}

	return Result;
}

FVoxelStackLayer FVoxelWeakStackLayer::Resolve() const
{
	return FVoxelStackLayer
	{
		Stack.Resolve(),
		Layer.Resolve()
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStackHeightLayer FVoxelWeakStackHeightLayer::Resolve() const
{
	return FVoxelStackHeightLayer
	{
		Stack.Resolve(),
		CastEnsured<UVoxelHeightLayer>(Layer.Resolve())
	};
}

FVoxelStackVolumeLayer FVoxelWeakStackVolumeLayer::Resolve() const
{
	return FVoxelStackVolumeLayer
	{
		Stack.Resolve(),
		CastEnsured<UVoxelVolumeLayer>(Layer.Resolve())
	};
}