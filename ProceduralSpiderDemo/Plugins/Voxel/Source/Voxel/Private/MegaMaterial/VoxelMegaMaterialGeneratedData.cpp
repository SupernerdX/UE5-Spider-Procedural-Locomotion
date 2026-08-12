// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelMegaMaterialGeneratedData.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "MegaMaterial/VoxelMegaMaterialCache.h"
#include "MegaMaterial/VoxelMegaMaterialGenerator.h"
#include "VoxelMaterialUsage.h"
#include "VoxelSettings.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "Surface/VoxelCubicSurfaceTypeAsset.h"
#include "Surface/VoxelCubicTextureTemplate.h"
#include "Nanite/VoxelMaterialSelectionCS.h"

#if WITH_EDITOR
#include "AssetCompilingManager.h"
#include "IAssetCompilingManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "UObject/ObjectSaveContext.h"
#include "MaterialEditor/PreviewMaterial.h"
#endif

#if WITH_EDITOR
class FVoxelMegaMaterialGeneratedDataManager : public FVoxelSingleton
{
public:
	TVoxelSet<TVoxelObjectPtr<UVoxelMegaMaterialGeneratedData>> GeneratedDataToRebuild;

	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override
	{
		UMaterial::OnMaterialCompilationFinished().AddLambda([](const UMaterialInterface* Material)
		{
			if (Material->IsA<UPreviewMaterial>())
			{
				return;
			}

			if (GetDefault<UVoxelSettings>()->bDisableMegaMaterialAutoRecompile)
			{
				return;
			}

			UVoxelMegaMaterialGeneratedData::RebuildIfNeeded(Material);
		});

		FCoreUObjectDelegates::OnObjectPreSave.AddLambda([](const UObject* Object, const FObjectPreSaveContext&)
		{
			if (!Object)
			{
				return;
			}

			if (GetDefault<UVoxelSettings>()->bDisableMegaMaterialAutoRecompile)
			{
				return;
			}

			UVoxelMegaMaterialGeneratedData::RebuildIfNeeded(Object);
		});

		FCoreUObjectDelegates::OnObjectPropertyChanged.AddLambda([](const UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent)
		{
			if (!Object)
			{
				return;
			}

			if (!Object->IsA<UMaterialInstance>() &&
				!Object->IsA<UVoxelSurfaceTypeAsset>() &&
				!Object->IsA<UVoxelCubicSurfaceTypeAsset>() &&
				!Object->IsA<UVoxelCubicTextureTemplate>())
			{
				return;
			}

			if (GetDefault<UVoxelSettings>()->bDisableMegaMaterialAutoRecompile)
			{
				return;
			}

			UVoxelMegaMaterialGeneratedData::RebuildIfNeeded(Object);
		});
	}
	virtual void Tick() override
	{
		VOXEL_FUNCTION_COUNTER();

		if (GeneratedDataToRebuild.Num() == 0)
		{
			return;
		}

		// Wait for all compilations to finish before rebuilding
		// Not doing this causes GPU crashes & compilation loops
		for (const IAssetCompilingManager* AssetCompilingManager : FAssetCompilingManager::Get().GetRegisteredManagers())
		{
			if (AssetCompilingManager->GetNumRemainingAssets() > 0)
			{
				return;
			}
		}

		for (auto It = GeneratedDataToRebuild.CreateIterator(); It; ++It)
		{
			UVoxelMegaMaterialGeneratedData* GeneratedData = It->Resolve();
			It.RemoveCurrent();

			if (!ensureVoxelSlow(GeneratedData))
			{
				continue;
			}

			GeneratedData->RebuildNow();

			// Rebuild one at a time
			break;
		}
	}
	//~ End FVoxelSingleton Interface
};
FVoxelMegaMaterialGeneratedDataManager* GVoxelMegaMaterialGeneratedDataManager = new FVoxelMegaMaterialGeneratedDataManager();
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelMegaMaterial* UVoxelMegaMaterialGeneratedData::GetMegaMaterial()
{
	VOXEL_FUNCTION_COUNTER();

	if (UVoxelMegaMaterial* MegaMaterial = WeakMegaMaterial.Resolve())
	{
		return MegaMaterial;
	}

	WeakMegaMaterial = SoftMegaMaterial.LoadSynchronous();
	return WeakMegaMaterial.Resolve();
}

