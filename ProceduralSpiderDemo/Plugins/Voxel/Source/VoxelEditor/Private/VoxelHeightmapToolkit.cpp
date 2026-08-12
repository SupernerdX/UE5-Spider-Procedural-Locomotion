// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelHeightmapToolkit.h"

#include "VoxelWorld.h"
#include "VoxelStampActor.h"
#include "Heightmap/VoxelHeightmapStamp.h"
#include "Heightmap/VoxelHeightmap_Height.h"
#include "Heightmap/VoxelHeightmap_Weight.h"
#include "SVoxelHeightmapList.h"

#include "ContextObjectStore.h"
#include "EditorViewportClient.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SphereComponent.h"
#include "BaseGizmos/GizmoViewContext.h"
#include "Components/StaticMeshComponent.h"
#include "BaseGizmos/GizmoCircleComponent.h"

void UVoxelHeightStampToolkitGizmoParameterSource::BeginModify()
{
	if (Actor)
	{
		Transaction = MakeShared<FVoxelTransaction>(Actor);

		const FVoxelHeightmapStamp& Stamp = CastStructChecked<FVoxelHeightmapStamp>(*Actor->GetStamp());

		Stamp.Heightmap->Height->bAllowUpdate = false;
		InitialValue = Stamp.Heightmap->Height->MinHeight;
	}
}

float UVoxelHeightStampToolkitGizmoParameterSource::GetParameter() const
{
	const FVoxelHeightmapStamp& Stamp = CastStructChecked<FVoxelHeightmapStamp>(*Actor->GetStamp());
	return Stamp.Heightmap->Height->MinHeight;
}

void UVoxelHeightStampToolkitGizmoParameterSource::SetParameter(float NewValue)
{
	if (!ensure(Actor))
	{
		return;
	}

	const FVoxelHeightmapStamp& Stamp = CastStructChecked<FVoxelHeightmapStamp>(*Actor->GetStamp());
	if (!ensure(Stamp.Heightmap) ||
		!ensure(Stamp.Heightmap->Height))
	{
		return;
	}

	UVoxelHeightmap_Height* Height = Stamp.Heightmap->Height;

	NewValue = NewValue / Height->ScaleZ;
	Height->MinHeight = FMath::Clamp(InitialValue + NewValue, 0.f, 1.f);
}

void UVoxelHeightStampToolkitGizmoParameterSource::EndModify()
{
	Transaction = nullptr;

	const FVoxelHeightmapStamp& Stamp = CastStructChecked<FVoxelHeightmapStamp>(*Actor->GetStamp());
	Stamp.Heightmap->Height->bAllowUpdate = true;
}

AVoxelHeightStampToolkitGizmoActor::AVoxelHeightStampToolkitGizmoActor()
{
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("GizmoCenter"));
	RootComponent = SphereComponent;

	SphereComponent->InitSphereRadius(1.0f);
	SphereComponent->SetVisibility(false);
	SphereComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

AVoxelHeightStampToolkitGizmoActor* AVoxelHeightStampToolkitGizmoActor::ConstructDefaultIntervalGizmo(UWorld* World, UGizmoViewContext* GizmoViewContext)
{
	const FActorSpawnParameters SpawnInfo;
	AVoxelHeightStampToolkitGizmoActor* NewActor = World->SpawnActor<AVoxelHeightStampToolkitGizmoActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

	if (ensure(NewActor))
	{
		NewActor->MinHeightComponent = AddDefaultCircleComponent(World, NewActor, GizmoViewContext, FColor::Red, FVector::ZAxisVector, 100.f);
	}

	return NewActor;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelHeightStampToolkitGizmo::Setup()
{
	Super::Setup();

	GizmoActor = AVoxelHeightStampToolkitGizmoActor::ConstructDefaultIntervalGizmo(World, GetGizmoManager()->GetContextObjectStore()->FindContext<UGizmoViewContext>());
}

void UVoxelHeightStampToolkitGizmo::Shutdown()
{
	SetActor(nullptr);

	if (GizmoActor)
	{
		GizmoActor->Destroy();
		GizmoActor = nullptr;
	}

	Super::Shutdown();
}

void UVoxelHeightStampToolkitGizmo::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GizmoActor ||
		!HeightStampActor)
	{
		return;
	}

	const FVoxelHeightmapStamp& Stamp = CastStructChecked<FVoxelHeightmapStamp>(*HeightStampActor->GetStamp());

	FVector Location = HeightStampActor->GetActorLocation();
	Location.Z = Stamp.Heightmap->Height->ScaleZ * Stamp.Heightmap->Height->MinHeight;
	GizmoActor->SetActorLocation(Location);
}

