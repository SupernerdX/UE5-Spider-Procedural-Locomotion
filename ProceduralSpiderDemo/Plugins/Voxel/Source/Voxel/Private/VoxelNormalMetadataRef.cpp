// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNormalMetadataRef.h"
#include "VoxelNormalMetadata.h"
#include "VoxelPCGUtilities.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Buffer/VoxelNormalBuffer.h"

DEFINE_VOXEL_METADATA_REF(FVoxelNormalMetadataRef);

void FVoxelNormalMetadataRef::Blend(
	const FVoxelNormalBuffer& Value,
	const FVoxelFloatBuffer& Alpha,
	FVoxelNormalBuffer& InOutResult)
{
	VOXEL_FUNCTION_COUNTER_NUM(InOutResult.Num(), 128);
	checkVoxelSlow(Value.IsConstant() || Value.Num() == InOutResult.Num());
	checkVoxelSlow(Alpha.IsConstant() || Alpha.Num() == InOutResult.Num());

	for (int32 Index = 0; Index < InOutResult.Num(); Index++)
	{
		InOutResult.Set(Index, FVoxelOctahedron(FMath::Lerp(
			InOutResult[Index].GetUnitVector(),
			Value[Index].GetUnitVector(),
			FMath::Clamp(Alpha[Index], 0.f, 1.f)).GetSafeNormal()));
	}
}

void FVoxelNormalMetadataRef::IndirectBlend(
	const TConstVoxelArrayView<int32> IndexToResult,
	const FVoxelNormalBuffer& Value,
	const FVoxelFloatBuffer& Alpha,
	FVoxelNormalBuffer& InOutResult)
{
	VOXEL_FUNCTION_COUNTER_NUM(IndexToResult.Num(), 128);
	checkVoxelSlow(Value.IsConstant() || Value.Num() == IndexToResult.Num());
	checkVoxelSlow(Alpha.IsConstant() || Alpha.Num() == IndexToResult.Num());

	for (int32 Index = 0; Index < IndexToResult.Num(); Index++)
	{
		const int32 ResultIndex = IndexToResult[Index];

		InOutResult.Set(ResultIndex, FVoxelOctahedron(FMath::Lerp(
			InOutResult[ResultIndex].GetUnitVector(),
			Value[Index].GetUnitVector(),
			FMath::Clamp(Alpha[Index], 0.f, 1.f)).GetSafeNormal()));
	}
}

void FVoxelNormalMetadataRef::AddToPCG(
	UPCGMetadata& PCGMetadata,
	const TConstVoxelArrayView<FPCGPoint> Points,
	const FName Name,
	const FVoxelNormalBuffer& Values)
{
	VOXEL_FUNCTION_COUNTER_NUM(Points.Num());
	checkVoxelSlow(Values.IsConstant() || Values.Num() == Points.Num());

	TVoxelArray<FVector> Normals;
	FVoxelUtilities::SetNumFast(Normals, Points.Num());

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		Normals[Index] = FVector(Values[Index].GetUnitVector());
	}

	FVoxelPCGUtilities::AddAttribute<FVector>(
		PCGMetadata,
		Points,
		Name,
		Normals);
}

void FVoxelNormalMetadataRef::WriteMaterialData(
	const FVoxelNormalBuffer& Values,
	const TVoxelArrayView<uint8> OutBytes,
	const EVoxelMetadataMaterialType MaterialType)
{
	check(MaterialType == EVoxelMetadataMaterialType::Normal);

	const TVoxelArrayView<FVoxelOctahedron> OutNormals = OutBytes.ReinterpretAs<FVoxelOctahedron>();
	VOXEL_FUNCTION_COUNTER_NUM(OutNormals.Num());
	checkVoxelSlow(Values.IsConstant() || Values.Num() == OutNormals.Num());

	for (int32 Index = 0; Index < OutNormals.Num(); Index++)
	{
		OutNormals[Index] = Values[Index];
	}
}

FVoxelFloatBuffer FVoxelNormalMetadataRef::GetChannel(
	const FVoxelNormalBuffer& Buffer,
	const EVoxelTextureChannel Channel)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num());

	switch (Channel)
	{
	default: ensure(false);
	case EVoxelTextureChannel::R: return Buffer.GetUnitVector().X;
	case EVoxelTextureChannel::G: return Buffer.GetUnitVector().Y;
	case EVoxelTextureChannel::B: return Buffer.GetUnitVector().Z;
	case EVoxelTextureChannel::A: return 0.f;
	}
}