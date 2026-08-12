// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampTransform.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelDisableMaxDistance, false,
	"voxel.DisableMaxDistance",
	"If true MaxDistance will be disabled, making stamp follow bounds instead of a more precise MaxDistance",
	Voxel::RefreshAll);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelHeightTransform FVoxelHeightTransform::Create(
	const FTransform& StampToWorld,
	const FTransform& WorldToQuery,
	const FVoxelBox& StampBounds,
	const float HeightPaddingMultiplier)
{
	ensure(!StampBounds.IsInfinite());
	ensure(StampBounds.IsValidAndNotEmpty());
	ensure(WorldToQuery.GetScale3D().Equals(FVector::OneVector));

	const FTransform StampToQuery = StampToWorld * WorldToQuery;

	const FQuat R = StampToQuery.GetRotation();

	FVoxelHeightTransform Result;

	Result.Scale = FVector2f(FVector2D(StampToQuery.GetScale3D()));

	Result.Rotation = FVector2f(
		FVector2D
		{
			FMath::Square(R.W) - FMath::Square(R.Z),
			2.f * R.Z * R.W
		} / (FMath::Square(R.W) + FMath::Square(R.Z)));

	Result.Position = FVector2D(StampToQuery.GetTranslation());

	{
		FQuat Swing;
		FQuat Twist;
		R.ToSwingTwist(FVector::UnitZ(), Swing, Twist);

		const FQuat S = Twist.Inverse() * R;

		const double X = S.X * S.W - S.Y * S.Z;
		const double Y = S.X * S.Z + S.Y * S.W;
		const double W = S.W * S.W + S.Z * S.Z;

		Result.Rotation3D = 2 * W * FVector2f(
			-Y / (W * W - Y * Y),
			X / (W * W - X * X));
	}

	Result.ScaleZ = StampToQuery.GetScale3D().Z;
	Result.OffsetZ = StampToQuery.GetTranslation().Z;

#if VOXEL_DEBUG
	ensure(FMath::IsNearlyEqual(Result.Rotation.Size(), 1.f, 0.01f));

	FQuat Swing;
	FQuat Twist;
	R.ToSwingTwist(FVector::UnitZ(), Swing, Twist);

	// Go through euler angles to avoid flipping at 240 degrees
	const float AngleZ = FMath::DegreesToRadians(Twist.Euler().Z);
	const FQuat2D NewRotation = FQuat2D(AngleZ);

	ensure(Result.Rotation.Equals(FVector2f(NewRotation.GetVector()), 0.01f));

	{
		const FQuat S = Twist.Inverse() * R;

		FQuat Swing2;
		FQuat Twist2;
		S.ToSwingTwist(FVector::UnitZ(), Swing2, Twist2);

		const FVector2D NewRotation3D = -FVector2D(
			FMath::Tan(Swing2.GetTwistAngle(FVector::UnitY())),
			FMath::Tan(Swing2.GetTwistAngle(-FVector::UnitX())));

		ensure(Result.Rotation3D.Equals(FVector2f(NewRotation3D), 0.01f));
	}
#endif

	const FVoxelBox QueryBounds = Result.GetBounds(StampBounds);

	Result.HeightPadding = QueryBounds.Size().Z * HeightPaddingMultiplier;
	Result.MinHeight = FVoxelUtilities::DoubleToFloat_Higher(QueryBounds.Min.Z - Result.HeightPadding);
	Result.MaxHeight = FVoxelUtilities::DoubleToFloat_Lower(QueryBounds.Max.Z + Result.HeightPadding);

	return Result;
}

