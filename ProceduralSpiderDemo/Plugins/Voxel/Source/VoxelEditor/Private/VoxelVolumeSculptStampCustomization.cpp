// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelLayers.h"
#include "VoxelToolEdMode.h"
#include "VoxelSculptChanges.h"
#include "VoxelStampCustomization.h"
#include "Sculpt/Volume/VoxelSculptVolume.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "Sculpt/Volume/VoxelSculptVolumeAsset.h"
#include "EditorModes.h"
#include "Misc/ITransaction.h"
#include "EditorModeManager.h"

class FVoxelVolumeSculptStampCustomization : public FVoxelStampCustomization
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
						FVoxelEditorUtilities::ForeachData<FVoxelVolumeSculptStamp>(PropertyHandle, [&](FVoxelVolumeSculptStamp& SculptStamp, UObject* Outer)
						{
							if (AVoxelSculptVolume* SculptActor = Outer->GetTypedOuter<AVoxelSculptVolume>())
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

					FVoxelEditorUtilities::ForeachData<FVoxelVolumeSculptStamp>(PropertyHandle, [&](FVoxelVolumeSculptStamp& SculptStamp, UObject* Outer)
					{
						AVoxelSculptVolume* SculptActor = Outer->GetTypedOuter<AVoxelSculptVolume>();
						if (!ensure(SculptActor))
						{
							return;
						}

						GUndo->StoreUndo(
							SculptActor,
							MakeUnique<FVoxelVolumeSculptChange>(SculptActor->GetSculptData()));

						SculptActor->SetSculptData(MakeVoxelBulkRef(MakeShared<FVoxelSculptVolumeData>()));
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
					FVoxelEditorUtilities::ForeachData<FVoxelVolumeSculptStamp>(PropertyHandle, [&](FVoxelVolumeSculptStamp& SculptStamp, UObject* Outer)
					{
						const AVoxelSculptVolume* SculptActor = Outer->GetTypedOuter<AVoxelSculptVolume>();
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

					FVoxelEditorUtilities::ForeachData<FVoxelVolumeSculptStamp>(PropertyHandle, [&](FVoxelVolumeSculptStamp& SculptStamp, UObject* Outer)
					{
						AVoxelSculptVolume* SculptActor = Outer->GetTypedOuter<AVoxelSculptVolume>();
						if (!ensure(SculptActor))
						{
							return;
						}

						GUndo->StoreUndo(
							SculptActor,
							MakeUnique<FVoxelVolumeSculptChange>(SculptActor->GetSculptData()));

						UVoxelSculptVolumeAsset* Asset = FVoxelUtilities::CreateNewAsset_Dialog<UVoxelSculptVolumeAsset>(SculptActor->GetActorLabel(), "/Game");
						if (!ensure(Asset))
						{
							return;
						}

						Asset->SetData(SculptActor->GetSculptData());
						SculptActor->SetSculptData(MakeVoxelBulkRef(MakeShared<FVoxelSculptVolumeData>()));

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
		if (Property != &FindFPropertyChecked(FVoxelVolumeSculptStamp, bStoreMovableDistances))
		{
			return;
		}

		const auto IsMovableDistancesEditable = [BasePropertyHandle]
		{
			bool bEditable = true;
			FVoxelEditorUtilities::ForeachData<FVoxelVolumeSculptStamp>(BasePropertyHandle, [&](FVoxelVolumeSculptStamp& SculptStamp, UObject* Outer)
			{
				if (const AVoxelSculptVolume* SculptActor = Outer->GetTypedOuter<AVoxelSculptVolume>())
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
		PropertyRow.EditCondition(MakeAttributeSPLambda(Handle.Get(), IsMovableDistancesEditable), {});
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
				.Visibility_Lambda([IsMovableDistancesEditable]
				{
					return
						!IsMovableDistancesEditable()
						? EVisibility::Visible
						: EVisibility::Collapsed;
				})
				.ToolTipText(INVTEXT("Movable distances cannot be modified, while this sculpt actor has active sculpt data."))
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
						.Text(INVTEXT("Cannot change Movable Distances storage"))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]
				]
			]
		];
	}
};

DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelVolumeSculptStamp, FVoxelVolumeSculptStampCustomization);