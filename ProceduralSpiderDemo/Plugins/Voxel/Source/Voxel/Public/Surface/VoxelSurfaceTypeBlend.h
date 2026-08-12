// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSurfaceType.h"
#include "VoxelSurfaceTypeBlend.generated.h"

struct VOXEL_API FVoxelSurfaceTypeBlendWeight
{
public:
	FVoxelSurfaceTypeBlendWeight() = default;

	FORCEINLINE static FVoxelSurfaceTypeBlendWeight One()
	{
		return FVoxelSurfaceTypeBlendWeight(MAX_uint16);
	}
	FORCEINLINE static FVoxelSurfaceTypeBlendWeight MakeUnsafe(const int32 Value)
	{
		checkVoxelSlow(FVoxelUtilities::IsValidUINT16(Value));
		return FVoxelSurfaceTypeBlendWeight(Value);
	}

	FORCEINLINE bool IsZero() const
	{
		return Value == 0;
	}
	FORCEINLINE float ToFloat() const
	{
		return FVoxelUtilities::UINT16ToFloat(Value);
	}
	FORCEINLINE int32 ToInt32() const
	{
		return Value;
	}

public:
	FORCEINLINE bool operator==(const FVoxelSurfaceTypeBlendWeight& Other) const
	{
		return Value == Other.Value;
	}

	FORCEINLINE bool operator<(const FVoxelSurfaceTypeBlendWeight& Other) const
	{
		return Value < Other.Value;
	}
	FORCEINLINE bool operator>(const FVoxelSurfaceTypeBlendWeight& Other) const
	{
		return Value > Other.Value;
	}

private:
	uint16 Value = 0;

	explicit FVoxelSurfaceTypeBlendWeight(const uint16 Value)
		: Value(Value)
	{
	}
};

struct alignas(4) VOXEL_API FVoxelSurfaceTypeBlendLayer
{
	FVoxelSurfaceType Type;
	FVoxelSurfaceTypeBlendWeight Weight;

	FVoxelSurfaceTypeBlendLayer() = default;

	FORCEINLINE FVoxelSurfaceTypeBlendLayer(
		const FVoxelSurfaceType& Type,
		const FVoxelSurfaceTypeBlendWeight& Weight)
		: Type(Type)
		, Weight(Weight)
	{
	}

	FORCEINLINE bool operator==(const FVoxelSurfaceTypeBlendLayer& Other) const
	{
		return ReinterpretCastRef<uint32>(*this) == ReinterpretCastRef<uint32>(Other);
	}

	FString GetSurfaceName() const;
	FString GetWeightString() const;
};

struct alignas(8) VOXEL_API FVoxelSurfaceTypeBlendBase
{
	static constexpr int32 MaxLayers = 15;

	TVoxelStaticArray<FVoxelSurfaceTypeBlendLayer, MaxLayers> Layers{ NoInit };
	int32 NumLayers = 0;

	void SortByType();
	void SortByWeight();
};

USTRUCT()
struct alignas(8) VOXEL_API FVoxelSurfaceTypeBlend
#if CPP
	: private FVoxelSurfaceTypeBlendBase
#endif
{
	GENERATED_BODY()

public:
	using FVoxelSurfaceTypeBlendBase::MaxLayers;

public:
	FORCEINLINE FVoxelSurfaceTypeBlend()
	{
		NumLayers = 0;
	}
	FORCEINLINE FVoxelSurfaceTypeBlend(const FVoxelSurfaceTypeBlend& Other)
	{
		*this = Other;
	}

	// *this = *this is safe as we have no allocations
	FORCEINLINE FVoxelSurfaceTypeBlend& operator=(const FVoxelSurfaceTypeBlend& Other)
	{
		NumLayers = Other.NumLayers;

		for (int32 Index = 0; Index < NumLayers; Index++)
		{
			Layers[Index] = Other.Layers[Index];
		}

		return *this;
	}

	FORCEINLINE static FVoxelSurfaceTypeBlend FromType(const FVoxelSurfaceType Type)
	{
		FVoxelSurfaceTypeBlend Result;
		Result.InitializeFromType(Type);
		return Result;
	}

	FORCEINLINE void InitializeNull()
	{
		NumLayers = 0;
	}
	FORCEINLINE void InitializeFromType(const FVoxelSurfaceType Type)
	{
		if (Type.IsNull())
		{
			NumLayers = 0;
			return;
		}

		Layers[0] = FVoxelSurfaceTypeBlendLayer(Type, FVoxelSurfaceTypeBlendWeight::One());
		NumLayers = 1;
	}

public:
	FORCEINLINE bool IsNull() const
	{
		Check();
		return NumLayers == 0;
	}

	FORCEINLINE const FVoxelSurfaceTypeBlendBase& AsBase() const
	{
		return *this;
	}

	FORCEINLINE TConstVoxelArrayView<FVoxelSurfaceTypeBlendLayer> GetLayers() const
	{
		Check();
		return Layers.View().LeftOf(NumLayers);
	}

	FORCEINLINE bool operator==(const FVoxelSurfaceTypeBlend& Other) const
	{
		Check();
		Other.Check();

		if (NumLayers != Other.NumLayers)
		{
			return false;
		}

		for (int32 Index = 0; Index < NumLayers; Index++)
		{
			if (Layers[Index] != Other.Layers[Index])
			{
				return false;
			}
		}

		return true;
	}

	FVoxelSurfaceTypeBlendLayer GetTopLayer() const;
	TVoxelArray<FVoxelSurfaceTypeBlendLayer> GetLayersSortedByWeight() const;

	bool Equals_Slow(
		const FVoxelSurfaceTypeBlend& Other,
		float Tolerance = KINDA_SMALL_NUMBER) const;

	void PopLayersForRendering();

public:
	static void Lerp(
		FVoxelSurfaceTypeBlend& OutResult,
		const FVoxelSurfaceTypeBlend& BlendA,
		const FVoxelSurfaceTypeBlend& BlendB,
		float Alpha);

	static void Lerp(
		FVoxelSurfaceTypeBlend& OutResult,
		const FVoxelSurfaceTypeBlend& BlendA,
		const FVoxelSurfaceType& SurfaceB,
		float Alpha);

public:
	static void BilinearInterpolation(
		FVoxelSurfaceTypeBlend& OutResult,
		TConstVoxelArrayView<FVoxelSurfaceTypeBlend> Blends,
		float AlphaX,
		float AlphaY);

	static void TrilinearInterpolation(
		FVoxelSurfaceTypeBlend& OutResult,
		TConstVoxelArrayView<FVoxelSurfaceTypeBlend> Blends,
		float AlphaX,
		float AlphaY,
		float AlphaZ);

private:
	void CheckImpl() const;

	FORCEINLINE void Check() const
	{
#if VOXEL_DEBUG
		CheckImpl();
#endif
	}

	friend class FVoxelSurfaceTypeBlendBuilder;
	friend struct FVoxelSurfaceTypeBlendUtilities;
};
checkStatic(sizeof(FVoxelSurfaceTypeBlend) == 64);
checkStatic(std::is_trivially_destructible_v<FVoxelSurfaceTypeBlend>);