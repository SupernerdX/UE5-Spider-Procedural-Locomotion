// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSmartSurfaceTypeResolver.h"
#include "Surface/VoxelSmartSurfaceType.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSurfaceTypeBlend.h"
#include "Surface/VoxelSmartSurfaceProxy.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "Surface/VoxelSurfaceTypeBlendBuilder.h"
#include "Surface/VoxelSmartSurfaceFunctionLibrary.h"
#include "Surface/VoxelOutputNode_OutputSurface.h"
#include "VoxelQuery.h"
#include "VoxelGraphQuery.h"
#include "VoxelGraphContext.h"
#include "VoxelGraphPositionParameter.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Graphs/VoxelStampGraphParameters.h"

FVoxelSmartSurfaceTypeResolver::FVoxelSmartSurfaceTypeResolver(
	const int32 LOD,
	const FVoxelWeakStackLayer& WeakLayer,
	const FVoxelLayers& Layers, const FVoxelSurfaceTypeTable& SurfaceTypeTable,
	FVoxelDependencyCollector& DependencyCollector,
	const FVoxelDoubleVectorBuffer& VertexPositions,
	const FVoxelVectorBuffer& VertexNormals,
	const TVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypeBlends)
	: LOD(LOD)
	, WeakLayer(WeakLayer)
	, Layers(Layers)
	, SurfaceTypeTable(SurfaceTypeTable)
	, DependencyCollector(DependencyCollector)
	, NumVertices(SurfaceTypeBlends.Num())
	, VertexPositions(VertexPositions)
	, VertexNormals(VertexNormals)
	, SurfaceTypeBlends(SurfaceTypeBlends)
{
	check(VertexPositions.Num() == NumVertices || VertexPositions.IsConstant());
	check(VertexNormals.Num() == NumVertices || VertexNormals.IsConstant());
}

void FVoxelSmartSurfaceTypeResolver::Resolve()
{
	VOXEL_FUNCTION_COUNTER_NUM(SurfaceTypeBlends.Num());

#if VOXEL_DEBUG
	ON_SCOPE_EXIT
	{
		for (const FVoxelSurfaceTypeBlend& SurfaceTypeBlend : SurfaceTypeBlends)
		{
			for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceTypeBlend.GetLayers())
			{
				ensure(Layer.Type.GetClass() == FVoxelSurfaceType::EClass::SurfaceTypeAsset);
			}
		}
	};
