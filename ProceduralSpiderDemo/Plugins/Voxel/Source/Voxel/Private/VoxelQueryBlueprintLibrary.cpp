// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelQueryBlueprintLibrary.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "VoxelStampDelta.h"
#include "VoxelStampRuntime.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"
#include "Utilities/VoxelBufferGradientUtilities.h"

#include "TextureResource.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"

TVoxelFuture<bool> UVoxelQueryBlueprintLibrary::ExportVoxelDataToRenderTarget(
	UWorld* World,
	UTextureRenderTarget2D* RenderTarget,
	const FVoxelStackHeightLayer& Layer,
	const FVoxelBox2D& ZoneToQuery,
	const FVoxelColorQuery& Query)
{
	VOXEL_FUNCTION_COUNTER();

	if (!World)
	{
		VOXEL_MESSAGE(Error, "World is null");
		return {};
	}

	if (!RenderTarget)
	{
		VOXEL_MESSAGE(Error, "RenderTarget is null");
		return {};
	}

	if (!Layer.IsValid())
	{
		VOXEL_MESSAGE(Error, "Layer is null");
		return {};
	}

	if (!ZoneToQuery.IsValidAndNotEmpty())
	{
		VOXEL_MESSAGE(Error, "ZoneToQuery is empty");
		return {};
	}

	if (RenderTarget->OverrideFormat != PF_Unknown &&
		RenderTarget->OverrideFormat != GetPixelFormatFromRenderTargetFormat(RenderTarget->RenderTargetFormat))
	{
		VOXEL_MESSAGE(Error, "RenderTarget has OverrideFormat set, this is not supported");
		return {};
	}

	Query.R.MetadataRef = FVoxelMetadataRef(Query.R.Metadata);
	Query.G.MetadataRef = FVoxelMetadataRef(Query.G.Metadata);
	Query.B.MetadataRef = FVoxelMetadataRef(Query.B.Metadata);
	Query.A.MetadataRef = FVoxelMetadataRef(Query.A.Metadata);

	return Voxel::AsyncTask([
		ZoneToQuery,
		Query,
		Layers = FVoxelLayers::Get(World),
		SurfaceTypeTable = FVoxelSurfaceTypeTable::Get(),
		Size = FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY),
		WeakRenderTarget = MakeWeakObjectPtr(RenderTarget),
		WeakLayer = FVoxelWeakStackLayer(Layer),
		Format = RenderTarget->RenderTargetFormat]() -> TVoxelFuture<bool>
	{
		VOXEL_FUNCTION_COUNTER();

		FVoxelDoubleVector2DBuffer Positions;
		{
			Positions.Allocate(Size.X * Size.Y);

			const FVector2D Step = ZoneToQuery.Size() / Size;

			for (int32 Y = 0; Y < Size.Y; Y++)
			{
				for (int32 X = 0; X < Size.X; X++)
				{
					const int32 Index = FVoxelUtilities::Get2DIndex<int32>(Size, X, Y);

					Positions.X.Set(Index, ZoneToQuery.Min.X + Step.X * X);
					Positions.Y.Set(Index, ZoneToQuery.Min.Y + Step.Y * Y);
				}
			}
		}

		TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
		{
			TVoxelArray<FVoxelMetadataRef> MetadataRefs;
			if (Query.R.Type == EVoxelFloatQueryType::Metadata) { MetadataRefs.AddUnique(Query.R.MetadataRef); }
			if (Query.G.Type == EVoxelFloatQueryType::Metadata) { MetadataRefs.AddUnique(Query.G.MetadataRef); }
			if (Query.B.Type == EVoxelFloatQueryType::Metadata) { MetadataRefs.AddUnique(Query.B.MetadataRef); }
			if (Query.A.Type == EVoxelFloatQueryType::Metadata) { MetadataRefs.AddUnique(Query.A.MetadataRef); }

			for (const FVoxelMetadataRef& MetadataRef : MetadataRefs)
			{
				if (MetadataRef.IsValid())
				{
					MetadataToBuffer.Add_EnsureNew(MetadataRef, MetadataRef.MakeDefaultBuffer(Positions.Num()));
				}
			}
		}

		const FVoxelQuery VoxelQuery(
			0,
			*Layers,
			*SurfaceTypeTable,
			FVoxelDependencyCollector::Null);

		const FVoxelFloatBuffer Heights = VoxelQuery.SampleHeightLayer(
			WeakLayer,
			Positions,
			{},
			MetadataToBuffer);

		const auto GetBuffer = [&](const FVoxelFloatQuery& Channel) -> FVoxelFloatBuffer
		{
			switch (Channel.Type)
			{
			default: ensure(false);
			case EVoxelFloatQueryType::Constant:
			{
				return Channel.Constant;
			}
			case EVoxelFloatQueryType::Height:
			{
				return Heights;
			}
			case EVoxelFloatQueryType::Metadata:
			{
				if (!Channel.MetadataRef.IsValid())
				{
					return 0.f;
				}

				return Channel.MetadataRef.GetChannel(
					*MetadataToBuffer[Channel.MetadataRef],
					Channel.ComponentToExtract);
			}
			}
		};

		TVoxelArray<uint8> Values = CreateRenderTargetData(
			Format,
			Size.X * Size.Y,
			GetBuffer(Query.R),
			GetBuffer(Query.G),
			GetBuffer(Query.B),
			GetBuffer(Query.A));

		if (Values.Num() == 0)
		{
			return {};
		}

		return Voxel::GameTask([Values = MakeSharedCopy(MoveTemp(Values)), WeakRenderTarget]() -> TVoxelFuture<bool>
		{
			UTextureRenderTarget2D* Texture = WeakRenderTarget.Get();
			if (!ensureVoxelSlow(Texture))
			{
				return {};
			}

			// Otherwise render target is black when updated in BeginPlay
			Texture->UpdateResourceImmediate();

			FTextureResource* Resource = Texture->GetResource();
			if (!ensureVoxelSlow(Resource))
			{
				return {};
			}

			return Voxel::RenderTask([Values, Resource]
			{
				FRHITexture* TextureRHI = Resource->GetTextureRHI();
				if (!ensure(TextureRHI))
				{
					return false;
				}

				const uint32 SizeX = TextureRHI->GetSizeX();
				const uint32 SizeY = TextureRHI->GetSizeY();
				const EPixelFormat PixelFormat = TextureRHI->GetFormat();

				if (!ensure(Values->Num() == SizeX * SizeY * GPixelFormats[PixelFormat].BlockBytes))
				{
					return false;
				}

				RHIUpdateTexture2D_Safe(
					TextureRHI,
					0,
					FUpdateTextureRegion2D(
						0, 0,
						0, 0,
						SizeX,
						SizeY),
					SizeX * GPixelFormats[PixelFormat].BlockBytes,
					*Values);

				return true;
			});
		});
	});
}

