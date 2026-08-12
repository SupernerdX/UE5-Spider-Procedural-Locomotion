// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelStampTypeSelector.h"
#include "VoxelStampRef.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"
#include "VoxelHeightStampRef.h"
#include "VoxelVolumeStampRef.h"
#include "SVoxelSegmentedControl.h"
#include "Styling/StarshipCoreStyle.h"

VOXEL_INITIALIZE_STYLE(VoxelStampTypeSelectorStyle)
{
	Set("StampTypeSelectorSectionStyle",
		FSegmentedControlStyle(FAppStyle::GetWidgetStyle<FSegmentedControlStyle>("SegmentedControl"))
		.SetBackgroundBrush(FSlateRoundedBoxBrush(FStyleColors::Recessed, CoreStyleConstants::InputFocusRadius))
		.SetUniformPadding(FMargin(4, 4)));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelStampTypeSelector::Construct(const FArguments& Args)
{
	VOXEL_FUNCTION_COUNTER();

	Handle = Args._Handle;
	PropertyUtilities = Args._PropertyUtilities;

	static const TVoxelArray<UScriptStruct*> Structs = INLINE_LAMBDA
	{
		VOXEL_SCOPE_COUNTER("Structs");

		const TVoxelArray<UScriptStruct*> NewStructs = GetDerivedStructs<FVoxelStamp>();

		TVoxelMap<UScriptStruct*, int32> StructToSortOrder;
		StructToSortOrder.Reserve(NewStructs.Num());

		for (UScriptStruct* Struct : NewStructs)
		{
			if (Struct->HasMetaData("Abstract") ||
				Struct->HasMetaData("Internal"))
			{
				continue;
			}

			int32 SortOrder = 0;
			if (Struct->HasMetaData("SortOrder"))
			{
				SortOrder = Struct->GetIntMetaData("SortOrder");
			}

			StructToSortOrder.Add_CheckNew(Struct, SortOrder);
		}

		StructToSortOrder.ValueSort();

		return StructToSortOrder.KeyArray();
	};

	HeightStructs.Reserve(Structs.Num());
	VolumeStructs.Reserve(Structs.Num());

	const UScriptStruct* PropertyStruct = FVoxelStampRef::StaticStruct();
	if (const FStructProperty* Property = CastField<FStructProperty>(Handle->GetProperty()))
	{
		PropertyStruct = Property->Struct;
	}

	for (UScriptStruct* Struct : Structs)
	{
		if (Args._HiddenStructs.Contains(Struct))
		{
			continue;
		}

		if (Struct->IsChildOf(StaticStructFast<FVoxelHeightStamp>()))
		{
			if (PropertyStruct == FVoxelStampRef::StaticStruct() ||
				PropertyStruct == FVoxelHeightStampRef::StaticStruct() ||
				PropertyStruct == FVoxelHeightInstancedStampRef::StaticStruct())
			{
				HeightStructs.Add(Struct);
			}
		}
		else
		{
			ensure(Struct->IsChildOf(StaticStructFast<FVoxelVolumeStamp>()));
			if (PropertyStruct == FVoxelStampRef::StaticStruct() ||
				PropertyStruct == FVoxelVolumeStampRef::StaticStruct() ||
				PropertyStruct == FVoxelVolumeInstancedStampRef::StaticStruct())
			{
				VolumeStructs.Add(Struct);
			}
		}
	}

	SetCanTick(true);

	ChildSlot
	.HAlign(HAlign_Fill)
	[
		SAssignNew(HorizontalBox, SHorizontalBox)
	];

	Refresh();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelStampTypeSelector::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	Refresh();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelStampTypeSelector::Refresh()
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelSet<UScriptStruct*> SelectedStructs;
	FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(Handle, [&](const FVoxelStampRef& StampRef)
	{
		if (!StampRef.IsValid())
		{
			return;
		}

		SelectedStructs.Add(StampRef.GetStruct());
	});

	UScriptStruct* NewSelectedStruct = nullptr;
	if (SelectedStructs.Num() == 1)
	{
		NewSelectedStruct = SelectedStructs.GetUniqueValue();
	}

	if (SelectedStruct == NewSelectedStruct &&
		HorizontalBox->NumSlots() > 0)
	{
		return;
	}

	SelectedStruct = NewSelectedStruct;
	RecreateWidgets();
}

void SVoxelStampTypeSelector::RecreateWidgets()
{
	VOXEL_FUNCTION_COUNTER();

	HorizontalBox->ClearChildren();

	const TSharedRef<SSegmentedControl<TSharedPtr<FTab>>> TypeSelector =
		SNew(SSegmentedControl<TSharedPtr<FTab>>)
		.MaxSegmentsPerLine(1)
		.Value_Lambda([this]
		{
			if (!SelectedStruct)
			{
				if (HeightStructs.Num() > 0)
				{
					return HeightTab;
				}
				else
				{
					ensure(VolumeStructs.Num() > 0);
					return VolumeTab;
				}
			}

			if (SelectedStruct->IsChildOf(StaticStruct<FVoxelHeightStamp>()))
			{
				return HeightTab;
			}
			else
			{
				ensure(SelectedStruct->IsChildOf(StaticStruct<FVoxelVolumeStamp>()));
				return VolumeTab;
			}
		})
		.OnValueChanged_Lambda([this](const TSharedPtr<FTab> Node)
		{
			if (Node == HeightTab)
			{
				if (ensure(HeightStructs.Num() > 0))
				{
					SetStruct(*HeightStructs[0]);
				}
			}
			else if (ensureVoxelSlow(Node == VolumeTab))
			{
				if (ensure(VolumeStructs.Num() > 0))
				{
					SetStruct(*VolumeStructs[0]);
				}
			}
		});

	if (HeightStructs.Num() > 0)
	{
		TypeSelector->AddSlot(HeightTab)
		[
			SNew(SVoxelDetailText)
			.Text(INVTEXT("Height"))
		];
	}
	if (VolumeStructs.Num() > 0)
	{
		TypeSelector->AddSlot(VolumeTab)
		[
			SNew(SVoxelDetailText)
			.Text(INVTEXT("Volume"))
		];
	}

	if (TypeSelector->NumSlots() > 1)
	{
		HorizontalBox->AddSlot()
		.FillWidth(0.15f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(2.f, 2.f, 15.f, 2.f)
		[
			TypeSelector
		];
	}

	TSharedPtr<SVoxelSegmentedControl<TSharedPtr<UScriptStruct*>>> SegmentedControl;

	HorizontalBox->AddSlot()
	.FillWidth(0.7f)
	.HAlign(HAlign_Fill)
	.Padding(0.f, 4.f)
	[
		SAssignNew(SegmentedControl, SVoxelSegmentedControl<TSharedPtr<UScriptStruct*>>)
		.Style(&FVoxelEditorStyle::Get(), "StampTypeSelectorSectionStyle")
		.MinDesiredSlotWidth(75.f)
		.OnValueChanged_Lambda([this](const TSharedPtr<UScriptStruct*> StructPtr)
		{
			if (!ensure(StructPtr))
			{
				return;
			}

			SetStruct(**StructPtr);
		})
	];

	if (TypeSelector->NumSlots() > 1)
	{
		HorizontalBox->AddSlot()
		.FillWidth(0.15f)
		[
			SNew(SSpacer)
		];
	}

	const TVoxelArray<UScriptStruct*> Structs = INLINE_LAMBDA
	{
		if (!SelectedStruct)
		{
			if (HeightStructs.Num() > 0)
			{
				return HeightStructs;
			}
			else
			{
				ensure(VolumeStructs.Num() > 0);
				return VolumeStructs;
			}
		}

		if (SelectedStruct->IsChildOf(StaticStruct<FVoxelHeightStamp>()))
		{
			return HeightStructs;
		}
		else
		{
			ensure(SelectedStruct->IsChildOf(StaticStruct<FVoxelVolumeStamp>()));
			return VolumeStructs;
		}
	};

	for (UScriptStruct* Struct : Structs)
	{
		const FSlateIcon Icon = INLINE_LAMBDA
		{
			if (!Struct->HasMetaData("Icon"))
			{
				ensure(!Struct->HasMetaData("StyleSet"));
				return FSlateIcon(FVoxelEditorStyle::GetStyleSetName(), "VoxelEdMode.Small");
			}

			const FName StyleName = INLINE_LAMBDA
			{
				if (Struct->HasMetaData("StyleSet"))
				{
					return FName(Struct->GetMetaData("StyleSet"));
				}
				else
				{
					return FAppStyle::GetAppStyleSetName();
				}
			};

			const FName IconName = FName(Struct->GetMetaData("Icon"));

			return FSlateIcon(StyleName, IconName);
		};

		const FText Name = INLINE_LAMBDA
		{
			if (Struct->HasMetaData("ShortName"))
			{
				return FText::FromString(Struct->GetMetaData("ShortName"));
			}

			return Struct->GetDisplayNameText();
		};

		const TSharedRef<UScriptStruct*> SharedStruct = MakeSharedCopy(Struct);

		SegmentedControl->AddSlot(SharedStruct)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SLayeredImage)
				.Visibility(EVisibility::HitTestInvisible)
				.DesiredSizeOverride(FVector2D(24.f))
				.Image(Icon.GetIcon())
			]
			+ SVerticalBox::Slot().AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SVoxelDetailText)
				.Text(Name)
			]
		];

		if (Struct == SelectedStruct)
		{
			SegmentedControl->SetValue(SharedStruct);
		}
	}
}

void SVoxelStampTypeSelector::SetStruct(UScriptStruct& Struct)
{
	VOXEL_FUNCTION_COUNTER();

	SelectedStruct = &Struct;

	if (!Handle ||
		!Handle->IsValidHandle())
	{
		return;
	}

	FScopedTransaction Transaction(INVTEXT("Set Struct"));

	Handle->NotifyPreChange();

	FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(Handle, [&Struct](FVoxelStampRef& StampRef)
	{
		StampRef.SetStruct_Editor(&Struct);
	});

	Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
	Handle->NotifyFinishedChangingProperties();

	if (PropertyUtilities.IsValid())
	{
		PropertyUtilities->RequestForceRefresh();
	}
}