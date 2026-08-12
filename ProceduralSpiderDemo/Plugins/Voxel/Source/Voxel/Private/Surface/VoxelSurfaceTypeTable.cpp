// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSmartSurfaceType.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "Surface/VoxelCubicSurfaceTypeAsset.h"
#include "Surface/VoxelSmartSurfaceProxy.h"
#include "VoxelDependency.h"
#include "VoxelInvalidationCallstack.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	Voxel::OnRefreshAll.AddLambda([]
	{
		ForEachObjectOfClass_Copy<UVoxelSmartSurfaceType>([&](UVoxelSmartSurfaceType& SmartSurface)
		{
			SmartSurface.GetDependency().Invalidate();
		});

		FVoxelSurfaceTypeTable::Refresh();
	});
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class FVoxelSurfaceTypeTableManager : public FVoxelSingleton
{
public:
	TSharedPtr<FVoxelSurfaceTypeTable> Table;
	TSharedPtr<FVoxelDependency> InvisibleSurfaceTypesDependency;
	TSharedPtr<FVoxelDependency> CubicOcclusionDependency;
	TVoxelMap<TVoxelObjectPtr<UVoxelSmartSurfaceType>, TSharedPtr<FVoxelSmartSurfaceProxy>> SmartSurfaceTypeToProxy;

	TSharedRef<FVoxelSurfaceTypeTable> CreateTable()
	{
		VOXEL_FUNCTION_COUNTER();

		TVoxelMap<FVoxelSurfaceType, TSharedPtr<FVoxelSmartSurfaceProxy>> SurfaceTypeToSmartSurfaceProxy;
		SurfaceTypeToSmartSurfaceProxy.Reserve(32);

		TVoxelArray<FVoxelSurfaceType> InvisibleSurfaceTypes;
		InvisibleSurfaceTypes.Reserve(8);

		TVoxelSet<FVoxelSurfaceType> CubicNonSelfOccluding;
		TVoxelSet<FVoxelSurfaceType> CubicNonOtherOccluding;
		TVoxelSet<FVoxelSurfaceType> CubicMaskedTypes;
		TVoxelSet<FVoxelSurfaceType> CubicTranslucentTypes;

		FVoxelSurfaceType::ForeachSurfaceType([&](const FVoxelSurfaceType& SurfaceType)
		{
			switch (SurfaceType.GetClass())
			{
			default: VOXEL_ASSUME(false);
			case FVoxelSurfaceType::EClass::SurfaceTypeAsset:
			{
				const UVoxelSurfaceTypeAsset* Type = SurfaceType.GetSurfaceTypeAsset().Resolve();
				if (!ensureVoxelSlow(Type) ||
					!Type->bInvisible)
				{
					return;
				}

				InvisibleSurfaceTypes.Add(SurfaceType);
			}
			break;
			case FVoxelSurfaceType::EClass::SmartSurfaceType:
			{
				UVoxelSmartSurfaceType* Type = SurfaceType.GetSmartSurfaceType().Resolve();
				if (!ensureVoxelSlow(Type))
				{
					return;
				}

				SurfaceTypeToSmartSurfaceProxy.Add_EnsureNew(
					SurfaceType,
					GetProxy(*Type));
			}
			break;
			case FVoxelSurfaceType::EClass::CubicSurfaceType:
			{
				const UVoxelCubicSurfaceTypeAsset* Type = SurfaceType.GetCubicSurfaceTypeAsset().Resolve();
				if (!ensureVoxelSlow(Type))
				{
					return;
				}

				if (!Type->bOccludesSameType)
				{
					CubicNonSelfOccluding.Add(SurfaceType);
				}
				if (!Type->bOccludesOtherTypes)
				{
					CubicNonOtherOccluding.Add(SurfaceType);
				}
				if (Type->BlendMode == BLEND_Masked)
				{
					CubicMaskedTypes.Add(SurfaceType);
				}
				else if (Type->BlendMode == BLEND_Translucent)
				{
					CubicTranslucentTypes.Add(SurfaceType);
				}
			}
			break;
			}
		});

		if (!InvisibleSurfaceTypesDependency)
		{
			InvisibleSurfaceTypesDependency = FVoxelDependency::Create("InvisibleSurfaces");
		}

		if (!CubicOcclusionDependency)
		{
			CubicOcclusionDependency = FVoxelDependency::Create("CubicOcclusion");
		}

		return MakeShareable(new FVoxelSurfaceTypeTable(
			MoveTemp(InvisibleSurfaceTypes),
			InvisibleSurfaceTypesDependency.ToSharedRef(),
			MoveTemp(CubicNonSelfOccluding),
			MoveTemp(CubicNonOtherOccluding),
			MoveTemp(CubicMaskedTypes),
			MoveTemp(CubicTranslucentTypes),
			CubicOcclusionDependency.ToSharedRef(),
			MoveTemp(SurfaceTypeToSmartSurfaceProxy)));
	}

	TSharedPtr<FVoxelSmartSurfaceProxy> GetProxy(UVoxelSmartSurfaceType& SurfaceType)
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		FVoxelInvalidationScope Scope(SurfaceType);

		const TSharedPtr<FVoxelSmartSurfaceProxy> OldProxy = SmartSurfaceTypeToProxy.FindRef(&SurfaceType);
		if (OldProxy &&
			!OldProxy->DependencyTracker->IsInvalidated())
		{
			return OldProxy.ToSharedRef();
		}

		TSharedPtr<FVoxelDependency> Dependency;
		if (OldProxy)
		{
			Dependency = OldProxy->Dependency;
			Dependency->Invalidate();
		}
		else
		{
			Dependency = FVoxelDependency::Create(SurfaceType.GetPathName());
		}

		FVoxelDependencyCollector DependencyCollector(SurfaceType.GetFName());
		DependencyCollector.AddDependency(SurfaceType.GetDependency());

		const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSurface> Evaluator = SurfaceType.CreateEvaluator(DependencyCollector);

		if (!Evaluator)
		{
			return {};
		}

		const TSharedRef<FVoxelSmartSurfaceProxy> NewProxy = MakeShareable(new FVoxelSmartSurfaceProxy(
			SurfaceType.GetFName(),
			Dependency.ToSharedRef(),
			Evaluator,
			DependencyCollector.Finalize(nullptr, [](const FVoxelInvalidationCallstack& Callstack)
			{
#if VOXEL_INVALIDATION_TRACKING
				const TSharedRef<const FVoxelInvalidationCallstack> SharedCallstack = Callstack.AsShared();
#endif

				Voxel::GameTask([=]
				{
#if VOXEL_INVALIDATION_TRACKING
					FVoxelInvalidationScope LocalScope(SharedCallstack);
#endif

					FVoxelSurfaceTypeTable::Refresh();
				});
			})));

		SmartSurfaceTypeToProxy.FindOrAdd(SurfaceType) = NewProxy;

		return NewProxy;
	}
};
FVoxelSurfaceTypeTableManager* GVoxelSurfaceTypeTableManager = new FVoxelSurfaceTypeTableManager();

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelSurfaceTypeTable> FVoxelSurfaceTypeTable::Get()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	TSharedPtr<FVoxelSurfaceTypeTable>& Table = GVoxelSurfaceTypeTableManager->Table;
	if (!Table)
	{
		Table = GVoxelSurfaceTypeTableManager->CreateTable();
	}

	return Table.ToSharedRef();
}

