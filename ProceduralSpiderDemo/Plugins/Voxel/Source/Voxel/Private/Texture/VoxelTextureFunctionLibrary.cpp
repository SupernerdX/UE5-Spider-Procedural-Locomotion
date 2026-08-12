// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Texture/VoxelTextureFunctionLibrary.h"
#include "Texture/VoxelTextureData.h"
#include "VoxelTextureFunctionLibraryImpl.ispc.generated.h"
#include "VoxelTextureFunctionLibraryImpl_Color.ispc.generated.h"

bool UVoxelTextureFunctionLibrary::IsValid(const FVoxelTextureRef& Texture) const
{
#if WITH_EDITOR
	if (Texture.Dependency)
	{
		Query->Context.DependencyCollector.AddDependency(*Texture.Dependency);
	}
#endif

	return Texture.TextureData.IsValid();
}

FIntPoint UVoxelTextureFunctionLibrary::GetSize(const FVoxelTextureRef& Texture) const
{
	if (!Texture.TextureData)
	{
		VOXEL_MESSAGE(Error, "{0}: Texture is null", this);
		return FIntPoint(ForceInit);
	}

#if WITH_EDITOR
	if (Texture.Dependency)
	{
		Query->Context.DependencyCollector.AddDependency(*Texture.Dependency);
	}
#endif

	return FIntPoint(
		Texture.TextureData->SizeX,
		Texture.TextureData->SizeY);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelLinearColorBuffer UVoxelTextureFunctionLibrary::SampleTexture(
	const FVoxelTextureRef& Texture,
	const FVoxelVector2DBuffer& Position,
	const float Scale,
	const EVoxelTextureInterpolation Interpolation,
	const bool bSRGB) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Position.Num());

	if (!Texture.TextureData)
	{
		VOXEL_MESSAGE(Error, "{0}: Texture is null", this);
		return DefaultBuffer;
	}

#if WITH_EDITOR
	if (Texture.Dependency)
	{
		Query->Context.DependencyCollector.AddDependency(*Texture.Dependency);
	}
