// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelMegaMaterialGeneratedMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialGenerator.h"
#include "VoxelMaterialUsage.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "Nanite/VoxelMaterialSelectionCS.h"

#if WITH_EDITOR
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "ComponentRecreateRenderStateContext.h"
#endif

#if WITH_EDITOR
void FVoxelMegaMaterialTargetGeneratedMaterial::Update(
	const UObject& ErrorOwner,
	const FVoxelMegaMaterialTargetState& NewState,
	TSharedPtr<FGlobalComponentRecreateRenderStateContext>& Context)
{
	VOXEL_FUNCTION_COUNTER();
	check(Material);
	check(Instance);

	switch (NewState.Target)
	{
	default: check(false);
	case EVoxelMegaMaterialTarget::NonNanite:
	{
		Material->bUsedWithNanite = false;
		Material->bUsedWithVoxelPlugin = true;
	}
	break;
	case EVoxelMegaMaterialTarget::NaniteWPO:
	{
		Material->bUsedWithNanite = true;
		Material->bEnableTessellation = false;
	}
	break;
	case EVoxelMegaMaterialTarget::NaniteDisplacement:
	{
		Material->bUsedWithNanite = true;
		Material->bEnableTessellation = true;
	}
	break;
	case EVoxelMegaMaterialTarget::NaniteMaterialSelection:
	{
		Material->bUsedWithNanite = false;
		Material->bEnableTessellation = true;
		Material->bUsedWithVoxelMaterialSelection = true;
	}
	break;
	case EVoxelMegaMaterialTarget::Lumen:
	{
		Material->bUsedWithNanite = false;
		Material->bUsedWithVoxelPlugin = true;
	}
	break;
	}

	const double StartTime = FPlatformTime::Seconds();

	FString Diff;

	if (State.Equals(NewState, Diff))
	{
		FMaterialInstanceParameterUpdateContext UpdateContext(Instance);

		bool bChanged = false;
		for (const auto& It : State.IndexToSurfaceType)
		{
			if (!ensure(It.Value.MaterialInterface.Object))
			{
				continue;
			}

			bChanged |= FVoxelUtilities::CopyParameterValues(
				UpdateContext,
				*Instance,
				*It.Value.MaterialInterface.Object,
				"VOXELMATERIAL_" + FString::FromInt(It.Key.Index) + "_");
		}

		bChanged |= FVoxelMegaMaterialGenerator::ApplyBlendSmoothness(
			State.IndexToSurfaceType,
			*Material,
			*Instance);

		if (bChanged)
		{
			Instance->PostEditChange();
		}

		LOG_VOXEL(Verbose, "MegaMaterial %s target %s took %s, no material changes detected.",
			*ErrorOwner.GetPathName(),
			LexToString(State.Target),
			*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - StartTime));

		return;
	}

	State = NewState;

	LOG_VOXEL(Log, "Diffing MegaMaterial %s target %s took %s, material changes detected. Regenerating. Changes:\n\t%s",
		*ErrorOwner.GetPathName(),
		LexToString(State.Target),
		*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - StartTime),
		*Diff.Replace(TEXT("\n"), TEXT("\n\t")));

	if (!Context)
	{
		Context = MakeShared<FGlobalComponentRecreateRenderStateContext>();
	}

	if (!ensure(FVoxelMegaMaterialGenerator::GenerateMaterialForTarget(
		ErrorOwner,
		State,
		*Material,
		*Instance)))
	{
		return;
	}

	const double EndTime = FPlatformTime::Seconds();

	LOG_VOXEL(Display, "Regenerating MegaMaterial %s target %s took %s",
		*ErrorOwner.GetPathName(),
		LexToString(State.Target),
		*FVoxelUtilities::SecondsToString(EndTime - StartTime));
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void FVoxelMegaMaterialMaterialGeneratedMaterial::Update(
	const UObject& ErrorOwner,
	const FVoxelMegaMaterialMaterialState& NewState,
	TSharedPtr<FGlobalComponentRecreateRenderStateContext>& Context)
{
	VOXEL_FUNCTION_COUNTER();
	check(Material);
	check(Instance);

	const double StartTime = FPlatformTime::Seconds();

	FString Diff;

	if (State.Equals(NewState, Diff))
	{
		FMaterialInstanceParameterUpdateContext UpdateContext(Instance);

		if (FVoxelUtilities::CopyParameterValues(
			UpdateContext,
			*Instance,
			*State.SurfaceType.MaterialInterface.Object,
			{}))
		{
			Instance->PostEditChange();
		}

		LOG_VOXEL(Verbose, "MegaMaterial %s material %s took %s, no material changes detected.",
			*ErrorOwner.GetPathName(),
			*State.SurfaceType.Object->GetName(),
			*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - StartTime));

		return;
	}

	State = NewState;

	LOG_VOXEL(Log, "Diffing MegaMaterial %s material %s took %s, material changes detected. Regenerating. Changes:\n\t%s",
		*ErrorOwner.GetPathName(),
		*State.SurfaceType.Object->GetName(),
		*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - StartTime),
		*Diff.Replace(TEXT("\n"), TEXT("\n\t")));

	if (!Context)
	{
		Context = MakeShared<FGlobalComponentRecreateRenderStateContext>();
	}

	if (!ensure(FVoxelMegaMaterialGenerator::GenerateMaterial(
		ErrorOwner,
		State,
		*Material,
		*Instance)))
	{
		return;
	}

	LOG_VOXEL(Display, "Regenerating MegaMaterial %s material %s took %s",
		*ErrorOwner.GetPathName(),
		*State.SurfaceType.Object->GetName(),
		*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - StartTime));
}
#endif