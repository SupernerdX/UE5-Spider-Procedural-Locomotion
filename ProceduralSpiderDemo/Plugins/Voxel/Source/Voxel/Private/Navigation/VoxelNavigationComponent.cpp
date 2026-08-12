// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Navigation/VoxelNavigationComponent.h"
#include "Navigation/VoxelNavigationMesh.h"
#include "Navigation/VoxelNavigationMeshSceneProxy.h"
#include "AI/NavigationSystemBase.h"
#include "AI/NavigationSystemHelpers.h"

UVoxelNavigationComponent::UVoxelNavigationComponent()
{
	bCanEverAffectNavigation = true;
	bHasCustomNavigableGeometry = EHasCustomNavigableGeometry::EvenIfNotCollidable;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelNavigationComponent::UpdateNavigationMesh()
{
	VOXEL_FUNCTION_COUNTER();

	UpdateBounds();
	MarkRenderStateDirty();

	if (IsRegistered() &&
		GetWorld() &&
		GetWorld()->GetNavigationSystem() &&
		FNavigationSystem::WantsComponentChangeNotifies())
	{
		VOXEL_SCOPE_COUNTER("UpdateComponentData");

		bNavigationRelevant = IsNavigationRelevant();
		FNavigationSystem::UpdateComponentData(*this);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelNavigationComponent::IsNavigationRelevant() const
{
	return GetNavigationMesh().IsValid();
}

bool UVoxelNavigationComponent::DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const
{
	VOXEL_FUNCTION_COUNTER();

	if (GetNavigationMesh())
	{
		const TArray<FVector> DoubleVertices(GetNavigationMesh()->Vertices);

		GeomExport.ExportCustomMesh(
			DoubleVertices.GetData(),
			GetNavigationMesh()->Vertices.Num(),
			GetNavigationMesh()->Indices.GetData(),
			GetNavigationMesh()->Indices.Num(),
			GetComponentTransform());
	}

	return false;
}

FBoxSphereBounds UVoxelNavigationComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	const FVoxelBox LocalBounds = GetNavigationMesh() ? GetNavigationMesh()->LocalBounds : FVoxelBox();
	ensure(LocalBounds.IsValid());
	return LocalBounds.TransformBy(LocalToWorld).ToFBox();
}

void UVoxelNavigationComponent::OnComponentDestroyed(const bool bDestroyingHierarchy)
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnComponentDestroyed(bDestroyingHierarchy);

	// Clear memory
	GetNavigationMesh().Reset();
}

FPrimitiveSceneProxy* UVoxelNavigationComponent::CreateSceneProxy()
{
	if (!GIsEditor ||
		!GetNavigationMesh())
	{
		return nullptr;
	}

	return new FVoxelNavigationMeshSceneProxy(*this, GetNavigationMesh().ToSharedRef());
}