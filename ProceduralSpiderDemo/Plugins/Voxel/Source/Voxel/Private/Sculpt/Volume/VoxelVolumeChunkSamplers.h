// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeBlendMode.h"
#include "Sculpt/Volume/VoxelVolumeChunkData.h"
#include "Sculpt/Volume/VoxelVolumeDistanceChunk.h"
#include "Sculpt/Volume/VoxelVolumeChunkTreeIterator.h"

struct FVoxelVolumeChunkSamplers
{
public:
	FORCEINLINE static float BlendDistances(
		const float OldDistance,
		const float NewDistance,
		const EVoxelVolumeBlendMode BlendMode,
		const bool bApplyOnVoid)
	{
		checkVoxelSlow(!FVoxelUtilities::IsNaN(NewDistance));

		switch (BlendMode)
		{
		default: VOXEL_ASSUME(false);
		case EVoxelVolumeBlendMode::Additive:
		{
			if (FVoxelUtilities::IsNaN(NewDistance))
			{
				return OldDistance;
			}
			if (FVoxelUtilities::IsNaN(OldDistance))
			{
				if (bApplyOnVoid)
				{
					return NewDistance;
				}
				else
				{
					return OldDistance;
				}
			}

			return FMath::Min(OldDistance, NewDistance);
		}
		case EVoxelVolumeBlendMode::Subtractive:
		{
			if (FVoxelUtilities::IsNaN(OldDistance))
			{
				if (bApplyOnVoid)
				{
					return NewDistance;
				}
				else
				{
					return OldDistance;
				}
			}

			return FMath::Max(OldDistance, NewDistance);
		}
		case EVoxelVolumeBlendMode::Intersect:
		{
			if (FVoxelUtilities::IsNaN(OldDistance))
			{
				return OldDistance;
			}

			return FMath::Max(OldDistance, NewDistance);
		}
		case EVoxelVolumeBlendMode::Override:
		{
			return NewDistance;
		}
		}
	}