#endif

	const FVoxelTextureData& TextureData = *Texture.TextureData;

	const FVoxelBuffer* GenericBuffer = TextureData.Buffer.Get();
	if (!ensure(GenericBuffer) ||
		!ensure(GenericBuffer->Num_Slow() == TextureData.SizeX * TextureData.SizeY))
	{
		VOXEL_MESSAGE(Error, "{0}: Texture buffer is invalid", this);
		return DefaultBuffer;
	}

	struct FFunctions
	{
		decltype(ispc::VoxelTextureFunctionLibrary_SampleNearest_uint8)* Byte;
		decltype(ispc::VoxelTextureFunctionLibrary_SampleNearest_uint16)* UInt16;
		decltype(ispc::VoxelTextureFunctionLibrary_SampleNearest_float)* Float;
		decltype(ispc::VoxelTextureFunctionLibrary_SampleNearest_Color)* Color;
	};

	const FFunctions Functions = INLINE_LAMBDA
	{
		switch (Interpolation)
		{
		default: ensure(false);
		case EVoxelTextureInterpolation::NearestNeighbor:
		{
			return FFunctions
			{
				ispc::VoxelTextureFunctionLibrary_SampleNearest_uint8,
				ispc::VoxelTextureFunctionLibrary_SampleNearest_uint16,
				ispc::VoxelTextureFunctionLibrary_SampleNearest_float,
				ispc::VoxelTextureFunctionLibrary_SampleNearest_Color
			};
		}
		case EVoxelTextureInterpolation::Bilinear:
		{
			return FFunctions
			{
				ispc::VoxelTextureFunctionLibrary_SampleBilinear_uint8,
				ispc::VoxelTextureFunctionLibrary_SampleBilinear_uint16,
				ispc::VoxelTextureFunctionLibrary_SampleBilinear_float,
				ispc::VoxelTextureFunctionLibrary_SampleBilinear_Color
			};
		}
		case EVoxelTextureInterpolation::Bicubic:
		{
			return FFunctions
			{
				ispc::VoxelTextureFunctionLibrary_SampleBicubic_uint8,
				ispc::VoxelTextureFunctionLibrary_SampleBicubic_uint16,
				ispc::VoxelTextureFunctionLibrary_SampleBicubic_float,
				ispc::VoxelTextureFunctionLibrary_SampleBicubic_Color
			};
		}
		}
	};

	if (const FVoxelByteBuffer* Buffer = GenericBuffer->As<FVoxelByteBuffer>())
	{
		FVoxelFloatBuffer Values;
		Values.Allocate(Position.Num());

		Functions.Byte(
			Values.GetData(),
			Buffer->GetData(),
			Position.X.GetData(),
			Position.Y.GetData(),
			Position.X.IsConstant(),
			Position.Y.IsConstant(),
			Position.Num(),
			Scale,
			TextureData.SizeX,
			TextureData.SizeY);

		FVoxelLinearColorBuffer Result;
		Result.R = Values;
		Result.G = Values;
		Result.B = Values;
		Result.A = Values;
		return Result;
	}

	if (const FVoxelUInt16Buffer* Buffer = GenericBuffer->As<FVoxelUInt16Buffer>())
	{
		FVoxelFloatBuffer Values;
		Values.Allocate(Position.Num());

		Functions.UInt16(
			Values.GetData(),
			Buffer->GetData(),
			Position.X.GetData(),
			Position.Y.GetData(),
			Position.X.IsConstant(),
			Position.Y.IsConstant(),
			Position.Num(),
			Scale,
			TextureData.SizeX,
			TextureData.SizeY);

		FVoxelLinearColorBuffer Result;
		Result.R = Values;
		Result.G = Values;
		Result.B = Values;
		Result.A = Values;
		return Result;
	}

	if (const FVoxelFloatBuffer* Buffer = GenericBuffer->As<FVoxelFloatBuffer>())
	{
		FVoxelFloatBuffer Values;
		Values.Allocate(Position.Num());

		Functions.Float(
			Values.GetData(),
			Buffer->GetData(),
			Position.X.GetData(),
			Position.Y.GetData(),
			Position.X.IsConstant(),
			Position.Y.IsConstant(),
			Position.Num(),
			Scale,
			TextureData.SizeX,
			TextureData.SizeY);

		FVoxelLinearColorBuffer Result;
		Result.R = Values;
		Result.G = Values;
		Result.B = Values;
		Result.A = Values;
		return Result;
	}

	if (const FVoxelColorBuffer* Buffer = GenericBuffer->As<FVoxelColorBuffer>())
	{
		FVoxelLinearColorBuffer Result;
		Result.Allocate(Position.Num());

		Functions.Color(
			Result.R.GetData(),
			Result.G.GetData(),
			Result.B.GetData(),
			Result.A.GetData(),
			ReinterpretCastPtr<int32>(Buffer->GetData()),
			Position.X.GetData(),
			Position.Y.GetData(),
			Position.X.IsConstant(),
			Position.Y.IsConstant(),
			Position.Num(),
			bSRGB,
			Scale,
			TextureData.SizeX,
			TextureData.SizeY);

		return Result;
	}

	if (const FVoxelLinearColorBuffer* Buffer = GenericBuffer->As<FVoxelLinearColorBuffer>())
	{
		FVoxelLinearColorBuffer Result;
		Result.Allocate(Position.Num());

		Functions.Float(
			Result.R.GetData(),
			Buffer->R.GetData(),
			Position.X.GetData(),
			Position.Y.GetData(),
			Position.X.IsConstant(),
			Position.Y.IsConstant(),
			Position.Num(),
			Scale,
			TextureData.SizeX,
			TextureData.SizeY);

		Functions.Float(
			Result.G.GetData(),
			Buffer->G.GetData(),
			Position.X.GetData(),
			Position.Y.GetData(),
			Position.X.IsConstant(),
			Position.Y.IsConstant(),
			Position.Num(),
			Scale,
			TextureData.SizeX,
			TextureData.SizeY);

		Functions.Float(
			Result.B.GetData(),
			Buffer->B.GetData(),
			Position.X.GetData(),
			Position.Y.GetData(),
			Position.X.IsConstant(),
			Position.Y.IsConstant(),
			Position.Num(),
			Scale,
			TextureData.SizeX,
			TextureData.SizeY);

		Functions.Float(
			Result.A.GetData(),
			Buffer->A.GetData(),
			Position.X.GetData(),
			Position.Y.GetData(),
			Position.X.IsConstant(),
			Position.Y.IsConstant(),
			Position.Num(),
			Scale,
			TextureData.SizeX,
			TextureData.SizeY);

		return Result;
	}

	ensure(false);
	return DefaultBuffer;
}