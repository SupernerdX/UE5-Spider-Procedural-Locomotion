// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.generated.h"

class UVoxelLayer;
class UVoxelLayerStack;
class UVoxelHeightLayer;
class UVoxelVolumeLayer;

UENUM()
enum class EVoxelLayerType : uint8
{
	Height,
	Volume
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelStackLayer
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ForceShowPluginContent))
	TObjectPtr<UVoxelLayerStack> Stack;

	// If null the last layer of Stack will be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ForceShowPluginContent))
	TObjectPtr<UVoxelLayer> Layer;

public:
	FVoxelStackLayer();
	FVoxelStackLayer(
		UVoxelLayerStack* Stack,
		UVoxelLayer* Layer)
		: Stack(Stack)
		, Layer(Layer)
	{
	}

public:
	bool IsValid() const;
	EVoxelLayerType GetType() const;

	bool operator==(const FVoxelStackLayer& Other) const
	{
		return
			Stack == Other.Stack &&
			Layer == Other.Layer;
	}
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelStackHeightLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelLayerStack> Stack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelHeightLayer> Layer;

public:
	FVoxelStackHeightLayer();
	FVoxelStackHeightLayer(
		UVoxelLayerStack* Stack,
		UVoxelHeightLayer* Layer)
		: Stack(Stack)
		, Layer(Layer)
	{
	}

public:
	operator FVoxelStackLayer() const;

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelStackVolumeLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelLayerStack> Stack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelVolumeLayer> Layer;

public:
	FVoxelStackVolumeLayer();
	FVoxelStackVolumeLayer(
		UVoxelLayerStack* Stack,
		UVoxelVolumeLayer* Layer)
		: Stack(Stack)
		, Layer(Layer)
	{
	}

public:
	operator FVoxelStackLayer() const;

	bool IsValid() const;
};

USTRUCT()
struct VOXEL_API FVoxelWeakStackLayer
{
	GENERATED_BODY()

public:
	EVoxelLayerType Type = {};
	TVoxelObjectPtr<UVoxelLayerStack> Stack;
	TVoxelObjectPtr<UVoxelLayer> Layer;

public:
	FVoxelWeakStackLayer() = default;
	FVoxelWeakStackLayer(const FVoxelStackLayer& StackLayer);
	explicit FVoxelWeakStackLayer(const FVoxelStackHeightLayer& StackLayer);
	explicit FVoxelWeakStackLayer(const FVoxelStackVolumeLayer& StackLayer);

	FString ToString() const;
	FVoxelStackLayer Resolve() const;

public:
	FORCEINLINE bool operator==(const FVoxelWeakStackLayer& Other) const
	{
		return
			Stack == Other.Stack &&
			Layer == Other.Layer;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelWeakStackLayer& WeakLayer)
	{
		return
			GetTypeHash(WeakLayer.Stack) ^
			GetTypeHash(WeakLayer.Layer);
	}
};

USTRUCT()
struct VOXEL_API FVoxelWeakStackHeightLayer : public FVoxelWeakStackLayer
{
	GENERATED_BODY()

	FVoxelWeakStackHeightLayer() = default;
	FVoxelWeakStackHeightLayer(const FVoxelStackHeightLayer& Layer)
		: FVoxelWeakStackLayer(Layer)
	{
	}

	FVoxelStackHeightLayer Resolve() const;
};

USTRUCT()
struct VOXEL_API FVoxelWeakStackVolumeLayer : public FVoxelWeakStackLayer
{
	GENERATED_BODY()

	FVoxelWeakStackVolumeLayer() = default;
	FVoxelWeakStackVolumeLayer(const FVoxelStackVolumeLayer& Layer)
		: FVoxelWeakStackLayer(Layer)
	{
	}

	FVoxelStackVolumeLayer Resolve() const;
};