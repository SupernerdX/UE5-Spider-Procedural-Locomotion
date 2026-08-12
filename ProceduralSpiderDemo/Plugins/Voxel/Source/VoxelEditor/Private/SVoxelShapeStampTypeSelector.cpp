// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelShapeStampTypeSelector.h"
#include "Shape/VoxelShape.h"
#include "SVoxelSegmentedControl.h"

void SVoxelShapeStampTypeSelector::Construct(const FArguments& Args)
{
	VOXEL_FUNCTION_COUNTER();

	Handle = Args._Handle;
	PropertyUtilities = Args._PropertyUtilities;

	static const TVoxelArray<UScriptStruct*> StaticStructs = INLINE_LAMBDA
	{
		VOXEL_SCOPE_COUNTER("Structs");

		const TVoxelArray<UScriptStruct*> NewStructs = GetDerivedStructs<FVoxelShape>();

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

	Structs = StaticStructs;

	SetCanTick(true);

	ChildSlot
	[
		SAssignNew(Content, SBox)
		.HAlign(HAlign_Center)
		.Padding(0.f, 4.f)
	];

	Refresh();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelShapeStampTypeSelector::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	Refresh();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelShapeStampTypeSelector::Refresh()
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelSet<UScriptStruct*> SelectedStructs;
	FVoxelEditorUtilities::ForeachData<FVoxelInstancedStruct>(Handle, [&](const FVoxelInstancedStruct& Shape)
	{
		if (!Shape.IsValid())
		{
			return;
		}

		SelectedStructs.Add(Shape.GetScriptStruct());
	});

	UScriptStruct* NewSelectedStruct = nullptr;
	if (SelectedStructs.Num() == 1)
	{
		NewSelectedStruct = SelectedStructs.GetUniqueValue();
	}

	if (SelectedStruct == NewSelectedStruct &&
		SegmentedControl)
	{
		return;
	}

	SelectedStruct = NewSelectedStruct;
	RecreateWidgets();
}

void SVoxelShapeStampTypeSelector::RecreateWidgets()
{
	VOXEL_FUNCTION_COUNTER();

	Content->SetContent(
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
		}));

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

void SVoxelShapeStampTypeSelector::SetStruct(UScriptStruct& Struct)
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

	FVoxelEditorUtilities::ForeachData<FVoxelInstancedStruct>(Handle, [&Struct](FVoxelInstancedStruct& Shape)
	{
		Shape.InitializeAs(&Struct);
	});

	Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
	Handle->NotifyFinishedChangingProperties();

	if (PropertyUtilities.IsValid())
	{
		PropertyUtilities->RequestForceRefresh();
	}
}