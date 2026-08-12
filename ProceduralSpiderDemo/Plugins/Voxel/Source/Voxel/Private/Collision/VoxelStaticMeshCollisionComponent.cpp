// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Collision/VoxelStaticMeshCollisionComponent.h"
#include "Collision/VoxelCollider.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "VoxelChaosTriangleMeshSceneProxy.h"
#include "Materials/Material.h"
#include "PhysicsEngine/BodySetup.h"
#include "Chaos/TriangleMeshImplicitObject.h"

UVoxelStaticMeshCollisionComponent::UVoxelStaticMeshCollisionComponent()
{
	bIgnoreStreamingManagerUpdate = true;
	bCanEverAffectNavigation = false;
}

void UVoxelStaticMeshCollisionComponent::SetBodyInstance(const FBodyInstance& NewBodyInstance)
{
	FVoxelUtilities::CopyBodyInstance(
		BodyInstance,
		NewBodyInstance);
}

void UVoxelStaticMeshCollisionComponent::SetCollider(
	const TSharedPtr<const FVoxelCollider>& NewCollider,
	const bool bDoubleSidedGeometry,
	const bool bInGenerateOverlapEvents)
{
	VOXEL_FUNCTION_COUNTER();

	Collider = NewCollider;

	if (BodySetup)
	{
		BodySetup->ClearPhysicsMeshes();
	}
	else
	{
		BodySetup = NewObject<UBodySetup>(this);
		BodySetup->bGenerateMirroredCollision = false;
		BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
	}

	BodySetup->bDoubleSidedGeometry =
#if WITH_EDITOR
		!GetWorld()->IsGameWorld() ||
#endif
		bDoubleSidedGeometry;

	SetGenerateOverlapEvents(bInGenerateOverlapEvents);

	OverrideMaterials.Reset();

	if (Collider)
	{
		Collider->UpdateStats();

		ensure(BodySetup->TriMeshGeometries.Num() == 0);

		// Copied from UBodySetup::FinishCreatingPhysicsMeshes_Chaos
#if TRACK_CHAOS_GEOMETRY
		Collider->TriangleMesh->Track(MakeSerializable(Collider->TriangleMesh), "Voxel Mesh");
#endif
		ensure(!Collider->TriangleMesh->GetDoCollide());

		BodySetup->TriMeshGeometries.Add(Collider->TriangleMesh);
		BodySetup->bCreatedPhysicsMeshes = true;

		for (const TVoxelObjectPtr<UVoxelSurfaceTypeAsset>& WeakSurfaceType : Collider->SurfaceTypes)
		{
			if (const UVoxelSurfaceTypeAsset* SurfaceType = WeakSurfaceType.Resolve())
			{
				OverrideMaterials.Add(SurfaceType->Material);
			}
			else
			{
				OverrideMaterials.Add(nullptr);
			}
		}
	}

	BodyInstance.OwnerComponent = this;
	BodyInstance.UpdatePhysicalMaterials();

	RecreatePhysicsState();
	MarkRenderStateDirty();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelStaticMeshCollisionComponent::ShouldCreatePhysicsState() const
{
	return Collider.IsValid();
}

FBoxSphereBounds UVoxelStaticMeshCollisionComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	const FBox LocalBounds =
		Collider.IsValid()
		? FBox(Collider->TriangleMesh->BoundingBox().Min(), Collider->TriangleMesh->BoundingBox().Max())
		: FBox(FVector::ZeroVector, FVector::ZeroVector);

	ensure(!LocalBounds.Min.ContainsNaN());
	ensure(!LocalBounds.Max.ContainsNaN());

	return LocalBounds.TransformBy(LocalToWorld);
}

void UVoxelStaticMeshCollisionComponent::OnComponentDestroyed(const bool bDestroyingHierarchy)
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnComponentDestroyed(bDestroyingHierarchy);

	// Clear memory
	Collider.Reset();

	if (BodySetup)
	{
		BodySetup->ClearPhysicsMeshes();
	}
}

FPrimitiveSceneProxy* UVoxelStaticMeshCollisionComponent::CreateSceneProxy()
{
	if (!GIsEditor ||
		!Collider)
	{
		return nullptr;
	}

	return new FVoxelChaosTriangleMeshSceneProxy(*this, Collider->TriangleMesh);
}