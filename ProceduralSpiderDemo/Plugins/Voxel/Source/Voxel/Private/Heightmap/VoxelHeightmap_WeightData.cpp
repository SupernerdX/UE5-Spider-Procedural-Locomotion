// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Heightmap/VoxelHeightmap_WeightData.h"
#include "Heightmap/VoxelHeightmap_Weight.h"
#if WITH_EDITOR
#include "Engine/Texture2D.h"
#include "DerivedDataCacheInterface.h"
#endif

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelHeightmap_WeightMemory);

#if WITH_EDITOR
TSharedPtr<const FVoxelHeightmap_WeightData> FVoxelHeightmap_WeightData::Create(const UVoxelHeightmap_Weight& Weightmap)
{
	VOXEL_FUNCTION_COUNTER();

	UTexture2D* Texture = Weightmap.Texture.LoadSynchronous();
	if (!Texture)
	{
		return nullptr;
	}

	const TSharedRef<FVoxelHeightmap_WeightData> Data = MakeShared<FVoxelHeightmap_WeightData>();
	Data->Texture = Texture;
	Data->TextureChannel = Weightmap.TextureChannel;

	const FString DerivedDataKey = INLINE_LAMBDA
	{
		FVoxelWriter Writer;
		Writer << Weightmap.TextureChannel;

		FString KeySuffix;
		KeySuffix += Texture->Source.GetId().ToString();
		KeySuffix += "_" + FVoxelUtilities::BlobToHex(Writer.Bytes);

		return FDerivedDataCacheInterface::BuildCacheKey(
			TEXT("VOXEL_WEIGHTMAP"),
			TEXT("3B7A6A9286674A0596F9314BF635B5B6"),
			*KeySuffix);
	};

	TArray<uint8> DerivedData;
	if (GetDerivedDataCacheRef().GetSynchronous(*DerivedDataKey, DerivedData, Weightmap.GetPathName()))
	{
		FVoxelReader Reader(DerivedData);
		Data->Serialize(Reader, nullptr);
		return Data;
	}

	if (!Data->CreateImpl())
	{
		return nullptr;
	}

	{
		FVoxelWriter Writer;
		Data->Serialize(Writer, nullptr);
		GetDerivedDataCacheRef().Put(*DerivedDataKey, Writer.Bytes, Weightmap.GetPathName());
	}

	return Data;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightmap_WeightData::Serialize(FArchive& Ar, UObject* Owner)
{
	VOXEL_FUNCTION_COUNTER();

	int32 Version = 0;
	Ar << Version;
	ensure(Version == 0);

	Ar << SizeX;
	Ar << SizeY;
	FVoxelUtilities::SerializeBulkData(Owner, BulkData, Ar, Weights);

	check(Weights.Num() == SizeX * SizeY);

	UpdateStats();
}

int64 FVoxelHeightmap_WeightData::GetAllocatedSize() const
{
	return Weights.GetAllocatedSize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool FVoxelHeightmap_WeightData::CreateImpl()
{
	VOXEL_FUNCTION_COUNTER();

	const UTexture2D* TextureObject = Texture.LoadSynchronous();
	if (!ensure(TextureObject))
	{
		return false;
	}

	TVoxelArray<float> Values;
	if (!FVoxelTextureUtilities::ExtractTextureChannel(
		*TextureObject,
		TextureChannel,
		SizeX,
		SizeY,
		Values))
	{
		return false;
	}

	FVoxelUtilities::SetNumFast(Weights, SizeX * SizeY);

	for (int32 Index = 0; Index < SizeX * SizeY; Index++)
	{
		Weights[Index] = FVoxelUtilities::ClampToUINT8(FMath::RoundToInt(MAX_uint8 * Values[Index]));
	}

	return true;
}
#endif