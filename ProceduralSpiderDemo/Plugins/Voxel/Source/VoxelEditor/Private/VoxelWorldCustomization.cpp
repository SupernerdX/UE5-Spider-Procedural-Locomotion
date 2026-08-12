// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelWorld.h"
#include "VoxelState.h"
#include "VoxelRuntime.h"
#include "Misc/ITransaction.h"
#include "Engine/RendererSettings.h"

VOXEL_CUSTOMIZE_CLASS(AVoxelWorld)(IDetailLayoutBuilder& DetailLayout)
{
	FVoxelEditorUtilities::EnableRealtime();

	DetailLayout.HideCategory("Rendering");
	DetailLayout.HideCategory("Replication");
	DetailLayout.HideCategory("Input");
	DetailLayout.HideCategory("Collision");
	DetailLayout.HideCategory("LOD");
	DetailLayout.HideCategory("HLOD");
	DetailLayout.HideCategory("Cooking");
	DetailLayout.HideCategory("DataLayers");
	DetailLayout.HideCategory("Networking");
	DetailLayout.HideCategory("Physics");

	FVoxelEditorUtilities::HideAndMoveToCategory(DetailLayout, "Actor", "Misc", { GET_MEMBER_NAME_STATIC(AActor, Tags) }, false);
	FVoxelEditorUtilities::HideAndMoveToCategory(DetailLayout, "WorldPartition", "Misc");
	FVoxelEditorUtilities::HideAndMoveToCategory(DetailLayout, "LevelInstance", "Misc");

	TArray<FName> Categories;
	DetailLayout.GetCategoryNames(Categories);
	for (const FName Category : Categories)
	{
		FString CategoryName = Category.ToString();
		if (!CategoryName.RemoveFromStart("Voxel ") ||
			CategoryName.Contains("|"))
		{
			continue;
		}

		DetailLayout.EditCategory(Category, FText::FromString(CategoryName));
	}

	const TArray<TWeakObjectPtr<AVoxelWorld>> WeakObjects = DetailLayout.GetObjectsOfTypeBeingCustomized<AVoxelWorld>();

	const auto UpdateRowWithWarning = [&](
		const TSharedRef<IPropertyHandle>& Handle,
		const FName Icon,
		const FString& WarningText,
		const FString& WarningToolTip, auto VisibilityLambda)
	{
		IDetailPropertyRow* Row = DetailLayout.EditDefaultProperty(Handle);
		if (!Row)
		{
			return;
		}

		Row->CustomWidget()
		.NameContent()
		[
			Handle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				Handle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(4.f, 0.f, 0.f, 0.f)
			.AutoWidth()
			[
				SNew(SBox)
				.Visibility_Lambda([VisibilityLambda]
				{
					return VisibilityLambda();
				})
				.ToolTipText(FText::FromString(WarningToolTip))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush(Icon))
					]
					+ SHorizontalBox::Slot()
					.Padding(4.f, 0.f, 0.f, 0.f)
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SVoxelDetailText)
						.Text(FText::FromString(WarningText))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]
				]
			]
		];
	};

	UpdateRowWithWarning(
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bEnableNanite), AVoxelWorld::StaticClass()),
		"Icons.Error",
		"Nanite unavailable",
		"Nanite is either disabled or not available for the current platform. Regular rendering will be used instead",
		[WeakObjects]
		{
			for (const TWeakObjectPtr<AVoxelWorld>& WeakObject : WeakObjects)
			{
				const AVoxelWorld* Object = WeakObject.Get();
				if (!Object)
				{
					continue;
				}

				const TSharedPtr<FVoxelRuntime> Runtime = Object->GetRuntime();
				if (!Runtime)
				{
					continue;
				}

				const TSharedPtr<FVoxelState> State = Runtime->GetNewestState();
				if (!State ||
					State->Config->CanEnableNanite() ||
					Object->bEnableCubicMesher)
				{
					continue;
				}

				return EVisibility::Visible;
			}

			return EVisibility::Collapsed;
		});

	UpdateRowWithWarning(
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bEnableNanite), AVoxelWorld::StaticClass()),
		"Icons.Info",
		"Nanite is unsupported for Cubic rendering",
		"Nanite is automatically disabled when Cubic rendering is enabled",
		[WeakObjects]
		{
			for (const TWeakObjectPtr<AVoxelWorld>& WeakObject : WeakObjects)
			{
				const AVoxelWorld* Object = WeakObject.Get();
				if (!Object ||
					!Object->bEnableCubicMesher)
				{
					continue;
				}

				return EVisibility::Visible;
			}

			return EVisibility::Collapsed;
		});

	UpdateRowWithWarning(
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, MaxLOD), AVoxelWorld::StaticClass()),
		"Icons.Info",
		"LODs are unsupported for Cubic rendering",
		"Max LOD is automatically set to 0 when Cubic rendering is enabled",
		[WeakObjects]
		{
			for (const TWeakObjectPtr<AVoxelWorld>& WeakObject : WeakObjects)
			{
				const AVoxelWorld* Object = WeakObject.Get();
				if (!Object ||
					!Object->bEnableCubicMesher)
				{
					continue;
				}

				return EVisibility::Visible;
			}

			return EVisibility::Collapsed;
		});

	UpdateRowWithWarning(
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, QualityExponent), AVoxelWorld::StaticClass()),
		"Icons.Info",
		"Quality Exponent is disabled for Cubic rendering",
		"Quality Exponent is set to 0.01 to force LOD 0 for all chunks. Increase LOD Quality to extend draw distance",
		[WeakObjects]
		{
			for (const TWeakObjectPtr<AVoxelWorld>& WeakObject : WeakObjects)
			{
				const AVoxelWorld* Object = WeakObject.Get();
				if (!Object ||
					!Object->bEnableCubicMesher)
				{
					continue;
				}

				return EVisibility::Visible;
			}

			return EVisibility::Collapsed;
		});

	UpdateRowWithWarning(
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bEnableLumen), AVoxelWorld::StaticClass()),
		"Icons.Error",
		"Lumen unavailable",
		"Lumen is either disabled or not available for the current platform",
		[WeakObjects]
		{
			for (const TWeakObjectPtr<AVoxelWorld>& WeakObject : WeakObjects)
			{
				const AVoxelWorld* Object = WeakObject.Get();
				if (!Object)
				{
					continue;
				}

				const TSharedPtr<FVoxelRuntime> Runtime = Object->GetRuntime();
				if (!Runtime)
				{
					continue;
				}

				const TSharedPtr<FVoxelState> State = Runtime->GetNewestState();
				if (!State ||
					State->Config->CanEnableLumen())
				{
					continue;
				}

				return EVisibility::Visible;
			}

			return EVisibility::Collapsed;
		});

	UpdateRowWithWarning(
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bEnableRaytracing), AVoxelWorld::StaticClass()),
		"Icons.Info",
		"Raytracing is currently force enabled",
		"Lumen is currently force-enabling raytracing",
		[WeakObjects]
		{
			for (const TWeakObjectPtr<AVoxelWorld>& WeakObject : WeakObjects)
			{
				const AVoxelWorld* Object = WeakObject.Get();
				if (!Object)
				{
					continue;
				}

				const TSharedPtr<FVoxelRuntime> Runtime = Object->GetRuntime();
				if (!Runtime)
				{
					continue;
				}

				const TSharedPtr<FVoxelState> State = Runtime->GetNewestState();
				if (!State)
				{
					continue;
				}

				if (Object->bEnableRaytracing ||
					!State->Config->bEnableRaytracing)
				{
					continue;
				}

				return EVisibility::Visible;
			}

			return EVisibility::Collapsed;
		});

	UpdateRowWithWarning(
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bGenerateMeshDistanceFields), AVoxelWorld::StaticClass()),
		"Icons.Info",
		"Mesh Distance Fields are currently force enabled",
		"Lumen is currently force-enabling Mesh Distance Fields",
		[WeakObjects]
		{
			for (const TWeakObjectPtr<AVoxelWorld>& WeakObject : WeakObjects)
			{
				const AVoxelWorld* Object = WeakObject.Get();
				if (!Object)
				{
					continue;
				}

				const TSharedPtr<FVoxelRuntime> Runtime = Object->GetRuntime();
				if (!Runtime)
				{
					continue;
				}

				const TSharedPtr<FVoxelState> State = Runtime->GetNewestState();
				if (!State)
				{
					continue;
				}

				if (Object->bGenerateMeshDistanceFields ||
					!State->Config->bGenerateMeshDistanceFields ||
					Object->bEnableCubicMesher)
				{
					continue;
				}

				return EVisibility::Visible;
			}

			return EVisibility::Collapsed;
		});

	UpdateRowWithWarning(
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bGenerateMeshDistanceFields), AVoxelWorld::StaticClass()),
		"Icons.Warning",
		"Unsupported for Cubic rendering",
		"Cubic rendering does not support distance fields. To use Lumen, use Hardware Raytracing instead.",
		[WeakObjects]
		{
			for (const TWeakObjectPtr<AVoxelWorld>& WeakObject : WeakObjects)
			{
				const AVoxelWorld* Object = WeakObject.Get();
				if (!Object ||
					!Object->bEnableCubicMesher)
				{
					continue;
				}

				return EVisibility::Visible;
			}

			return EVisibility::Collapsed;
		});

	UpdateRowWithWarning(
		DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelWorld, bGenerateNavigationInsideNavMeshBounds), AVoxelWorld::StaticClass()),
		"Icons.Info",
		"NavMesh is currently generated inside all bounds",
		"NavMesh generation inside NavMesh bounds is force enabled. Enable 'Generate Navigation Only Around Navigation Invokers' in navigation system project settings, to generate only around invokers.",
		[WeakObjects]
		{
			for (const TWeakObjectPtr<AVoxelWorld>& WeakObject : WeakObjects)
			{
				const AVoxelWorld* Object = WeakObject.Get();
				if (!Object)
				{
					continue;
				}

				const TSharedPtr<FVoxelRuntime> Runtime = Object->GetRuntime();
				if (!Runtime)
				{
					continue;
				}

				const TSharedPtr<FVoxelState> State = Runtime->GetNewestState();
				if (!State)
				{
					continue;
				}

				if (Object->bGenerateNavigationInsideNavMeshBounds ||
					!State->Config->bGenerateInsideNavMeshBounds)
				{
					continue;
				}

				return EVisibility::Visible;
			}

			return EVisibility::Collapsed;
		});
}