void FVoxelSurfaceTypeTable::Refresh()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	FVoxelInvalidationScope Scope("FVoxelSurfaceTable::Refresh");

	const TSharedPtr<FVoxelSurfaceTypeTable> OldTable = GVoxelSurfaceTypeTableManager->Table;

	GVoxelSurfaceTypeTableManager->Table = GVoxelSurfaceTypeTableManager->CreateTable();

	if (OldTable &&
		OldTable->InvisibleSurfaceTypes != GVoxelSurfaceTypeTableManager->Table->InvisibleSurfaceTypes)
	{
		GVoxelSurfaceTypeTableManager->InvisibleSurfaceTypesDependency->Invalidate();
	}

	if (OldTable &&
		(!OldTable->CubicNonSelfOccluding.OrderIndependentEqual(GVoxelSurfaceTypeTableManager->Table->CubicNonSelfOccluding) ||
		 !OldTable->CubicNonOtherOccluding.OrderIndependentEqual(GVoxelSurfaceTypeTableManager->Table->CubicNonOtherOccluding) ||
		 !OldTable->CubicMaskedTypes.OrderIndependentEqual(GVoxelSurfaceTypeTableManager->Table->CubicMaskedTypes) ||
		 !OldTable->CubicTranslucentTypes.OrderIndependentEqual(GVoxelSurfaceTypeTableManager->Table->CubicTranslucentTypes)))
	{
		GVoxelSurfaceTypeTableManager->CubicOcclusionDependency->Invalidate();
	}
}