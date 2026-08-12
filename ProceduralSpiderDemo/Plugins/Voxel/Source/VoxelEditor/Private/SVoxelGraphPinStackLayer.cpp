// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphPinStackLayer.h"
#include "SVoxelGraphStackLayerSelector.h"
#include "VoxelLayer.h"
#include "VoxelPinValue.h"
#include "VoxelGraphVisuals.h"
#include "SLevelOfDetailBranchNode.h"

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FVoxelGraphVisuals::PinWidgetFactories.Add([](const FVoxelPinType& InnerType, UEdGraphPin* Pin) -> TSharedPtr<SGraphPin>
	{
		if (InnerType.Is<FVoxelStackLayer>() ||
			InnerType.Is<FVoxelStackHeightLayer>() ||
			InnerType.Is<FVoxelStackVolumeLayer>())
		{
			return SNew(SVoxelGraphPinStackLayer, Pin);
		}

		return nullptr;
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelGraphPinStackLayer::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
	GetLabelAndValue()->SetWrapSize(300.f);

	const auto GetFullWidgetSize = [this]
	{
		static FVector2D DefaultSize(20.f, 57.f);

		const TSharedPtr<SHorizontalBox> Box = FullPinHorizontalRowWidget.Pin();
		if (!Box)
		{
			return DefaultSize;
		}

		const FVector2D Size = Box->GetDesiredSize();
		if (Size == FVector2D::ZeroVector)
		{
			return DefaultSize;
		}

		return Size;
	};

	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &SVoxelGraphPinStackLayer::GetPinBorder)
		.BorderBackgroundColor(this, &SVoxelGraphPinStackLayer::GetHighlightColor)
		.OnMouseButtonDown(this, &SVoxelGraphPinStackLayer::OnPinNameMouseDown)
		[
			SNew(SBorder)
			.BorderImage(CachedImg_Pin_DiffOutline)
			.BorderBackgroundColor(this, &SVoxelGraphPinStackLayer::GetPinDiffColor)
			[
				SNew(SLevelOfDetailBranchNode)
				.UseLowDetailSlot(this, &SVoxelGraphPinStackLayer::UseLowDetailPinNames)
				.LowDetail()
				[
					SNew(SBox)
					.WidthOverride_Lambda([=]
					{
						return GetFullWidgetSize().X;
					})
					.HeightOverride_Lambda([=]
					{
						return GetFullWidgetSize().Y;
					})
					.HAlign(GetDirection() == EGPD_Input ? HAlign_Left : HAlign_Right)
					.VAlign(VAlign_Center)
					[
						PinImage.ToSharedRef()
					]
				]
				.HighDetail()
				[
					FullPinHorizontalRowWidget.Pin().ToSharedRef()
				]
			]
		]
	);
}

TSharedRef<SWidget>	SVoxelGraphPinStackLayer::GetDefaultValueWidget()
{
	const FVoxelPinValue DefaultValue = FVoxelPinValue::MakeFromPinDefaultValue(*GraphPinObj);

	if (DefaultValue.Is<FVoxelStackLayer>())
	{
		CurrentLayer = DefaultValue.Get<FVoxelStackLayer>();
	}
	else if (DefaultValue.Is<FVoxelStackHeightLayer>())
	{
		CurrentLayer = DefaultValue.Get<FVoxelStackHeightLayer>();
		LayerType = EVoxelLayerType::Height;
	}
	else if (ensure(DefaultValue.Is<FVoxelStackVolumeLayer>()))
	{
		CurrentLayer = DefaultValue.Get<FVoxelStackVolumeLayer>();
		LayerType = EVoxelLayerType::Volume;
	}

	return
		SNew(SVoxelGraphStackLayerSelector)
		.Visibility(this, &SVoxelGraphPinStackLayer::GetDefaultValueVisibility)
		.IsEnabled(this, &SVoxelGraphPinStackLayer::IsEditingEnabled)
		.ThumbnailSize(FIntPoint(48, 48))
		.ThumbnailPool(FVoxelEditorUtilities::GetThumbnailPool())
		.LayerType(LayerType)
		.StackLayer_Lambda([this]() -> FVoxelStackLayer
		{
			return CurrentLayer;
		})
		.OnUpdateStackLayer_Lambda([this](const FVoxelStackLayer& NewStackLayer)
		{
			if (!ensure(!GraphPinObj->IsPendingKill()))
			{
				return;
			}

			const FVoxelTransaction Transaction(GraphPinObj, "Set Stack Layer");

			FVoxelPinValue Value;

			if (!LayerType.IsSet())
			{
				Value = FVoxelPinValue::Make(NewStackLayer);
			}
			else
			{
				if (LayerType == EVoxelLayerType::Height)
				{
					Value = FVoxelPinValue::Make(FVoxelStackHeightLayer
					{
						NewStackLayer.Stack,
						Cast<UVoxelHeightLayer>(NewStackLayer.Layer)
					});
				}
				else if (ensure(LayerType == EVoxelLayerType::Volume))
				{
					Value = FVoxelPinValue::Make(FVoxelStackVolumeLayer
					{
						NewStackLayer.Stack,
						Cast<UVoxelVolumeLayer>(NewStackLayer.Layer)
					});
				}
			}
			GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, Value.ExportToString());

			CurrentLayer = NewStackLayer;
		});
}