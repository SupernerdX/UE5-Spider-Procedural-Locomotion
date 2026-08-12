// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Utilities/VoxelBufferTransformUtilities.h"
#include "VoxelBufferAccessor.h"
#include "VoxelBufferTransformUtilitiesImpl.ispc.generated.h"

FVoxelVectorBuffer FVoxelBufferTransformUtilities::ApplyTransform(const FVoxelVectorBuffer& Buffer, const FTransform& Transform)
{
	if (Transform.Equals(FTransform::Identity))
	{
		return Buffer;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelVectorBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferTransformUtilities_ApplyTransform(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Z.GetData(), Buffer.Z.IsConstant(),
		Buffer.Num(),
		GetISPCValue(FVector3f(Transform.GetTranslation())),
		GetISPCValue(FVector4f(
			Transform.GetRotation().X,
			Transform.GetRotation().Y,
			Transform.GetRotation().Z,
			Transform.GetRotation().W)),
		GetISPCValue(FVector3f(Transform.GetScale3D())),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

FVoxelVectorBuffer FVoxelBufferTransformUtilities::ApplyInverseTransform(const FVoxelVectorBuffer& Buffer, const FTransform& Transform)
{
	if (Transform.Equals(FTransform::Identity))
	{
		return Buffer;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelVectorBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferTransformUtilities_ApplyInverseTransform(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Z.GetData(), Buffer.Z.IsConstant(),
		Buffer.Num(),
		GetISPCValue(FVector3f(Transform.GetTranslation())),
		GetISPCValue(FVector4f(
			Transform.GetRotation().X,
			Transform.GetRotation().Y,
			Transform.GetRotation().Z,
			Transform.GetRotation().W)),
		GetISPCValue(FVector3f(Transform.GetScale3D())),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVectorBuffer FVoxelBufferTransformUtilities::ApplyTransform(const FVoxelVectorBuffer& Buffer, const FVoxelTransformBuffer& Transform)
{
	if (Transform.IsConstant())
	{
		return ApplyTransform(Buffer, FTransform(Transform.GetConstant()));
	}

	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelVectorBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferTransformUtilities_ApplyTransform_Buffer(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Z.GetData(), Buffer.Z.IsConstant(),
		Buffer.Num(),
		Transform.Translation.X.GetData(), Transform.Translation.X.IsConstant(),
		Transform.Translation.Y.GetData(), Transform.Translation.Y.IsConstant(),
		Transform.Translation.Z.GetData(), Transform.Translation.Z.IsConstant(),
		Transform.Rotation.X.GetData(), Transform.Rotation.X.IsConstant(),
		Transform.Rotation.Y.GetData(), Transform.Rotation.Y.IsConstant(),
		Transform.Rotation.Z.GetData(), Transform.Rotation.Z.IsConstant(),
		Transform.Rotation.W.GetData(), Transform.Rotation.W.IsConstant(),
		Transform.Scale.X.GetData(), Transform.Scale.X.IsConstant(),
		Transform.Scale.Y.GetData(), Transform.Scale.Y.IsConstant(),
		Transform.Scale.Z.GetData(), Transform.Scale.Z.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

FVoxelVectorBuffer FVoxelBufferTransformUtilities::ApplyInverseTransform(const FVoxelVectorBuffer& Buffer, const FVoxelTransformBuffer& Transform)
{
	if (Transform.IsConstant())
	{
		return ApplyInverseTransform(Buffer, FTransform(Transform.GetConstant()));
	}

	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelVectorBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferTransformUtilities_ApplyInverseTransform_Buffer(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Z.GetData(), Buffer.Z.IsConstant(),
		Buffer.Num(),
		Transform.Translation.X.GetData(), Transform.Translation.X.IsConstant(),
		Transform.Translation.Y.GetData(), Transform.Translation.Y.IsConstant(),
		Transform.Translation.Z.GetData(), Transform.Translation.Z.IsConstant(),
		Transform.Rotation.X.GetData(), Transform.Rotation.X.IsConstant(),
		Transform.Rotation.Y.GetData(), Transform.Rotation.Y.IsConstant(),
		Transform.Rotation.Z.GetData(), Transform.Rotation.Z.IsConstant(),
		Transform.Rotation.W.GetData(), Transform.Rotation.W.IsConstant(),
		Transform.Scale.X.GetData(), Transform.Scale.X.IsConstant(),
		Transform.Scale.Y.GetData(), Transform.Scale.Y.IsConstant(),
		Transform.Scale.Z.GetData(), Transform.Scale.Z.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelDoubleVectorBuffer FVoxelBufferTransformUtilities::ApplyTransform(const FVoxelDoubleVectorBuffer& Buffer, const FTransform& Transform)
{
	if (Transform.Equals(FTransform::Identity))
	{
		return Buffer;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelDoubleVectorBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferTransformUtilities_ApplyTransform_Double(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Z.GetData(), Buffer.Z.IsConstant(),
		Buffer.Num(),
		GetISPCValue(FVector3d(Transform.GetTranslation())),
		GetISPCValue(FVector4d(
			Transform.GetRotation().X,
			Transform.GetRotation().Y,
			Transform.GetRotation().Z,
			Transform.GetRotation().W)),
		GetISPCValue(FVector3d(Transform.GetScale3D())),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

FVoxelDoubleVectorBuffer FVoxelBufferTransformUtilities::ApplyInverseTransform(const FVoxelDoubleVectorBuffer& Buffer, const FTransform& Transform)
{
	if (Transform.Equals(FTransform::Identity))
	{
		return Buffer;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelDoubleVectorBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferTransformUtilities_ApplyInverseTransform_Double(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Z.GetData(), Buffer.Z.IsConstant(),
		Buffer.Num(),
		GetISPCValue(FVector3d(Transform.GetTranslation())),
		GetISPCValue(FVector4d(
			Transform.GetRotation().X,
			Transform.GetRotation().Y,
			Transform.GetRotation().Z,
			Transform.GetRotation().W)),
		GetISPCValue(FVector3d(Transform.GetScale3D())),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVector2DBuffer FVoxelBufferTransformUtilities::ApplyTransform(const FVoxelVector2DBuffer& Buffer, const FTransform2d& Transform)
{
	if (Transform.IsIdentity())
	{
		return Buffer;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelVector2DBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferTransformUtilities_ApplyTransform2D(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Num(),
		GetISPCValue(FVoxelUtilities::MakeTransform2f(Transform)),
		Result.X.GetData(),
		Result.Y.GetData());

	return Result;
}

FVoxelDoubleVector2DBuffer FVoxelBufferTransformUtilities::ApplyTransform(const FVoxelDoubleVector2DBuffer& Buffer, const FTransform2d& Transform)
{
	if (Transform.IsIdentity())
	{
		return Buffer;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelDoubleVector2DBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferTransformUtilities_ApplyTransform2D_Double(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Num(),
		GetISPCValue(Transform),
		Result.X.GetData(),
		Result.Y.GetData());

	return Result;
}

FVoxelVector2DBuffer FVoxelBufferTransformUtilities::ApplyInverseTransform(const FVoxelVector2DBuffer& Buffer, const FTransform2d& Transform)
{
	ensure(Transform.Inverse().Inverse() == Transform);
	return ApplyTransform(Buffer, Transform.Inverse());
}

FVoxelDoubleVector2DBuffer FVoxelBufferTransformUtilities::ApplyInverseTransform(const FVoxelDoubleVector2DBuffer& Buffer, const FTransform2d& Transform)
{
	ensure(Transform.Inverse().Inverse() == Transform);
	return ApplyTransform(Buffer, Transform.Inverse());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVectorBuffer FVoxelBufferTransformUtilities::ApplyTransform(
	const FVoxelVectorBuffer& Buffer,
	const FVoxelVectorBuffer& Translation,
	const FVoxelQuaternionBuffer& Rotation,
	const FVoxelVectorBuffer& Scale)
{
	const FVoxelBufferAccessor BufferAccessor(Buffer, Translation, Rotation, Scale);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelBuffer::DefaultBuffer;
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num(), 1024);

	FVoxelVectorBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferTransformUtilities_ApplyTransform_Bulk(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Z.GetData(), Buffer.Z.IsConstant(),
		Translation.X.GetData(), Translation.X.IsConstant(),
		Translation.Y.GetData(), Translation.Y.IsConstant(),
		Translation.Z.GetData(), Translation.Z.IsConstant(),
		Rotation.X.GetData(), Rotation.X.IsConstant(),
		Rotation.Y.GetData(), Rotation.Y.IsConstant(),
		Rotation.Z.GetData(), Rotation.Z.IsConstant(),
		Rotation.W.GetData(), Rotation.W.IsConstant(),
		Scale.X.GetData(), Scale.X.IsConstant(),
		Scale.Y.GetData(), Scale.Y.IsConstant(),
		Scale.Z.GetData(), Scale.Z.IsConstant(),
		BufferAccessor.Num(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVectorBuffer FVoxelBufferTransformUtilities::ApplyTransform(const FVoxelVectorBuffer& Buffer, const FMatrix& Matrix)
{
	const FTransform Transform(Matrix);
	if (Transform.ToMatrixWithScale().Equals(Matrix))
	{
		return ApplyTransform(Buffer, Transform);
	}

	ensure(!Matrix.GetScaleVector().IsUniform());
	VOXEL_SCOPE_COUNTER_NUM("Non-Uniform Scale", Buffer.Num(), 1024);

	const FVector Translation = Matrix.GetOrigin();
	const FMatrix BaseMatrix = Matrix.RemoveTranslation();
	ensure(Matrix.Equals(BaseMatrix * FTranslationMatrix(Translation)));

	ensure(FMath::IsNearlyEqual(BaseMatrix.M[0][3], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[1][3], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[2][3], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[3][0], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[3][1], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[3][2], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[3][3], 1));

	FVoxelVectorBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferTransformUtilities_ApplyTransform_Matrix(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Z.GetData(), Buffer.Z.IsConstant(),
		Buffer.Num(),
		GetISPCValue(FVector3f(Translation)),
		GetISPCValue(FVector3f(BaseMatrix.M[0][0], BaseMatrix.M[0][1], BaseMatrix.M[0][2])),
		GetISPCValue(FVector3f(BaseMatrix.M[1][0], BaseMatrix.M[1][1], BaseMatrix.M[1][2])),
		GetISPCValue(FVector3f(BaseMatrix.M[2][0], BaseMatrix.M[2][1], BaseMatrix.M[2][2])),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

FVoxelDoubleVectorBuffer FVoxelBufferTransformUtilities::ApplyTransform(const FVoxelDoubleVectorBuffer& Buffer, const FMatrix& Matrix)
{
	const FTransform Transform(Matrix);
	if (Transform.ToMatrixWithScale().Equals(Matrix))
	{
		return ApplyTransform(Buffer, Transform);
	}

	ensure(!Matrix.GetScaleVector().IsUniform());
	VOXEL_SCOPE_COUNTER_NUM("Non-Uniform Scale", Buffer.Num(), 1024);

	const FVector Translation = Matrix.GetOrigin();
	const FMatrix BaseMatrix = Matrix.RemoveTranslation();
	ensure(Matrix.Equals(BaseMatrix * FTranslationMatrix(Translation)));

	ensure(FMath::IsNearlyEqual(BaseMatrix.M[0][3], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[1][3], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[2][3], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[3][0], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[3][1], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[3][2], 0));
	ensure(FMath::IsNearlyEqual(BaseMatrix.M[3][3], 1));

	FVoxelDoubleVectorBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferTransformUtilities_ApplyTransform_Matrix_Double(
		Buffer.X.GetData(), Buffer.X.IsConstant(),
		Buffer.Y.GetData(), Buffer.Y.IsConstant(),
		Buffer.Z.GetData(), Buffer.Z.IsConstant(),
		Buffer.Num(),
		GetISPCValue(FVector3d(Translation)),
		GetISPCValue(FVector3d(BaseMatrix.M[0][0], BaseMatrix.M[0][1], BaseMatrix.M[0][2])),
		GetISPCValue(FVector3d(BaseMatrix.M[1][0], BaseMatrix.M[1][1], BaseMatrix.M[1][2])),
		GetISPCValue(FVector3d(BaseMatrix.M[2][0], BaseMatrix.M[2][1], BaseMatrix.M[2][2])),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer FVoxelBufferTransformUtilities::TransformDistance(const FVoxelFloatBuffer& Distance, const FMatrix& Transform)
{
	const float Scale = Transform.GetScaleVector().GetAbsMax();
	if (Scale == 1.f)
	{
		return Distance;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Distance.Num(), 1024);

	FVoxelFloatBuffer Result;
	Result.Allocate(Distance.Num());

	ispc::VoxelBufferTransformUtilities_TransformDistance(
		Distance.GetData(),
		Result.GetData(),
		Scale,
		Distance.Num());

	return Result;
}