FVoxelFuture UVoxelQueryBlueprintLibrary::K2_ExportVoxelDataToRenderTarget(
	bool& bSuccess,
	UWorld* World,
	UTextureRenderTarget2D* RenderTarget,
	const FVoxelStackHeightLayer Layer,
	const FVoxelBox2D ZoneToQuery,
	const FVoxelColorQuery Query)
{
	bSuccess = false;

	return
		ExportVoxelDataToRenderTarget(
			World,
			RenderTarget,
			Layer,
			ZoneToQuery,
			Query)
		.Then_GameThread([&bSuccess](const bool bNewSuccess)
		{
			bSuccess = bNewSuccess;
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelArray<uint8> UVoxelQueryBlueprintLibrary::CreateRenderTargetData(
	const ETextureRenderTargetFormat Format,
	const int32 Num,
	const FVoxelFloatBuffer& R,
	const FVoxelFloatBuffer& G,
	const FVoxelFloatBuffer& B,
	const FVoxelFloatBuffer& A)
{
	VOXEL_FUNCTION_COUNTER();

	switch (Format)
	{
	case RTF_R8:
	{
		TVoxelArray<uint8> Result;
		FVoxelUtilities::SetNumFast(Result, Num);

		for (int32 Index = 0; Index < Num; Index++)
		{
			Result[Index] = FVoxelUtilities::FloatToUINT8(R[Index]);
		}

		return Result;
	}
	case RTF_RG8:
	{
		TVoxelArray<uint8> Result;
		FVoxelUtilities::SetNumFast(Result, 2 * Num);

		for (int32 Index = 0; Index < Num; Index++)
		{
			Result[2 * Index + 0] = FVoxelUtilities::FloatToUINT8(R[Index]);
			Result[2 * Index + 1] = FVoxelUtilities::FloatToUINT8(G[Index]);
		}

		return Result;
	}
	case RTF_RGBA8:
	{
		TVoxelArray<uint8> Result;
		FVoxelUtilities::SetNumFast(Result, 4 * Num);

		for (int32 Index = 0; Index < Num; Index++)
		{
			Result[4 * Index + 0] = FVoxelUtilities::FloatToUINT8(R[Index]);
			Result[4 * Index + 1] = FVoxelUtilities::FloatToUINT8(G[Index]);
			Result[4 * Index + 2] = FVoxelUtilities::FloatToUINT8(B[Index]);
			Result[4 * Index + 3] = FVoxelUtilities::FloatToUINT8(A[Index]);
		}

		return Result;
	}
	case RTF_RGBA8_SRGB:
	{
		TVoxelArray<uint8> Result;
		FVoxelUtilities::SetNumFast(Result, 4 * Num);

		for (int32 Index = 0; Index < Num; Index++)
		{
			const FColor Color = FLinearColor(R[Index], G[Index], B[Index], A[Index]).ToFColorSRGB();

			Result[4 * Index + 0] = Color.R;
			Result[4 * Index + 1] = Color.G;
			Result[4 * Index + 2] = Color.B;
			Result[4 * Index + 3] = Color.A;
		}

		return Result;
	}
	case RTF_R16f:
	{
		TVoxelArray<uint8> Result;
		FVoxelUtilities::SetNumFast(Result, Num * sizeof(FFloat16));

		for (int32 Index = 0; Index < Num; Index++)
		{
			Result.View<FFloat16>()[Index] = R[Index];
		}

		return Result;
	}
	case RTF_RG16f:
	{
		TVoxelArray<uint8> Result;
		FVoxelUtilities::SetNumFast(Result, 2 * Num * sizeof(FFloat16));

		for (int32 Index = 0; Index < Num; Index++)
		{
			Result.View<FFloat16>()[2 * Index + 0] = R[Index];
			Result.View<FFloat16>()[2 * Index + 1] = G[Index];
		}

		return Result;
	}
	case RTF_RGBA16f:
	{
		TVoxelArray<uint8> Result;
		FVoxelUtilities::SetNumFast(Result, 4 * Num * sizeof(FFloat16));

		for (int32 Index = 0; Index < Num; Index++)
		{
			Result.View<FFloat16>()[4 * Index + 0] = R[Index];
			Result.View<FFloat16>()[4 * Index + 1] = G[Index];
			Result.View<FFloat16>()[4 * Index + 2] = B[Index];
			Result.View<FFloat16>()[4 * Index + 3] = A[Index];
		}

		return Result;
	}
	case RTF_R32f:
	{
		TVoxelArray<uint8> Result;
		FVoxelUtilities::SetNumFast(Result, Num * sizeof(float));

		for (int32 Index = 0; Index < Num; Index++)
		{
			Result.View<float>()[Index] = R[Index];
		}

		return Result;
	}
	case RTF_RG32f:
	{
		TVoxelArray<uint8> Result;
		FVoxelUtilities::SetNumFast(Result, 2 * Num * sizeof(float));

		for (int32 Index = 0; Index < Num; Index++)
		{
			Result.View<float>()[2 * Index + 0] = R[Index];
			Result.View<float>()[2 * Index + 1] = G[Index];
		}

		return Result;
	}
	case RTF_RGBA32f:
	{
		TVoxelArray<uint8> Result;
		FVoxelUtilities::SetNumFast(Result, 4 * Num * sizeof(float));

		for (int32 Index = 0; Index < Num; Index++)
		{
			Result.View<float>()[4 * Index + 0] = R[Index];
			Result.View<float>()[4 * Index + 1] = G[Index];
			Result.View<float>()[4 * Index + 2] = B[Index];
			Result.View<float>()[4 * Index + 3] = A[Index];
		}

		return Result;
	}
	default: ensure(false);
	case RTF_RGB10A2:
	{
		VOXEL_MESSAGE(Error, "ExportVoxelDataToRenderTarget: Unsupported render target format {0}", GetEnumDisplayName(Format));

		return {};
	}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelFuture<TOptional<FVoxelQueryResult>> UVoxelQueryBlueprintLibrary::QueryVoxelLayer(
	UWorld* World,
	const FVoxelStackLayer& Layer,
	const FVector& Position,
	const TArray<UVoxelMetadata*>& MetadatasToQuery,
	const float GradientStep)
{
	return
		MultiQueryVoxelLayer(
			World,
			Layer,
			{ Position },
			MetadatasToQuery,
			GradientStep)
		.Then_AnyThread([](const TOptional<TArray<FVoxelQueryResult>>& Results) -> TOptional<FVoxelQueryResult>
		{
			if (!Results)
			{
				return {};
			}

			return (*Results)[0];
		});
}

TVoxelFuture<TOptional<TArray<FVoxelQueryResult>>> UVoxelQueryBlueprintLibrary::MultiQueryVoxelLayer(
	UWorld* World,
	const FVoxelStackLayer& Layer,
	const TArray<FVector>& Positions,
	const TArray<UVoxelMetadata*>& MetadatasToQuery,
	const float GradientStep)
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num());
	ensure(IsInGameThread());

	if (!World)
	{
		VOXEL_MESSAGE(Error, "World is null");
		return {};
	}

	if (!Layer.IsValid())
	{
		VOXEL_MESSAGE(Error, "Layer is null");
		return {};
	}

	return Voxel::AsyncTask([
		Positions = Positions,
		MetadatasToQuery = FVoxelMetadataRef::GetUniqueValidRefs(MetadatasToQuery),
		WeakLayer = FVoxelWeakStackLayer(Layer),
		GradientStep,
		Layers = FVoxelLayers::Get(World),
		SurfaceTypeTable = FVoxelSurfaceTypeTable::Get()]
	{
		VOXEL_FUNCTION_COUNTER();

		FVoxelDoubleVectorBuffer PositionsBuffer;
		{
			PositionsBuffer.Allocate(Positions.Num());

			for (int32 Index = 0; Index < Positions.Num(); Index++)
			{
				PositionsBuffer.Set(Index, Positions[Index]);
			}
		}

		FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
		SurfaceTypes.AllocateZeroed(Positions.Num());

		TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
		{
			MetadataToBuffer.Reserve(MetadatasToQuery.Num());

			for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
			{
				MetadataToBuffer.Add_EnsureNew(
					MetadataToQuery,
					MetadataToQuery.MakeDefaultBuffer(Positions.Num()));
			}
		}

		const FVoxelQuery Query(
			0,
			*Layers,
			*SurfaceTypeTable,
			FVoxelDependencyCollector::Null);

		FVoxelFloatBuffer Values;
		FVoxelVectorBuffer Normals;

		if (WeakLayer.Type == EVoxelLayerType::Height)
		{
			FVoxelDoubleVector2DBuffer Positions2D;
			Positions2D.X = PositionsBuffer.X;
			Positions2D.Y = PositionsBuffer.Y;

			Values = Query.SampleHeightLayer(
				WeakLayer,
				Positions2D,
				SurfaceTypes.View(),
				MetadataToBuffer);

			const FVoxelDoubleVector2DBuffer GradientPositions = FVoxelBufferGradientUtilities::SplitPositions2D(Positions2D, GradientStep);
			const FVoxelFloatBuffer GradientHeights = Query.SampleHeightLayer(WeakLayer, GradientPositions);

			Normals = FVoxelBufferGradientUtilities::CollapseGradient2DToNormal(GradientHeights, Positions.Num(), GradientStep);
		}
		else
		{
			Values = Query.SampleVolumeLayer(
				WeakLayer,
				PositionsBuffer,
				SurfaceTypes.View(),
				MetadataToBuffer);

			const FVoxelDoubleVectorBuffer GradientPositions = FVoxelBufferGradientUtilities::SplitPositions3D(PositionsBuffer, GradientStep);
			const FVoxelFloatBuffer GradientHeights = Query.SampleVolumeLayer(WeakLayer, GradientPositions);

			Normals = FVoxelBufferGradientUtilities::CollapseGradient3D(GradientHeights, Positions.Num(), GradientStep);
		}

		TArray<FVoxelQueryResult> NewResults;
		{
			FVoxelUtilities::SetNum(NewResults, Positions.Num());

			for (int32 Index = 0; Index < Positions.Num(); Index++)
			{
				FVoxelQueryResult& Result = NewResults[Index];
				Result.Value = Values[Index];
				Result.Normal = FVector(Normals[Index]);
				Result.WeakUnresolvedSurfaceType = SurfaceTypes[Index].GetTopLayer().Type.GetSurfaceTypeInterface();

				for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
				{
					Result.WeakMetadataToValue.Add_EnsureNew(
						MetadataToQuery.GetMetadata(),
						MetadataToQuery.GetValue(*MetadataToBuffer[MetadataToQuery], Index));
				}
			}
		}

		FVoxelSmartSurfaceTypeResolver Resolver(
			0,
			WeakLayer,
			*Layers,
			*SurfaceTypeTable,
			FVoxelDependencyCollector::Null,
			PositionsBuffer,
			Normals,
			SurfaceTypes.View());

		Resolver.Resolve();

		for (int32 Index = 0; Index < Positions.Num(); Index++)
		{
			NewResults[Index].WeakSurfaceType = SurfaceTypes[Index].GetTopLayer().Type.GetSurfaceTypeInterface();
		}

		return Voxel::GameTask([NewResults = MakeSharedCopy(MoveTemp(NewResults))]() -> TOptional<TArray<FVoxelQueryResult>>
		{
			VOXEL_FUNCTION_COUNTER();

			for (FVoxelQueryResult& Result : *NewResults)
			{
				Result.UnresolvedSurfaceType = Result.WeakUnresolvedSurfaceType.Resolve_Ensured();
				Result.SurfaceType = Result.WeakSurfaceType.Resolve_Ensured();

				Result.MetadataToValue.Reserve(Result.WeakMetadataToValue.Num());

				for (auto& It : Result.WeakMetadataToValue)
				{
					Result.MetadataToValue.Add(
						It.Key.Resolve_Ensured(),
						MoveTemp(It.Value));
				}

				Result.WeakMetadataToValue.Empty();
			}

			return MoveTemp(*NewResults);
		});
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFuture UVoxelQueryBlueprintLibrary::K2_QueryVoxelLayer(
	bool& bSuccess,
	FVoxelQueryResult& Result,
	UWorld* World,
	const FVoxelStackLayer Layer,
	const FVector Position,
	const TArray<UVoxelMetadata*> MetadatasToQuery,
	const float GradientStep)
{
	bSuccess = false;
	Result = {};

	return
		QueryVoxelLayer(
			World,
			Layer,
			Position,
			MetadatasToQuery,
			GradientStep)
		.Then_GameThread([
				&bSuccess,
				&Result](const TOptional<FVoxelQueryResult>& NewResult)
			{
				if (!NewResult)
				{
					return;
				}

				bSuccess = true;
				Result = *NewResult;
			});
}

FVoxelFuture UVoxelQueryBlueprintLibrary::K2_MultiQueryVoxelLayer(
	bool& bSuccess,
	TArray<FVoxelQueryResult>& Results,
	UWorld* World,
	const FVoxelStackLayer Layer,
	const TArray<FVector> Positions,
	const TArray<UVoxelMetadata*> MetadatasToQuery,
	const float GradientStep)
{
	bSuccess = false;
	Results.Reset();

	return
		MultiQueryVoxelLayer(
			World,
			Layer,
			Positions,
			MetadatasToQuery,
			GradientStep)
		.Then_GameThread([&bSuccess, &Results](const TOptional<TArray<FVoxelQueryResult>>& NewResults)
		{
			if (!NewResults)
			{
				return;
			}

			bSuccess = true;
			Results = *NewResults;
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelFuture<TOptional<TArray<FVoxelQueryStampResult>>> UVoxelQueryBlueprintLibrary::QueryVoxelStamps(
	UWorld* World,
	const FVoxelStackLayer& Layer,
	const FVector& Position)
{
	ensure(IsInGameThread());

	if (!World)
	{
		VOXEL_MESSAGE(Error, "World is null");
		return {};
	}

	if (!Layer.IsValid())
	{
		VOXEL_MESSAGE(Error, "Layer is null");
		return {};
	}

	return Voxel::AsyncTask([
		Position,
		WeakLayer = FVoxelWeakStackLayer(Layer),
		Layers = FVoxelLayers::Get(World),
		SurfaceTypeTable = FVoxelSurfaceTypeTable::Get()]
	{
		VOXEL_FUNCTION_COUNTER();

		TVoxelArray<FVoxelStampDelta> StampDeltas = Layers->GetStampDeltas(
			WeakLayer,
			Position,
			0);

		return Voxel::GameTask([NewStampDeltas = MakeSharedCopy(MoveTemp(StampDeltas))]() -> TOptional<TArray<FVoxelQueryStampResult>>
		{
			TArray<FVoxelQueryStampResult> Result;
			for (const FVoxelStampDelta& Delta : *NewStampDeltas)
			{
				FVoxelQueryStampResult StampData;
				StampData.DistanceBeforeStamp = Delta.DistanceBefore;
				StampData.DistanceAfterStamp = Delta.DistanceAfter;
				StampData.Layer = Delta.Layer.Resolve();
				StampData.StampComponent = Delta.Stamp->GetComponent().Resolve();
				StampData.StampIndex = Delta.Stamp->GetInstanceIndex();
				Result.Add(MoveTemp(StampData));
			}

			return Result;
		});
	});
}

FVoxelFuture UVoxelQueryBlueprintLibrary::K2_QueryVoxelStamps(
	bool& bSuccess,
	TArray<FVoxelQueryStampResult>& Results,
	UWorld* World,
	const FVoxelStackLayer& Layer,
	const FVector& Position)
{
	bSuccess = false;
	Results.Reset();

	return
		QueryVoxelStamps(
			World,
			Layer,
			Position)
		.Then_GameThread([&bSuccess, &Results](const TOptional<TArray<FVoxelQueryStampResult>>& NewResults)
		{
			if (!NewResults)
			{
				return;
			}

			bSuccess = true;
			Results = *NewResults;
		});
}