// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

namespace ispc
{
#ifndef __ISPC_STRUCT_FVoxelHeightTransform__
#define __ISPC_STRUCT_FVoxelHeightTransform__
struct FVoxelHeightTransform
{
	float2 Scale;
	float2 Rotation;
	double2 Position;
	float2 Rotation3D;
	float ScaleZ;
	float OffsetZ;
	float MinHeight;
	float MaxHeight;
};
#endif

#ifndef __ISPC_STRUCT_FVoxelVolumeTransform__
#define __ISPC_STRUCT_FVoxelVolumeTransform__
struct FVoxelVolumeTransform
{
	float3 Scale;
	float4 Rotation;
	double3 Position;

	float MaxDistance;
	float DistanceScale;
};
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct VOXEL_API FVoxelHeightTransform
{
public:
	FVector2f Scale = FVector2f(1.f);
	FVector2f Rotation = FVector2f(1.f, 0.f);
	FVector2d Position = FVector2d(0.f);

	FVector2f Rotation3D = FVector2f(0.f);
	float ScaleZ = 1.f;
	float OffsetZ = 0.f;

	float HeightPadding = 0.f;
	float MinHeight = -MAX_flt;
	float MaxHeight = MAX_flt;

public:
	FVoxelHeightTransform() = default;

	static FVoxelHeightTransform Create(
		const FTransform& StampToWorld,
		const FTransform& WorldToQuery,
		const FVoxelBox& StampBounds,
		float HeightPaddingMultiplier);

public:
	FORCEINLINE ispc::FVoxelHeightTransform ISPC() const
	{
		return ispc::FVoxelHeightTransform
		{
			GetISPCValue(Scale),
			GetISPCValue(Rotation),
			GetISPCValue(Position),
			GetISPCValue(Rotation3D),
			ScaleZ,
			OffsetZ,
			MinHeight,
			MaxHeight
		};
	}

public:
	FORCEINLINE FVector2d Transform(const FVector2d& Point) const
	{
		const FVector2d ScaledPoint = Point * FVector2d(Scale);

		const FVector2d RotatedPoint
		{
			ScaledPoint.X * Rotation.X - ScaledPoint.Y * Rotation.Y,
			ScaledPoint.Y * Rotation.X + ScaledPoint.X * Rotation.Y
		};

		return Position + RotatedPoint;
	}
	FORCEINLINE FVector2d InverseTransform(const FVector2d& Point) const
	{
		const FVector2d TranslatedPoint = Point - Position;

		const FVector2d RotatedPoint
		{
			TranslatedPoint.X * Rotation.X + TranslatedPoint.Y * Rotation.Y,
			TranslatedPoint.Y * Rotation.X - TranslatedPoint.X * Rotation.Y
		};

		return RotatedPoint / FVector2d(Scale);
	}

public:
	template<typename T>
	FORCEINLINE T TransformHeight(
		const T Height,
		const FVector2D& LocalPosition) const
	{
		const T Rotation3DHeight = LocalPosition.X * Rotation3D.X + LocalPosition.Y * Rotation3D.Y;

		return (Height + Rotation3DHeight) * ScaleZ + OffsetZ;
	}
	template<typename T>
	FORCEINLINE T InverseTransformHeight(
		const T Height,
		const FVector2D& LocalPosition) const
	{
		const T Rotation3DHeight = LocalPosition.X * Rotation3D.X + LocalPosition.Y * Rotation3D.Y;

		return (Height - OffsetZ) / ScaleZ - Rotation3DHeight;
	}

	template<typename T>
	FORCEINLINE T ClampHeight(const T Height) const
	{
		if (FVoxelUtilities::IsNaN(Height))
		{
			return Height;
		}

		return FMath::Clamp(
			Height,
			MinHeight,
			MaxHeight);
	}

public:
	FVoxelBox GetBounds(const FVoxelBox& LocalBounds) const;
	FVoxelBox2D InverseTransform(const FVoxelBox2D& Bounds) const;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct VOXEL_API FVoxelVolumeTransform
{
public:
	FVector3f Scale = FVector3f(1.f);
	FQuat4f Rotation = FQuat4f::Identity;
	FVector3d Position = FVector3d(0.f);

	float MaxDistance = FVoxelUtilities::FloatInf();
	float DistanceScale = 1.f;

public:
	FVoxelVolumeTransform() = default;

	static FVoxelVolumeTransform Create(
		const FTransform& StampToWorld,
		const FTransform& WorldToQuery,
		const FVoxelBox& StampBounds,
		float BoundsExtensionMultiplier,
		float MaximumBoundsExtension);

public:
	FORCEINLINE ispc::FVoxelVolumeTransform ISPC() const
	{
		return ispc::FVoxelVolumeTransform
		{
			GetISPCValue(Scale),
			GetISPCValue(ReinterpretCastRef<FVector4f>(Rotation)),
			GetISPCValue(Position),
			MaxDistance,
			DistanceScale
		};
	}

public:
	FORCEINLINE FVector3d Transform(const FVector3d& Point) const
	{
		const FVector3d ScaledPoint = Point * FVector3d(Scale);
		const FVector3d RotatedPoint = FQuat4d(Rotation).RotateVector(ScaledPoint);
		return Position + RotatedPoint;
	}
	FORCEINLINE FVector3d InverseTransform(const FVector3d& Point) const
	{
		const FVector3d TranslatedPoint = Point - Position;
		const FVector3d RotatedPoint = FQuat4d(Rotation).UnrotateVector(TranslatedPoint);
		return RotatedPoint / FVector3d(Scale);
	}

public:
	FORCEINLINE float TransformDistance(float Distance) const
	{
		Distance *= DistanceScale;

		if (Distance > MaxDistance)
		{
			return FVoxelUtilities::NaNf();
		}

		return Distance;
	}
	FORCEINLINE float TransformDistance_NoMaxDistance(const float Distance) const
	{
		return Distance * DistanceScale;
	}

public:
	FVoxelBox GetBounds(const FVoxelBox& LocalBounds) const;
	FVoxelBox InverseTransform(const FVoxelBox& Bounds) const;
};