// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelStampDebug.h"
#include "VoxelQuery.h"
#include "VoxelWorld.h"
#include "VoxelState.h"
#include "VoxelLayers.h"
#include "VoxelViewport.h"
#include "VoxelLayerStack.h"
#include "VoxelBreadcrumbs.h"
#include "VoxelStampRuntime.h"
#include "VoxelStampComponent.h"
#include "Components/BoxComponent.h"

#if 0 // TODO
FVoxelBox FVoxelDebugVolumeStampRuntime::GetLocalBounds() const
{
	return Stamp.Bounds;
}

void FVoxelDebugVolumeStampRuntime::Apply(
	const FVoxelQuery& Query,
	const TVoxelArrayView<float> Distances,
	const FVoxelMetadataView& Metadata,
	const TConstVoxelArrayView<double> PositionsX,
	const TConstVoxelArrayView<double> PositionsY,
	const TConstVoxelArrayView<double> PositionsZ,
	const FVoxelBox& FilterBounds,
	const float MaxDistance,
	const float DistanceScale,
	const FMatrix& PositionToStamp) const
{
	VOXEL_FUNCTION_COUNTER();

	const int32 Num = Distances.Num();
	check(Num == PositionsX.Num());
	check(Num == PositionsY.Num());
	check(Num == PositionsZ.Num());

	FVoxelBitArray IsInside;
	TVoxelArray<double> NewPositionsX;
	TVoxelArray<double> NewPositionsY;
	TVoxelArray<double> NewPositionsZ;
	{
		VOXEL_SCOPE_COUNTER("Transform positions");

		FVoxelUtilities::SetNumFast(NewPositionsX, Num);
		FVoxelUtilities::SetNumFast(NewPositionsY, Num);
		FVoxelUtilities::SetNumFast(NewPositionsZ, Num);
		IsInside.SetNum(Num, false);

		for (int32 Index = 0; Index < Num; Index++)
		{
			const FVector Position = FVector(
				PositionsX[Index],
				PositionsY[Index],
				PositionsZ[Index]);

			const FVector NewPosition = Stamp.Transform.TransformPosition(Position);

			IsInside[Index] = FilterBounds.Contains(Position);
			NewPositionsX[Index] = NewPosition.X;
			NewPositionsY[Index] = NewPosition.Y;
			NewPositionsZ[Index] = NewPosition.Z;
		}
	}

	const auto OldDistances = TVoxelArray<float>(Distances);

	FVoxelMetadataStorage OldMetadata;
	OldMetadata.CopyFrom(Metadata);

	const TSharedRef<FVoxelBreadcrumb> Breadcrumb = MakeShared<FVoxelBreadcrumb>();

	FVoxelQuery NewQuery(Query.LOD, *Stamp.Layers, Query.DependencyCollector, &Breadcrumb.Get());
	NewQuery.ShouldStopTraversal = [&](const FVoxelStampRuntime& NextStamp)
	{
		return NextStamp.GetWeakStampRef() == Stamp.LastStampNextRef;
	};

	const TVoxelArray<float> NewDistances = NewQuery.SampleVolumeLayer(
		Stamp.LayerToSample,
		NewPositionsX,
		NewPositionsY,
		NewPositionsZ,
		Metadata);

	FVoxelUtilities::Memcpy(Distances, NewDistances);

	Stamp.ProcessBreadcrumb(Breadcrumb);

	for (int32 Index = 0; Index < Num; Index++)
	{
		// if (IsInside[Index])
		{
			Distances[Index] *= DistanceScale;
			continue;
		}

		Distances[Index] = OldDistances[Index];

		if (Metadata.HasMaterials())
		{
			Metadata.GetMaterials()[Index] = OldMetadata.GetMaterials()[Index];
		}

#if 0 // TODO
		for (const FVoxelMetadataRef& MetadataToQuery : Metadata.GetMetadatasToQuery())
		{
			Metadata.GetMetadataBuffer(MetadataToQuery)[Index] = OldMetadata.GetMetadataBuffer(MetadataToQuery)[Index];
		}
#endif
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelStampDebug::Construct(
	const FArguments& Args,
	const FVoxelState& State,
	const FVector& Position)
{
#if 0
	{
		FDetailsViewArgs DetailViewArgs;
		DetailViewArgs.bHideSelectionTip = true;
		DetailViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
		DetailViewArgs.NotifyHook = this;

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		DetailsView = PropertyModule.CreateDetailView(DetailViewArgs);
	}

	Layers = State.Layers;
	LayerToSample = State.Config->LayerToRender;

	Settings = NewObject<UVoxelStampDebugSettings>();
	Settings->Transform = FTransform(State.Config->LocalToWorld.InverseTransformPosition(Position));
	DetailsView->SetObject(Settings);

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(0.2)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsView.ToSharedRef()
			]
			+ SVerticalBox::Slot()
			[
				SAssignNew(TreeView, STreeView<TSharedPtr<FVoxelBreadcrumbs>>)
				.TreeItemsSource(&RootBreadcrumbs)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow_Lambda([this](const TSharedPtr<FVoxelBreadcrumbs>& Breadcrumb, const TSharedRef<STableViewBase>& OwnerTable)
				{
					const FString Name = INLINE_LAMBDA
					{
						switch (Breadcrumb->Type)
						{
						default: ensure(false);
						case EVoxelBreadcrumbType::Root: return LayerToSample.Stack.GetName();
						case EVoxelBreadcrumbType::Layer: return Breadcrumb->GetLayer().Layer.GetName();
						case EVoxelBreadcrumbType::Stamp: return Breadcrumb->GetStamp()->GetStatName().ToString();
						}
					};

					return
						SNew(STableRow<TSharedPtr<FVoxelBreadcrumbs>>, OwnerTable)
						[
							SNew(SBox)
							.MinDesiredWidth(150.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Name))
								.Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
							]
						];
				})
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FVoxelBreadcrumbs>& Breadcrumb, ESelectInfo::Type)
				{
					if (!Breadcrumb ||
						Breadcrumb->Type != EVoxelBreadcrumbType::Stamp)
					{
						return;
					}

					const FVoxelWeakStampRef LastStampNextRef = INLINE_LAMBDA -> FVoxelWeakStampRef
					{
						bool bFoundStamp = false;

						for (const TSharedPtr<FVoxelBreadcrumbs>& Layer : RootBreadcrumbs[0]->GetChildren())
						{
							for (const TSharedPtr<FVoxelBreadcrumbs>& Stamp : Layer->GetChildren())
							{
								if (!ensure(Stamp->Type == EVoxelBreadcrumbType::Stamp))
								{
									continue;
								}

								if (Stamp == Breadcrumb)
								{
									ensure(!bFoundStamp);
									bFoundStamp = true;
								}
								else if (bFoundStamp)
								{
									return Stamp->GetStamp()->GetWeakStampRef();
								}
							}
						}

						return {};
					};

					SetLastStampNextRef(LastStampNextRef);
				})
				// .OnContextMenuOpening(InArgs._OnContextMenuOpening)
				.OnGetChildren_Lambda([this](const TSharedPtr<FVoxelBreadcrumbs>& Breadcrumb, TArray<TSharedPtr<FVoxelBreadcrumbs>>& OutChildren)
				{
					OutChildren = Breadcrumb->GetChildren();
				})
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.8)
		[
			SAssignNew(Viewport, SVoxelViewport)
		]
	];

	INLINE_LAMBDA
	{
		AVoxelWorld* VoxelWorld = Viewport->SpawnActor<AVoxelWorld>();
		if (!ensure(VoxelWorld))
		{
			return;
		}

		const AVoxelWorld* SourceVoxelWorld = State.Config->VoxelWorld.Resolve();
		if (!ensure(SourceVoxelWorld))
		{
			return;
		}

		for (FProperty& Property : GetClassProperties<AVoxelWorld>(EFieldIterationFlags::None))
		{
			Property.CopyCompleteValue_InContainer(VoxelWorld, SourceVoxelWorld);
		}

		VoxelWorld->LayerStack = UVoxelLayerStack::Default();
		WeakVoxelWorld = VoxelWorld;

		UBoxComponent* BoundsComponent = Viewport->CreateComponent<UBoxComponent>();
		if (!ensure(BoundsComponent))
		{
			return;
		}

		BoundsComponent->SetWorldTransform(FTransform(FVector(Settings->BoxSize / 2)));
		BoundsComponent->SetBoxExtent(FVector(Settings->BoxSize / 2));
		WeakBoundsComponent = BoundsComponent;

		WeakStampComponent = Viewport->CreateComponent<UVoxelStampComponent>();
	};

	SetLastStampNextRef({});

	Viewport->Initialize(SharedThis(this));
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelStampDebug::Tick(
	const FGeometry& AllottedGeometry,
	const double InCurrentTime,
	const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	UBoxComponent* BoundsComponent = WeakBoundsComponent.Resolve();
	if (!ensure(Settings) ||
		!ensure(BoundsComponent))
	{
		return;
	}

	BoundsComponent->SetWorldTransform(FTransform(FVector(Settings->BoxSize / 2)));
	BoundsComponent->SetBoxExtent(FVector(Settings->BoxSize / 2));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelStampDebug::NotifyPostChange(
	const FPropertyChangedEvent& PropertyChangedEvent,
	FProperty* PropertyThatChanged)
{
	VOXEL_FUNCTION_COUNTER();

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
	{
		return;
	}

	SetLastStampNextRef(LastStampNextRef);
}

void SVoxelStampDebug::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Settings);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelStampDebug::SetLastStampNextRef(const FVoxelWeakStampRef& NewLastStampNextRef)
{
	VOXEL_FUNCTION_COUNTER();

	LastStampNextRef = NewLastStampNextRef;

	UVoxelStampComponent* StampComponent = WeakStampComponent.Resolve();
	if (!ensure(StampComponent))
	{
		return;
	}

#if 0 // TODO
	FVoxelDebugVolumeStamp Stamp;
	Stamp.LOD = Settings->LOD;
	Stamp.Bounds = FVoxelBox(0, Settings->BoxSize);
	Stamp.Transform = Settings->Transform;
	Stamp.Layers = Layers;
	Stamp.LayerToSample = LayerToSample;
	Stamp.LastStampNextRef = NewLastStampNextRef;
	Stamp.ProcessBreadcrumb = MakeWeakPtrLambda(this, [this](const TSharedRef<FVoxelBreadcrumb>& NewBreadcrumb)
	{
		Voxel::GameTask(MakeWeakPtrLambda(this, [=, this]
		{
			UpdateBreadcrumbs(NewBreadcrumb);
		}));
	});

	StampComponent->SetStamp(Stamp);
#endif
}

void SVoxelStampDebug::UpdateBreadcrumbs(const TSharedRef<FVoxelBreadcrumbs>& RootBreadcrumb)
{
	VOXEL_FUNCTION_COUNTER();

#if 0
	RootBreadcrumbs.Reset();
	RootBreadcrumbs.Add(RootBreadcrumb);

	TreeView->RequestTreeRefresh();

	TreeView->SetItemExpansion(RootBreadcrumb, true);

	for (const TSharedPtr<FVoxelBreadcrumbs>& Layer : RootBreadcrumb->GetChildren())
	{
		TreeView->SetItemExpansion(Layer, true);
	}
#endif
}