	FORCEINLINE static float BlendDistances(
		const float OldDistance,
		const float AdditiveDistance,
		const float SubtractiveDistance,
		const EVoxelVolumeBlendMode BlendMode,
		const bool bApplyOnVoid)
	{
		switch (BlendMode)
		{
		default: VOXEL_ASSUME(false);
		case EVoxelVolumeBlendMode::Additive:
		{
			if (FVoxelUtilities::IsNaN(AdditiveDistance))
			{
				return OldDistance;
			}

			if (FVoxelUtilities::IsNaN(OldDistance))
			{
				if (bApplyOnVoid)
				{
					return AdditiveDistance;
				}
				else
				{
					return OldDistance;
				}
			}

			return FMath::Min(OldDistance, AdditiveDistance);
		}
		case EVoxelVolumeBlendMode::Subtractive:
		{
			if (FVoxelUtilities::IsNaN(SubtractiveDistance))
			{
				return OldDistance;
			}

			if (FVoxelUtilities::IsNaN(OldDistance))
			{
				if (bApplyOnVoid)
				{
					return SubtractiveDistance;
				}
				else
				{
					return OldDistance;
				}
			}

			return FMath::Max(OldDistance, SubtractiveDistance);
		}
		case EVoxelVolumeBlendMode::Intersect:
		{
			if (FVoxelUtilities::IsNaN(OldDistance))
			{
				return OldDistance;
			}
			if (FVoxelUtilities::IsNaN(AdditiveDistance))
			{
				return AdditiveDistance;
			}

			return FMath::Max(OldDistance, AdditiveDistance);
		}
		case EVoxelVolumeBlendMode::Override:
		{
			float Distance = OldDistance;
			if (!FVoxelUtilities::IsNaN(AdditiveDistance))
			{
				if (FVoxelUtilities::IsNaN(Distance))
				{
					Distance = AdditiveDistance;
				}
				else
				{
					Distance = FMath::Min(Distance, AdditiveDistance);
				}
			}
			if (!FVoxelUtilities::IsNaN(SubtractiveDistance))
			{
				if (FVoxelUtilities::IsNaN(Distance))
				{
					Distance = SubtractiveDistance;
				}
				else
				{
					Distance = FMath::Max(Distance, SubtractiveDistance);
				}
			}
			return Distance;
		}
		}
	}

public:
	template<typename PackedLambdaType, typename MovableLambdaType, typename SkipLambdaType>
	static void ProcessDistances(
		const TVoxelVolumeChunkTreeIterator<const FVoxelVolumeDistanceChunk>& Iterator,
		PackedLambdaType PackedLambda,
		MovableLambdaType MovableLambda,
		SkipLambdaType SkipLambda)
	{
		VOXEL_FUNCTION_COUNTER();

		Iterator.Iterate(
			[&](const int32 Index,
				const FVoxelVolumeDistanceChunk* Chunk0, const int32 Index0)
			{
				if (!Chunk0)
				{
					SkipLambda(Index);
					return;
				}

				if (Chunk0->bIsPacked)
				{
					PackedLambda(
						Index,
						Chunk0->GetPackedDistance(Index0));

					return;
				}

				MovableLambda(
					Index,
					Chunk0->GetMovableDistancesWithFallback(Index0));
			},
			[&](const int32 Index,
				const FVoxelVolumeDistanceChunk* Chunk0, const int32 Index0,
				const FVoxelVolumeDistanceChunk* Chunk1, const int32 Index1,
				const float AlphaX)
			{
				if (!Chunk0 ||
					!Chunk1)
				{
					SkipLambda(Index);
					return;
				}

				if (Chunk0->bIsPacked &&
					Chunk1->bIsPacked)
				{
					PackedLambda(
						Index,
						FMath::Lerp(
							Chunk0->GetPackedDistance(Index0),
							Chunk1->GetPackedDistance(Index1),
							AlphaX));

					return;
				}

				MovableLambda(
					Index,
					FMath::Lerp(
						Chunk0->GetMovableDistancesWithFallback(Index0),
						Chunk1->GetMovableDistancesWithFallback(Index1),
						AlphaX));
			},
			[&](const int32 Index,
				const FVoxelVolumeDistanceChunk* Chunk0, const int32 Index0,
				const FVoxelVolumeDistanceChunk* Chunk1, const int32 Index1,
				const FVoxelVolumeDistanceChunk* Chunk2, const int32 Index2,
				const FVoxelVolumeDistanceChunk* Chunk3, const int32 Index3,
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

				if (Chunk0->bIsPacked &&
					Chunk1->bIsPacked &&
					Chunk2->bIsPacked &&
					Chunk3->bIsPacked)
				{
					PackedLambda(
						Index,
						FVoxelUtilities::BilinearInterpolation(
							Chunk0->GetPackedDistance(Index0),
							Chunk1->GetPackedDistance(Index1),
							Chunk2->GetPackedDistance(Index2),
							Chunk3->GetPackedDistance(Index3),
							AlphaX,
							AlphaY));

					return;
				}

				MovableLambda(
					Index,
					FVoxelUtilities::BilinearInterpolation(
						Chunk0->GetMovableDistancesWithFallback(Index0),
						Chunk1->GetMovableDistancesWithFallback(Index1),
						Chunk2->GetMovableDistancesWithFallback(Index2),
						Chunk3->GetMovableDistancesWithFallback(Index3),
						AlphaX,
						AlphaY));
			},
			[&](const int32 Index,
				const FVoxelVolumeDistanceChunk* Chunk0, const int32 Index0,
				const FVoxelVolumeDistanceChunk* Chunk1, const int32 Index1,
				const FVoxelVolumeDistanceChunk* Chunk2, const int32 Index2,
				const FVoxelVolumeDistanceChunk* Chunk3, const int32 Index3,
				const FVoxelVolumeDistanceChunk* Chunk4, const int32 Index4,
				const FVoxelVolumeDistanceChunk* Chunk5, const int32 Index5,
				const FVoxelVolumeDistanceChunk* Chunk6, const int32 Index6,
				const FVoxelVolumeDistanceChunk* Chunk7, const int32 Index7,
				const float AlphaX,
				const float AlphaY,
				const float AlphaZ)
			{
				if (!Chunk0 ||
					!Chunk1 ||
					!Chunk2 ||
					!Chunk3 ||
					!Chunk4 ||
					!Chunk5 ||
					!Chunk6 ||
					!Chunk7)
				{
					SkipLambda(Index);
					return;
				}

				if (Chunk0->bIsPacked &&
					Chunk1->bIsPacked &&
					Chunk2->bIsPacked &&
					Chunk3->bIsPacked &&
					Chunk4->bIsPacked &&
					Chunk5->bIsPacked &&
					Chunk6->bIsPacked &&
					Chunk7->bIsPacked)
				{
					PackedLambda(
						Index,
						FVoxelUtilities::TrilinearInterpolation(
							Chunk0->GetPackedDistance(Index0),
							Chunk1->GetPackedDistance(Index1),
							Chunk2->GetPackedDistance(Index2),
							Chunk3->GetPackedDistance(Index3),
							Chunk4->GetPackedDistance(Index4),
							Chunk5->GetPackedDistance(Index5),
							Chunk6->GetPackedDistance(Index6),
							Chunk7->GetPackedDistance(Index7),
							AlphaX,
							AlphaY,
							AlphaZ));

					return;
				}

				MovableLambda(
					Index,
					FVoxelUtilities::TrilinearInterpolation(
						Chunk0->GetMovableDistancesWithFallback(Index0),
						Chunk1->GetMovableDistancesWithFallback(Index1),
						Chunk2->GetMovableDistancesWithFallback(Index2),
						Chunk3->GetMovableDistancesWithFallback(Index3),
						Chunk4->GetMovableDistancesWithFallback(Index4),
						Chunk5->GetMovableDistancesWithFallback(Index5),
						Chunk6->GetMovableDistancesWithFallback(Index6),
						Chunk7->GetMovableDistancesWithFallback(Index7),
						AlphaX,
						AlphaY,
						AlphaZ));
			});
	}

public:
	template<typename LambdaType>
	static void ProcessSurfaceTypes(
		const TVoxelVolumeChunkTreeIterator<const FVoxelVolumeSurfaceTypeChunk>& Iterator,
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
				const FVoxelVolumeSurfaceTypeChunk* Chunk0, const int32 Index0)
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
				const FVoxelVolumeSurfaceTypeChunk* Chunk0, const int32 Index0,
				const FVoxelVolumeSurfaceTypeChunk* Chunk1, const int32 Index1,
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
				const FVoxelVolumeSurfaceTypeChunk* Chunk0, const int32 Index0,
				const FVoxelVolumeSurfaceTypeChunk* Chunk1, const int32 Index1,
				const FVoxelVolumeSurfaceTypeChunk* Chunk2, const int32 Index2,
				const FVoxelVolumeSurfaceTypeChunk* Chunk3, const int32 Index3,
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
			},
			[&](const int32 Index,
				const FVoxelVolumeSurfaceTypeChunk* Chunk0, const int32 Index0,
				const FVoxelVolumeSurfaceTypeChunk* Chunk1, const int32 Index1,
				const FVoxelVolumeSurfaceTypeChunk* Chunk2, const int32 Index2,
				const FVoxelVolumeSurfaceTypeChunk* Chunk3, const int32 Index3,
				const FVoxelVolumeSurfaceTypeChunk* Chunk4, const int32 Index4,
				const FVoxelVolumeSurfaceTypeChunk* Chunk5, const int32 Index5,
				const FVoxelVolumeSurfaceTypeChunk* Chunk6, const int32 Index6,
				const FVoxelVolumeSurfaceTypeChunk* Chunk7, const int32 Index7,
				const float AlphaX,
				const float AlphaY,
				const float AlphaZ)
			{
				TVoxelStaticArray<float, 8> Alphas{ NoInit };
				TVoxelStaticArray<FVoxelSurfaceTypeBlend, 8> SurfaceTypes{ NoInit };

				Load(0);
				Load(1);
				Load(2)
				Load(3);
				Load(4);
				Load(5);
				Load(6);
				Load(7);

				FVoxelSurfaceTypeBlend SurfaceType;
				FVoxelSurfaceTypeBlend::TrilinearInterpolation(
					SurfaceType,
					SurfaceTypes,
					AlphaX,
					AlphaY,
					AlphaZ);

				Lambda(
					Index,
					FVoxelUtilities::TrilinearInterpolation(
						Alphas[0],
						Alphas[1],
						Alphas[2],
						Alphas[3],
						Alphas[4],
						Alphas[5],
						Alphas[6],
						Alphas[7],
						AlphaX,
						AlphaY,
						AlphaZ),
					SurfaceType);
			});

