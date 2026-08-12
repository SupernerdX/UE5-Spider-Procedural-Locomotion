// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampTransform.h"
#include "VoxelHeightBlendMode.h"
#include "VoxelVolumeBlendMode.h"

struct VOXEL_API FVoxelBlendModeUtilities
{
	FORCEINLINE static float ComputeHeight(
		const FVoxelHeightTransform& StampToQuery,
		const float OldHeight,
		const float Height,
		const FVector2D& Position,
		const EVoxelHeightBlendMode BlendMode,
		const bool bApplyOnVoid,
		const float Smoothness,
		float* OutAlpha)
	{
		const float NewHeight = StampToQuery.TransformHeight(Height, Position);

		if (FVoxelUtilities::IsNaN(NewHeight))
		{
			if (OutAlpha)
			{
				*OutAlpha = 0.f;
			}

			return OldHeight;
		}

		switch (BlendMode)
		{
		default: VOXEL_ASSUME(false);
		case EVoxelHeightBlendMode::Max:
		{
			if (FVoxelUtilities::IntBits(OldHeight) == FVoxelUtilities::NaNf_uint)
			{
				if (bApplyOnVoid)
				{
					if (OutAlpha)
					{
						*OutAlpha = 1.f;
					}

					return StampToQuery.ClampHeight(NewHeight);
				}
				else
				{
					if (OutAlpha)
					{
						*OutAlpha = 0.f;
					}

					return OldHeight;
				}
			}
			else
			{
				if (OutAlpha)
				{
					*OutAlpha = FVoxelUtilities::GetSmoothMaxAlpha(OldHeight, NewHeight, Smoothness);
				}

				return FMath::Max(
					OldHeight,
					StampToQuery.ClampHeight(FVoxelUtilities::SmoothMax(OldHeight, NewHeight, Smoothness)));
			}
		}
		case EVoxelHeightBlendMode::Min:
		{
			if (FVoxelUtilities::IntBits(OldHeight) == FVoxelUtilities::NaNf_uint)
			{
				if (bApplyOnVoid)
				{
					if (OutAlpha)
					{
						*OutAlpha = 1.f;
					}

					return StampToQuery.ClampHeight(NewHeight);
				}
				else
				{
					if (OutAlpha)
					{
						*OutAlpha = 0.f;
					}

					return OldHeight;
				}
			}
			else
			{
				if (OutAlpha)
				{
					*OutAlpha = FVoxelUtilities::GetSmoothMinAlpha(OldHeight, NewHeight, Smoothness);
				}

				return FMath::Min(
					OldHeight,
					StampToQuery.ClampHeight(FVoxelUtilities::SmoothMin(OldHeight, NewHeight, Smoothness)));
			}
		}
		case EVoxelHeightBlendMode::Override:
		{
			// Override is computing alphas from graph
			checkVoxelSlow(!OutAlpha);

			return StampToQuery.ClampHeight(NewHeight);
		}
		}
	}

	FORCEINLINE static float ComputeDistance(
		const FVoxelVolumeTransform& StampToQuery,
		const float OldDistance,
		const float Distance,
		const EVoxelVolumeBlendMode BlendMode,
		const bool bApplyOnVoid,
		const float Smoothness,
		float* OutAlpha)
	{
		const float NewDistance = StampToQuery.TransformDistance(Distance);

		if (FVoxelUtilities::IsNaN(NewDistance))
		{
			ensureVoxelSlow(BlendMode != EVoxelVolumeBlendMode::Intersect);

			if (OutAlpha)
			{
				*OutAlpha = 0.f;
			}

			return OldDistance;
		}

		switch (BlendMode)
		{
		default: VOXEL_ASSUME(false);
		case EVoxelVolumeBlendMode::Additive:
		{
			if (FVoxelUtilities::IntBits(OldDistance) == FVoxelUtilities::NaNf_uint)
			{
				if (bApplyOnVoid)
				{
					if (OutAlpha)
					{
						*OutAlpha = 1.f;
					}

					return NewDistance;
				}
				else
				{
					if (OutAlpha)
					{
						*OutAlpha = 0.f;
					}

					return OldDistance;
				}
			}
			else
			{
				if (OutAlpha)
				{
					*OutAlpha = FVoxelUtilities::GetSmoothMinAlpha(OldDistance, NewDistance, Smoothness);
				}

				return FVoxelUtilities::SmoothMin(OldDistance, NewDistance, Smoothness);
			}
		}
		case EVoxelVolumeBlendMode::Subtractive:
		{
			if (FVoxelUtilities::IntBits(OldDistance) == FVoxelUtilities::NaNf_uint)
			{
				if (bApplyOnVoid)
				{
					if (OutAlpha)
					{
						*OutAlpha = 1.f;
					}

					return -NewDistance;
				}
				else
				{
					if (OutAlpha)
					{
						*OutAlpha = 0.f;
					}

					return OldDistance;
				}
			}
			else
			{
				if (OutAlpha)
				{
					*OutAlpha = FVoxelUtilities::GetSmoothMaxAlpha(OldDistance, -NewDistance, Smoothness);
				}

				return FVoxelUtilities::SmoothMax(OldDistance, -NewDistance, Smoothness);
			}
		}
		case EVoxelVolumeBlendMode::Intersect:
		{
			if (FVoxelUtilities::IntBits(OldDistance) == FVoxelUtilities::NaNf_uint)
			{
				if (OutAlpha)
				{
					*OutAlpha = 0.f;
				}

				return OldDistance;
			}
			else
			{
				if (OutAlpha)
				{
					*OutAlpha = FVoxelUtilities::GetSmoothMaxAlpha(OldDistance, NewDistance, Smoothness);
				}

				return FVoxelUtilities::SmoothMax(OldDistance, NewDistance, Smoothness);
			}
		}
		case EVoxelVolumeBlendMode::Override:
		{
			// Override is computing alphas from graph
			checkVoxelSlow(!OutAlpha);

			return NewDistance;
		}
		}
	}
};