FVoxelBox FVoxelHeightTransform::GetBounds(const FVoxelBox& LocalBounds) const
{
	ensure(!LocalBounds.IsInfinite());

	const FVector2D P00 = Transform(FVector2D(LocalBounds.Min.X, LocalBounds.Min.Y));
	const FVector2D P01 = Transform(FVector2D(LocalBounds.Max.X, LocalBounds.Min.Y));
	const FVector2D P10 = Transform(FVector2D(LocalBounds.Min.X, LocalBounds.Max.Y));
	const FVector2D P11 = Transform(FVector2D(LocalBounds.Max.X, LocalBounds.Max.Y));

	const double H000 = TransformHeight(LocalBounds.Min.Z, FVector2D(LocalBounds.Min.X, LocalBounds.Min.Y));
	const double H001 = TransformHeight(LocalBounds.Min.Z, FVector2D(LocalBounds.Max.X, LocalBounds.Min.Y));
	const double H010 = TransformHeight(LocalBounds.Min.Z, FVector2D(LocalBounds.Min.X, LocalBounds.Max.Y));
	const double H011 = TransformHeight(LocalBounds.Min.Z, FVector2D(LocalBounds.Max.X, LocalBounds.Max.Y));
	const double H100 = TransformHeight(LocalBounds.Max.Z, FVector2D(LocalBounds.Min.X, LocalBounds.Min.Y));
	const double H101 = TransformHeight(LocalBounds.Max.Z, FVector2D(LocalBounds.Max.X, LocalBounds.Min.Y));
	const double H110 = TransformHeight(LocalBounds.Max.Z, FVector2D(LocalBounds.Min.X, LocalBounds.Max.Y));
	const double H111 = TransformHeight(LocalBounds.Max.Z, FVector2D(LocalBounds.Max.X, LocalBounds.Max.Y));

	FVoxelBox NewBox;

	NewBox.Min.X = FMath::Min3(P00.X, P01.X, FMath::Min(P10.X, P11.X));
	NewBox.Max.X = FMath::Max3(P00.X, P01.X, FMath::Max(P10.X, P11.X));

	NewBox.Min.Y = FMath::Min3(P00.Y, P01.Y, FMath::Min(P10.Y, P11.Y));
	NewBox.Max.Y = FMath::Max3(P00.Y, P01.Y, FMath::Max(P10.Y, P11.Y));

	NewBox.Min.Z = FMath::Min3(H000, H001, FMath::Min3(H010, H011, FMath::Min3(H100, H101, FMath::Min(H110, H111))));
	NewBox.Max.Z = FMath::Max3(H000, H001, FMath::Max3(H010, H011, FMath::Max3(H100, H101, FMath::Max(H110, H111))));

	NewBox.Min.Z -= HeightPadding;
	NewBox.Max.Z += HeightPadding;

	return NewBox;
}

FVoxelBox2D FVoxelHeightTransform::InverseTransform(const FVoxelBox2D& Bounds) const
{
	ensure(!Bounds.IsInfinite());

	const FVector2D P00 = InverseTransform(FVector2D(Bounds.Min.X, Bounds.Min.Y));
	const FVector2D P01 = InverseTransform(FVector2D(Bounds.Max.X, Bounds.Min.Y));
	const FVector2D P10 = InverseTransform(FVector2D(Bounds.Min.X, Bounds.Max.Y));
	const FVector2D P11 = InverseTransform(FVector2D(Bounds.Max.X, Bounds.Max.Y));

	FVoxelBox2D NewBox;

	NewBox.Min.X = FMath::Min3(P00.X, P01.X, FMath::Min(P10.X, P11.X));
	NewBox.Max.X = FMath::Max3(P00.X, P01.X, FMath::Max(P10.X, P11.X));

	NewBox.Min.Y = FMath::Min3(P00.Y, P01.Y, FMath::Min(P10.Y, P11.Y));
	NewBox.Max.Y = FMath::Max3(P00.Y, P01.Y, FMath::Max(P10.Y, P11.Y));

	return NewBox;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVolumeTransform FVoxelVolumeTransform::Create(
	const FTransform& StampToWorld,
	const FTransform& WorldToQuery,
	const FVoxelBox& StampBounds,
	const float BoundsExtensionMultiplier,
	const float MaximumBoundsExtension)
{
	ensure(!StampBounds.IsInfinite());
	ensure(StampBounds.IsValidAndNotEmpty());
	ensure(WorldToQuery.GetScale3D().Equals(FVector::OneVector));

	const FTransform StampToQuery = StampToWorld * WorldToQuery;

	FVoxelVolumeTransform Result;
	Result.Scale = FVector3f(StampToQuery.GetScale3D());
	Result.Rotation = FQuat4f(StampToQuery.GetRotation());
	Result.Position = StampToQuery.GetLocation();
	Result.DistanceScale = StampToQuery.GetScale3D().GetAbsMax();

	const FVoxelBox WorldBounds = StampBounds.TransformBy(StampToWorld);

	// Always compute MaxDistance on WorldBounds to stay consistent
	Result.MaxDistance = FMath::Min(WorldBounds.Size().GetAbsMin() * BoundsExtensionMultiplier, MaximumBoundsExtension);

	if (GVoxelDisableMaxDistance)
	{
		Result.MaxDistance = FVoxelUtilities::FloatInf();
	}

	return Result;
}

FVoxelBox FVoxelVolumeTransform::GetBounds(const FVoxelBox& LocalBounds) const
{
	ensure(!LocalBounds.IsInfinite());

	const FTransform Transform(
		FQuat4d(Rotation),
		Position,
		FVector3d(Scale));

	FVoxelBox Bounds = LocalBounds.TransformBy(Transform);

	if (FVoxelUtilities::IsFinite(MaxDistance))
	{
		Bounds = Bounds.Extend(MaxDistance);
	}

	return Bounds;
}

FVoxelBox FVoxelVolumeTransform::InverseTransform(const FVoxelBox& Bounds) const
{
	ensure(!Bounds.IsInfinite());

	const FTransform Transform(
		FQuat4d(Rotation),
		Position,
		FVector3d(Scale));

	return Bounds.InverseTransformBy(Transform);
}