#undef Load
	}

public:
	template<typename InnerType, typename LambdaType>
	static void ProcessMetadata(
		const TVoxelVolumeChunkTreeIterator<const FVoxelVolumeMetadataChunk>& Iterator,
		LambdaType Lambda)
	{
		VOXEL_FUNCTION_COUNTER();

		Iterator.Iterate(
			[&](const int32 Index,
				const FVoxelVolumeMetadataChunk* Chunk0, const int32 Index0)
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
				const FVoxelVolumeMetadataChunk* Chunk0, const int32 Index0,
				const FVoxelVolumeMetadataChunk* Chunk1, const int32 Index1,
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
				const FVoxelVolumeMetadataChunk* Chunk0, const int32 Index0,
				const FVoxelVolumeMetadataChunk* Chunk1, const int32 Index1,
				const FVoxelVolumeMetadataChunk* Chunk2, const int32 Index2,
				const FVoxelVolumeMetadataChunk* Chunk3, const int32 Index3,
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
			},
			[&](const int32 Index,
				const FVoxelVolumeMetadataChunk* Chunk0, const int32 Index0,
				const FVoxelVolumeMetadataChunk* Chunk1, const int32 Index1,
				const FVoxelVolumeMetadataChunk* Chunk2, const int32 Index2,
				const FVoxelVolumeMetadataChunk* Chunk3, const int32 Index3,
				const FVoxelVolumeMetadataChunk* Chunk4, const int32 Index4,
				const FVoxelVolumeMetadataChunk* Chunk5, const int32 Index5,
				const FVoxelVolumeMetadataChunk* Chunk6, const int32 Index6,
				const FVoxelVolumeMetadataChunk* Chunk7, const int32 Index7,
				const float AlphaX,
				const float AlphaY,
				const float AlphaZ)
			{
				if (!Chunk0 ||
					!Chunk1 ||
					!Chunk2 ||
					!Chunk3 ||
					!Chunk4 ||
					!Chunk5 ||
					!Chunk6 ||
					!Chunk7)
				{
					return;
				}

				Lambda(
					Index,
					FVoxelUtilities::TrilinearInterpolation(
						Chunk0->GetAlpha(Index0),
						Chunk1->GetAlpha(Index1),
						Chunk2->GetAlpha(Index2),
						Chunk3->GetAlpha(Index3),
						Chunk4->GetAlpha(Index4),
						Chunk5->GetAlpha(Index5),
						Chunk6->GetAlpha(Index6),
						Chunk7->GetAlpha(Index7),
						AlphaX,
						AlphaY,
						AlphaZ),
					FVoxelUtilities::TrilinearInterpolation(
						Chunk0->GetValue<InnerType>(Index0),
						Chunk1->GetValue<InnerType>(Index1),
						Chunk2->GetValue<InnerType>(Index2),
						Chunk3->GetValue<InnerType>(Index3),
						Chunk4->GetValue<InnerType>(Index4),
						Chunk5->GetValue<InnerType>(Index5),
						Chunk6->GetValue<InnerType>(Index6),
						Chunk7->GetValue<InnerType>(Index7),
						AlphaX,
						AlphaY,
						AlphaZ));
			});
	}
};