void UVoxelMegaMaterialGeneratedData::SetMegaMaterial(UVoxelMegaMaterial* NewMegaMaterial)
{
	SoftMegaMaterial = NewMegaMaterial;
	WeakMegaMaterial = NewMegaMaterial;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelMegaMaterialGeneratedData::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelMegaMaterialGeneratedData::ForceRebuild()
{
	IndexToGeneratedMaterial.Reset();
	TargetToGeneratedMaterial.Reset();

	QueueRebuild();
}

void UVoxelMegaMaterialGeneratedData::QueueRebuild()
{
	GVoxelMegaMaterialGeneratedDataManager->GeneratedDataToRebuild.Add(this);
}

bool UVoxelMegaMaterialGeneratedData::IsRebuildQueued() const
{
	return GVoxelMegaMaterialGeneratedDataManager->GeneratedDataToRebuild.Contains(MakeVoxelObjectPtr(ConstCast(this)));
}

void UVoxelMegaMaterialGeneratedData::RebuildNow()
{
	VOXEL_FUNCTION_COUNTER();

	static const bool bNoVoxelMegaMaterial = FParse::Param(FCommandLine::Get(), TEXT("NoVoxelMegaMaterial"));
	if (bNoVoxelMegaMaterial)
	{
		return;
	}

	// Combine all recreates that will be triggered below
	TSharedPtr<FGlobalComponentRecreateRenderStateContext> Context;

	UVoxelMegaMaterial* MegaMaterial = GetMegaMaterial();
	if (!ensureVoxelSlow(MegaMaterial))
	{
		return;
	}

	ON_SCOPE_EXIT
	{
		TSet<TObjectPtr<const UObject>> NewWatchedObjects;
		NewWatchedObjects.Reserve(WatchedObjects.Num());

		WatchedObjects.Reset();

		ForeachObjectReference(*this, [&](const UObject* Object)
		{
			if (Object)
			{
				NewWatchedObjects.Add(Object);
			}
		});

		WatchedObjects = MoveTemp(NewWatchedObjects);

		if (const UVoxelMegaMaterialCache* Cache = GetTypedOuter<UVoxelMegaMaterialCache>())
		{
			Cache->AutoSaveIfEnabled();
		}

		const TSharedRef<FVoxelMegaMaterialProxy> OldProxy = MegaMaterial->GetProxy();
		const TSharedRef<FVoxelMegaMaterialProxy> NewProxy = MakeShareable(new FVoxelMegaMaterialProxy(*MegaMaterial));
		NewProxy->Initialize(&OldProxy.Get());

		if (NewProxy->Equals(*OldProxy))
		{
			// Only change the proxy if needed
			return;
		}

		MegaMaterial->Proxy = NewProxy;
	};

	IndexToSurfaceInfo.Reset();
	MetadataIndexToMetadata.Reset();
	TargetToMaterial.Reset();

	int32 Counter = 1;
	for (UVoxelSurfaceTypeAsset* SurfaceType : MegaMaterial->SurfaceTypes)
	{
		if (!SurfaceType ||
			!SurfaceType->Material)
		{
			continue;
		}

		const bool bAlreadyAdded = INLINE_LAMBDA
		{
			for (const auto& It : IndexToSurfaceInfo)
			{
				if (It.Value.SurfaceType == SurfaceType)
				{
					return true;
				}
			}

			return false;
		};

		if (bAlreadyAdded)
		{
			continue;
		}

		const UMaterial* BaseMaterial = SurfaceType->Material->GetMaterial();
		if (!BaseMaterial)
		{
			continue;
		}

		FVoxelMegaMaterialSurfaceInfo& MaterialInfo = IndexToSurfaceInfo.Add(FVoxelMaterialRenderIndex(Counter++));
		MaterialInfo.SurfaceType = SurfaceType;

		for (UVoxelMetadata* Metadata : FVoxelMegaMaterialGenerator::GetUsedMetadatas(*BaseMaterial))
		{
			MaterialInfo.UsedMetadatas.Add(Metadata);
			MetadataIndexToMetadata.AddUnique(Metadata);
		}

		if (MegaMaterial->AttributePostProcess)
		{
			// Add PostProcess to every material

			for (UVoxelMetadata* Metadata : FVoxelMegaMaterialGenerator::GetUsedMetadatas(*MegaMaterial->AttributePostProcess))
			{
				MaterialInfo.UsedMetadatas.Add(Metadata);
				MetadataIndexToMetadata.AddUnique(Metadata);
			}
		}
	}

	const FVoxelMegaMaterialState MegaMaterialState(*MegaMaterial);

	TMap<FVoxelMaterialRenderIndex, FVoxelSurfaceTypeState> IndexToSurfaceType;
	{
		IndexToSurfaceType.Reserve(IndexToSurfaceInfo.Num());

		for (const auto& It : IndexToSurfaceInfo)
		{
			IndexToSurfaceType.Add(It.Key, FVoxelSurfaceTypeState(*It.Value.SurfaceType));
		}
	}

	{
		VOXEL_SCOPE_COUNTER("Generate targets");

		FVoxelMegaMaterialTargetBaseState BaseState;
		BaseState.MegaMaterial = MegaMaterialState;
		BaseState.IndexToSurfaceType = IndexToSurfaceType;
		BaseState.MetadataIndexToMetadata = MetadataIndexToMetadata;

		if (MegaMaterial->NonNaniteMaterialType == EVoxelMegaMaterialGenerationType::Custom)
		{
			TargetToMaterial.Add(EVoxelMegaMaterialTarget::NonNanite, MegaMaterial->CustomNonNaniteMaterial);
			TargetToGeneratedMaterial.Remove(EVoxelMegaMaterialTarget::NonNanite);
		}
		else
		{
			GenerateMaterialForTarget(*MegaMaterial, EVoxelMegaMaterialTarget::NonNanite, BaseState, Context);
		}

		if (MegaMaterial->NaniteDisplacementMaterialType == EVoxelMegaMaterialGenerationType::Custom)
		{
			if (MegaMaterial->CustomNaniteDisplacementMaterial)
			{
				if (!MegaMaterial->CustomNaniteDisplacementMaterial->IsTessellationEnabled())
				{
					VOXEL_MESSAGE(Error, "{0}: NaniteDisplacementMaterial {1} should have tessellation enabled",
						MegaMaterial,
						MegaMaterial->CustomNaniteDisplacementMaterial);
				}

				if (UMaterial* Material = MegaMaterial->CustomNaniteDisplacementMaterial->GetMaterial())
				{
					if (!Material->bUsedWithVoxelMaterialSelection)
					{
						Material->Modify();
						Material->CheckMaterialUsage(MATUSAGE_VirtualHeightfieldMesh);
					}
				}
			}

			TargetToMaterial.Add(EVoxelMegaMaterialTarget::NaniteWPO, MegaMaterial->CustomNaniteDisplacementMaterial);
			TargetToMaterial.Add(EVoxelMegaMaterialTarget::NaniteDisplacement, MegaMaterial->CustomNaniteDisplacementMaterial);
			TargetToMaterial.Add(EVoxelMegaMaterialTarget::NaniteMaterialSelection, MegaMaterial->CustomNaniteDisplacementMaterial);

			TargetToGeneratedMaterial.Remove(EVoxelMegaMaterialTarget::NaniteWPO);
			TargetToGeneratedMaterial.Remove(EVoxelMegaMaterialTarget::NaniteDisplacement);
			TargetToGeneratedMaterial.Remove(EVoxelMegaMaterialTarget::NaniteMaterialSelection);
		}
		else
		{
			GenerateMaterialForTarget(*MegaMaterial, EVoxelMegaMaterialTarget::NaniteWPO, BaseState, Context);
			GenerateMaterialForTarget(*MegaMaterial, EVoxelMegaMaterialTarget::NaniteDisplacement, BaseState, Context);
			GenerateMaterialForTarget(*MegaMaterial, EVoxelMegaMaterialTarget::NaniteMaterialSelection, BaseState, Context);
		}

		if (MegaMaterial->LumenMaterialType == EVoxelMegaMaterialGenerationType::Custom)
		{
			TargetToMaterial.Add(EVoxelMegaMaterialTarget::Lumen, MegaMaterial->CustomLumenMaterial);
			TargetToGeneratedMaterial.Remove(EVoxelMegaMaterialTarget::Lumen);
		}
		else
		{
			GenerateMaterialForTarget(*MegaMaterial, EVoxelMegaMaterialTarget::Lumen, BaseState, Context);
		}
	}

	{
		VOXEL_SCOPE_COUNTER("Generate materials");

		const TMap<FVoxelMaterialRenderIndex, FVoxelMegaMaterialMaterialGeneratedMaterial> OldIndexToGeneratedMaterial = MoveTemp(IndexToGeneratedMaterial);
		check(IndexToGeneratedMaterial.Num() == 0);

		for (const auto& It : IndexToSurfaceType)
		{
			FVoxelMegaMaterialMaterialGeneratedMaterial& GeneratedMaterial = IndexToGeneratedMaterial.Add(It.Key);

			if (const FVoxelMegaMaterialMaterialGeneratedMaterial* OldGeneratedMaterial = OldIndexToGeneratedMaterial.Find(It.Key))
			{
				GeneratedMaterial = *OldGeneratedMaterial;
			}

			FVoxelMegaMaterialMaterialState State;
			State.MegaMaterial = MegaMaterialState;
			State.SurfaceType = It.Value;
			State.MetadataIndexToMetadata = MetadataIndexToMetadata;

			GenerateMaterial(
				*MegaMaterial,
				State,
				GeneratedMaterial,
				Context);
		}
	}

	// Process cubic surface types
	{
		VOXEL_SCOPE_COUNTER("Generate cubic texture arrays");

		if (MegaMaterial->CubicSurfaceTypes.Num() > 0)
		{
			FString Error;
			if (!CubicTextureArrayData.RebuildTextureArrays(this, MegaMaterial->CubicSurfaceTypes, Error))
			{
				VOXEL_MESSAGE(Error, "{0}: {1}", MegaMaterial, Error);
			}

			// Extract metadata from the cubic template material
			if (CubicTextureArrayData.Template &&
				CubicTextureArrayData.Template->Material)
			{
				if (const UMaterial* CubicBaseMaterial = CubicTextureArrayData.Template->Material->GetMaterial())
				{
					for (UVoxelMetadata* Metadata : FVoxelMegaMaterialGenerator::GetUsedMetadatas(*CubicBaseMaterial))
					{
						MetadataIndexToMetadata.AddUnique(Metadata);
					}
				}
			}

			// Generate material that uses texture arrays
			// Material is stored in MegaMaterial (not GeneratedData) to avoid invalidation loops
			if (!CubicTextureArrayData.GenerateCubicMaterial(*MegaMaterial, Error))
			{
				VOXEL_MESSAGE(Error, "{0}: Cubic material generation failed: {1}", MegaMaterial, Error);
			}

			// Ensure cubic material has the usage flag set
			FVoxelMaterialUsage::CheckMaterial(MegaMaterial->GeneratedCubicMaterialInstance);
		}
		else
		{
			CubicTextureArrayData.Reset();
			MegaMaterial->GeneratedCubicMaterial = nullptr;
			MegaMaterial->GeneratedCubicMaterialInstance = nullptr;
		}
	}

	// Fixup user-provided materials
	for (auto& It : TargetToMaterial)
	{
		if (!It.Value)
		{
			It.Value = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Default/VoxelWorldGridMaterial.VoxelWorldGridMaterial"));
			ensure(It.Value);
		}

		// Ensure material has the usage flag set
		FVoxelMaterialUsage::CheckMaterial(It.Value);
	}
}

void UVoxelMegaMaterialGeneratedData::RebuildIfNeeded(const UObject* ChangedObject)
{
	VOXEL_FUNCTION_COUNTER();

	ForEachObjectOfClass<UVoxelMegaMaterialGeneratedData>([&](UVoxelMegaMaterialGeneratedData& GeneratedData)
	{
		if (GeneratedData.WatchedObjects.Contains(ChangedObject))
		{
			GeneratedData.QueueRebuild();
		}
	});
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelMegaMaterialGeneratedData::GenerateMaterialForTarget(
	const UVoxelMegaMaterial& MegaMaterial,
	const EVoxelMegaMaterialTarget Target,
	const FVoxelMegaMaterialTargetBaseState& BaseState,
	TSharedPtr<FGlobalComponentRecreateRenderStateContext>& Context)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelMegaMaterialTargetGeneratedMaterial& GeneratedMaterial = TargetToGeneratedMaterial.FindOrAdd(Target);
	if (!GeneratedMaterial.Material)
	{
		GeneratedMaterial.Material = FVoxelUtilities::NewObject_Safe<UMaterial>(
			this,
			FName(MegaMaterial.GetName() + "_Generated_" + LexToString(Target)));
	}
	if (!GeneratedMaterial.Instance ||
		!ensureVoxelSlow(GeneratedMaterial.Instance->Parent == GeneratedMaterial.Material))
	{
		GeneratedMaterial.Instance = FVoxelUtilities::NewObject_Safe<UMaterialInstanceConstant>(
			this,
			FName(MegaMaterial.GetName() + "_GeneratedInstance_" + LexToString(Target)));

		GeneratedMaterial.Instance->Parent = GeneratedMaterial.Material;
	}

	FVoxelMegaMaterialTargetState State;
	static_cast<FVoxelMegaMaterialTargetBaseState&>(State) = BaseState;
	State.Target = Target;

	GeneratedMaterial.Update(
		MegaMaterial,
		State,
		Context);

	TargetToMaterial.Add(State.Target, GeneratedMaterial.Instance);
}

void UVoxelMegaMaterialGeneratedData::GenerateMaterial(
	const UVoxelMegaMaterial& MegaMaterial,
	const FVoxelMegaMaterialMaterialState& State,
	FVoxelMegaMaterialMaterialGeneratedMaterial& GeneratedMaterial,
	TSharedPtr<FGlobalComponentRecreateRenderStateContext>& Context)
{
	VOXEL_FUNCTION_COUNTER();

	if (!GeneratedMaterial.Material)
	{
		GeneratedMaterial.Material = FVoxelUtilities::NewObject_Safe<UMaterial>(
			this,
			FName(MegaMaterial.GetName() + "_Generated_" + State.SurfaceType.Object->GetName()));
	}
	if (!GeneratedMaterial.Instance ||
		!ensureVoxelSlow(GeneratedMaterial.Instance->Parent == GeneratedMaterial.Material))
	{
		GeneratedMaterial.Instance = FVoxelUtilities::NewObject_Safe<UMaterialInstanceConstant>(
			this,
			FName(MegaMaterial.GetName() + "_GeneratedInstance_" + State.SurfaceType.Object->GetName()));

		GeneratedMaterial.Instance->Parent = GeneratedMaterial.Material;
	}

	GeneratedMaterial.Update(
		MegaMaterial,
		State,
		Context);
}
#endif