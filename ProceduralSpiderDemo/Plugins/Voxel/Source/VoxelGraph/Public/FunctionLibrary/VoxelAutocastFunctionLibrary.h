// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFunctionLibrary.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Buffer/VoxelIntegerBuffers.h"
#include "Utilities/VoxelBufferConversionUtilities.h"
#include "FunctionLibrary/VoxelMathFunctionLibrary.h"
#include "VoxelAutocastFunctionLibrary.generated.h"

UCLASS(Category = "Math|Conversions")
class VOXELGRAPH_API UVoxelAutocastFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	///////////////////////////////////////////////////////////////
	///////////////////////// Vector2D -> /////////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer Vector2DToVector(
		const FVoxelVector2DBuffer& Vector2D,
		const FVoxelFloatBuffer& Z) const
	{
		CheckVoxelBuffersNum_Return(Vector2D, Z);

		FVoxelVectorBuffer Result;
		Result.X = Vector2D.X;
		Result.Y = Vector2D.Y;
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer Vector2DToLinearColor(
		const FVoxelVector2DBuffer& Vector2D,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector2D, B, A);

		FVoxelLinearColorBuffer Result;
		Result.R = Vector2D.X;
		Result.G = Vector2D.Y;
		Result.B = B;
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer Vector2DToDoubleVector2D(
		const FVoxelVector2DBuffer& Vector2D) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::FloatToDouble(Vector2D.X);
		Result.Y = FVoxelBufferConversionUtilities::FloatToDouble(Vector2D.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer Vector2DToDoubleVector(
		const FVoxelVector2DBuffer& Vector2D,
		const FVoxelDoubleBuffer& Z) const
	{
		CheckVoxelBuffersNum_Return(Vector2D, Z);

		FVoxelDoubleVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::FloatToDouble(Vector2D.X);
		Result.Y = FVoxelBufferConversionUtilities::FloatToDouble(Vector2D.Y);
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer Vector2DToDoubleLinearColor(
		const FVoxelVector2DBuffer& Vector2D,
		const FVoxelDoubleBuffer& B,
		const FVoxelDoubleBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector2D, B, A);

		FVoxelDoubleLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::FloatToDouble(Vector2D.X);
		Result.G = FVoxelBufferConversionUtilities::FloatToDouble(Vector2D.Y);
		Result.B = B;
		Result.A = A;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	////////////////////////// Vector -> //////////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer VectorToVector2D(
		const FVoxelVectorBuffer& Vector) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer VectorToLinearColor(
		const FVoxelVectorBuffer& Vector,
		const FVoxelFloatBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector, A);

		FVoxelLinearColorBuffer Result;
		Result.R = Vector.X;
		Result.G = Vector.Y;
		Result.B = Vector.Z;
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer VectorToDoubleVector2D(
		const FVoxelVectorBuffer& Vector) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::FloatToDouble(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::FloatToDouble(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer VectorToDoubleVector(
		const FVoxelVectorBuffer& Vector) const
	{
		FVoxelDoubleVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::FloatToDouble(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::FloatToDouble(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::FloatToDouble(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer VectorToDoubleLinearColor(
		const FVoxelVectorBuffer& Vector,
		const FVoxelDoubleBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector, A);

		FVoxelDoubleLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::FloatToDouble(Vector.X);
		Result.G = FVoxelBufferConversionUtilities::FloatToDouble(Vector.Y);
		Result.B = FVoxelBufferConversionUtilities::FloatToDouble(Vector.Z);
		Result.A = A;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	//////////////////////// LinearColor -> ///////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer LinearColorToVector2D(
		const FVoxelLinearColorBuffer& Color) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = Color.R;
		Result.Y = Color.G;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer LinearColorToVector(
		const FVoxelLinearColorBuffer& Color) const
	{
		FVoxelVectorBuffer Result;
		Result.X = Color.R;
		Result.Y = Color.G;
		Result.Z = Color.B;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer LinearColorToDoubleVector2D(
		const FVoxelLinearColorBuffer& Color) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::FloatToDouble(Color.R);
		Result.Y = FVoxelBufferConversionUtilities::FloatToDouble(Color.G);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer LinearColorToDoubleVector(
		const FVoxelLinearColorBuffer& Color) const
	{
		FVoxelDoubleVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::FloatToDouble(Color.R);
		Result.Y = FVoxelBufferConversionUtilities::FloatToDouble(Color.G);
		Result.Z = FVoxelBufferConversionUtilities::FloatToDouble(Color.B);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer LinearColorToDoubleLinearColor(
		const FVoxelLinearColorBuffer& Color) const
	{
		FVoxelDoubleLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::FloatToDouble(Color.R);
		Result.G = FVoxelBufferConversionUtilities::FloatToDouble(Color.G);
		Result.B = FVoxelBufferConversionUtilities::FloatToDouble(Color.B);
		Result.A = FVoxelBufferConversionUtilities::FloatToDouble(Color.A);
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	////////////////////// DoubleVector2D -> //////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer DoubleVector2DToDoubleVector(
		const FVoxelDoubleVector2DBuffer& Vector2D,
		const FVoxelDoubleBuffer& Z) const
	{
		CheckVoxelBuffersNum_Return(Vector2D, Z);

		FVoxelDoubleVectorBuffer Result;
		Result.X = Vector2D.X;
		Result.Y = Vector2D.Y;
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer DoubleVector2DToDoubleLinearColor(
		const FVoxelDoubleVector2DBuffer& Vector2D,
		const FVoxelDoubleBuffer& B,
		const FVoxelDoubleBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector2D, B, A);

		FVoxelDoubleLinearColorBuffer Result;
		Result.R = Vector2D.X;
		Result.G = Vector2D.Y;
		Result.B = B;
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer DoubleVector2DToVector2D(
		const FVoxelDoubleVector2DBuffer& Vector2D) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::DoubleToFloat(Vector2D.X);
		Result.Y = FVoxelBufferConversionUtilities::DoubleToFloat(Vector2D.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer DoubleVector2DToVector(
		const FVoxelDoubleVector2DBuffer& Vector2D,
		const FVoxelFloatBuffer& Z) const
	{
		CheckVoxelBuffersNum_Return(Vector2D, Z);

		FVoxelVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::DoubleToFloat(Vector2D.X);
		Result.Y = FVoxelBufferConversionUtilities::DoubleToFloat(Vector2D.Y);
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer DoubleVector2DToLinearColor(
		const FVoxelDoubleVector2DBuffer& Vector2D,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector2D, B, A);

		FVoxelLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::DoubleToFloat(Vector2D.X);
		Result.G = FVoxelBufferConversionUtilities::DoubleToFloat(Vector2D.Y);
		Result.B = B;
		Result.A = A;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	/////////////////////// DoubleVector -> ///////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer DoubleVectorToDoubleVector2D(
		const FVoxelDoubleVectorBuffer& Vector) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer DoubleVectorToDoubleLinearColor(
		const FVoxelDoubleVectorBuffer& Vector,
		const FVoxelDoubleBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector, A);

		FVoxelDoubleLinearColorBuffer Result;
		Result.R = Vector.X;
		Result.G = Vector.Y;
		Result.B = Vector.Z;
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer DoubleVectorToVector2D(
		const FVoxelDoubleVectorBuffer& Vector) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::DoubleToFloat(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::DoubleToFloat(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer DoubleVectorToVector(
		const FVoxelDoubleVectorBuffer& Vector) const
	{
		FVoxelVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::DoubleToFloat(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::DoubleToFloat(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::DoubleToFloat(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer DoubleVectorToLinearColor(
		const FVoxelDoubleVectorBuffer& Vector,
		const FVoxelFloatBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector, A);

		FVoxelLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::DoubleToFloat(Vector.X);
		Result.G = FVoxelBufferConversionUtilities::DoubleToFloat(Vector.Y);
		Result.B = FVoxelBufferConversionUtilities::DoubleToFloat(Vector.Z);
		Result.A = A;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	///////////////////// DoubleLinearColor -> ////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer DoubleLinearColorToDoubleVector2D(
		const FVoxelDoubleLinearColorBuffer& Color) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = Color.R;
		Result.Y = Color.G;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer DoubleLinearColorToDoubleVector(
		const FVoxelDoubleLinearColorBuffer& Color) const
	{
		FVoxelDoubleVectorBuffer Result;
		Result.X = Color.R;
		Result.Y = Color.G;
		Result.Z = Color.B;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer DoubleLinearColorToVector2D(
		const FVoxelDoubleLinearColorBuffer& Color) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::DoubleToFloat(Color.R);
		Result.Y = FVoxelBufferConversionUtilities::DoubleToFloat(Color.G);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer DoubleLinearColorToVector(
		const FVoxelDoubleLinearColorBuffer& Color) const
	{
		FVoxelVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::DoubleToFloat(Color.R);
		Result.Y = FVoxelBufferConversionUtilities::DoubleToFloat(Color.G);
		Result.Z = FVoxelBufferConversionUtilities::DoubleToFloat(Color.B);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer DoubleLinearColorToLinearColor(
		const FVoxelDoubleLinearColorBuffer& Color) const
	{
		FVoxelLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::DoubleToFloat(Color.R);
		Result.G = FVoxelBufferConversionUtilities::DoubleToFloat(Color.G);
		Result.B = FVoxelBufferConversionUtilities::DoubleToFloat(Color.B);
		Result.A = FVoxelBufferConversionUtilities::DoubleToFloat(Color.A);
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	///////////////////////// IntPoint -> /////////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelIntVectorBuffer IntPointToIntVector(
		const FVoxelIntPointBuffer& IntPoint,
		const FVoxelInt32Buffer& Z) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z);

		FVoxelIntVectorBuffer Result;
		Result.X = IntPoint.X;
		Result.Y = IntPoint.Y;
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVector4Buffer IntPointToIntVector4(
		const FVoxelIntPointBuffer& IntPoint,
		const FVoxelInt32Buffer& Z,
		const FVoxelInt32Buffer& W) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z, W);

		FVoxelIntVector4Buffer Result;
		Result.X = IntPoint.X;
		Result.Y = IntPoint.Y;
		Result.Z = Z;
		Result.W = W;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer IntPointToVector2D(
		const FVoxelIntPointBuffer& IntPoint) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToFloat(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToFloat(IntPoint.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer IntPointToVector(
		const FVoxelIntPointBuffer& IntPoint,
		const FVoxelFloatBuffer& Z) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z);

		FVoxelVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToFloat(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToFloat(IntPoint.Y);
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer IntPointToLinearColor(
		const FVoxelIntPointBuffer& IntPoint,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, B, A);

		FVoxelLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int32ToFloat(IntPoint.X);
		Result.G = FVoxelBufferConversionUtilities::Int32ToFloat(IntPoint.Y);
		Result.B = B;
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer IntPointToDoubleVector2D(
		const FVoxelIntPointBuffer& IntPoint) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToDouble(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToDouble(IntPoint.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer IntPointToDoubleVector(
		const FVoxelIntPointBuffer& IntPoint,
		const FVoxelDoubleBuffer& Z) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z);

		FVoxelDoubleVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToDouble(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToDouble(IntPoint.Y);
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer IntPointToDoubleLinearColor(
		const FVoxelIntPointBuffer& IntPoint,
		const FVoxelDoubleBuffer& B,
		const FVoxelDoubleBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, B, A);

		FVoxelDoubleLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int32ToDouble(IntPoint.X);
		Result.G = FVoxelBufferConversionUtilities::Int32ToDouble(IntPoint.Y);
		Result.B = B;
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelInt64PointBuffer IntPointToInt64Point(
		const FVoxelIntPointBuffer& IntPoint) const
	{
		FVoxelInt64PointBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToInt64(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToInt64(IntPoint.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64VectorBuffer IntPointToInt64Vector(
		const FVoxelIntPointBuffer& IntPoint,
		const FVoxelInt64Buffer& Z) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z);

		FVoxelInt64VectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToInt64(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToInt64(IntPoint.Y);
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64Vector4Buffer IntPointToInt64Vector4(
		const FVoxelIntPointBuffer& IntPoint,
		const FVoxelInt64Buffer& Z,
		const FVoxelInt64Buffer& W) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z, W);

		FVoxelInt64Vector4Buffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToInt64(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToInt64(IntPoint.Y);
		Result.Z = Z;
		Result.W = W;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	///////////////////////// IntVector -> ////////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelIntPointBuffer IntVectorToIntPoint(
		const FVoxelIntVectorBuffer& Vector) const
	{
		FVoxelIntPointBuffer Result;
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVector4Buffer IntVectorToIntVector4(
		const FVoxelIntVectorBuffer& Vector,
		const FVoxelInt32Buffer& W) const
	{
		CheckVoxelBuffersNum_Return(Vector, W);

		FVoxelIntVector4Buffer Result;
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		Result.Z = Vector.Z;
		Result.W = W;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer IntVectorToVector2D(
		const FVoxelIntVectorBuffer& Vector) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer IntVectorToVector(
		const FVoxelIntVectorBuffer& Vector) const
	{
		FVoxelVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer IntVectorToLinearColor(
		const FVoxelIntVectorBuffer& Vector,
		const FVoxelFloatBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector, A);

		FVoxelLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.X);
		Result.G = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.Y);
		Result.B = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.Z);
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer IntVectorToDoubleVector2D(
		const FVoxelIntVectorBuffer& Vector) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer IntVectorToDoubleVector(
		const FVoxelIntVectorBuffer& Vector) const
	{
		FVoxelDoubleVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer IntVectorToDoubleLinearColor(
		const FVoxelIntVectorBuffer& Vector,
		const FVoxelDoubleBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector, A);

		FVoxelDoubleLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.X);
		Result.G = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.Y);
		Result.B = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.Z);
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelInt64PointBuffer IntVectorToInt64Point(
		const FVoxelIntVectorBuffer& Vector) const
	{
		FVoxelInt64PointBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64VectorBuffer IntVectorToInt64Vector(
		const FVoxelIntVectorBuffer& Vector) const
	{
		FVoxelInt64VectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64Vector4Buffer IntVectorToInt64Vector4(
		const FVoxelIntVectorBuffer& Vector,
		const FVoxelInt64Buffer& W) const
	{
		CheckVoxelBuffersNum_Return(Vector, W);

		FVoxelInt64Vector4Buffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.Z);
		Result.W = W;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	//////////////////////// IntVector4 -> ////////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelIntPointBuffer IntVector4ToIntPoint(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelIntPointBuffer Result;
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVectorBuffer IntVector4ToIntVector(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelIntVectorBuffer Result;
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		Result.Z = Vector.Z;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer IntVector4ToVector2D(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer IntVector4ToVector(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer IntVector4ToLinearColor(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.X);
		Result.G = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.Y);
		Result.B = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.Z);
		Result.A = FVoxelBufferConversionUtilities::Int32ToFloat(Vector.W);
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer IntVector4ToDoubleVector2D(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer IntVector4ToDoubleVector(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelDoubleVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer IntVector4ToDoubleLinearColor(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelDoubleLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.X);
		Result.G = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.Y);
		Result.B = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.Z);
		Result.A = FVoxelBufferConversionUtilities::Int32ToDouble(Vector.W);
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelInt64PointBuffer IntVector4ToInt64Point(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelInt64PointBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64VectorBuffer IntVector4ToInt64Vector(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelInt64VectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64Vector4Buffer IntVector4ToInt64Vector4(
		const FVoxelIntVector4Buffer& Vector) const
	{
		FVoxelInt64Vector4Buffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.Z);
		Result.W = FVoxelBufferConversionUtilities::Int32ToInt64(Vector.W);
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	//////////////////////// Int64Point -> ////////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelInt64VectorBuffer Int64PointToInt64Vector(
		const FVoxelInt64PointBuffer& IntPoint,
		const FVoxelInt64Buffer& Z) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z);

		FVoxelInt64VectorBuffer Result;
		Result.X = IntPoint.X;
		Result.Y = IntPoint.Y;
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64Vector4Buffer Int64PointToInt64Vector4(
		const FVoxelInt64PointBuffer& IntPoint,
		const FVoxelInt64Buffer& Z,
		const FVoxelInt64Buffer& W) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z, W);

		FVoxelInt64Vector4Buffer Result;
		Result.X = IntPoint.X;
		Result.Y = IntPoint.Y;
		Result.Z = Z;
		Result.W = W;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer Int64PointToVector2D(
		const FVoxelInt64PointBuffer& IntPoint) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToFloat(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToFloat(IntPoint.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer Int64PointToVector(
		const FVoxelInt64PointBuffer& IntPoint,
		const FVoxelFloatBuffer& Z) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z);

		FVoxelVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToFloat(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToFloat(IntPoint.Y);
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer Int64PointToLinearColor(
		const FVoxelInt64PointBuffer& IntPoint,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, B, A);

		FVoxelLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int64ToFloat(IntPoint.X);
		Result.G = FVoxelBufferConversionUtilities::Int64ToFloat(IntPoint.Y);
		Result.B = B;
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer Int64PointToDoubleVector2D(
		const FVoxelInt64PointBuffer& IntPoint) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToDouble(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToDouble(IntPoint.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer Int64PointToDoubleVector(
		const FVoxelInt64PointBuffer& IntPoint,
		const FVoxelDoubleBuffer& Z) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z);

		FVoxelDoubleVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToDouble(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToDouble(IntPoint.Y);
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer Int64PointToDoubleLinearColor(
		const FVoxelInt64PointBuffer& IntPoint,
		const FVoxelDoubleBuffer& B,
		const FVoxelDoubleBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, B, A);

		FVoxelDoubleLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int64ToDouble(IntPoint.X);
		Result.G = FVoxelBufferConversionUtilities::Int64ToDouble(IntPoint.Y);
		Result.B = B;
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelIntPointBuffer Int64PointToIntPoint(
		const FVoxelInt64PointBuffer& IntPoint) const
	{
		FVoxelIntPointBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToInt32(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToInt32(IntPoint.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVectorBuffer Int64PointToIntVector(
		const FVoxelInt64PointBuffer& IntPoint,
		const FVoxelInt32Buffer& Z) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z);

		FVoxelIntVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToInt32(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToInt32(IntPoint.Y);
		Result.Z = Z;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVector4Buffer Int64PointToIntVector4(
		const FVoxelInt64PointBuffer& IntPoint,
		const FVoxelInt32Buffer& Z,
		const FVoxelInt32Buffer& W) const
	{
		CheckVoxelBuffersNum_Return(IntPoint, Z, W);

		FVoxelIntVector4Buffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToInt32(IntPoint.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToInt32(IntPoint.Y);
		Result.Z = Z;
		Result.W = W;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	//////////////////////// Int64Vector -> ///////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelInt64PointBuffer Int64VectorToInt64Point(
		const FVoxelInt64VectorBuffer& Vector) const
	{
		FVoxelInt64PointBuffer Result;
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64Vector4Buffer Int64VectorToInt64Vector4(
		const FVoxelInt64VectorBuffer& Vector,
		const FVoxelInt64Buffer& W) const
	{
		CheckVoxelBuffersNum_Return(Vector, W);

		FVoxelInt64Vector4Buffer Result;
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		Result.Z = Vector.Z;
		Result.W = W;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer Int64VectorToVector2D(
		const FVoxelInt64VectorBuffer& Vector) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer Int64VectorToVector(
		const FVoxelInt64VectorBuffer& Vector) const
	{
		FVoxelVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer Int64VectorToLinearColor(
		const FVoxelInt64VectorBuffer& Vector,
		const FVoxelFloatBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector, A);

		FVoxelLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.X);
		Result.G = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.Y);
		Result.B = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.Z);
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer Int64VectorToDoubleVector2D(
		const FVoxelInt64VectorBuffer& Vector) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer Int64VectorToDoubleVector(
		const FVoxelInt64VectorBuffer& Vector) const
	{
		FVoxelDoubleVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer Int64VectorToDoubleLinearColor(
		const FVoxelInt64VectorBuffer& Vector,
		const FVoxelDoubleBuffer& A) const
	{
		CheckVoxelBuffersNum_Return(Vector, A);

		FVoxelDoubleLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.X);
		Result.G = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.Y);
		Result.B = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.Z);
		Result.A = A;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelIntPointBuffer Int64VectorToIntPoint(
		const FVoxelInt64VectorBuffer& Vector) const
	{
		FVoxelIntPointBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVectorBuffer Int64VectorToIntVector(
		const FVoxelInt64VectorBuffer& Vector) const
	{
		FVoxelIntVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVector4Buffer Int64VectorToIntVector4(
		const FVoxelInt64VectorBuffer& Vector,
		const FVoxelInt32Buffer& W) const
	{
		CheckVoxelBuffersNum_Return(Vector, W);

		FVoxelIntVector4Buffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.Z);
		Result.W = W;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	/////////////////////// Int64Vector4 -> ///////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelInt64PointBuffer Int64Vector4ToInt64Point(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelInt64PointBuffer Result;
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64VectorBuffer Int64Vector4ToInt64Vector(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelInt64VectorBuffer Result;
		Result.X = Vector.X;
		Result.Y = Vector.Y;
		Result.Z = Vector.Z;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer Int64Vector4ToVector2D(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer Int64Vector4ToVector(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer Int64Vector4ToLinearColor(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.X);
		Result.G = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.Y);
		Result.B = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.Z);
		Result.A = FVoxelBufferConversionUtilities::Int64ToFloat(Vector.W);
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer Int64Vector4ToDoubleVector2D(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer Int64Vector4ToDoubleVector(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelDoubleVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer Int64Vector4ToDoubleLinearColor(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelDoubleLinearColorBuffer Result;
		Result.R = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.X);
		Result.G = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.Y);
		Result.B = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.Z);
		Result.A = FVoxelBufferConversionUtilities::Int64ToDouble(Vector.W);
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelIntPointBuffer Int64Vector4ToIntPoint(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelIntPointBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.Y);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVectorBuffer Int64Vector4ToIntVector(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelIntVectorBuffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.Z);
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVector4Buffer Int64Vector4ToIntVector4(
		const FVoxelInt64Vector4Buffer& Vector) const
	{
		FVoxelIntVector4Buffer Result;
		Result.X = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.X);
		Result.Y = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.Y);
		Result.Z = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.Z);
		Result.W = FVoxelBufferConversionUtilities::Int64ToInt32(Vector.W);
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	////////////////////////// Float -> ///////////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer FloatToVector(const FVoxelFloatBuffer& Value) const
	{
		FVoxelVectorBuffer Result;
		Result.X = Value;
		Result.Y = Value;
		Result.Z = Value;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelVector2DBuffer FloatToVector2D(const FVoxelFloatBuffer& Value) const
	{
		FVoxelVector2DBuffer Result;
		Result.X = Value;
		Result.Y = Value;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelLinearColorBuffer FloatToLinearColor(const FVoxelFloatBuffer& Value) const
	{
		FVoxelLinearColorBuffer Result;
		Result.R = Value;
		Result.G = Value;
		Result.B = Value;
		Result.A = Value;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	////////////////////////// Double -> //////////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVectorBuffer DoubleToDoubleVector(const FVoxelDoubleBuffer& Value) const
	{
		FVoxelDoubleVectorBuffer Result;
		Result.X = Value;
		Result.Y = Value;
		Result.Z = Value;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleVector2DBuffer DoubleToDoubleVector2D(const FVoxelDoubleBuffer& Value) const
	{
		FVoxelDoubleVector2DBuffer Result;
		Result.X = Value;
		Result.Y = Value;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelDoubleLinearColorBuffer DoubleToDoubleLinearColor(const FVoxelDoubleBuffer& Value) const
	{
		FVoxelDoubleLinearColorBuffer Result;
		Result.R = Value;
		Result.G = Value;
		Result.B = Value;
		Result.A = Value;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	////////////////////////// Int32 -> ///////////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelIntPointBuffer Int32ToIntPoint(const FVoxelInt32Buffer& Value) const
	{
		FVoxelIntPointBuffer Result;
		Result.X = Value;
		Result.Y = Value;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVectorBuffer Int32ToIntVector(const FVoxelInt32Buffer& Value) const
	{
		FVoxelIntVectorBuffer Result;
		Result.X = Value;
		Result.Y = Value;
		Result.Z = Value;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelIntVector4Buffer Int32ToIntVector4(const FVoxelInt32Buffer& Value) const
	{
		FVoxelIntVector4Buffer Result;
		Result.X = Value;
		Result.Y = Value;
		Result.Z = Value;
		Result.W = Value;
		return Result;
	}

public:
	///////////////////////////////////////////////////////////////
	////////////////////////// Int64 -> ///////////////////////////
	///////////////////////////////////////////////////////////////

public:
	UFUNCTION(meta = (Autocast))
	FVoxelInt64PointBuffer Int64ToInt64Point(const FVoxelInt64Buffer& Value) const
	{
		FVoxelInt64PointBuffer Result;
		Result.X = Value;
		Result.Y = Value;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64VectorBuffer Int64ToInt64Vector(const FVoxelInt64Buffer& Value) const
	{
		FVoxelInt64VectorBuffer Result;
		Result.X = Value;
		Result.Y = Value;
		Result.Z = Value;
		return Result;
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt64Vector4Buffer Int64ToInt64Vector4(const FVoxelInt64Buffer& Value) const
	{
		FVoxelInt64Vector4Buffer Result;
		Result.X = Value;
		Result.Y = Value;
		Result.Z = Value;
		Result.W = Value;
		return Result;
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelInt32Buffer BooleanToInt32(const FVoxelBoolBuffer& Value) const
	{
		return FVoxelBufferConversionUtilities::BooleanToInt32(Value);
	}

	UFUNCTION(meta = (Autocast))
	FVoxelInt32Buffer ByteToInt32(const FVoxelByteBuffer& Value) const
	{
		return FVoxelBufferConversionUtilities::ByteToInt32(Value);
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelFloatBuffer Int32ToFloat(const FVoxelInt32Buffer& Value) const
	{
		return FVoxelBufferConversionUtilities::Int32ToFloat(Value);
	}
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleBuffer Int32ToDouble(const FVoxelInt32Buffer& Value) const
	{
		return FVoxelBufferConversionUtilities::Int32ToDouble(Value);
	}
	UFUNCTION(meta = (Autocast))
	FVoxelInt64Buffer Int32ToInt64(const FVoxelInt32Buffer& Value) const
	{
		return FVoxelBufferConversionUtilities::Int32ToInt64(Value);
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelFloatBuffer Int64ToFloat(const FVoxelInt64Buffer& Value) const
	{
		return FVoxelBufferConversionUtilities::Int64ToFloat(Value);
	}
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleBuffer Int64ToDouble(const FVoxelInt64Buffer& Value) const
	{
		return FVoxelBufferConversionUtilities::Int64ToDouble(Value);
	}
	UFUNCTION(meta = (Autocast))
	FVoxelInt32Buffer Int64ToInt32(const FVoxelInt64Buffer& Value) const
	{
		return FVoxelBufferConversionUtilities::Int64ToInt32(Value);
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelDoubleBuffer FloatToDouble(const FVoxelFloatBuffer& Value) const
	{
		return FVoxelBufferConversionUtilities::FloatToDouble(Value);
	}
	UFUNCTION(meta = (Autocast))
	FVoxelFloatBuffer DoubleToFloat(const FVoxelDoubleBuffer& Value) const
	{
		return FVoxelBufferConversionUtilities::DoubleToFloat(Value);
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelSeedBuffer Int32ToSeed(const FVoxelInt32Buffer& Value) const
	{
		return Value.ReinterpretAs<FVoxelSeedBuffer>();
	}
	UFUNCTION(meta = (Autocast))
	FVoxelInt32Buffer SeedToInt32(const FVoxelSeedBuffer& Value) const
	{
		return Value.ReinterpretAs<FVoxelInt32Buffer>();
	}

public:
	UFUNCTION(meta = (Autocast))
	FVoxelVectorBuffer NormalToVector(const FVoxelNormalBuffer& Value) const
	{
		FVoxelVectorBuffer Result;
		ReinterpretCastPtr<UVoxelMathFunctionLibrary>(this)->BreakNormal(
			Value,
			Result.X,
			Result.Y,
			Result.Z);
		return Result;
	}
	UFUNCTION(meta = (Autocast))
	FVoxelNormalBuffer VectorToNormal(const FVoxelVectorBuffer& Value) const
	{
		return ReinterpretCastPtr<UVoxelMathFunctionLibrary>(this)->MakeNormal(
			Value.X,
			Value.Y,
			Value.Z);
	}
};