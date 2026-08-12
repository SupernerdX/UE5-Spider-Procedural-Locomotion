// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightBlendMode.h"
#include "Surface/VoxelSurfaceTypeBlend.h"
#include "Sculpt/Height/VoxelHeightHeightChunk.h"
#include "Sculpt/Height/VoxelHeightMetadataChunk.h"
#include "Sculpt/Height/VoxelHeightSurfaceTypeChunk.h"
#include "Sculpt/Height/VoxelHeightChunkTreeIterator.h"

struct FVoxelHeightChunkSamplers
{
public:
	FORCEINLINE static float BlendHeights(
		const float OldHeight,
		const float NewHeight,
		const EVoxelHeightBlendMode BlendMode,
		const bool bApplyOnVoid)
	{
		if (FVoxelUtilities::IsNaN(NewHeight))
		{
			return OldHeight;
		}

		switch (BlendMode)
		{
		default: VOXEL_ASSUME(false);
		case EVoxelHeightBlendMode::Min:
		{
			if (FVoxelUtilities::IsNaN(OldHeight))
			{
				if (bApplyOnVoid)
				{
					return NewHeight;
				}
				else
				{
					return OldHeight;
				}
			}

			return FMath::Min(OldHeight, NewHeight);
		}
		case EVoxelHeightBlendMode::Max:
		{
			if (FVoxelUtilities::IsNaN(OldHeight))
			{
				if (bApplyOnVoid)
				{
					return NewHeight;
				}
				else
				{
					return OldHeight;
				}
			}

			return FMath::Max(OldHeight, NewHeight);
		}
		case EVoxelHeightBlendMode::Override:
		{
			return NewHeight;
		}
		}
	}

public:
	template<typename AbsoluteHeightsLambdaType, typename RelativeHeightsLambdaType, typename SkipLambdaType>
	static void ProcessHeights(
		const TVoxelHeightChunkTreeIterator<const FVoxelHeightHeightChunk>& Iterator,
		AbsoluteHeightsLambdaType AbsoluteHeightsLambda,
		RelativeHeightsLambdaType RelativeHeightsLambda,
		SkipLambdaType SkipLambda)
	{
		VOXEL_FUNCTION_COUNTER();

		Iterator.Iterate(
			[&](const int32 Index,
				const FVoxelHeightHeightChunk* Chunk0, const int32 Index0)
			{
				if (!Chunk0)
				{
					SkipLambda(Index);
					return;
				}

				if (Chunk0->bIsRelative)
				{
					RelativeHeightsLambda(
						Index,
						Chunk0->GetHeight(Index0));

					return;
				}

				AbsoluteHeightsLambda(
					Index,
					Chunk0->GetHeight(Index0));
			},
			[&](const int32 Index,
				const FVoxelHeightHeightChunk* Chunk0, const int32 Index0,
				const FVoxelHeightHeightChunk* Chunk1, const int32 Index1,
				const float AlphaX)
			{
				if (!Chunk0 ||
					!Chunk1)
				{
					SkipLambda(Index);
					return;
				}

				if (Chunk0->bIsRelative &&
					Chunk1->bIsRelative)
				{
					RelativeHeightsLambda(
						Index,
						FMath::Lerp(
							Chunk0->GetHeight(Index0),
							Chunk1->GetHeight(Index1),
							AlphaX));

					return;
				}

				AbsoluteHeightsLambda(
					Index,
					FMath::Lerp(
						Chunk0->GetHeight(Index0),
						Chunk1->GetHeight(Index1),
						AlphaX));
			},
			[&](const int32 Index,
				const FVoxelHeightHeightChunk* Chunk0, const int32 Index0,
				const FVoxelHeightHeightChunk* Chunk1, const int32 Index1,
				const FVoxelHeightHeightChunk* Chunk2, const int32 Index2,
				const FVoxelHeightHeightChunk* Chunk3, const int32 Index3,
				const float AlphaX,
				const float AlphaY)
			{
				if (!Chunk0 ||
					!Chunk1 ||
					!Chunk2 ||
					!Chunk3)
				{
					SkipLambda(Index);
					return;
				}

				if (Chunk0->bIsRelative &&
					Chunk1->bIsRelative &&
					Chunk2->bIsRelative &&
					Chunk3->bIsRelative)
				{
					RelativeHeightsLambda(
						Index,
						FVoxelUtilities::BilinearInterpolation(
							Chunk0->GetHeight(Index0),
							Chunk1->GetHeight(Index1),
							Chunk2->GetHeight(Index2),
							Chunk3->GetHeight(Index3),
							AlphaX,
							AlphaY));

					return;
				}

				AbsoluteHeightsLambda(
					Index,
					FVoxelUtilities::BilinearInterpolation(
						Chunk0->GetHeight(Index0),
						Chunk1->GetHeight(Index1),
						Chunk2->GetHeight(Index2),
						Chunk3->GetHeight(Index3),
						AlphaX,
						AlphaY));
			});
	}

public:
	template<typename LambdaType>
	static void ProcessSurfaceTypes(
		const TVoxelHeightChunkTreeIterator<const FVoxelHeightSurfaceTypeChunk>& Iterator,
		LambdaType Lambda)
	{
		VOXEL_FUNCTION_COUNTER();

#define Load(InIndex) \
	if (Chunk ## InIndex) \
	{ \
		Chunk ## InIndex->GetSurfaceType(Index ## InIndex, Alphas[InIndex], SurfaceTypes[InIndex]); \
	} \
	else \
	{ \
		Alphas[InIndex] = 0.f; \
		SurfaceTypes[InIndex].InitializeNull(); \
	}

		Iterator.Iterate(
			[&](const int32 Index,
				const FVoxelHeightSurfaceTypeChunk* Chunk0, const int32 Index0)
			{
				float Alpha0 = 0.f;
				FVoxelSurfaceTypeBlend SurfaceType0;

				if (Chunk0)
				{
					Chunk0->GetSurfaceType(Index0, Alpha0, SurfaceType0);
				}

				Lambda(
					Index,
					Alpha0,
					SurfaceType0);
			},
			[&](const int32 Index,
				const FVoxelHeightSurfaceTypeChunk* Chunk0, const int32 Index0,
				const FVoxelHeightSurfaceTypeChunk* Chunk1, const int32 Index1,
				const float AlphaX)
			{
				TVoxelStaticArray<float, 2> Alphas{ NoInit };
				TVoxelStaticArray<FVoxelSurfaceTypeBlend, 2> SurfaceTypes{ NoInit };

				Load(0);
				Load(1);

				FVoxelSurfaceTypeBlend SurfaceType;
				FVoxelSurfaceTypeBlend::Lerp(
					SurfaceType,
					SurfaceTypes[0],
					SurfaceTypes[1],
					AlphaX);

				Lambda(
					Index,
					FMath::Lerp(
						Alphas[0],
						Alphas[1],
						AlphaX),
					SurfaceType);
			},
			[&](const int32 Index,
				const FVoxelHeightSurfaceTypeChunk* Chunk0, const int32 Index0,
				const FVoxelHeightSurfaceTypeChunk* Chunk1, const int32 Index1,
				const FVoxelHeightSurfaceTypeChunk* Chunk2, const int32 Index2,
				const FVoxelHeightSurfaceTypeChunk* Chunk3, const int32 Index3,
				const float AlphaX,
				const float AlphaY)
			{
				TVoxelStaticArray<float, 4> Alphas{ NoInit };
				TVoxelStaticArray<FVoxelSurfaceTypeBlend, 4> SurfaceTypes{ NoInit };

				Load(0);
				Load(1);
				Load(2);
				Load(3);

				FVoxelSurfaceTypeBlend SurfaceType;
				FVoxelSurfaceTypeBlend::BilinearInterpolation(
					SurfaceType,
					SurfaceTypes,
					AlphaX,
					AlphaY);

				Lambda(
					Index,
					FVoxelUtilities::BilinearInterpolation(
						Alphas[0],
						Alphas[1],
						Alphas[2],
						Alphas[3],
						AlphaX,
						AlphaY),
					SurfaceType);
			});

#undef Load
	}

public:
	template<typename InnerType, typename LambdaType>
	static void ProcessMetadata(
		const TVoxelHeightChunkTreeIterator<const FVoxelHeightMetadataChunk>& Iterator,
		LambdaType Lambda)
	{
		VOXEL_FUNCTION_COUNTER();

		Iterator.Iterate(
			[&](const int32 Index,
				const FVoxelHeightMetadataChunk* Chunk0, const int32 Index0)
			{
				if (!Chunk0)
				{
					return;
				}

				Lambda(
					Index,
					Chunk0->GetAlpha(Index0),
					Chunk0->GetValue<InnerType>(Index0));
			},
			[&](const int32 Index,
				const FVoxelHeightMetadataChunk* Chunk0, const int32 Index0,
				const FVoxelHeightMetadataChunk* Chunk1, const int32 Index1,
				const float AlphaX)
			{
				if (!Chunk0 ||
					!Chunk1)
				{
					return;
				}

				Lambda(
					Index,
					FMath::Lerp(
						Chunk0->GetAlpha(Index0),
						Chunk1->GetAlpha(Index1),
						AlphaX),
					FMath::Lerp(
						Chunk0->GetValue<InnerType>(Index0),
						Chunk1->GetValue<InnerType>(Index1),
						AlphaX));
			},
			[&](const int32 Index,
				const FVoxelHeightMetadataChunk* Chunk0, const int32 Index0,
				const FVoxelHeightMetadataChunk* Chunk1, const int32 Index1,
				const FVoxelHeightMetadataChunk* Chunk2, const int32 Index2,
				const FVoxelHeightMetadataChunk* Chunk3, const int32 Index3,
				const float AlphaX,
				const float AlphaY)
			{
				if (!Chunk0 ||
					!Chunk1 ||
					!Chunk2 ||
					!Chunk3)
				{
					return;
				}

				Lambda(
					Index,
					FVoxelUtilities::BilinearInterpolation(
						Chunk0->GetAlpha(Index0),
						Chunk1->GetAlpha(Index1),
						Chunk2->GetAlpha(Index2),
						Chunk3->GetAlpha(Index3),
						AlphaX,
						AlphaY),
					FVoxelUtilities::BilinearInterpolation(
						Chunk0->GetValue<InnerType>(Index0),
						Chunk1->GetValue<InnerType>(Index1),
						Chunk2->GetValue<InnerType>(Index2),
						Chunk3->GetValue<InnerType>(Index3),
						AlphaX,
						AlphaY));
			});
	}
};