void UVoxelHeightStampToolkitGizmo::SetActor(AVoxelStampActor* Actor)
{
	HeightStampActor = Actor;

	if (MinHeightGizmo)
	{
		GetGizmoManager()->DestroyGizmo(MinHeightGizmo);
	}

	if (!Actor)
	{
		return;
	}

	{
		UGizmoBaseComponent* Component = GizmoActor->MinHeightComponent;

		MinHeightGizmo = GetGizmoManager()->CreateGizmo<UAxisPositionGizmo>(UInteractiveGizmoManager::DefaultAxisPositionBuilderIdentifier);
		MinHeightGizmo->bEnableSignedAxis = false;
		MinHeightGizmo->AxisSource = UGizmoComponentAxisSource::Construct(Component, 2, false);
		MinHeightGizmo->ParameterSource = UVoxelHeightStampToolkitGizmoParameterSource::Construct<UVoxelHeightStampToolkitGizmoParameterSource>(Actor);

		UGizmoComponentHitTarget* HitTarget = UGizmoComponentHitTarget::Construct(Component);
		HitTarget->UpdateHoverFunction = [=](const bool bHovering)
		{
			Component->UpdateHoverState(bHovering);
		};
		MinHeightGizmo->HitTarget = HitTarget;
		MinHeightGizmo->StateTarget = UGizmoObjectModifyStateTarget::Construct(Actor, INVTEXT("Change min height"), GetGizmoManager());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelHeightStampToolkitEdMode::UVoxelHeightStampToolkitEdMode()
{
	Info = FEditorModeInfo(
		"VoxelHeightStampToolkitEdMode",
		INVTEXT("VoxelHeightStampToolkitEdMode"),
		FSlateIcon(),
		false,
		600
	);
}

void UVoxelHeightStampToolkitEdMode::Enter()
{
	Super::Enter();

	GetToolManager()->GetPairedGizmoManager()->RegisterGizmoType("VoxelHeightStampToolkitGizmo", NewObject<UVoxelHeightStampGizmoBuilder>());
}

void UVoxelHeightStampToolkitEdMode::Exit()
{
	DestroyGizmos();

	GetToolManager()->GetPairedGizmoManager()->DeregisterGizmoType("VoxelHeightStampToolkitGizmo");

	Super::Exit();
}

bool UVoxelHeightStampToolkitEdMode::Select(AActor* InActor, bool bInSelected)
{
	DestroyGizmos();

	AVoxelStampActor* ActorBase = Cast<AVoxelStampActor>(InActor);
	if (!ActorBase)
	{
		return false;
	}

	UInteractiveGizmoManager* GizmoManager = GetToolManager()->GetPairedGizmoManager();
	if (!ensure(GizmoManager))
	{
		return false;
	}

	UVoxelHeightStampToolkitGizmo* Gizmo = GizmoManager->CreateGizmo<UVoxelHeightStampToolkitGizmo>("VoxelHeightStampToolkitGizmo");
	Gizmo->SetActor(ActorBase);
	InteractiveGizmo = Gizmo;

	return false;
}

bool UVoxelHeightStampToolkitEdMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	DestroyGizmos();

	if (Click.GetKey() != EKeys::LeftMouseButton ||
		(Click.GetEvent() != IE_Released && Click.GetEvent() != IE_DoubleClick) ||
		!HitProxy ||
		!HitProxy->IsA(HActor::StaticGetType()))
	{
		return false;
	}

	const HActor* ActorHitProxy = HitProxyCast<HActor>(HitProxy);
	if (!ActorHitProxy->Actor)
	{
		return false;
	}

	AVoxelStampActor* TargetActor = nullptr;
	for (TActorIterator<AVoxelStampActor> It(GetWorld()); It; ++It)
	{
		TargetActor = *It;
	}

	if (!TargetActor)
	{
		return false;
	}

	const FVoxelHeightmapStamp& Stamp = CastStructChecked<FVoxelHeightmapStamp>(*TargetActor->GetStamp());

	if (Stamp.Heightmap->Height->bEnableMinHeight)
	{
		Select(TargetActor, true);
	}

	return ILegacyEdModeViewportInterface::HandleClick(InViewportClient, HitProxy, Click);
}

void UVoxelHeightStampToolkitEdMode::DestroyGizmos()
{
	UInteractiveGizmoManager* GizmoManager = GetToolManager()->GetPairedGizmoManager();
	if (!ensure(GizmoManager))
	{
		return;
	}

	if (InteractiveGizmo)
	{
		GizmoManager->DestroyGizmo(InteractiveGizmo);
	}

	InteractiveGizmo = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightmapToolkit::Initialize()
{
	Super::Initialize();

	HeightStampEntriesList =
		SNew(SVoxelHeightmapList)
		.Asset(Asset)
		.NotifyHook(GetNotifyHook());
}

void FVoxelHeightmapToolkit::RegisterTabs(const FRegisterTab RegisterTab)
{
	// Bypass FVoxelSimpleAssetToolkit
	FVoxelToolkit::RegisterTabs(RegisterTab);

	RegisterTab(DetailsTabId, INVTEXT("Details"), "LevelEditor.Tabs.Details", HeightStampEntriesList);
	RegisterTab(ViewportTabId, INVTEXT("Viewport"), "LevelEditor.Tabs.Viewports", GetViewport().AsShared());
}

void FVoxelHeightmapToolkit::Tick()
{
	Super::Tick();
	UpdateMinHeightPlane();
}

void FVoxelHeightmapToolkit::PostEditChange(const FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChange(PropertyChangedEvent);

	if (ensure(HeightStampEntriesList))
	{
		HeightStampEntriesList->OnPropertyChanged(PropertyChangedEvent);
	}

	UpdateMinHeightPlane();
	UpdateStats();

	float SkyScale = 2000.f;
	if (const TSharedPtr<const FVoxelHeightmap_HeightData>& Data = Asset->Height->GetData())
	{
		SkyScale = FMath::Max(SkyScale, FMath::Max(Data->SizeX, Data->SizeY) * Asset->ScaleXY / 50.f);
	}

	GetViewport().SetSkyScale(SkyScale);
}

void FVoxelHeightmapToolkit::PostUndo()
{
	if (ensure(HeightStampEntriesList))
	{
		HeightStampEntriesList->PostUndo();
	}
}

void FVoxelHeightmapToolkit::SetupPreview()
{
	Super::SetupPreview();

	VoxelWorld = GetViewport().SpawnActor<AVoxelWorld>();
	VoxelWorld->SetupForPreview();
	VoxelWorld->VoxelSize = 10;

	VoxelHeightStamp = GetViewport().SpawnActor<AVoxelStampActor>();

	FVoxelHeightmapStamp Stamp;
	Stamp.Heightmap = Asset;
	VoxelHeightStamp->SetStamp(Stamp);

	MinHeightPlane = GetViewport().SpawnActor<AStaticMeshActor>();
	MinHeightPlane->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane")));
	MinHeightPlane->GetStaticMeshComponent()->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/EditorAssets/M_StampPreview.M_StampPreview")));
	UpdateMinHeightPlane();

	UpdateStats();

	float SkyScale = 2000.f;
	if (const TSharedPtr<const FVoxelHeightmap_HeightData>& Data = Asset->Height->GetData())
	{
		SkyScale = FMath::Max(SkyScale, FMath::Max(Data->SizeX, Data->SizeY) * Asset->ScaleXY / 50.f);
	}

	GetViewport().SetSkyScale(SkyScale);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightmapToolkit::UpdateMinHeightPlane() const
{
	MinHeightPlane->SetIsTemporarilyHiddenInEditor(!Asset->Height->bEnableMinHeight);

	FVector Scale = FVector::OneVector;
	const float HeightOffset = Asset->Height->MinHeight * Asset->Height->ScaleZ;

	if (const TSharedPtr<const FVoxelHeightmap_HeightData>& Data = Asset->Height->GetData())
	{
		Scale.X = Data->SizeX * Asset->ScaleXY / 100.f;
		Scale.Y = Data->SizeY * Asset->ScaleXY / 100.f;
	}

	MinHeightPlane->SetActorScale3D(Scale);
	MinHeightPlane->SetActorLocation(FVector(0.f, 0.f, HeightOffset - 3.f));
}

void FVoxelHeightmapToolkit::UpdateStats()
{
	const TSharedPtr<const FVoxelHeightmap_HeightData> HeightData = Asset->Height->GetData();
	if (!HeightData)
	{
		GetViewport().SetStatsText("Invalid heightmap");
		return;
	}

	int64 Size = HeightData->GetAllocatedSize();
	for (const UVoxelHeightmap_Weight* Weightmap : Asset->Weights)
	{
		if (const TSharedPtr<const FVoxelHeightmap_WeightData> WeightData = Weightmap->GetData())
		{
			Size += WeightData->GetAllocatedSize();
		}
	}

	GetViewport().SetStatsText(FString() +
		"<TextBlock.ShadowedText>Total Memory Size: </><TextBlock.ShadowedTextWarning>" + FVoxelUtilities::BytesToString(Size) + "</>\n" +
		"<TextBlock.ShadowedText>Size: </><TextBlock.ShadowedTextWarning>" + LexToString(HeightData->SizeX) + "x" + LexToString(HeightData->SizeY) + "</>");
}