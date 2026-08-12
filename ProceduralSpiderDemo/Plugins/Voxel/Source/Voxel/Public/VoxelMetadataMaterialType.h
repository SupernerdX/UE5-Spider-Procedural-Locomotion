// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelMetadataRef;
struct FVoxelRuntimePinValue;
enum EMaterialValueType : UE_506_SWITCH(int32, uint64);
enum ECustomMaterialOutputType : int32;

enum class EVoxelMetadataMaterialType : uint8
{
	Float8_1,
	Float8_2,
	Float8_3,
	Float8_4,
	Float16_1,
	Float16_2,
	Float16_3,
	Float16_4,
	Float32_1,
	Float32_2,
	Float32_3,
	Float32_4,
	Int1,
	Int2,
	Int3,
	Int4,
	Normal,
};

struct VOXEL_API FVoxelMetadataMaterialType
{
	static int32 GetTypeSize(EVoxelMetadataMaterialType Type);
	static EPixelFormat GetPixelFormat(EVoxelMetadataMaterialType Type);
	static EMaterialValueType GetMaterialValueType(EVoxelMetadataMaterialType Type);
	static ECustomMaterialOutputType GetCustomMaterialOutputType(EVoxelMetadataMaterialType Type);

#if WITH_EDITOR

	static int32 Constant(
		FMaterialCompiler& Compiler,
		const FVoxelMetadataRef& Metadata,
		EVoxelMetadataMaterialType Type,
		const FVoxelRuntimePinValue& Value);

	static FString Constant(
		const FVoxelMetadataRef& Metadata,
		EVoxelMetadataMaterialType Type,
		const FVoxelRuntimePinValue& Value);
#endif
};