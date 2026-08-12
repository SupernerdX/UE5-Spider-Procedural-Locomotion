// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSurfaceTypeBlend.h"
#include "Surface/VoxelSurfaceTypeBlendBuilder.h"
#include "VoxelSortNetwork.h"

FString FVoxelSurfaceTypeBlendLayer::GetSurfaceName() const
{
	return Type.GetName();
}

FString FVoxelSurfaceTypeBlendLayer::GetWeightString() const
{
	const float Percent = Weight.ToFloat() * 100.f;

	int32 NumFractionalDigits = 0;
	if (Percent < 1.f)
	{
		NumFractionalDigits = 2;
	}
	else if (Percent < 10.f)
	{
		NumFractionalDigits = 1;
	}

	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = NumFractionalDigits;
	Options.MaximumFractionalDigits = NumFractionalDigits;

	return FText::AsNumber(Percent, &Options).ToString() + "%";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSurfaceTypeBlendBase::SortByType()
{
	const auto Swap = [&](const int32 A, const int32 B)
	{
		if (Layers[A].Type > Layers[B].Type)
		{
			::Swap(Layers[A], Layers[B]);
		}
	};

	switch (NumLayers)
	{
	default: VOXEL_ASSUME(false);
	case 0: return;
	case 1: return;
	case 2: TVoxelSortNetwork<2>::Apply(Swap); return;
	case 3: TVoxelSortNetwork<3>::Apply(Swap); return;
	case 4: TVoxelSortNetwork<4>::Apply(Swap); return;
	case 5: TVoxelSortNetwork<5>::Apply(Swap); return;
	case 6: TVoxelSortNetwork<6>::Apply(Swap); return;
	case 7: TVoxelSortNetwork<7>::Apply(Swap); return;
	case 8: TVoxelSortNetwork<8>::Apply(Swap); return;
	case 9: TVoxelSortNetwork<9>::Apply(Swap); return;
	case 10: TVoxelSortNetwork<10>::Apply(Swap); return;
	case 11: TVoxelSortNetwork<11>::Apply(Swap); return;
	case 12: TVoxelSortNetwork<12>::Apply(Swap); return;
	case 13: TVoxelSortNetwork<13>::Apply(Swap); return;
	case 14: TVoxelSortNetwork<14>::Apply(Swap); return;
	case 15: TVoxelSortNetwork<15>::Apply(Swap); return;
	}
}

void FVoxelSurfaceTypeBlendBase::SortByWeight()
{
	const auto Swap = [&](const int32 A, const int32 B)
	{
		if (Layers[A].Weight < Layers[B].Weight)
		{
			::Swap(Layers[A], Layers[B]);
		}
	};

	switch (NumLayers)
	{
	default: VOXEL_ASSUME(false);
	case 0: return;
	case 1: return;
	case 2: TVoxelSortNetwork<2>::Apply(Swap); return;
	case 3: TVoxelSortNetwork<3>::Apply(Swap); return;
	case 4: TVoxelSortNetwork<4>::Apply(Swap); return;
	case 5: TVoxelSortNetwork<5>::Apply(Swap); return;
	case 6: TVoxelSortNetwork<6>::Apply(Swap); return;
	case 7: TVoxelSortNetwork<7>::Apply(Swap); return;
	case 8: TVoxelSortNetwork<8>::Apply(Swap); return;
	case 9: TVoxelSortNetwork<9>::Apply(Swap); return;
	case 10: TVoxelSortNetwork<10>::Apply(Swap); return;
	case 11: TVoxelSortNetwork<11>::Apply(Swap); return;
	case 12: TVoxelSortNetwork<12>::Apply(Swap); return;
	case 13: TVoxelSortNetwork<13>::Apply(Swap); return;
	case 14: TVoxelSortNetwork<14>::Apply(Swap); return;
	case 15: TVoxelSortNetwork<15>::Apply(Swap); return;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSurfaceTypeBlendLayer FVoxelSurfaceTypeBlend::GetTopLayer() const
{
	Check();

	if (NumLayers == 0)
	{
		return {};
	}

	FVoxelSurfaceTypeBlendLayer Result = Layers[0];
	for (int32 Index = 1; Index < NumLayers; Index++)
	{
		const FVoxelSurfaceTypeBlendLayer& Layer = Layers[Index];

		if (Layer.Weight > Result.Weight)
		{
			Result = Layer;
		}
	}
	return Result;
}

TVoxelArray<FVoxelSurfaceTypeBlendLayer> FVoxelSurfaceTypeBlend::GetLayersSortedByWeight() const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<FVoxelSurfaceTypeBlendLayer> SortedLayers = GetLayers().Array();
	SortedLayers.Sort([](const FVoxelSurfaceTypeBlendLayer& A, const FVoxelSurfaceTypeBlendLayer& B)
	{
		return A.Weight > B.Weight;
	});
	return SortedLayers;
}

bool FVoxelSurfaceTypeBlend::Equals_Slow(
	const FVoxelSurfaceTypeBlend& Other,
	const float Tolerance) const
{
	Check();
	Other.Check();

	TVoxelInlineSet<FVoxelSurfaceType, 16> AllTypes;
	TVoxelInlineMap<FVoxelSurfaceType, float, 16> TypeToWeight;
	TVoxelInlineMap<FVoxelSurfaceType, float, 16> OtherTypeToWeight;

	for (const FVoxelSurfaceTypeBlendLayer& Layer : GetLayers())
	{
		AllTypes.Add(Layer.Type);
		TypeToWeight.Add_EnsureNew(Layer.Type, Layer.Weight.ToFloat());
	}
	for (const FVoxelSurfaceTypeBlendLayer& Layer : Other.GetLayers())
	{
		AllTypes.Add(Layer.Type);
		OtherTypeToWeight.Add_EnsureNew(Layer.Type, Layer.Weight.ToFloat());
	}

	for (const FVoxelSurfaceType& Type : AllTypes)
	{
		const float Weight = TypeToWeight.FindRef(Type);
		const float OtherWeight = OtherTypeToWeight.FindRef(Type);

		if (!FMath::IsNearlyEqual(Weight, OtherWeight, Tolerance))
		{
			return false;
		}
	}

	return true;
}

void FVoxelSurfaceTypeBlend::PopLayersForRendering()
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(NumLayers > 8);

	while (NumLayers > 8)
	{
		FVoxelSurfaceTypeBlendWeight LowestWeight = Layers[0].Weight;
		int32 LowestIndex = 0;

		for (int32 Index = 1; Index < NumLayers; Index++)
		{
			const FVoxelSurfaceTypeBlendLayer& Layer = Layers[Index];
			if (Layer.Weight > LowestWeight)
			{
				continue;
			}

			LowestWeight = Layer.Weight;
			LowestIndex = Index;
		}

		checkVoxelSlow(!LowestWeight.IsZero());

		// Move data
		for (int32 Index = LowestIndex + 1; Index < NumLayers; Index++)
		{
			Layers[Index - 1] = Layers[Index];
		}

		NumLayers--;
	}

	checkVoxelSlow(NumLayers == 8);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSurfaceTypeBlend::Lerp(
	FVoxelSurfaceTypeBlend& OutResult,
	const FVoxelSurfaceTypeBlend& BlendA,
	const FVoxelSurfaceTypeBlend& BlendB,
	const float Alpha)
{
	BlendA.Check();
	BlendB.Check();
	ensureVoxelSlow(0 <= Alpha && Alpha <= 1);

	ON_SCOPE_EXIT
	{
		OutResult.Check();
	};

	if (Alpha == 0)
	{
		OutResult = BlendA;
		return;
	}
	if (Alpha == 1)
	{
		OutResult = BlendB;
		return;
	}

	if (BlendA.IsNull())
	{
		OutResult = BlendB;
		return;
	}
	if (BlendB.IsNull())
	{
		OutResult = BlendA;
		return;
	}

	if (BlendA == BlendB)
	{
		OutResult = BlendA;
		return;
	}

	FVoxelSurfaceTypeBlendBuilder Builder;

	const float AlphaA = 1.f - Alpha;
	const float AlphaB = Alpha;

	int32 IndexA = 0;
	int32 IndexB = 0;
	while (
		IndexA < BlendA.NumLayers &&
		IndexB < BlendB.NumLayers)
	{
		const FVoxelSurfaceTypeBlendLayer& LayerA = BlendA.Layers[IndexA];
		const FVoxelSurfaceTypeBlendLayer& LayerB = BlendB.Layers[IndexB];

		if (LayerA.Type < LayerB.Type)
		{
			Builder.AddLayer_CheckNew
			(
				LayerA.Type,
				LayerA.Weight.ToInt32() * AlphaA
			);

			IndexA++;
		}
		else if (LayerB.Type < LayerA.Type)
		{
			Builder.AddLayer_CheckNew
			(
				LayerB.Type,
				LayerB.Weight.ToInt32() * AlphaB
			);

			IndexB++;
		}
		else
		{
			checkVoxelSlow(LayerA.Type == LayerB.Type);

			Builder.AddLayer_CheckNew
			(
				LayerA.Type,
				LayerA.Weight.ToInt32() * AlphaA + LayerB.Weight.ToInt32() * AlphaB
			);

			IndexA++;
			IndexB++;
		}
	}

	while (IndexA < BlendA.NumLayers)
	{
		const FVoxelSurfaceTypeBlendLayer& LayerA = BlendA.Layers[IndexA];

		Builder.AddLayer_CheckNew
		(
			LayerA.Type,
			LayerA.Weight.ToInt32() * AlphaA
		);

		IndexA++;
	}

	while (IndexB < BlendB.NumLayers)
	{
		const FVoxelSurfaceTypeBlendLayer& LayerB = BlendB.Layers[IndexB];

		Builder.AddLayer_CheckNew
		(
			LayerB.Type,
			LayerB.Weight.ToInt32() * AlphaB
		);

		IndexB++;
	}

	Builder.Build(OutResult, true);
}

void FVoxelSurfaceTypeBlend::Lerp(
	FVoxelSurfaceTypeBlend& OutResult,
	const FVoxelSurfaceTypeBlend& BlendA,
	const FVoxelSurfaceType& SurfaceB,
	const float Alpha)
{
	BlendA.Check();
	ensureVoxelSlow(0 <= Alpha && Alpha <= 1);

#if VOXEL_DEBUG
	const FVoxelSurfaceTypeBlend BlendACopy = BlendA;
	ON_SCOPE_EXIT
	{
		OutResult.Check();

		FVoxelSurfaceTypeBlend ExpectedResult;
		Lerp(ExpectedResult, BlendACopy, FromType(SurfaceB), Alpha);
		ensure(OutResult.Equals_Slow(ExpectedResult, 0.01f));
	};
#endif

	if (Alpha == 0)
	{
		OutResult = BlendA;
		return;
	}
	if (Alpha == 1)
	{
		OutResult.InitializeFromType(SurfaceB);
		return;
	}

	if (BlendA.IsNull())
	{
		OutResult.InitializeFromType(SurfaceB);
		return;
	}
	if (SurfaceB.IsNull())
	{
		OutResult = BlendA;
		return;
	}

	FVoxelSurfaceTypeBlendBuilder Builder;

	const float AlphaA = 1.f - Alpha;
	const float AlphaB = Alpha;

	bool bAdded = false;
	for (const FVoxelSurfaceTypeBlendLayer& Layer : BlendA.GetLayers())
	{
		float Weight = Layer.Weight.ToFloat();
		Weight *= AlphaA;

		if (SurfaceB < Layer.Type &&
			!bAdded)
		{
			bAdded = true;
			Builder.AddLayer_CheckNew(SurfaceB, AlphaB);
		}

		if (Layer.Type == SurfaceB)
		{
			checkVoxelSlow(!bAdded);
			bAdded = true;

			Weight += AlphaB;
		}

		Builder.AddLayer_CheckNew(Layer.Type, Weight);
	}

	if (!bAdded)
	{
		Builder.AddLayer_CheckNew(SurfaceB, AlphaB);
	}

	Builder.Build(OutResult, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSurfaceTypeBlend::BilinearInterpolation(
	FVoxelSurfaceTypeBlend& OutResult,
	const TConstVoxelArrayView<FVoxelSurfaceTypeBlend> Blends,
	const float AlphaX,
	const float AlphaY)
{
	checkVoxelSlow(Blends.Num() == 4);

	for (const FVoxelSurfaceTypeBlend& Blend : Blends)
	{
		Blend.Check();
	}

	ensureVoxelSlow(0 <= AlphaX && AlphaX <= 1);
	ensureVoxelSlow(0 <= AlphaY && AlphaY <= 1);

	ON_SCOPE_EXIT
	{
		OutResult.Check();

#if VOXEL_DEBUG && 0
		FVoxelSurfaceTypeBlend AB;
		FVoxelSurfaceTypeBlend CD;
		Lerp(AB, Blends[0], Blends[1], AlphaX);
		Lerp(CD, Blends[1], Blends[2], AlphaX);

		FVoxelSurfaceTypeBlend ExpectedResult;
		Lerp(ExpectedResult, AB, CD, AlphaY);
		ensure(OutResult.Equals(ExpectedResult, 0.01f));
#endif
	};

	if (AlphaX == 0)
	{
		Lerp(OutResult, Blends[0], Blends[2], AlphaY);
		return;
	}
	if (AlphaX == 1)
	{
		Lerp(OutResult, Blends[1], Blends[3], AlphaY);
		return;
	}

	if (AlphaY == 0)
	{
		Lerp(OutResult, Blends[0], Blends[1], AlphaX);
		return;
	}
	if (AlphaY == 1)
	{
		Lerp(OutResult, Blends[2], Blends[3], AlphaX);
		return;
	}

	TVoxelStaticArray<double, 4> Alphas{ NoInit };
	Alphas[0] = double(1.f - AlphaY) * double(1.f - AlphaX);
	Alphas[1] = double(1.f - AlphaY) * double(AlphaX);
	Alphas[2] = double(AlphaY) * double(1.f - AlphaX);
	Alphas[3] = double(AlphaY) * double(AlphaX);

	FVoxelSurfaceTypeBlendBuilder Builder;

	TVoxelStaticArray<int32, 4> LayerIndices{ ForceInit };
	while (true)
	{
		FVoxelSurfaceType BestLayerType = ReinterpretCastRef<FVoxelSurfaceType>(MAX_uint16);
		double BestLayerWeight = 0.;

		for (int32 Index = 0; Index < 4; Index++)
		{
			const FVoxelSurfaceTypeBlend& Blend = Blends[Index];
			const int32 LayerIndex = LayerIndices[Index];

			if (LayerIndex >= Blend.NumLayers)
			{
				continue;
			}

			const FVoxelSurfaceTypeBlendLayer Layer = Blend.Layers[LayerIndex];

			if (Layer.Type < BestLayerType)
			{
				BestLayerType = Layer.Type;
				BestLayerWeight = Layer.Weight.ToInt32() * Alphas[Index];
			}
			else if (Layer.Type == BestLayerType)
			{
				BestLayerWeight += Layer.Weight.ToInt32() * Alphas[Index];
			}
		}

		if (ReinterpretCastRef<uint16>(BestLayerType) == MAX_uint16)
		{
			break;
		}

		for (int32 Index = 0; Index < 4; Index++)
		{
			const FVoxelSurfaceTypeBlend& Blend = Blends[Index];
			int32& LayerIndex = LayerIndices[Index];

			if (LayerIndex >= Blend.NumLayers)
			{
				continue;
			}

			if (Blend.Layers[LayerIndex].Type == BestLayerType)
			{
				LayerIndex++;
			}
		}

		Builder.AddLayer_CheckNew(BestLayerType, BestLayerWeight);
	}

	Builder.Build(OutResult);
}

void FVoxelSurfaceTypeBlend::TrilinearInterpolation(
	FVoxelSurfaceTypeBlend& OutResult,
	const TConstVoxelArrayView<FVoxelSurfaceTypeBlend> Blends,
	const float AlphaX,
	const float AlphaY,
	const float AlphaZ)
{
	checkVoxelSlow(Blends.Num() == 8);

	for (const FVoxelSurfaceTypeBlend& Blend : Blends)
	{
		Blend.Check();
	}

	ensureVoxelSlow(0 <= AlphaX && AlphaX <= 1);
	ensureVoxelSlow(0 <= AlphaY && AlphaY <= 1);
	ensureVoxelSlow(0 <= AlphaZ && AlphaZ <= 1);

	ON_SCOPE_EXIT
	{
		OutResult.Check();

#if VOXEL_DEBUG && 0
		FVoxelSurfaceTypeBlend ABCD;
		FVoxelSurfaceTypeBlend EFGH;
		BilinearInterpolation(ABCD, Blends.LeftOf(4), AlphaX, AlphaY);
		BilinearInterpolation(EFGH, Blends.RightOf(4), AlphaX, AlphaY);

		FVoxelSurfaceTypeBlend ExpectedResult;
		Lerp(ExpectedResult, ABCD, EFGH, AlphaZ);
		ensure(OutResult.Equals(ExpectedResult, 0.01f));
#endif
	};

	if (AlphaZ == 0)
	{
		BilinearInterpolation(OutResult, Blends.LeftOf(4), AlphaX, AlphaY);
		return;
	}
	if (AlphaZ == 1)
	{
		BilinearInterpolation(OutResult, Blends.RightOf(4), AlphaX, AlphaY);
		return;
	}

	TVoxelStaticArray<double, 8> Alphas{ NoInit };
	Alphas[0] = double(1.f - AlphaZ) * double(1.f - AlphaY) * double(1.f - AlphaX);
	Alphas[1] = double(1.f - AlphaZ) * double(1.f - AlphaY) * double(AlphaX);
	Alphas[2] = double(1.f - AlphaZ) * double(AlphaY) * double(1.f - AlphaX);
	Alphas[3] = double(1.f - AlphaZ) * double(AlphaY) * double(AlphaX);
	Alphas[4] = double(AlphaZ) * double(1.f - AlphaY) * double(1.f - AlphaX);
	Alphas[5] = double(AlphaZ) * double(1.f - AlphaY) * double(AlphaX);
	Alphas[6] = double(AlphaZ) * double(AlphaY) * double(1.f - AlphaX);
	Alphas[7] = double(AlphaZ) * double(AlphaY) * double(AlphaX);

	FVoxelSurfaceTypeBlendBuilder Builder;

	TVoxelStaticArray<int32, 8> LayerIndices{ ForceInit };
	while (true)
	{
		FVoxelSurfaceType BestLayerType = ReinterpretCastRef<FVoxelSurfaceType>(MAX_uint16);
		double BestLayerWeight = 0.;

		for (int32 Index = 0; Index < 8; Index++)
		{
			const FVoxelSurfaceTypeBlend& Blend = Blends[Index];
			const int32 LayerIndex = LayerIndices[Index];

			if (LayerIndex >= Blend.NumLayers)
			{
				continue;
			}

			const FVoxelSurfaceTypeBlendLayer Layer = Blend.Layers[LayerIndex];

			if (Layer.Type < BestLayerType)
			{
				BestLayerType = Layer.Type;
				BestLayerWeight = Layer.Weight.ToInt32() * Alphas[Index];
			}
			else if (Layer.Type == BestLayerType)
			{
				BestLayerWeight += Layer.Weight.ToInt32() * Alphas[Index];
			}
		}

		if (ReinterpretCastRef<uint16>(BestLayerType) == MAX_uint16)
		{
			break;
		}

		for (int32 Index = 0; Index < 8; Index++)
		{
			const FVoxelSurfaceTypeBlend& Blend = Blends[Index];
			int32& LayerIndex = LayerIndices[Index];

			if (LayerIndex >= Blend.NumLayers)
			{
				continue;
			}

			if (Blend.Layers[LayerIndex].Type == BestLayerType)
			{
				LayerIndex++;
			}
		}

		Builder.AddLayer_CheckNew(BestLayerType, BestLayerWeight);
	}

	Builder.Build(OutResult);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSurfaceTypeBlend::CheckImpl() const
{
	check(0 <= NumLayers && NumLayers <= MaxLayers);

	if (NumLayers == 0)
	{
		return;
	}

	int32 WeightSum = 0;
	for (int32 Index = 0; Index < NumLayers; Index++)
	{
		const FVoxelSurfaceTypeBlendLayer& Layer = Layers[Index];
		ensure(!Layer.Type.IsNull());
		ensure(!Layer.Weight.IsZero());

		WeightSum += Layer.Weight.ToInt32();

		if (Index != 0)
		{
			ensure(Layers[Index - 1].Type < Layer.Type);
		}
	}
	ensure(WeightSum == MAX_uint16);
}