// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMetadataMaterialType.h"
#include "VoxelBuffer.h"
#include "VoxelMetadataRef.h"
#include "MaterialCompiler.h"
#include "Materials/MaterialExpressionCustom.h"

int32 FVoxelMetadataMaterialType::GetTypeSize(const EVoxelMetadataMaterialType Type)
{
	switch (Type)
	{
	default: ensure(false);
	case EVoxelMetadataMaterialType::Float8_1: return 1 * sizeof(uint8);
	case EVoxelMetadataMaterialType::Float8_2: return 2 * sizeof(uint8);
	case EVoxelMetadataMaterialType::Float8_3: return 3 * sizeof(uint8);
	case EVoxelMetadataMaterialType::Float8_4: return 4 * sizeof(uint8);
	case EVoxelMetadataMaterialType::Float16_1: return 1 * sizeof(FFloat16);
	case EVoxelMetadataMaterialType::Float16_2: return 2 * sizeof(FFloat16);
	case EVoxelMetadataMaterialType::Float16_3: return 3 * sizeof(FFloat16);
	case EVoxelMetadataMaterialType::Float16_4: return 4 * sizeof(FFloat16);
	case EVoxelMetadataMaterialType::Float32_1: return 1 * sizeof(float);
	case EVoxelMetadataMaterialType::Float32_2: return 2 * sizeof(float);
	case EVoxelMetadataMaterialType::Float32_3: return 3 * sizeof(float);
	case EVoxelMetadataMaterialType::Float32_4: return 4 * sizeof(float);
	case EVoxelMetadataMaterialType::Int1: return 1 * sizeof(int32);
	case EVoxelMetadataMaterialType::Int2: return 2 * sizeof(int32);
	case EVoxelMetadataMaterialType::Int3: return 3 * sizeof(int32);
	case EVoxelMetadataMaterialType::Int4: return 4 * sizeof(int32);
	case EVoxelMetadataMaterialType::Normal: return sizeof(FVoxelOctahedron);
	}
}

EPixelFormat FVoxelMetadataMaterialType::GetPixelFormat(const EVoxelMetadataMaterialType Type)
{
	switch (Type)
	{
	default: ensure(false);
	case EVoxelMetadataMaterialType::Float8_1: return PF_R8;
	case EVoxelMetadataMaterialType::Float8_2: return PF_R8G8;
	case EVoxelMetadataMaterialType::Float8_3: return PF_B8G8R8A8;
	case EVoxelMetadataMaterialType::Float8_4: return PF_B8G8R8A8;
	case EVoxelMetadataMaterialType::Float16_1: return PF_R16F;
	case EVoxelMetadataMaterialType::Float16_2: return PF_G16R16F;
	case EVoxelMetadataMaterialType::Float16_3: return PF_FloatRGBA;
	case EVoxelMetadataMaterialType::Float16_4: return PF_FloatRGBA;
	case EVoxelMetadataMaterialType::Float32_1: return PF_R32_FLOAT;
	case EVoxelMetadataMaterialType::Float32_2: return PF_G32R32F;
	case EVoxelMetadataMaterialType::Float32_3: return PF_A32B32G32R32F;
	case EVoxelMetadataMaterialType::Float32_4: return PF_A32B32G32R32F;
	case EVoxelMetadataMaterialType::Int1: return PF_R32_UINT;
	case EVoxelMetadataMaterialType::Int2: return PF_R32G32_UINT;
	case EVoxelMetadataMaterialType::Int3: return PF_R32G32B32A32_UINT;
	case EVoxelMetadataMaterialType::Int4: return PF_R32G32B32A32_UINT;
	case EVoxelMetadataMaterialType::Normal: return PF_R8G8;
	}
}

EMaterialValueType FVoxelMetadataMaterialType::GetMaterialValueType(const EVoxelMetadataMaterialType Type)
{
	switch (Type)
	{
	default: ensure(false);
	case EVoxelMetadataMaterialType::Float8_1:
	case EVoxelMetadataMaterialType::Float16_1:
	case EVoxelMetadataMaterialType::Float32_1: return MCT_Float;
	case EVoxelMetadataMaterialType::Float8_2:
	case EVoxelMetadataMaterialType::Float16_2:
	case EVoxelMetadataMaterialType::Float32_2: return MCT_Float2;
	case EVoxelMetadataMaterialType::Float8_3:
	case EVoxelMetadataMaterialType::Float16_3:
	case EVoxelMetadataMaterialType::Float32_3: return MCT_Float3;
	case EVoxelMetadataMaterialType::Float8_4:
	case EVoxelMetadataMaterialType::Float16_4:
	case EVoxelMetadataMaterialType::Float32_4: return MCT_Float4;
	case EVoxelMetadataMaterialType::Int1: return MCT_Float;
	case EVoxelMetadataMaterialType::Int2: return MCT_Float2;
	case EVoxelMetadataMaterialType::Int3: return MCT_Float3;
	case EVoxelMetadataMaterialType::Int4: return MCT_Float4;
	case EVoxelMetadataMaterialType::Normal: return MCT_Float3;
	}
}

