// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelLayers.h"
#include "VoxelToolEdMode.h"
#include "VoxelSculptChanges.h"
#include "VoxelStampComponent.h"
#include "VoxelStampCustomization.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"
#include "Sculpt/Height/VoxelSculptHeight.h"
#include "Sculpt/Height/VoxelHeightSculptStamp.h"
#include "Sculpt/Height/VoxelSculptHeightAsset.h"
#include "EditorModes.h"
#include "Misc/ITransaction.h"
#include "EditorModeManager.h"

class FVoxelHeightSculptStampCustomization : public FVoxelStampCustomization
{
public:
	virtual void CustomizeChildren(
		const TSharedRef<IPropertyHandle> PropertyHandle,
		IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		FVoxelStampCustomization::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);

		ChildBuilder.AddCustomRow(INVTEXT("Sculpt Data"))
		.WholeRowContent()
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(0.f, 2.f)
			.AutoWidth()
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Text_Lambda([]
				{
					return
						GLevelEditorModeTools().IsModeActive("VoxelToolEdMode")
						? INVTEXT("Exit Sculpt Mode")
						: INVTEXT("Enter Sculpt Mode");
				})
				.OnClicked_Lambda([PropertyHandle]
				{
					if (GLevelEditorModeTools().IsModeActive("VoxelToolEdMode"))
					{
						GLevelEditorModeTools().DeactivateMode("VoxelToolEdMode");
						GLevelEditorModeTools().ActivateDefaultMode();
						return FReply::Handled();
					}

					GLevelEditorModeTools().DeactivateAllModes();
					GLevelEditorModeTools().ActivateMode("VoxelToolEdMode");

					if (FVoxelToolEdMode* EdMode = static_cast<FVoxelToolEdMode*>(GLevelEditorModeTools().GetActiveMode("VoxelToolEdMode")))
					{
						FVoxelEditorUtilities::ForeachData<FVoxelHeightSculptStamp>(PropertyHandle, [&](FVoxelHeightSculptStamp& SculptStamp, UObject* Outer)
						{
							if (AVoxelSculptHeight* SculptActor = Outer->GetTypedOuter<AVoxelSculptHeight>())
							{
								EdMode->SwitchSculptActor(SculptActor);
							}
						});
					}
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(0.f, 2.f)
			.AutoWidth()
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Text(INVTEXT("Clear Sculpt Data"))
				.OnClicked_Lambda([PropertyHandle]
				{
					FScopedTransaction Transaction(INVTEXT("Clear Sculpt Data"));

					FVoxelEditorUtilities::ForeachData<FVoxelHeightSculptStamp>(PropertyHandle, [&](FVoxelHeightSculptStamp& SculptStamp, UObject* Outer)
					{
						AVoxelSculptHeight* SculptActor = Outer->GetTypedOuter<AVoxelSculptHeight>();
						if (!ensure(SculptActor))
						{
							return;
						}

						GUndo->StoreUndo(
							SculptActor,
							MakeUnique<FVoxelHeightSculptChange>(SculptActor->GetSculptData()));

						SculptActor->SetSculptData(MakeVoxelBulkRef(MakeShared<FVoxelSculptHeightData>()));
					});

					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(0.f, 2.f)
			.AutoWidth()
			[
				SNew(SButton)
				.Visibility_Lambda([PropertyHandle]
				{
					EVisibility Visibility = EVisibility::Visible;
					FVoxelEditorUtilities::ForeachData<FVoxelHeightSculptStamp>(PropertyHandle, [&](FVoxelHeightSculptStamp& SculptStamp, UObject* Outer)
					{
						const AVoxelSculptHeight* SculptActor = Outer->GetTypedOuter<AVoxelSculptHeight>();
						if (!ensure(SculptActor))
						{
							return;
						}

						if (SculptActor->GetComponent().ExternalAsset ||
							SculptActor->GetSculptData()->IsEmpty())
						{
							Visibility = EVisibility::Collapsed;
						}
					});

					return Visibility;
				})
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Text(INVTEXT("Create Sculpt Asset"))
				.ToolTipText(INVTEXT("Create Sculpt Asset from actor Sculpt Data"))
				.OnClicked_Lambda([PropertyHandle]
				{
					FScopedTransaction Transaction(INVTEXT("Create Sculpt Asset"));

					FVoxelEditorUtilities::ForeachData<FVoxelHeightSculptStamp>(PropertyHandle, [&](FVoxelHeightSculptStamp& SculptStamp, UObject* Outer)
					{
						AVoxelSculptHeight* SculptActor = Outer->GetTypedOuter<AVoxelSculptHeight>();
						if (!ensure(SculptActor))
						{
							return;
						}

						GUndo->StoreUndo(
							SculptActor,
							MakeUnique<FVoxelHeightSculptChange>(SculptActor->GetSculptData()));

						UVoxelSculptHeightAsset* Asset = FVoxelUtilities::CreateNewAsset_Dialog<UVoxelSculptHeightAsset>(SculptActor->GetActorLabel(), "/Game");
						if (!ensure(Asset))
						{
							return;
						}

						Asset->SetData(SculptActor->GetSculptData());
						SculptActor->SetSculptData(MakeVoxelBulkRef(MakeShared<FVoxelSculptHeightData>()));

						SculptActor->GetComponent().ExternalAsset = Asset;
						SculptActor->SetSculptData(Asset->GetData(), Asset->GetBulkLoader());
					});

					return FReply::Handled();
				})
			]
		];
	}

	virtual void CustomizePropertyRow(
		TSharedRef<IPropertyHandle> BasePropertyHandle,
		FProperty* Property,
		IDetailPropertyRow& PropertyRow) override
	{
		if (Property != &FindFPropertyChecked(FVoxelHeightSculptStamp, bStoreRelativeHeights))
		{
			return;
		}

		const auto IsRelativeHeightsEditable = [BasePropertyHandle]
		{
			bool bEditable = true;
			FVoxelEditorUtilities::ForeachData<FVoxelHeightSculptStamp>(BasePropertyHandle, [&](FVoxelHeightSculptStamp& SculptStamp, UObject* Outer)
			{
				if (const AVoxelSculptHeight* SculptActor = Outer->GetTypedOuter<AVoxelSculptHeight>())
				{
					if (!SculptActor->GetSculptData()->IsEmpty())
					{
						bEditable = false;
					}
				}
			});
			return bEditable;
		};

		const TSharedPtr<IPropertyHandle> Handle = PropertyRow.GetPropertyHandle();
		PropertyRow.EditCondition(MakeAttributeSPLambda(Handle.Get(), IsRelativeHeightsEditable), {});
		FDetailWidgetRow& Row = PropertyRow.CustomWidget();

		Row
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
				.Visibility_Lambda([IsRelativeHeightsEditable]
				{
					return
						!IsRelativeHeightsEditable()
						? EVisibility::Visible
						: EVisibility::Collapsed;
				})
				.ToolTipText(INVTEXT("Relative heights cannot be modified, while this sculpt actor has active sculpt data."))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.Error"))
					]
					+ SHorizontalBox::Slot()
					.Padding(4.f, 0.f, 0.f, 0.f)
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SVoxelDetailText)
						.Text(INVTEXT("Cannot change Relative Heights storage"))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]
				]
			]
		];
	}
};

DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelHeightSculptStamp, FVoxelHeightSculptStampCustomization);