// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nanite/VoxelNaniteMaterialRenderer.h"
#include "Nanite/VoxelNaniteMesh.h"
#include "Nanite/VoxelNaniteMaterialRendererImpl.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "MegaMaterial/VoxelMegaMaterialRenderUtilities.h"
#include "VoxelMesh.h"
#include "Render/VoxelTexturePool.h"
#include "Render/VoxelTextureManager.h"
#include "Render/VoxelRenderSubsystem.h"

#include "TextureResource.h"
#include "MaterialCachedData.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"

FVoxelNaniteMaterialRenderer::FVoxelNaniteMaterialRenderer(const TSharedRef<FVoxelMegaMaterialProxy>& MegaMaterialProxy)
	: Impl(FVoxelNaniteMaterialRendererImpl::Create(MegaMaterialProxy))
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelMaterialInstanceRef> FVoxelNaniteMaterialRenderer::GetMaterialInstance(const FVoxelMaterialRenderIndex RenderIndex) const
{
	return Impl->GetMaterialInstance(RenderIndex);
}

void FVoxelNaniteMaterialRenderer::PrepareRender(TVoxelSet<TSharedPtr<const FVoxelNaniteMesh>>&& NewMeshes)
{
	VOXEL_FUNCTION_COUNTER();

	Meshes = MoveTemp(NewMeshes);

	{
		TVoxelSet<FVoxelSurfaceType> NewUsedSurfaceTypes;
		NewUsedSurfaceTypes.Reserve(64);

		for (const TSharedPtr<const FVoxelNaniteMesh>& Mesh : Meshes)
		{
			NewUsedSurfaceTypes.Append(Mesh->Mesh->UsedSurfaceTypes);
		}

		UsedSurfaceTypes = NewUsedSurfaceTypes.Array();
	}
	UsedSurfaceTypes.AddUnique(FVoxelSurfaceType());
	UsedSurfaceTypes.Sort();
}

void FVoxelNaniteMaterialRenderer::UpdateRender(
	const FVoxelRenderSubsystem& Subsystem,
	const FTransform& NewLocalToWorld)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	{
		VOXEL_SCOPE_COUNTER("Update material instances");

		for (const auto& It : Impl->MaterialIndexToMaterialInstance)
		{
			UMaterialInstanceDynamic* Instance = It.Value->GetInstance();
			if (!ensure(Instance))
			{
				continue;
			}

			Subsystem.GetTextureManager().UpdateInstance(*Instance);
		}
	}

	using FQueuedData = FVoxelNaniteMaterialRendererImpl::FQueuedData;

	const TSharedRef<FQueuedData> QueuedData = MakeSharedCopy(FQueuedData
	{
		Subsystem.GetMaterialInstanceRef(EVoxelMegaMaterialTarget::NaniteMaterialSelection),
		NewLocalToWorld,
		UsedSurfaceTypes,
		Subsystem.GetConfig().VoxelWorldId
	});

	Voxel::RenderTask([Impl = Impl, QueuedData]
	{
		Impl->QueuedData_RenderThread = QueuedData;
	});
}