ECustomMaterialOutputType FVoxelMetadataMaterialType::GetCustomMaterialOutputType(const EVoxelMetadataMaterialType Type)
{
	switch (Type)
	{
	default: ensure(false);
	case EVoxelMetadataMaterialType::Float8_1:
	case EVoxelMetadataMaterialType::Float16_1:
	case EVoxelMetadataMaterialType::Float32_1: return CMOT_Float1;
	case EVoxelMetadataMaterialType::Float8_2:
	case EVoxelMetadataMaterialType::Float16_2:
	case EVoxelMetadataMaterialType::Float32_2: return CMOT_Float2;
	case EVoxelMetadataMaterialType::Float8_3:
	case EVoxelMetadataMaterialType::Float16_3:
	case EVoxelMetadataMaterialType::Float32_3: return CMOT_Float3;
	case EVoxelMetadataMaterialType::Float8_4:
	case EVoxelMetadataMaterialType::Float16_4:
	case EVoxelMetadataMaterialType::Float32_4: return CMOT_Float4;
	case EVoxelMetadataMaterialType::Int1: return CMOT_Float1;
	case EVoxelMetadataMaterialType::Int2: return CMOT_Float2;
	case EVoxelMetadataMaterialType::Int3: return CMOT_Float3;
	case EVoxelMetadataMaterialType::Int4: return CMOT_Float4;
	case EVoxelMetadataMaterialType::Normal: return CMOT_Float3;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
int32 FVoxelMetadataMaterialType::Constant(
	FMaterialCompiler& Compiler,
	const FVoxelMetadataRef& Metadata,
	const EVoxelMetadataMaterialType Type,
	const FVoxelRuntimePinValue& Value)
{
	check(Type == Metadata.GetMaterialType());

	TVoxelArray<uint8> Bytes;
	Bytes.SetNumZeroed(GetTypeSize(Type));

	const TSharedRef<FVoxelBuffer> ConstantBuffer = FVoxelBuffer::MakeConstant(Value);
	Metadata.WriteMaterialData(*ConstantBuffer, Bytes, Type);

	switch (Type)
	{
	default: ensure(false);
	case EVoxelMetadataMaterialType::Float8_1:
	case EVoxelMetadataMaterialType::Float16_1:
	case EVoxelMetadataMaterialType::Float32_1:
	{
		return Compiler.Constant(FVoxelUtilities::CastBytes<float>(Bytes));
	}
	case EVoxelMetadataMaterialType::Float8_2:
	case EVoxelMetadataMaterialType::Float16_2:
	case EVoxelMetadataMaterialType::Float32_2:
	{
		return Compiler.Constant2(
			FVoxelUtilities::CastBytes<FVector2f>(Bytes).X,
			FVoxelUtilities::CastBytes<FVector2f>(Bytes).Y);
	}
	case EVoxelMetadataMaterialType::Float8_3:
	case EVoxelMetadataMaterialType::Float16_3:
	case EVoxelMetadataMaterialType::Float32_3:
	{
		return Compiler.Constant3(
			FVoxelUtilities::CastBytes<FVector3f>(Bytes).X,
			FVoxelUtilities::CastBytes<FVector3f>(Bytes).Y,
			FVoxelUtilities::CastBytes<FVector3f>(Bytes).Z);
	}
	case EVoxelMetadataMaterialType::Float8_4:
	case EVoxelMetadataMaterialType::Float16_4:
	case EVoxelMetadataMaterialType::Float32_4:
	{
		return Compiler.Constant4(
			FVoxelUtilities::CastBytes<FVector4f>(Bytes).X,
			FVoxelUtilities::CastBytes<FVector4f>(Bytes).Y,
			FVoxelUtilities::CastBytes<FVector4f>(Bytes).Z,
			FVoxelUtilities::CastBytes<FVector4f>(Bytes).W);
	}
	case EVoxelMetadataMaterialType::Int1:
	{
		return Compiler.Constant(FVoxelUtilities::CastBytes<int32>(Bytes));
	}
	case EVoxelMetadataMaterialType::Int2:
	{
		return Compiler.Constant2(
			FVoxelUtilities::CastBytes<FIntVector2>(Bytes).X,
			FVoxelUtilities::CastBytes<FIntVector2>(Bytes).Y);
	}
	case EVoxelMetadataMaterialType::Int3:
	{
		return Compiler.Constant3(
			FVoxelUtilities::CastBytes<FIntVector>(Bytes).X,
			FVoxelUtilities::CastBytes<FIntVector>(Bytes).Y,
			FVoxelUtilities::CastBytes<FIntVector>(Bytes).Z);
	}
	case EVoxelMetadataMaterialType::Int4:
	{
		return Compiler.Constant4(
			FVoxelUtilities::CastBytes<FIntVector4>(Bytes).X,
			FVoxelUtilities::CastBytes<FIntVector4>(Bytes).Y,
			FVoxelUtilities::CastBytes<FIntVector4>(Bytes).Z,
			FVoxelUtilities::CastBytes<FIntVector4>(Bytes).W);
	}
	case EVoxelMetadataMaterialType::Normal:
	{
		return Compiler.Constant3(
			FVoxelUtilities::CastBytes<FVoxelOctahedron>(Bytes).GetUnitVector().X,
			FVoxelUtilities::CastBytes<FVoxelOctahedron>(Bytes).GetUnitVector().Y,
			FVoxelUtilities::CastBytes<FVoxelOctahedron>(Bytes).GetUnitVector().Z);
	}
	}
}

FString FVoxelMetadataMaterialType::Constant(
	const FVoxelMetadataRef& Metadata,
	const EVoxelMetadataMaterialType Type,
	const FVoxelRuntimePinValue& Value)
{
	check(Type == Metadata.GetMaterialType());

	TVoxelArray<uint8> Bytes;
	Bytes.SetNumZeroed(GetTypeSize(Type));

	const TSharedRef<FVoxelBuffer> ConstantBuffer = FVoxelBuffer::MakeConstant(Value);
	Metadata.WriteMaterialData(*ConstantBuffer, Bytes, Type);

	switch (Type)
	{
	default: ensure(false);
	case EVoxelMetadataMaterialType::Float8_1:
	case EVoxelMetadataMaterialType::Float16_1:
	case EVoxelMetadataMaterialType::Float32_1:
	{
		return FString::Printf(TEXT("float(%f)"), FVoxelUtilities::CastBytes<float>(Bytes));
	}
	case EVoxelMetadataMaterialType::Float8_2:
	case EVoxelMetadataMaterialType::Float16_2:
	case EVoxelMetadataMaterialType::Float32_2:
	{
		return FString::Printf(TEXT("float2(%f, %f)"),
			FVoxelUtilities::CastBytes<FVector2f>(Bytes).X,
			FVoxelUtilities::CastBytes<FVector2f>(Bytes).Y);
	}
	case EVoxelMetadataMaterialType::Float8_3:
	case EVoxelMetadataMaterialType::Float16_3:
	case EVoxelMetadataMaterialType::Float32_3:
	{
		return FString::Printf(TEXT("float3(%f, %f, %f)"),
			FVoxelUtilities::CastBytes<FVector3f>(Bytes).X,
			FVoxelUtilities::CastBytes<FVector3f>(Bytes).Y,
			FVoxelUtilities::CastBytes<FVector3f>(Bytes).Z);
	}
	case EVoxelMetadataMaterialType::Float8_4:
	case EVoxelMetadataMaterialType::Float16_4:
	case EVoxelMetadataMaterialType::Float32_4:
	{
		return FString::Printf(TEXT("float4(%f, %f, %f, %f)"),
			FVoxelUtilities::CastBytes<FVector4f>(Bytes).X,
			FVoxelUtilities::CastBytes<FVector4f>(Bytes).Y,
			FVoxelUtilities::CastBytes<FVector4f>(Bytes).Z,
			FVoxelUtilities::CastBytes<FVector4f>(Bytes).W);
	}
	case EVoxelMetadataMaterialType::Int1:
	{
		return FString::Printf(TEXT("int(%d)"), FVoxelUtilities::CastBytes<int32>(Bytes));
	}
	case EVoxelMetadataMaterialType::Int2:
	{
		return FString::Printf(TEXT("int2(%d, %d)"),
			FVoxelUtilities::CastBytes<FIntVector2>(Bytes).X,
			FVoxelUtilities::CastBytes<FIntVector2>(Bytes).Y);
	}
	case EVoxelMetadataMaterialType::Int3:
	{
		return FString::Printf(TEXT("int(%d, %d, %d)"),
			FVoxelUtilities::CastBytes<FIntVector>(Bytes).X,
			FVoxelUtilities::CastBytes<FIntVector>(Bytes).Y,
			FVoxelUtilities::CastBytes<FIntVector>(Bytes).Z);
	}
	case EVoxelMetadataMaterialType::Int4:
	{
		return FString::Printf(TEXT("int(%d, %d, %d, %d)"),
			FVoxelUtilities::CastBytes<FIntVector4>(Bytes).X,
			FVoxelUtilities::CastBytes<FIntVector4>(Bytes).Y,
			FVoxelUtilities::CastBytes<FIntVector4>(Bytes).Z,
			FVoxelUtilities::CastBytes<FIntVector4>(Bytes).W);
	}
	case EVoxelMetadataMaterialType::Normal:
	{
		return FString::Printf(TEXT("float3(%f, %f, %f)"),
			FVoxelUtilities::CastBytes<FVoxelOctahedron>(Bytes).GetUnitVector().X,
			FVoxelUtilities::CastBytes<FVoxelOctahedron>(Bytes).GetUnitVector().Y,
			FVoxelUtilities::CastBytes<FVoxelOctahedron>(Bytes).GetUnitVector().Z);
	}
	}
}
#endif