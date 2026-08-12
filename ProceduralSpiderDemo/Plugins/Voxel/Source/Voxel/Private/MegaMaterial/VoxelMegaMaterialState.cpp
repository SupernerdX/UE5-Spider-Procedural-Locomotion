// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelMegaMaterialState.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialInstance.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialPrivateAccess.h"

#if WITH_EDITOR
FVoxelMaterialFunctionBaseState::FVoxelMaterialFunctionBaseState(const UMaterialFunction& MaterialFunction)
{
	Object = &MaterialFunction;
	StateId = MaterialFunction.StateId;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelMaterialFunctionState::FVoxelMaterialFunctionState(const UMaterialFunction& MaterialFunction)
	: FVoxelMaterialFunctionBaseState(MaterialFunction)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelSet<UMaterialFunction*> MaterialFunctions;
	MaterialFunctions.Reserve(32);

	FVoxelUtilities::GetMaterialFunctionsUsed(MaterialFunction, MaterialFunctions);

	UsedFunctions.Reserve(MaterialFunctions.Num());

	for (const UMaterialFunction* UsedFunction : MaterialFunctions)
	{
		UsedFunctions.Emplace(*UsedFunction);
	}

	UsedFunctions.Sort([](const FVoxelMaterialFunctionBaseState& A, const FVoxelMaterialFunctionBaseState& B)
	{
		return A.Object->GetFName().LexicalLess(B.Object->GetFName());
	});
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelMaterialState::FVoxelMaterialState(const UMaterial& Material)
{
	VOXEL_FUNCTION_COUNTER();

	Object = &Material;
	StateId = Material.StateId;
	bTangentSpaceNormal = Material.bTangentSpaceNormal;
	ShadingModel = PrivateAccess::ShadingModel(Material);

	TVoxelSet<UMaterialFunction*> MaterialFunctions;
	MaterialFunctions.Reserve(32);

	FVoxelUtilities::GetMaterialFunctionsUsed(Material, MaterialFunctions);

	UsedFunctions.Reserve(MaterialFunctions.Num());

	for (const UMaterialFunction* MaterialFunction : MaterialFunctions)
	{
		UsedFunctions.Emplace(*MaterialFunction);
	}

	UsedFunctions.Sort([](const FVoxelMaterialFunctionBaseState& A, const FVoxelMaterialFunctionBaseState& B)
	{
		return A.Object->GetFName().LexicalLess(B.Object->GetFName());
	});
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelMaterialInstanceBaseState::FVoxelMaterialInstanceBaseState(const UMaterialInstance& MaterialInstance)
{
	Object = &MaterialInstance;
	BasePropertyOverrides = MaterialInstance.BasePropertyOverrides;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelMaterialInterfaceState::FVoxelMaterialInterfaceState(const UMaterialInterface& MaterialInterface)
{
	VOXEL_FUNCTION_COUNTER();

	Object = &MaterialInterface;
	DisplacementScaling = MaterialInterface.GetDisplacementScaling();

	if (const UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(&MaterialInterface))
	{
		TVoxelInlineSet<const UMaterialInstance*, 8> VisitedMaterialInstances;

		while (MaterialInstance)
		{
			if (!ensure(VisitedMaterialInstances.TryAdd(MaterialInstance)))
			{
				// Loop
				break;
			}

			MaterialInstanceHierarchy.Emplace(*MaterialInstance);
			MaterialInstance = Cast<UMaterialInstance>(MaterialInstance->Parent);
		}
	}

	if (const UMaterial* MaterialObject = MaterialInterface.GetMaterial())
	{
		Material = FVoxelMaterialState(*MaterialObject);
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelMegaMaterialState::FVoxelMegaMaterialState(const UVoxelMegaMaterial& MegaMaterial)
{
	VOXEL_FUNCTION_COUNTER();

	if (MegaMaterial.AttributePostProcess)
	{
		AttributePostProcess = FVoxelMaterialFunctionState(*MegaMaterial.AttributePostProcess);
	}

	bEnableSmoothBlends = MegaMaterial.bEnableSmoothBlends;
	bEnableDitherNoiseTexture = MegaMaterial.bEnableDitherNoiseTexture;
	DitherNoiseTexture = MegaMaterial.DitherNoiseTexture;
	bGenerateMaskedMaterial = MegaMaterial.bGenerateMaskedMaterial;
	bGenerateTwoSidedMaterial = MegaMaterial.bGenerateTwoSidedMaterial;
	bSetHasPixelAnimation = MegaMaterial.bSetHasPixelAnimation;
	bEnablePixelDepthOffset = MegaMaterial.bEnablePixelDepthOffset;

	if (MegaMaterial.CustomOutputsMaterial)
	{
		CustomOutputsMaterial = FVoxelMaterialInterfaceState(*MegaMaterial.CustomOutputsMaterial);
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelSurfaceTypeState::FVoxelSurfaceTypeState(const UVoxelSurfaceTypeAsset& Asset)
{
	VOXEL_FUNCTION_COUNTER();

	Object = &Asset;
	BlendSmoothness = Asset.BlendSmoothness;
	Seed = Asset.Seed;

	if (Asset.Material)
	{
		MaterialInterface = FVoxelMaterialInterfaceState(*Asset.Material);
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelMegaMaterialTargetState::Equals(
	const FVoxelMegaMaterialTargetState& New,
	FString& OutDiff) const
{
	VOXEL_FUNCTION_COUNTER();

	const FConstVoxelStructView ThisView = FConstVoxelStructView::Make(*this);
	const FConstVoxelStructView NewView = FConstVoxelStructView::Make(New);

	if (ThisView.Identical(NewView))
	{
		return true;
	}

	OutDiff = FString::Join(ThisView.GetChanges(NewView), TEXT("\n"));
	return false;
}

bool FVoxelMegaMaterialMaterialState::Equals(
	const FVoxelMegaMaterialMaterialState& New,
	FString& OutDiff) const
{
	VOXEL_FUNCTION_COUNTER();

	const FConstVoxelStructView ThisView = FConstVoxelStructView::Make(*this);
	const FConstVoxelStructView NewView = FConstVoxelStructView::Make(New);

	if (ThisView.Identical(NewView))
	{
		return true;
	}

	OutDiff = FString::Join(ThisView.GetChanges(NewView), TEXT("\n"));
	return false;
}