#endif

	if (SurfaceTypeTable.SurfaceTypeToSmartSurfaceProxy.Num() == 0)
	{
		return;
	}

	if (!TryFindIncompleteVertices())
	{
		// No smart surface types
		return;
	}

	for (int32 Pass = 0 ; Pass < 64; Pass++)
	{
		if (TryResolve())
		{
			return;
		}
	}

	VOXEL_MESSAGE(Error, "Failed to resolve smart surface types: smart surface types are likely recursive");

	for (FVoxelSurfaceTypeBlend& SurfaceTypeBlend : SurfaceTypeBlends)
	{
		for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceTypeBlend.GetLayers())
		{
			if (Layer.Type.GetClass() == FVoxelSurfaceType::EClass::SmartSurfaceType)
			{
				SurfaceTypeBlend = {};
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelSmartSurfaceTypeResolver::TryFindIncompleteVertices()
{
	VOXEL_FUNCTION_COUNTER();

	for (int32 Index = 0; Index < NumVertices; Index++)
	{
		const FVoxelSurfaceTypeBlend& SurfaceTypeBlend = SurfaceTypeBlends[Index];

		const bool bHasSmartSurfaceTypes = INLINE_LAMBDA
		{
			for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceTypeBlend.GetLayers())
			{
				if (Layer.Type.GetClass() == FVoxelSurfaceType::EClass::SmartSurfaceType)
				{
					return true;
				}
			}

			return false;
		};

		if (!bHasSmartSurfaceTypes)
		{
			continue;
		}

		if (IncompleteVertices.Num() == 0)
		{
			IncompleteVertices.SetNum(NumVertices, false);
		}

		IncompleteVertices[Index] = true;
	}

	return IncompleteVertices.Num() > 0;
}

bool FVoxelSmartSurfaceTypeResolver::TryResolve()
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelInlineMap<FVoxelSurfaceType, TVoxelArray<int32>, 16> SurfaceTypeToVerticesToCompute;
	{
		VOXEL_SCOPE_COUNTER("SurfaceTypeToVerticesToCompute");

		for (const int32 VertexIndex : IncompleteVertices.IterateSetBits())
		{
			const FVoxelSurfaceTypeBlend& SurfaceTypeBlend = SurfaceTypeBlends[VertexIndex];

			for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceTypeBlend.GetLayers())
			{
				if (Layer.Type.GetClass() == FVoxelSurfaceType::EClass::SurfaceTypeAsset)
				{
					continue;
				}

				SurfaceTypeToVerticesToCompute.FindOrAdd_Reserve(Layer.Type, NumVertices).Add_EnsureNoGrow(VertexIndex);
			}
		}
	}

	struct FData
	{
		int32 ReadIndex = 0;
		int32 Num = 0;
		FVoxelSurfaceTypeBlendBuffer Buffer;
	};

	TVoxelInlineMap<FVoxelSurfaceType, FData, 16> SurfaceTypeToData;
	SurfaceTypeToData.Reserve(SurfaceTypeToVerticesToCompute.Num());

	for (const auto& It : SurfaceTypeToVerticesToCompute)
	{
		const FVoxelSurfaceType SurfaceType = It.Key;
		const TVoxelArray<int32>& VerticesToCompute = It.Value;

		FVoxelSurfaceTypeBlendBuffer SurfaceTypeBlendBuffer = ComputeSurfaceBlends(SurfaceType, VerticesToCompute);

		if (!ensure(
			SurfaceTypeBlendBuffer.IsConstant() ||
			SurfaceTypeBlendBuffer.Num() == VerticesToCompute.Num()))
		{
			SurfaceTypeBlendBuffer = FVoxelSurfaceTypeBlendBuffer::MakeDefault();
		}

		SurfaceTypeToData.Add_EnsureNew(SurfaceType, FData
		{
			0,
			VerticesToCompute.Num(),
			MoveTemp(SurfaceTypeBlendBuffer)
		});
	}

	VOXEL_SCOPE_COUNTER_NUM("Apply", IncompleteVertices.CountSetBits());

	bool bFullyResolved = true;
	for (const int32 Index : IncompleteVertices.IterateSetBits())
	{
		FVoxelSurfaceTypeBlend& SurfaceTypeBlend = SurfaceTypeBlends[Index];

		FVoxelSurfaceTypeBlendBuilder Builder;

		for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceTypeBlend.GetLayers())
		{
			if (Layer.Type.GetClass() == FVoxelSurfaceType::EClass::SurfaceTypeAsset)
			{
				Builder.AddLayer(Layer.Type, Layer.Weight.ToFloat());
				continue;
			}

			FData& Data = SurfaceTypeToData[Layer.Type];
			const FVoxelSurfaceTypeBlend& NewSurfaceTypeBlend = Data.Buffer[Data.ReadIndex++];

			for (const FVoxelSurfaceTypeBlendLayer& NewLayer : NewSurfaceTypeBlend.GetLayers())
			{
				Builder.AddLayer
				(
					NewLayer.Type,
					NewLayer.Weight.ToFloat() * Layer.Weight.ToFloat()
				);
			}
		}

		Builder.Build(SurfaceTypeBlend);

		const bool bHasSmartSurfaceTypes = INLINE_LAMBDA
		{
			for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceTypeBlend.GetLayers())
			{
				if (Layer.Type.GetClass() == FVoxelSurfaceType::EClass::SmartSurfaceType)
				{
					return true;
				}
			}

			return false;
		};

		if (bHasSmartSurfaceTypes)
		{
			bFullyResolved = false;
		}
		else
		{
			checkVoxelSlow(IncompleteVertices[Index]);
			IncompleteVertices[Index] = false;
		}
	}

	for (const auto& It : SurfaceTypeToData)
	{
		ensure(It.Value.ReadIndex == It.Value.Num);
	}

	return bFullyResolved;
}

FVoxelSurfaceTypeBlendBuffer FVoxelSmartSurfaceTypeResolver::ComputeSurfaceBlends(
	const FVoxelSurfaceType& SurfaceType,
	const TConstVoxelArrayView<int32> VerticesToCompute) const
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(SurfaceTypeTable.SurfaceTypeToSmartSurfaceProxy.Contains(SurfaceType));

	const TSharedPtr<FVoxelSmartSurfaceProxy> Proxy = SurfaceTypeTable.SurfaceTypeToSmartSurfaceProxy.FindRef(SurfaceType);
	if (!Proxy)
	{
		// Evaluator failed to compile
		return FVoxelSurfaceTypeBlendBuffer::MakeDefault();
	}

	VOXEL_SCOPE_COUNTER_FNAME(Proxy->Name);

	DependencyCollector.AddDependency(*Proxy->Dependency);

	{
		VOXEL_SCOPE_COUNTER_NUM("Positions & Normals", VerticesToCompute.Num());

		PositionBuffer.Allocate(VerticesToCompute.Num());
		NormalBuffer.Allocate(VerticesToCompute.Num());

		for (int32 Index = 0; Index < VerticesToCompute.Num(); Index++)
		{
			const int32 VertexIndex = VerticesToCompute[Index];

			PositionBuffer.Set(Index, VertexPositions[VertexIndex]);
			NormalBuffer.Set(Index, VertexNormals[VertexIndex]);
		}
	}

	FVoxelGraphContext Context = Proxy->Evaluator.MakeContext(DependencyCollector);

	const FVoxelQuery Query(
		LOD,
		Layers,
		SurfaceTypeTable,
		DependencyCollector);

	FVoxelGraphQueryImpl& GraphQuery = Context.MakeQuery();
	GraphQuery.AddParameter<FVoxelGraphParameters::FLOD>().Value = LOD;
	GraphQuery.AddParameter<FVoxelGraphParameters::FQuery>(Query);
	GraphQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetLocalPosition(PositionBuffer);
	GraphQuery.AddParameter<FVoxelGraphParameters::FSmartSurface>().Normals = NormalBuffer;
	GraphQuery.AddParameter<FVoxelGraphParameters::FSmartSurfaceUniform>().WeakLayer = WeakLayer;

	return *Proxy->Evaluator->SurfaceTypePin.GetSynchronous(GraphQuery);
}