// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Texture/VoxelTextureData.h"
#include "Texture/VoxelTexture.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Net/Serialization/FastArraySerializer.h"
#if WITH_EDITOR
#include "Engine/Texture2D.h"
#endif

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelTextureMemory);

EVoxelTextureDataType GetVoxelTextureDataType(const FVoxelBuffer& Buffer)
{
	if (Buffer.IsA<FVoxelByteBuffer>())
	{
		return EVoxelTextureDataType::Byte;
	}

	if (Buffer.IsA<FVoxelColorBuffer>())
	{
		return EVoxelTextureDataType::Color;
	}

	if (Buffer.IsA<FVoxelLinearColorBuffer>())
	{
		return EVoxelTextureDataType::LinearColor;
	}

	if (Buffer.IsA<FVoxelUInt16Buffer>())
	{
		return EVoxelTextureDataType::UINT16;
	}

	if (Buffer.IsA<FVoxelFloatBuffer>())
	{
		return EVoxelTextureDataType::Float;
	}

	check(false);
	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float FVoxelTextureData::Sample(
	const FVector2D& Position,
	const EVoxelTextureChannel Channel) const
{
	checkVoxelSlow(Type == GetVoxelTextureDataType(*Buffer));

	const double X = Position.X;
	const double Y = Position.Y;

	const int64 MinX = FMath::FloorToInt(X);
	const int64 MinY = FMath::FloorToInt(Y);

	const int64 MaxX = FMath::CeilToInt(X);
	const int64 MaxY = FMath::CeilToInt(Y);

	const float AlphaX = float(X - MinX);
	const float AlphaY = float(Y - MinY);

	const bool bIsPowerOfTwo =
		FMath::IsPowerOfTwo(SizeX) &&
		FMath::IsPowerOfTwo(SizeY);

	const auto Sample = [&](const int64 SampleX, const int64 SampleY)
	{
		int32 SafeX;
		int32 SafeY;
		if (bIsPowerOfTwo)
		{
			SafeX = SampleX & int64(SizeX - 1);
			SafeY = SampleY & int64(SizeY - 1);
		}
		else
		{
			SafeX = FVoxelUtilities::PositiveMod(SampleX, int64(SizeX));
			SafeY = FVoxelUtilities::PositiveMod(SampleY, int64(SizeY));
		}

		checkVoxelSlow(SafeX == FVoxelUtilities::PositiveMod(SampleX, int64(SizeX)));
		checkVoxelSlow(SafeY == FVoxelUtilities::PositiveMod(SampleY, int64(SizeY)));

		const int32 Index = FVoxelUtilities::Get2DIndex<int32>(SizeX, SizeY, SafeX, SafeY);

		switch (Type)
		{
		default: VOXEL_ASSUME(false);
		case EVoxelTextureDataType::Byte: return FVoxelUtilities::UINT8ToFloat(Buffer->AsChecked<FVoxelByteBuffer>()[Index]);
		case EVoxelTextureDataType::Color: return FLinearColor(Buffer->AsChecked<FVoxelColorBuffer>()[Index]).Component(int32(Channel));
		case EVoxelTextureDataType::LinearColor: return Buffer->AsChecked<FVoxelLinearColorBuffer>()[Index].Component(int32(Channel));
		case EVoxelTextureDataType::UINT16: return FVoxelUtilities::UINT16ToFloat(Buffer->AsChecked<FVoxelUInt16Buffer>()[Index]);
		case EVoxelTextureDataType::Float: return Buffer->AsChecked<FVoxelFloatBuffer>()[Index];
		}
	};

	return FVoxelUtilities::BilinearInterpolation(
		Sample(MinX, MinY),
		Sample(MaxX, MinY),
		Sample(MinX, MaxY),
		Sample(MaxX, MaxY),
		AlphaX,
		AlphaY);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
TSharedPtr<const FVoxelTextureData> FVoxelTextureData::Create_EditorOnly(const UVoxelTexture& VoxelTexture)
{
	VOXEL_FUNCTION_COUNTER();

	UTexture2D* Texture = VoxelTexture.Texture.LoadSynchronous();
	if (!Texture)
	{
		return {};
	}

	int32 SizeX = 0;
	int32 SizeY = 0;
	const TSharedPtr<FVoxelBuffer> Buffer = CreateImpl_EditorOnly(
		VoxelTexture,
		SizeX,
		SizeY);

	if (!Buffer)
	{
		return {};
	}

	const TSharedRef<const FVoxelTextureData> Data = CreateFromBuffer(SizeX, SizeY, Buffer.ToSharedRef());
	Data->Texture = Texture;
	return Data;
}
#endif

TSharedRef<const FVoxelTextureData> FVoxelTextureData::CreateFromBuffer(
	const int32 SizeX,
	const int32 SizeY,
	const TSharedRef<FVoxelBuffer>& Buffer)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelTextureData> Data = MakeShared<FVoxelTextureData>();
	Data->SizeX = SizeX;
	Data->SizeY = SizeY;
	Data->Buffer = Buffer;
	Data->Type = GetVoxelTextureDataType(*Buffer);
	Data->UpdateStats();
	return Data;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelTextureData::Serialize(FArchive& Ar, UObject* Owner)
{
	VOXEL_FUNCTION_COUNTER();

	ON_SCOPE_EXIT
	{
		UpdateStats();
	};

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;
	ensure(Version == FVersion::LatestVersion);

	Ar << SizeX;
	Ar << SizeY;

	UScriptStruct* BufferStruct = nullptr;
	if (Ar.IsSaving())
	{
		check(Buffer);
		BufferStruct = Buffer->GetStruct();
	}

	FVoxelUtilities::SerializeStruct(Ar, BufferStruct);

	if (!ensureVoxelSlow(BufferStruct))
	{
		SizeX = 0;
		SizeY = 0;
		Buffer.Reset();
		return;
	}

	if (Ar.IsLoading())
	{
		Buffer = MakeSharedStruct<FVoxelBuffer>(BufferStruct);
	}

	Buffer->SerializeData(Ar);

	if (Ar.IsLoading())
	{
		Type = GetVoxelTextureDataType(*Buffer);
	}
	else
	{
		ensure(Type == GetVoxelTextureDataType(*Buffer));
	}
}

int64 FVoxelTextureData::GetAllocatedSize() const
{
	if (!Buffer)
	{
		return 0;
	}

	return Buffer->GetAllocatedSize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
TSharedPtr<FVoxelBuffer> FVoxelTextureData::CreateImpl_EditorOnly(
	const UVoxelTexture& VoxelTexture,
	int32& OutSizeX,
	int32& OutSizeY)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	UTexture2D* Texture2D = VoxelTexture.Texture.LoadSynchronous();
	if (!ensure(Texture2D))
	{
		return {};
	}

	FTextureSource& Source = Texture2D->Source;
	if (!ensureVoxelSlow(Source.IsValid()))
	{
		return {};
	}

	// Flush async tasks to ensure FTextureSource::GetMipData is not called while we extract data
	Texture2D->BlockOnAnyAsyncBuild();

	OutSizeX = Source.GetSizeX();
	OutSizeY = Source.GetSizeY();

	const TConstVoxelArrayView64<uint8> SourceByteData(
		Source.LockMipReadOnly(0, 0, 0),
		Source.CalcMipSize(0, 0, 0));

	const TSharedPtr<FVoxelBuffer> Buffer = CreateBufferFromTexture(
		SourceByteData,
		Source.GetFormat(),
		Source.GetSizeX(),
		Source.GetSizeY());

	Source.UnlockMip(0, 0, 0);

	return Buffer;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelBuffer> FVoxelTextureData::CreateBufferFromTexture(
	const TConstVoxelArrayView64<uint8> SourceByteData,
	const ETextureSourceFormat Format,
	const int32 SizeX,
	const int32 SizeY)
{
	VOXEL_FUNCTION_COUNTER_NUM(SizeX * SizeY);

	const int32 NumPixels = SizeX * SizeY;

	switch (Format)
	{
	default:
	{
		ensureVoxelSlow(false);
		VOXEL_MESSAGE(Error, "Unsupported texture format: {0}", UEnum::GetValueAsString(Format));
		return {};
	}
	case TSF_G8:
	{
		if (!ensure(SourceByteData.Num() == NumPixels))
		{
			return {};
		}

		const TSharedRef<FVoxelByteBuffer> Result = MakeShared<FVoxelByteBuffer>();
		Result->Allocate(NumPixels);

		FVoxelUtilities::Memcpy(Result->View(), SourceByteData);

		return Result;
	}
	case TSF_BGRA8:
	case TSF_BGRE8:
	{
		const TConstVoxelArrayView64<FColor> SourceData = SourceByteData.ReinterpretAs<FColor>();
		if (!ensure(SourceData.Num() == NumPixels))
		{
			return {};
		}

		const TSharedRef<FVoxelColorBuffer> Result = MakeShared<FVoxelColorBuffer>();
		Result->Allocate(NumPixels);

		FVoxelUtilities::Memcpy(Result->View(), SourceData);

		return Result;
	}
	case TSF_RGBA16:
	{
		struct FColor16
		{
			uint16 R;
			uint16 G;
			uint16 B;
			uint16 A;

			FORCEINLINE operator FLinearColor() const
			{
				return FLinearColor(R, G, B, A);
			}
		};

		const TConstVoxelArrayView64<FColor16> SourceData = SourceByteData.ReinterpretAs<FColor16>();
		if (!ensure(SourceData.Num() == NumPixels))
		{
			return {};
		}

		const TSharedRef<FVoxelLinearColorBuffer> Result = MakeShared<FVoxelLinearColorBuffer>();
		Result->Allocate(NumPixels);

		for (int32 Index = 0; Index < NumPixels; Index++)
		{
			Result->Set(Index, FLinearColor(SourceData[Index]));
		}

		return Result;
	}
	case TSF_RGBA16F:
	{
		struct FColor16F
		{
			FFloat16 R;
			FFloat16 G;
			FFloat16 B;
			FFloat16 A;

			FORCEINLINE operator FLinearColor() const
			{
				return FLinearColor(R, G, B, A);
			}
		};

		const TConstVoxelArrayView64<FColor16F> SourceData = SourceByteData.ReinterpretAs<FColor16F>();
		if (!ensure(SourceData.Num() == NumPixels))
		{
			return {};
		}

		const TSharedRef<FVoxelLinearColorBuffer> Result = MakeShared<FVoxelLinearColorBuffer>();
		Result->Allocate(NumPixels);

		for (int32 Index = 0; Index < NumPixels; Index++)
		{
			Result->Set(Index, FLinearColor(SourceData[Index]));
		}

		return Result;
	}
	case TSF_G16:
	{
		const TConstVoxelArrayView64<uint16> SourceData = SourceByteData.ReinterpretAs<uint16>();
		if (!ensure(SourceData.Num() == NumPixels))
		{
			return {};
		}

		const TSharedRef<FVoxelUInt16Buffer> Result = MakeShared<FVoxelUInt16Buffer>();
		Result->Allocate(NumPixels);

		FVoxelUtilities::Memcpy(Result->View(), SourceData);

		return Result;
	}
	case TSF_RGBA32F:
	{
		const TConstVoxelArrayView64<FLinearColor> SourceData = SourceByteData.ReinterpretAs<FLinearColor>();
		if (!ensure(SourceData.Num() == NumPixels))
		{
			return {};
		}

		const TSharedRef<FVoxelLinearColorBuffer> Result = MakeShared<FVoxelLinearColorBuffer>();
		Result->Allocate(NumPixels);

		for (int32 Index = 0; Index < NumPixels; Index++)
		{
			Result->Set(Index, SourceData[Index]);
		}

		return Result;
	}
	case TSF_R16F:
	{
		const TConstVoxelArrayView64<FFloat16> SourceData = SourceByteData.ReinterpretAs<FFloat16>();
		if (!ensure(SourceData.Num() == NumPixels))
		{
			return {};
		}

		const TSharedRef<FVoxelFloatBuffer> Result = MakeShared<FVoxelFloatBuffer>();
		Result->Allocate(NumPixels);

		for (int32 Index = 0; Index < NumPixels; Index++)
		{
			Result->Set(Index, SourceData[Index]);
		}

		return Result;
	}
	case TSF_R32F:
	{
		const TConstVoxelArrayView64<float> SourceData = SourceByteData.ReinterpretAs<float>();
		if (!ensure(SourceData.Num() == NumPixels))
		{
			return {};
		}

		const TSharedRef<FVoxelFloatBuffer> Result = MakeShared<FVoxelFloatBuffer>();
		Result->Allocate(NumPixels);

		FVoxelUtilities::Memcpy(Result->View(), SourceData);

		return Result;
	}
	}
}