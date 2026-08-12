// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPlaceStampsDefaults.h"
#include "VoxelSettings.h"
#include "VoxelStampActor.h"
#include "Shape/VoxelShapeStamp.h"
#include "StaticMesh/VoxelMeshStamp.h"
#include "Graphs/VoxelHeightGraphStamp.h"
#include "Graphs/VoxelVolumeGraphStamp.h"
#include "Heightmap/VoxelHeightmapStamp.h"
#include "Spline/VoxelHeightSplineStamp.h"
#include "Spline/VoxelVolumeSplineStamp.h"
#include "Surface/VoxelSurfaceTypeInterface.h"

void FVoxelPlaceStampDefaults::ApplyOnStamp(const FVoxelStampRef& StampRef) const
{
	if (!StampRef.IsValid())
	{
		return;
	}

	StampRef->Behavior = Behavior;
	StampRef->Smoothness = Smoothness;
	StampRef->MetadataOverrides = MetadataOverrides;
	StampRef->LODRange = LODRange;

	if (FVoxelHeightStamp* HeightStamp = StampRef.As<FVoxelHeightStamp>())
	{
		HeightStamp->Layer = HeightLayer;
		HeightStamp->BlendMode = HeightBlendMode;
		HeightStamp->bApplyOnVoid = bHeightApplyOnVoid;
		HeightStamp->AdditionalLayers = HeightAdditionalLayers;
		HeightStamp->HeightPaddingMultiplier = HeightPaddingMultiplier;
	}
	else if (FVoxelVolumeStamp* VolumeStamp = StampRef.As<FVoxelVolumeStamp>())
	{
		VolumeStamp->Layer = VolumeLayer;
		VolumeStamp->BlendMode = VolumeBlendMode;
		VolumeStamp->bApplyOnVoid = bVolumeApplyOnVoid;
		VolumeStamp->AdditionalLayers = VolumeAdditionalLayers;
		VolumeStamp->BoundsExtensionMultiplier = BoundsExtensionMultiplier;
		VolumeStamp->MaximumBoundsExtension = MaximumBoundsExtension;
	}

	INLINE_LAMBDA
	{
		if (!SurfaceType)
		{
			return;
		}

		if (FVoxelHeightmapStamp* Stamp = StampRef.As<FVoxelHeightmapStamp>())
		{
			Stamp->DefaultSurfaceType = SurfaceType;
			return;
		}
		if (FVoxelMeshStamp* Stamp = StampRef.As<FVoxelMeshStamp>())
		{
			Stamp->SurfaceType = SurfaceType;
			return;
		}
		if (FVoxelShapeStamp* Stamp = StampRef.As<FVoxelShapeStamp>())
		{
			Stamp->SurfaceType = SurfaceType;
			return;
		}

		IVoxelParameterOverridesOwner* OverridesOwner = nullptr;
		if (FVoxelHeightGraphStamp* HeightGraphStamp = StampRef.As<FVoxelHeightGraphStamp>())
		{
			OverridesOwner = HeightGraphStamp;
		}
		else if (FVoxelHeightSplineStamp* HeightSplineStamp = StampRef.As<FVoxelHeightSplineStamp>())
		{
			OverridesOwner = HeightSplineStamp;
		}
		else if (FVoxelVolumeGraphStamp* VolumeGraphStamp = StampRef.As<FVoxelVolumeGraphStamp>())
		{
			OverridesOwner = VolumeGraphStamp;
		}
		else if (FVoxelVolumeSplineStamp* VolumeSplineStamp = StampRef.As<FVoxelVolumeSplineStamp>())
		{
			OverridesOwner = VolumeSplineStamp;
		}

		if (!OverridesOwner)
		{
			return;
		}

		const FVoxelPinValue Parameter = OverridesOwner->GetParameter("Surface Type");
		if (Parameter.Is<UVoxelSurfaceTypeInterface>())
		{
			OverridesOwner->SetParameter("Surface Type", FVoxelPinValue::Make(SurfaceType.Get()));
		}
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPlaceStampDefaultsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	if (ActiveTabName == "Heightmaps")
	{
		DetailLayout.HideCategory("Volume Defaults");
	}
	else if (
		ActiveTabName == "Shapes" ||
		ActiveTabName == "Meshes")
	{
		DetailLayout.HideCategory("Height Defaults");
	}

	const auto UpdateBlendModeRow = [&](const TSharedRef<IPropertyHandle>& BlendModeHandle, const TSharedRef<IPropertyHandle>& ApplyOnVoidHandle)
	{
		IDetailPropertyRow* ApplyOnVoidRow = DetailLayout.EditDefaultProperty(ApplyOnVoidHandle);
		ApplyOnVoidHandle->MarkHiddenByCustomization();

		IDetailPropertyRow* Row = DetailLayout.EditDefaultProperty(BlendModeHandle);

		TSharedPtr<SWidget> DummyNameWidget;
		TSharedPtr<SWidget> AdditionalRowValueWidget;
		ApplyOnVoidRow->GetDefaultWidgets(DummyNameWidget, AdditionalRowValueWidget, true);

		AdditionalRowValueWidget->SetToolTipText(ApplyOnVoidHandle->GetToolTipText());

		Row->CustomWidget(true)
		.NameContent()
		[
			BlendModeHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			.ToolTipText(ApplyOnVoidHandle->GetToolTipText())
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				BlendModeHandle->CreatePropertyValueWidgetWithCustomization(nullptr)
			]
			+ SHorizontalBox::Slot()
			.MaxWidth(15.f)
			[
				SNew(SSpacer)
				.Size(15.f)
			]
			+ SHorizontalBox::Slot()
			.FillContentWidth(1.f)
			.Padding(2.f, 0.f, 0.f, 0.f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				.Text(FText::Format(INVTEXT("{0}: "), ApplyOnVoidHandle->GetPropertyDisplayName()))
			]
			+ SHorizontalBox::Slot()
			.Padding(2.f, 0.f, 0.f, 0.f)
			.AutoWidth()
			[
				AdditionalRowValueWidget.ToSharedRef()
			]
		];
	};

#define GET_HANDLE(Name) DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(FVoxelPlaceStampDefaults, Name), FVoxelPlaceStampDefaults::StaticStruct())

	UpdateBlendModeRow(GET_HANDLE(HeightBlendMode), GET_HANDLE(bHeightApplyOnVoid));
	UpdateBlendModeRow(GET_HANDLE(VolumeBlendMode), GET_HANDLE(bVolumeApplyOnVoid));

#undef GET_HANDLE
}