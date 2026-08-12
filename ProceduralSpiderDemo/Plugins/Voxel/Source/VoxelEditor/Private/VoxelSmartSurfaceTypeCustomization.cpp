// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelGraphEnvironment.h"
#include "VoxelParameterOverridesDetails.h"
#include "Surface/VoxelSmartSurfaceType.h"

VOXEL_CUSTOMIZE_CLASS(UVoxelSmartSurfaceType)(IDetailLayoutBuilder& DetailLayout)
{
	FVoxelEditorUtilities::EnableRealtime();

	KeepAlive(FVoxelParameterOverridesDetails::Create(
		DetailLayout,
		[&](TVoxelArray<FVoxelParameterOverridesDetails::FWeakOwner>& OutOwners)
		{
			for (UVoxelSmartSurfaceType* SurfaceType : GetObjectsBeingCustomized(DetailLayout))
			{
				OutOwners.Add(FVoxelParameterOverridesDetails::FWeakOwner
				{
					SurfaceType,
					MakeWeakObjectPtrLambda(SurfaceType, [SurfaceType]
					{
						return SurfaceType->GetParameterOverrides().GetHash();
					}),
					MakeWeakObjectPtrLambda(SurfaceType, [SurfaceType](FVoxelDependencyCollector& DependencyCollector)
					{
						return FVoxelGraphEnvironment::CreatePreview(
							SurfaceType,
							*SurfaceType,
							FTransform::Identity,
							DependencyCollector);
					}),
					MakeWeakObjectPtrLambda(SurfaceType, [=]
					{
						SurfaceType->PreEditChange(&FindFPropertyChecked(UVoxelSmartSurfaceType, ParameterOverrides));
					}),
					MakeWeakObjectPtrLambda(SurfaceType, [=]
					{
						FPropertyChangedEvent Event(&FindFPropertyChecked(UVoxelSmartSurfaceType, ParameterOverrides));
						SurfaceType->PostEditChangeProperty(Event);
					})
				});
			}
		},
		FVoxelEditorUtilities::MakeRefreshDelegate(this, DetailLayout)));
}