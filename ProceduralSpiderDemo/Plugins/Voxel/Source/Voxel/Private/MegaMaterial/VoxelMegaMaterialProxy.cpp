// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialUsageTracker.h"
#include "MegaMaterial/VoxelMegaMaterialGeneratedData.h"
#include "MegaMaterial/VoxelCubicTextureArrayData.h"
#include "VoxelWorld.h"
#include "VoxelSortNetwork.h"
#include "Surface/VoxelSurfaceTypeBlend.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "Surface/VoxelCubicSurfaceTypeAsset.h"
#include "Surface/VoxelCubicTextureTemplate.h"

#include "MaterialShared.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DArray.h"
#include "Materials/MaterialInstanceConstant.h"

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "ScopedTransaction.h"
#include "Editor/EditorEngine.h"
#include "Subsystems/AssetEditorSubsystem.h"
#endif

TSharedRef<FVoxelMegaMaterialProxy> FVoxelMegaMaterialProxy::Default()
{
	check(IsInGameThread());

	UVoxelMegaMaterial* MegaMaterial = LoadObject<UVoxelMegaMaterial>(nullptr, TEXT("/Voxel/Default/DefaultMegaMaterial.DefaultMegaMaterial"));
	if (ensure(MegaMaterial))
	{
		// Ensure the default material doesn't get GCed, as it's not referenced by the voxel world
		MegaMaterial->AddToRoot();
	}
	else
	{
		MegaMaterial = GetMutableDefault<UVoxelMegaMaterial>();
	}

	return MegaMaterial->GetProxy();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMaterialRenderIndex FVoxelMegaMaterialProxy::GetRenderIndex(const FVoxelSurfaceType SurfaceType) const
{
	if (SurfaceType.IsNull())
	{
		return FVoxelMaterialRenderIndex(0);
	}

	if (const FVoxelMaterialRenderIndex* RenderIndexPtr = SurfaceTypeToRenderIndex.Find(SurfaceType))
	{
		return *RenderIndexPtr;
	}

#if WITH_EDITOR
	if (GIsEditor &&
		bDetectNewSurfaces)
	{
		UsageTracker->NotifyMissingSurfaceType(SurfaceType);
	}
#endif

	return FVoxelMaterialRenderIndex(0);
}

TConstVoxelArrayView<FVoxelMetadataRef> FVoxelMegaMaterialProxy::GetUsedMetadatas(const FVoxelMaterialRenderIndex RenderIndex) const
{
	if (RenderIndex.Index == 0 ||
		!ensureVoxelSlow(RenderIndexToUsedMetadatas.Contains(RenderIndex)))
	{
		return {};
	}

	return RenderIndexToUsedMetadatas[RenderIndex];
}

int32 FVoxelMegaMaterialProxy::GetCubicSliceIndex(const FVoxelMaterialRenderIndex RenderIndex) const
{
	const int32* SliceIndexPtr = RenderIndexToCubicSliceIndex.Find(RenderIndex);
	if (SliceIndexPtr)
	{
		return *SliceIndexPtr;
	}
	return -1; // -1 indicates not a cubic surface
}

TVoxelArray<FVoxelRenderMaterial> FVoxelMegaMaterialProxy::GetRenderMaterials(const TConstVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypes) const
{
	VOXEL_FUNCTION_COUNTER_NUM(SurfaceTypes.Num(), 1);

	TVoxelMap<FVoxelSurfaceType, FVoxelMaterialRenderIndex> LocalSurfaceTypeToRenderIndex;
	LocalSurfaceTypeToRenderIndex.Reserve(32);

	TVoxelArray<FVoxelRenderMaterial> RenderMaterials;
	FVoxelUtilities::SetNumZeroed(RenderMaterials, SurfaceTypes.Num());

	for (int32 BlendIndex = 0; BlendIndex < SurfaceTypes.Num(); BlendIndex++)
	{
		FVoxelSurfaceTypeBlend SurfaceType = SurfaceTypes[BlendIndex];
		FVoxelRenderMaterial& RenderMaterial = RenderMaterials[BlendIndex];

		// Default value of 255
		RenderMaterial.Indices.Memset(0xFF);

		if (SurfaceType.IsNull())
		{
			continue;
		}

		if (SurfaceType.GetLayers().Num() > 8)
		{
			SurfaceType.PopLayersForRendering();
		}
		const TConstVoxelArrayView<FVoxelSurfaceTypeBlendLayer> Layers = SurfaceType.GetLayers();
		checkVoxelSlow(Layers.Num() <= 8);

		// Always re-normalize to reduce float inaccuracies
		double Multiplier;
		{
			double WeightSum = 0.f;
			for (const FVoxelSurfaceTypeBlendLayer& Layer : Layers)
			{
				WeightSum += Layer.Weight.ToFloat();
			}
			checkVoxelSlow(WeightSum > 0.f);

			Multiplier = 1.f / WeightSum;
		}

		for (int32 Index = 0; Index < Layers.Num(); Index++)
		{
			const FVoxelSurfaceTypeBlendLayer& Layer = Layers[Index];

			const FVoxelMaterialRenderIndex* RenderIndexPtr = LocalSurfaceTypeToRenderIndex.Find(Layer.Type);
			if (!RenderIndexPtr)
			{
				RenderIndexPtr = &LocalSurfaceTypeToRenderIndex.Add_CheckNew(Layer.Type, GetRenderIndex(Layer.Type));
			}

			// Otherwise LayerMask breaks on the GPU side
			checkVoxelSlow(RenderIndexPtr->Index < 128);

			RenderMaterial.Indices[Index] = *RenderIndexPtr;
			RenderMaterial.Alphas[Index] = FVoxelUtilities::FloatToUINT8(Layer.Weight.ToFloat() * Multiplier);
		}

		TVoxelSortNetwork<8>::Apply([&](const int32 IndexA, const int32 IndexB)
		{
			if (RenderMaterial.Indices[IndexA] < RenderMaterial.Indices[IndexB])
			{
				return;
			}

			Swap(RenderMaterial.Indices[IndexA], RenderMaterial.Indices[IndexB]);
			Swap(RenderMaterial.Alphas[IndexA], RenderMaterial.Alphas[IndexB]);
		});
	}

	return RenderMaterials;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void FVoxelMegaMaterialProxy::LogErrors(
	const ERHIFeatureLevel::Type FeatureLevel,
	const TSet<EVoxelMegaMaterialTarget>& Targets,
	AVoxelWorld* VoxelWorld) const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	UVoxelMegaMaterial* MegaMaterial = WeakMegaMaterial.Resolve();
	if (!ensure(MegaMaterial))
	{
		return;
	}

	if (MegaMaterial->GetPathName() == "/Voxel/Default/DefaultMegaMaterial.DefaultMegaMaterial")
	{
		INLINE_LAMBDA
		{
			if (const TSharedPtr<FVoxelNotification> Notification = MegaMaterial->EmptyMegaMaterialNotification.Pin())
			{
				Notification->ResetExpiration();
				return;
			}

			const TSharedRef<FVoxelNotification> Notification = FVoxelNotification::Create("Voxel World has no mega material");

			Notification->AddButton_ExpireOnClick(
				"Show actor",
				"Will select voxel world without Mega Material",
				MakeWeakObjectPtrLambda(VoxelWorld, [VoxelWorld]
				{
					FVoxelUtilities::FocusObject(VoxelWorld);
				}));

			Notification->AddButton_ExpireOnClick(
				"Create material",
				"Will create new Mega Material and assign it to Voxel World",
				MakeWeakObjectPtrLambda(VoxelWorld, [VoxelWorld]
				{
					IVoxelFactory* Factory = IVoxelAutoFactoryInterface::GetInterface().MakeFactory(UVoxelMegaMaterial::StaticClass());
					if (!ensure(Factory))
					{
						return;
					}

					FScopedTransaction Transaction(TEXT("Create Mega Material"), INVTEXT("Create Mega Material"), VoxelWorld);

					const FString DefaultPath = "/Game";
					const FString DefaultName = "VoxelMegaMaterial";

					const FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

					UVoxelMegaMaterial* NewMegaMaterial = Cast<UVoxelMegaMaterial>(AssetToolsModule.Get().CreateAssetWithDialog(DefaultName, DefaultPath, UVoxelMegaMaterial::StaticClass(), Factory->GetUFactory()));
					if (!NewMegaMaterial)
					{
						return;
					}

					VoxelWorld->MegaMaterial = NewMegaMaterial;

					GEditor->SelectNone(false, true);
					GEditor->SelectActor(VoxelWorld, true, false, true);
					GEditor->NoteSelectionChange();

					GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorsForAssets({ NewMegaMaterial });
				}));

			Notification->ExpireAndFadeout();

			MegaMaterial->EmptyMegaMaterialNotification = Notification;
		};
	}

	if (MegaMaterial->bEditorOnly)
	{
		TVoxelArray<UMaterialInterface*> Materials;
		for (const EVoxelMegaMaterialTarget Target : TEnumRange<EVoxelMegaMaterialTarget>())
		{
			Materials.Add(TargetToMaterial.FindRef(Target).Resolve());
		}

#if VOXEL_ENGINE_VERSION >= 507
		const EShaderPlatform ShaderPlatform = GetFeatureLevelShaderPlatform_Checked(FeatureLevel);
#endif
		for (const UMaterialInterface* Material : Materials)
		{
			if (!Material)
			{
				continue;
			}

			const FMaterialResource* Resource = Material->GetMaterialResource(UE_507_SWITCH(FeatureLevel, ShaderPlatform));
			if (!Resource ||
				Resource->GetCompileErrors().Num() == 0)
			{
				continue;
			}

			// Clear materials
			MegaMaterial->SurfaceTypes.Reset();
			MegaMaterial->GetGeneratedData_EditorOnly().QueueRebuild();
			return;
		}

		return;
	}

	if (MegaMaterial->GetGeneratedData_EditorOnly().IsRebuildQueued())
	{
		GEngine->AddOnScreenDebugMessage(
			uint64(this),
			0.1f,
			FColor::Yellow,
			MegaMaterial->GetName() + " is waiting for shaders to compile");
		return;
	}

	TVoxelArray<UMaterialInterface*> Materials;
	for (const EVoxelMegaMaterialTarget Target : TEnumRange<EVoxelMegaMaterialTarget>())
	{
		Materials.Add(TargetToMaterial.FindRef(Target).Resolve());
	}

	for (const UMaterialInterface* Material : Materials)
	{
		if (!Material)
		{
			GEngine->AddOnScreenDebugMessage(
				uint64(this),
				0.1f,
				FColor::Yellow,
				MegaMaterial->GetName() + " is not generated");
			return;
		}
	}

	for (const UMaterialInterface* Material : Materials)
	{
		if (Material->IsCompiling())
		{
			GEngine->AddOnScreenDebugMessage(
				uint64(this),
				0.1f,
				FColor::Yellow,
				MegaMaterial->GetName() + " is compiling");
			return;
		}
	}

#if VOXEL_ENGINE_VERSION >= 507
	const EShaderPlatform ShaderPlatform = GetFeatureLevelShaderPlatform_Checked(FeatureLevel);
#endif

	TArray<FString> CompileErrors;
	for (const UMaterialInterface* Material : Materials)
	{
		const FMaterialResource* Resource = Material->GetMaterialResource(UE_507_SWITCH(FeatureLevel, ShaderPlatform));
		if (!Resource)
		{
			GEngine->AddOnScreenDebugMessage(
				uint64(this),
				0.1f,
				FColor::Red,
				MegaMaterial->GetName() + " failed to compile (resource is null)");
			return;
		}

		CompileErrors.Append(Resource->GetCompileErrors());
	}

	for (const EVoxelMegaMaterialTarget Target : Targets)
	{
		const UMaterialInterface* Material;
		switch (Target)
		{
		default: ensure(false);
		case EVoxelMegaMaterialTarget::NonNanite:
		{
			if (MegaMaterial->NonNaniteMaterialType == EVoxelMegaMaterialGenerationType::Generated)
			{
				continue;
			}

			Material = MegaMaterial->CustomNonNaniteMaterial;
		}
		break;
		case EVoxelMegaMaterialTarget::NaniteWPO:
		case EVoxelMegaMaterialTarget::NaniteDisplacement:
		case EVoxelMegaMaterialTarget::NaniteMaterialSelection:
		{
			if (MegaMaterial->NaniteDisplacementMaterialType == EVoxelMegaMaterialGenerationType::Generated)
			{
				continue;
			}

			Material = MegaMaterial->CustomNaniteDisplacementMaterial;
		}
		break;
		case EVoxelMegaMaterialTarget::Lumen:
		{
			if (MegaMaterial->LumenMaterialType == EVoxelMegaMaterialGenerationType::Generated)
			{
				continue;
			}

			Material = MegaMaterial->CustomLumenMaterial;
		}
		break;
		}

		if (Material)
		{
			continue;
		}

		if (const TSharedPtr<FVoxelNotification> Notification = MegaMaterial->TargetToNotification.FindRef(Target).Pin())
		{
			Notification->ResetExpiration();
			continue;
		}

		const TSharedRef<FVoxelNotification> Notification = FVoxelNotification::Create(MegaMaterial->GetName() + " is missing a material for target " + LexToString(Target));

		Notification->AddButton(
			"Generate",
			"Will set the material to generate automatically",
			MakeWeakObjectPtrLambda(MegaMaterial, [Target, MegaMaterial]
			{
				const FString Text = FString("Set ") + LexToString(Target) + " to generate";
				const FScopedTransaction Transaction(*Text, FText::FromString(Text), MegaMaterial);
				MegaMaterial->PreEditChange(nullptr);

				switch (Target)
				{
				default: ensure(false);
				case EVoxelMegaMaterialTarget::NonNanite:
				{
					MegaMaterial->NonNaniteMaterialType = EVoxelMegaMaterialGenerationType::Generated;
				}
				break;
				case EVoxelMegaMaterialTarget::NaniteWPO:
				case EVoxelMegaMaterialTarget::NaniteDisplacement:
				case EVoxelMegaMaterialTarget::NaniteMaterialSelection:
				{
					MegaMaterial->NaniteDisplacementMaterialType = EVoxelMegaMaterialGenerationType::Generated;
				}
				break;
				case EVoxelMegaMaterialTarget::Lumen:
				{
					MegaMaterial->LumenMaterialType = EVoxelMegaMaterialGenerationType::Generated;
				}
				break;
				}

				MegaMaterial->PostEditChange();
			}));

		Notification->AddButton(
			"Open",
			"Open " + MegaMaterial->GetName(),
			MakeWeakObjectPtrLambda(MegaMaterial, [MegaMaterial]
			{
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorsForAssets({ MegaMaterial });
			}));

		Notification->ExpireAndFadeout();

		MegaMaterial->TargetToNotification.FindOrAdd(Target) = Notification;
	}

	if (CompileErrors.Num() == 0)
	{
		return;
	}

	CompileErrors = TSet<FString>(CompileErrors).Array();

	CompileErrors.RemoveAll([](const FString& Error)
	{
		return Error.Contains("warning: Gradient operations are not affected by wave-sensitive data or control flow");
	});

	GEngine->AddOnScreenDebugMessage(
		uint64(this),
		0.1f,
		FColor::Red,
		MegaMaterial->GetName() + " failed to compile\n" + FString::Join(CompileErrors, TEXT("\n")));
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMegaMaterialProxy::FVoxelMegaMaterialProxy(UVoxelMegaMaterial& MegaMaterial)
	: WeakMegaMaterial(MegaMaterial)
	, bDetectNewSurfaces(MegaMaterial.bDetectNewSurfaces)
#if WITH_EDITOR
	, UsageTracker(MegaMaterial.UsageTracker.ToSharedRef())
#endif
{
}

void FVoxelMegaMaterialProxy::Initialize(const FVoxelMegaMaterialProxy* OldProxy)
{
	VOXEL_FUNCTION_COUNTER();

	UVoxelMegaMaterial* MegaMaterial = WeakMegaMaterial.Resolve();
	if (!ensure(MegaMaterial))
	{
		return;
	}

#if WITH_EDITOR
	UVoxelMegaMaterialGeneratedData* GeneratedData = &MegaMaterial->GetGeneratedData_EditorOnly();
#else
	UVoxelMegaMaterialGeneratedData* GeneratedData = MegaMaterial->GeneratedData;
#endif

	if (!GeneratedData)
	{
		return;
	}

	if (!ensureVoxelSlow(GeneratedData->IndexToSurfaceInfo.Num() == GeneratedData->IndexToGeneratedMaterial.Num()))
	{
#if WITH_EDITOR
		// Force a re-save
		GeneratedData->QueueRebuild();
		GeneratedData->MarkPackageDirty();
#endif

		return;
	}

	for (const auto& It : GeneratedData->IndexToSurfaceInfo)
	{
		if (!ensureVoxelSlow(It.Value.SurfaceType) ||
			!ensureVoxelSlow(GeneratedData->IndexToGeneratedMaterial.Contains(It.Key)))
		{
			return;
		}
	}

	RenderIndexToMaterial.Reserve(GeneratedData->IndexToSurfaceInfo.Num());
	RenderIndexToUsedMetadatas.Reserve(GeneratedData->IndexToSurfaceInfo.Num());
	SurfaceTypeToRenderIndex.Reserve(GeneratedData->IndexToSurfaceInfo.Num());

	for (const auto& It : GeneratedData->IndexToSurfaceInfo)
	{
		const FVoxelMegaMaterialSurfaceInfo& SurfaceInfo = It.Value;
		const FVoxelMegaMaterialMaterialGeneratedMaterial& GeneratedMaterial = GeneratedData->IndexToGeneratedMaterial[It.Key];

		RenderIndexToMaterial.Add_EnsureNew(
			It.Key,
			MakeVoxelObjectPtr(GeneratedMaterial.Instance));

		RenderIndexToUsedMetadatas.Add_EnsureNew(
			It.Key,
			TVoxelArray<FVoxelMetadataRef>(SurfaceInfo.UsedMetadatas));

		SurfaceTypeToRenderIndex.Add_EnsureNew(
			FVoxelSurfaceType(SurfaceInfo.SurfaceType),
			It.Key);
	}

	MetadataIndexToMetadata.Reserve(GeneratedData->MetadataIndexToMetadata.Num());
	for (UVoxelMetadata* Metadata : GeneratedData->MetadataIndexToMetadata)
	{
		MetadataIndexToMetadata.Add_EnsureNoGrow(FVoxelMetadataRef(Metadata));
	}

	TargetToMaterial.Reserve(GeneratedData->TargetToMaterial.Num());
	for (const auto& It : GeneratedData->TargetToMaterial)
	{
		TargetToMaterial.Add_EnsureNew(It.Key, It.Value);
	}

	// Initialize cubic data
	if (MegaMaterial->CubicTextureTemplate)
	{
		// Use generated material if available, otherwise fall back to template material
		if (MegaMaterial->GeneratedCubicMaterialInstance)
		{
			CubicMaterial = MegaMaterial->GeneratedCubicMaterialInstance;
		}
		else
		{
			CubicMaterial = MegaMaterial->CubicTextureTemplate->Material;
		}

		// Use generated translucent material if available, otherwise fall back to base cubic material
		if (MegaMaterial->GeneratedCubicMaterialTranslucentInstance)
		{
			CubicMaterialTranslucent = MegaMaterial->GeneratedCubicMaterialTranslucentInstance;
		}
		else
		{
			// Fall back to base cubic material for translucent surfaces
			// This won't have proper translucency but at least renders something
			CubicMaterialTranslucent = CubicMaterial;
		}

		bHideFacesAtNaNBoundaries = MegaMaterial->CubicTextureTemplate->bHideFacesAtNaNBoundaries;
	}

	for (const auto& It : GeneratedData->CubicTextureArrayData.SlotToTextureArray)
	{
		if (It.Value.TextureArray)
		{
			CubicTextureArrays.Add_EnsureNew(It.Value.SlotName, It.Value.TextureArray);
		}
	}

	// Register cubic surfaces and build render index -> slice index mapping
	// Cubic surfaces use the shared CubicMaterial rather than generated per-surface materials
	// Render indices are assigned based on slice index directly (+ 1 since 0 is reserved for null)
	// This is safe because a mesh never uses both cubic and non-cubic surfaces, so indices can overlap
	for (const auto& It : GeneratedData->CubicTextureArrayData.CubicSurfaceToSliceIndex)
	{
		if (UVoxelCubicSurfaceTypeAsset* CubicSurface = It.Key)
		{
			// Creating FVoxelSurfaceType auto-registers the surface and assigns an internal index
			const FVoxelSurfaceType SurfaceType(CubicSurface);

			// Assign render index based on slice index directly
			// + 1 because render index 0 is reserved for null/default surfaces
			const FVoxelMaterialRenderIndex RenderIndex(It.Value + 1);

			SurfaceTypeToRenderIndex.Add_EnsureNew(SurfaceType, RenderIndex);
			RenderIndexToCubicSliceIndex.Add_EnsureNew(RenderIndex, It.Value);
		}
	}

	// Add cubic materials to RenderIndexToMaterial for debugging:
	// Index 1 = opaque/masked, Index 2 = translucent
	if (CubicMaterial.Resolve())
	{
		RenderIndexToMaterial.FindOrAdd(FVoxelMaterialRenderIndex(1)) = CubicMaterial;
	}
	if (CubicMaterialTranslucent.Resolve())
	{
		RenderIndexToMaterial.FindOrAdd(FVoxelMaterialRenderIndex(2)) = CubicMaterialTranslucent;
	}

	// If we have cubic surfaces, use the cubic material for all rendering targets
	// This ensures cubic rendering uses the template's material regardless of custom/generated toggles
	if (HasCubicSurfaces() &&
		CubicMaterial.Resolve())
	{
		for (const EVoxelMegaMaterialTarget Target : TEnumRange<EVoxelMegaMaterialTarget>())
		{
			TargetToMaterial.FindOrAdd(Target) = CubicMaterial;
		}
	}
}

bool FVoxelMegaMaterialProxy::Equals(const FVoxelMegaMaterialProxy& Other) const
{
	VOXEL_FUNCTION_COUNTER();

	if (WeakMegaMaterial != Other.WeakMegaMaterial ||
		bDetectNewSurfaces != Other.bDetectNewSurfaces)
	{
		return false;
	}

	if (!RenderIndexToMaterial.OrderIndependentEqual(Other.RenderIndexToMaterial) ||
		!RenderIndexToUsedMetadatas.OrderIndependentEqual(Other.RenderIndexToUsedMetadatas) ||
		!SurfaceTypeToRenderIndex.OrderIndependentEqual(Other.SurfaceTypeToRenderIndex))
	{
		return false;
	}

	if (MetadataIndexToMetadata != Other.MetadataIndexToMetadata ||
		!TargetToMaterial.OrderIndependentEqual(Other.TargetToMaterial))
	{
		return false;
	}

	// Compare cubic data
	if (CubicMaterial != Other.CubicMaterial ||
		CubicMaterialTranslucent != Other.CubicMaterialTranslucent ||
		bHideFacesAtNaNBoundaries != Other.bHideFacesAtNaNBoundaries ||
		!CubicTextureArrays.OrderIndependentEqual(Other.CubicTextureArrays) ||
		!RenderIndexToCubicSliceIndex.OrderIndependentEqual(Other.RenderIndexToCubicSliceIndex))
	{
		return false;
	}

	return true;
}