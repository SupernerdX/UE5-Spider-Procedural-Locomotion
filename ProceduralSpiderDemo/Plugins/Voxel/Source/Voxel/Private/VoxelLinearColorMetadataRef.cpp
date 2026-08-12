// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelLinearColorMetadataRef.h"
#include "VoxelLinearColorMetadata.h"
#include "VoxelPCGUtilities.h"
#include "Buffer/VoxelFloatBuffers.h"

DEFINE_VOXEL_METADATA_REF(FVoxelLinearColorMetadataRef);

void FVoxelLinearColorMetadataRef::Blend(
	const FVoxelLinearColorBuffer& Value,
	const FVoxelFloatBuffer& Alpha,
	FVoxelLinearColorBuffer& InOutResult)
{
	VOXEL_FUNCTION_COUNTER_NUM(InOutResult.Num(), 128);
	checkVoxelSlow(Value.IsConstant() || Value.Num() == InOutResult.Num());
	checkVoxelSlow(Alpha.IsConstant() || Alpha.Num() == InOutResult.Num());

	for (int32 Index = 0; Index < InOutResult.Num(); Index++)
	{
		InOutResult.Set(Index, FMath::Lerp(
			InOutResult[Index],
			Value[Index],
			FMath::Clamp(Alpha[Index], 0.f, 1.f)));
	}
}

void FVoxelLinearColorMetadataRef::IndirectBlend(
	const TConstVoxelArrayView<int32> IndexToResult,
	const FVoxelLinearColorBuffer& Value,
	const FVoxelFloatBuffer& Alpha,
	FVoxelLinearColorBuffer& InOutResult)
{
	VOXEL_FUNCTION_COUNTER_NUM(IndexToResult.Num(), 128);
	checkVoxelSlow(Value.IsConstant() || Value.Num() == IndexToResult.Num());
	checkVoxelSlow(Alpha.IsConstant() || Alpha.Num() == IndexToResult.Num());

	for (int32 Index = 0; Index < IndexToResult.Num(); Index++)
	{
		const int32 ResultIndex = IndexToResult[Index];

		InOutResult.Set(ResultIndex, FMath::Lerp(
			InOutResult[ResultIndex],
			Value[Index],
			FMath::Clamp(Alpha[Index], 0.f, 1.f)));
	}
}

void FVoxelLinearColorMetadataRef::AddToPCG(
	UPCGMetadata& PCGMetadata,
	const TConstVoxelArrayView<FPCGPoint> Points,
	const FName Name,
	const FVoxelLinearColorBuffer& Values)
{
	VOXEL_FUNCTION_COUNTER_NUM(Points.Num());
	checkVoxelSlow(Values.IsConstant() || Values.Num() == Points.Num());

	TVoxelArray<FVector4> Colors;
	FVoxelUtilities::SetNumFast(Colors, Points.Num());

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		Colors[Index] = FVector4(Values[Index]);
	}

	FVoxelPCGUtilities::AddAttribute<FVector4>(
		PCGMetadata,
		Points,
		Name,
		Colors);
}

void FVoxelLinearColorMetadataRef::WriteMaterialData(
	const FVoxelLinearColorBuffer& Values,
	const TVoxelArrayView<uint8> OutBytes,
	const EVoxelMetadataMaterialType MaterialType)
{
	if (MaterialType == EVoxelMetadataMaterialType::Float32_4)
	{
		const TVoxelArrayView<FLinearColor> OutLinearColors = OutBytes.ReinterpretAs<FLinearColor>();
		VOXEL_FUNCTION_COUNTER_NUM(OutLinearColors.Num());
		checkVoxelSlow(Values.IsConstant() || Values.Num() == OutLinearColors.Num());

		for (int32 Index = 0; Index < OutLinearColors.Num(); Index++)
		{
			OutLinearColors[Index] = Values[Index];
		}
	}
	else if (MaterialType == EVoxelMetadataMaterialType::Float16_4)
	{
		const TVoxelArrayView<FFloat16Color> OutFloats = OutBytes.ReinterpretAs<FFloat16Color>();
		VOXEL_FUNCTION_COUNTER_NUM(OutFloats.Num());
		checkVoxelSlow(Values.IsConstant() || Values.Num() == OutFloats.Num());

		if (Values.IsConstant())
		{
			FVoxelUtilities::SetAll(OutFloats, FFloat16Color(Values.GetConstant()));
		}
		else
		{
			for (int32 Index = 0; Index < OutFloats.Num(); Index++)
			{
				OutFloats[Index] = FFloat16Color(Values[Index]);
			}
		}
	}
	else
	{
		check(MaterialType == EVoxelMetadataMaterialType::Float8_4);

		const TVoxelArrayView<FColor> OutFloats = OutBytes.ReinterpretAs<FColor>();
		VOXEL_FUNCTION_COUNTER_NUM(OutFloats.Num());
		checkVoxelSlow(Values.IsConstant() || Values.Num() == OutFloats.Num());

		for (int32 Index = 0; Index < OutFloats.Num(); Index++)
		{
			OutFloats[Index] = Values[Index].ToFColor(false);
		}
	}
}

FVoxelFloatBuffer FVoxelLinearColorMetadataRef::GetChannel(
	const FVoxelLinearColorBuffer& Buffer,
	const EVoxelTextureChannel Channel)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num());

	switch (Channel)
	{
	default: ensure(false);
	case EVoxelTextureChannel::R: return Buffer.R;
	case EVoxelTextureChannel::G: return Buffer.G;
	case EVoxelTextureChannel::B: return Buffer.B;
	case EVoxelTextureChannel::A: return Buffer.A;
	}
}