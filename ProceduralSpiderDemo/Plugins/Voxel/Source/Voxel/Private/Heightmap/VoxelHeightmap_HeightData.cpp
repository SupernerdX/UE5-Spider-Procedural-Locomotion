// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Heightmap/VoxelHeightmap_HeightData.h"
#include "Heightmap/VoxelHeightmap_Height.h"
#include "VoxelJumpFlood.h"
#if WITH_EDITOR
#include "Engine/Texture2D.h"
#include "Misc/ScopedSlowTask.h"
#include "DerivedDataCacheInterface.h"
#endif

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelHeightmap_HeightMemory);

#if WITH_EDITOR
TSharedPtr<const FVoxelHeightmap_HeightData> FVoxelHeightmap_HeightData::Create(const UVoxelHeightmap_Height& Height)
{
	VOXEL_FUNCTION_COUNTER();

	UTexture2D* Texture = Height.Texture.LoadSynchronous();
	if (!Texture)
	{
		return nullptr;
	}

	const TSharedRef<FVoxelHeightmap_HeightData> Data = MakeShared<FVoxelHeightmap_HeightData>();
	Data->Texture = Texture;
	Data->TextureChannel = Height.TextureChannel;
	Data->bCompressTo16Bits = Height.bCompressTo16Bits;
	Data->bEnableMinHeight = Height.bEnableMinHeight;
	Data->MinHeight = Height.MinHeight;
	Data->MinHeightSlope = Height.MinHeightSlope;

	const FString DerivedDataKey = INLINE_LAMBDA
	{
		FVoxelWriter Writer;
		Writer << Height.TextureChannel;
		Writer << Height.bCompressTo16Bits;
		Writer << Height.bEnableMinHeight;
		Writer << Height.MinHeight;
		Writer << Height.MinHeightSlope;

		FString KeySuffix;
		KeySuffix += Texture->Source.GetId().ToString();
		KeySuffix += "_" + FVoxelUtilities::BlobToHex(Writer.Bytes);

		return FDerivedDataCacheInterface::BuildCacheKey(
			TEXT("VOXEL_HEIGHTMAP"),
			TEXT("5C3D57EF98C44A578ECD842322B8FF7C"),
			*KeySuffix);
	};

	TArray<uint8> DerivedData;
	if (GetDerivedDataCacheRef().GetSynchronous(*DerivedDataKey, DerivedData, Height.GetPathName()))
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
		GetDerivedDataCacheRef().Put(*DerivedDataKey, Writer.Bytes, Height.GetPathName());
	}

	return Data;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightmap_HeightData::Serialize(FArchive& Ar, UObject* Owner)
{
	VOXEL_FUNCTION_COUNTER();

	int32 Version = 0;
	Ar << Version;
	ensure(Version == 0);

	Ar << SizeX;
	Ar << SizeY;
	Ar << bIsUINT16;
	Ar << InternalScaleZ;
	Ar << InternalOffsetZ;
	Ar << DataMin;
	Ar << DataMax;
	FVoxelUtilities::SerializeBulkData(Owner, BulkData, Ar, RawData);

	const int32 TypeSize = bIsUINT16 ? sizeof(uint16) : sizeof(float);

	check(RawData.Num() % TypeSize == 0);
	check(RawData.Num() / TypeSize == SizeX * SizeY);

	UpdateStats();
}

int64 FVoxelHeightmap_HeightData::GetAllocatedSize() const
{
	return RawData.GetAllocatedSize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool FVoxelHeightmap_HeightData::CreateImpl()
{
	VOXEL_FUNCTION_COUNTER();

	const UTexture2D* TextureObject = Texture.LoadSynchronous();
	if (!ensure(TextureObject))
	{
		return false;
	}

	TVoxelArray<float> SourceHeights;
	if (!FVoxelTextureUtilities::ExtractTextureChannel(
		*TextureObject,
		TextureChannel,
		SizeX,
		SizeY,
		SourceHeights))
	{
		return false;
	}

	FScopedSlowTask SlowTask(1.f, INVTEXT("Updating heightmap"));
	SlowTask.MakeDialog();
	SlowTask.EnterProgressFrame();

	if (!bEnableMinHeight)
	{
		EncodeHeights(SourceHeights);
		return true;
	}

	TVoxelArray<FIntPoint> Positions;
	FVoxelUtilities::SetNumFast(Positions, SizeX * SizeY);

	for (int32 Y = 0; Y < SizeY; Y++)
	{
		for (int32 X = 0; X < SizeX; X++)
		{
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(SizeX, SizeY, X, Y);
			const float Height = SourceHeights[Index];

			if (Height <= MinHeight)
			{
				Positions[Index] = FIntPoint(MAX_int32);
			}
			else
			{
				Positions[Index] = FIntPoint(X, Y);
			}
		}
	}

	FVoxelJumpFlood::JumpFlood2D(FIntPoint(SizeX, SizeY), Positions);

	TVoxelArray64<float> Heights;
	FVoxelUtilities::SetNumFast(Heights, SizeX * SizeY);

	for (int32 Y = 0; Y < SizeY; Y++)
	{
		for (int32 X = 0; X < SizeX; X++)
		{
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(SizeX, SizeY, X, Y);
			const FIntPoint Position = Positions[Index];

			if (Position == FIntPoint(X, Y) ||
				Position == MAX_int32)
			{
				Heights[Index] = SourceHeights[Index];
				continue;
			}

			const float Distance = FVoxelUtilities::Size(Position - FIntPoint(X, Y));
			const int32 PositionIndex = FVoxelUtilities::Get2DIndex<int32>(SizeX, SizeY, Position.X, Position.Y);

			Heights[Index] = SourceHeights[PositionIndex] - MinHeightSlope * Distance;
		}
	}

	EncodeHeights(Heights);
	return true;
}

void FVoxelHeightmap_HeightData::EncodeHeights(const TConstVoxelArrayView<float> Heights)
{
	VOXEL_FUNCTION_COUNTER();
	check(Heights.Num() == SizeX * SizeY);

	if (bCompressTo16Bits)
	{
		const FFloatInterval MinMax = FVoxelUtilities::GetMinMax(Heights);
		const float Min = MinMax.Min;
		const float Max = MinMax.Max;

		TVoxelArray64<uint16> NormalizedHeights;
		FVoxelUtilities::SetNumFast(NormalizedHeights, SizeX * SizeY);

		for (int32 Index = 0; Index < SizeX * SizeY; Index++)
		{
			const float Value = (Heights[Index] - Min) / (Max - Min);
			NormalizedHeights[Index] = FVoxelUtilities::ClampToUINT16(FMath::RoundToInt(MAX_uint16 * Value));
		}

		bIsUINT16 = true;
		InternalScaleZ = Max - Min;
		InternalOffsetZ = Min;
		DataMin = FVoxelUtilities::GetMin(NormalizedHeights);
		DataMax = FVoxelUtilities::GetMax(NormalizedHeights);
		RawData = TVoxelArray64<uint8>(MakeVoxelArrayView(NormalizedHeights).ReinterpretAs<uint8>());
	}
	else
	{
		bIsUINT16 = false;
		InternalScaleZ = 1;
		InternalOffsetZ = 0;
		DataMin = FVoxelUtilities::GetMin(Heights);
		DataMax = FVoxelUtilities::GetMax(Heights);
		RawData = TVoxelArray64<uint8>(Heights.ReinterpretAs<uint8>());
	}
}
#endif