// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelFloatMetadataRef.h"
#include "VoxelFloatMetadata.h"
#include "VoxelPCGUtilities.h"
#include "Buffer/VoxelBaseBuffers.h"

DEFINE_VOXEL_METADATA_REF(FVoxelFloatMetadataRef);

void FVoxelFloatMetadataRef::Blend(
	const FVoxelFloatBuffer& Value,
	const FVoxelFloatBuffer& Alpha,
	FVoxelFloatBuffer& InOutResult)
{
	VOXEL_FUNCTION_COUNTER_NUM(InOutResult.Num(), 128);
	checkVoxelSlow(Value.IsConstant() || Value.Num() == InOutResult.Num());
	checkVoxelSlow(Alpha.IsConstant() || Alpha.Num() == InOutResult.Num());

	for (int32 Index = 0; Index < InOutResult.Num(); Index++)
	{
		float& ResultRef = InOutResult.View()[Index];

		ResultRef = FMath::Lerp(
			ResultRef,
			Value[Index],
			FMath::Clamp(Alpha[Index], 0.f, 1.f));
	}
}

void FVoxelFloatMetadataRef::IndirectBlend(
	const TConstVoxelArrayView<int32> IndexToResult,
	const FVoxelFloatBuffer& Value,
	const FVoxelFloatBuffer& Alpha,
	FVoxelFloatBuffer& InOutResult)
{
	VOXEL_FUNCTION_COUNTER_NUM(IndexToResult.Num(), 128);
	checkVoxelSlow(Value.IsConstant() || Value.Num() == IndexToResult.Num());
	checkVoxelSlow(Alpha.IsConstant() || Alpha.Num() == IndexToResult.Num());

	for (int32 Index = 0; Index < IndexToResult.Num(); Index++)
	{
		float& ResultRef = InOutResult.View()[IndexToResult[Index]];

		ResultRef = FMath::Lerp(
			ResultRef,
			Value[Index],
			FMath::Clamp(Alpha[Index], 0.f, 1.f));
	}
}

void FVoxelFloatMetadataRef::AddToPCG(
	UPCGMetadata& PCGMetadata,
	const TConstVoxelArrayView<FPCGPoint> Points,
	const FName Name,
	const FVoxelFloatBuffer& Values)
{
	VOXEL_FUNCTION_COUNTER_NUM(Points.Num());
	checkVoxelSlow(Values.IsConstant() || Values.Num() == Points.Num());

	FVoxelFloatBuffer LocalValues = Values;
	LocalValues.ExpandConstantIfNeeded(Points.Num());

	FVoxelPCGUtilities::AddAttribute<float>(
		PCGMetadata,
		Points,
		Name,
		LocalValues.View());
}

void FVoxelFloatMetadataRef::WriteMaterialData(
	const FVoxelFloatBuffer& Values,
	const TVoxelArrayView<uint8> OutBytes,
	const EVoxelMetadataMaterialType MaterialType)
{
	if (MaterialType == EVoxelMetadataMaterialType::Float32_1)
	{
		const TVoxelArrayView<float> OutFloats = OutBytes.ReinterpretAs<float>();
		VOXEL_FUNCTION_COUNTER_NUM(OutFloats.Num());
		checkVoxelSlow(Values.IsConstant() || Values.Num() == OutFloats.Num());

		if (Values.IsConstant())
		{
			FVoxelUtilities::SetAll(OutFloats, Values.GetConstant());
		}
		else
		{
			FVoxelUtilities::Memcpy(OutFloats, Values.View());
		}
	}
	else if (MaterialType == EVoxelMetadataMaterialType::Float16_1)
	{
		const TVoxelArrayView<FFloat16> OutFloats = OutBytes.ReinterpretAs<FFloat16>();
		VOXEL_FUNCTION_COUNTER_NUM(OutFloats.Num());
		checkVoxelSlow(Values.IsConstant() || Values.Num() == OutFloats.Num());

		if (Values.IsConstant())
		{
			FVoxelUtilities::SetAll(OutFloats, FFloat16(Values.GetConstant()));
		}
		else
		{
			for (int32 Index = 0; Index < OutFloats.Num(); Index++)
			{
				OutFloats[Index] = FFloat16(Values[Index]);
			}
		}
	}
	else
	{
		check(MaterialType == EVoxelMetadataMaterialType::Float8_1);

		VOXEL_FUNCTION_COUNTER_NUM(OutBytes.Num());
		checkVoxelSlow(Values.IsConstant() || Values.Num() == OutBytes.Num());

		for (int32 Index = 0; Index < OutBytes.Num(); Index++)
		{
			OutBytes[Index] = FVoxelUtilities::FloatToUINT8(Values[Index]);
		}
	}
}

FVoxelFloatBuffer FVoxelFloatMetadataRef::GetChannel(
	const FVoxelFloatBuffer& Buffer,
	const EVoxelTextureChannel Channel)
{
	return Buffer;
}