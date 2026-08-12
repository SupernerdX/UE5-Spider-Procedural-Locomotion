// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelLODVolumeComponent.h"
#include "SceneView.h"
#include "SceneManagement.h"
#include "VoxelDependency.h"
#include "PrimitiveSceneProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialRenderProxy.h"
#include "Render/VoxelRenderInvokersManager.h"

UVoxelLODVolumeComponent::UVoxelLODVolumeComponent()
{
	// Make sure to disable collision so that GetComponentsBoundingBox ignores us
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UVoxelLODVolumeComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

void UVoxelLODVolumeComponent::OnRegister()
{
	Super::OnRegister();

	const UWorld* World = GetWorld();
	if (ensure(World))
	{
		FVoxelRenderInvokersManager::Get(World)->LODVolumeComponents.Add_EnsureNew(this);
	}
}

void UVoxelLODVolumeComponent::OnUnregister()
{
	const UWorld* World = GetWorld();
	if (ensure(World))
	{
		FVoxelRenderInvokersManager::Get(World)->LODVolumeComponents.Remove_Ensure(this);
	}

	Super::OnUnregister();
}

FPrimitiveSceneProxy* UVoxelLODVolumeComponent::CreateSceneProxy()
{
	class FVoxelLODVolumeSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		FVoxelLODVolumeSceneProxy(const UVoxelLODVolumeComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
			, bDrawOnlyIfSelected(InComponent->bDrawOnlyIfSelected)
			, Type(InComponent->Type)
			, SphereRadius(InComponent->SphereRadius)
			, BoxExtent(InComponent->BoxExtent)
			, BoxColor(InComponent->ShapeColor)
			, LineThickness(InComponent->LineThickness)
			, PositionPrecision(InComponent->PositionPrecision)
			, RotationPrecision(InComponent->RotationPrecision)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			FTransform Transform = FTransform(GetLocalToWorld());
			Transform.SetLocation(FVoxelUtilities::RoundToFloat(Transform.GetLocation() / PositionPrecision) * PositionPrecision);

			const FVector Euler = FVoxelUtilities::RoundToFloat(Transform.Rotator().Euler() / RotationPrecision) * RotationPrecision;
			Transform.SetRotation(FRotator::MakeFromEuler(Euler).Quaternion());
			
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];

					const FLinearColor DrawColor = GetViewSelectionColor(BoxColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected() );

					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

					switch (Type)
					{
					case EVoxelLODInvokerType::Sphere:
					{
						DrawWireSphere(
							PDI,
							Transform.GetLocation(),
							DrawColor,
							SphereRadius,
							16,
							IsSelected() ? SDPG_Foreground : SDPG_World,
							LineThickness,
							0.f,
							true);
						break;
					}
					case EVoxelLODInvokerType::Box:
					{
						DrawOrientedWireBox(
							PDI,
							Transform.GetLocation(),
							Transform.GetScaledAxis(EAxis::X),
							Transform.GetScaledAxis(EAxis::Y),
							Transform.GetScaledAxis(EAxis::Z),
							BoxExtent,
							DrawColor,
							IsSelected() ? SDPG_Foreground : SDPG_World,
							LineThickness,
							0.f,
							true);
						break;
					}
					}
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			const bool bProxyVisible = !bDrawOnlyIfSelected || IsSelected();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance =
				IsShown(View) &&
				bProxyVisible;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}

		virtual uint32 GetMemoryFootprint() const override
		{
			return sizeof(*this) + GetAllocatedSize();
		}

		virtual SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

	private:
		const bool bDrawOnlyIfSelected;
		const EVoxelLODInvokerType Type;
		const float SphereRadius;
		const FVector BoxExtent;
		const FColor BoxColor;
		const float LineThickness;
		const float PositionPrecision;
		const float RotationPrecision;
	};

	return new FVoxelLODVolumeSceneProxy(this);
}

FBoxSphereBounds UVoxelLODVolumeComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FVector BoundsBoxExtent = FVector::OneVector;
	float Radius = 1.f;
	switch (Type)
	{
	case EVoxelLODInvokerType::Sphere: BoundsBoxExtent = FVector(SphereRadius); Radius = SphereRadius; break;
	case EVoxelLODInvokerType::Box: BoundsBoxExtent = BoxExtent; Radius = BoxExtent.GetMax(); break;
	}
	return FBoxSphereBounds(FVector::ZeroVector, BoundsBoxExtent, Radius).TransformBy(LocalToWorld);
}