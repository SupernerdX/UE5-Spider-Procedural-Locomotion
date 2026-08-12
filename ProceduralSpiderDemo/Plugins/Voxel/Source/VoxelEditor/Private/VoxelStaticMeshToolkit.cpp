// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStaticMeshToolkit.h"
#include "VoxelWorld.h"
#include "VoxelStampActor.h"
#include "StaticMesh/VoxelMeshStamp.h"
#include "StaticMesh/VoxelStaticMeshData.h"

#include "SceneManagement.h"
#include "SEditorViewportToolBarMenu.h"

void FVoxelStaticMeshToolkit::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick();

	VoxelWorld->VoxelSize = Asset->bPreviewAssetVoxelSize ? Asset->VoxelSize : Asset->PreviewVoxelSize;

	const FNumberFormattingOptions Options = FNumberFormattingOptions().SetMinimumFractionalDigits(2).SetMaximumFractionalDigits(2);

	const TSharedPtr<const FVoxelStaticMeshData> Data = Asset->GetData_NoPrepare();
	if (!Data)
	{
		return;
	}

	const double SizeInMB = Data->GetAllocatedSize() / double(1 << 20);

	GetViewport().SetStatsText(FString() +
		"<TextBlock.ShadowedText>Memory size: </>" +
		"<TextBlock.ShadowedTextWarning>" + FText::AsNumber(SizeInMB, &Options).ToString() + "MB</>\n" +
		"<TextBlock.ShadowedText>Data size: " + Data->Size.ToString() + "</>");
}

void FVoxelStaticMeshToolkit::SetupPreview()
{
	VOXEL_FUNCTION_COUNTER();

	Super::SetupPreview();

	VoxelWorld = GetViewport().SpawnActor<AVoxelWorld>();
	if (!ensure(VoxelWorld))
	{
		return;
	}

	VoxelWorld->SetupForPreview();

	StampActor = GetViewport().SpawnActor<AVoxelStampActor>();
	if (!ensure(StampActor))
	{
		return;
	}

	FVoxelMeshStamp Stamp;
	Stamp.NewMesh = Asset;
	Stamp.SurfaceType = Asset->PreviewMaterial;
	StampActor->SetStamp(Stamp);
	VoxelWorld->VoxelSize = Asset->bPreviewAssetVoxelSize ? Asset->VoxelSize : Asset->PreviewVoxelSize;
}

void FVoxelStaticMeshToolkit::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	Super::Draw(View, PDI);

	if (!ensure(Asset) ||
		!Asset->Mesh ||
		Asset->Mesh->IsCompiling())
	{
		return;
	}

	const TSharedPtr<const FVoxelStaticMeshData> Data = Asset->GetData_NoPrepare();
	if (!Data)
	{
		return;
	}

	const FBox Box =
		FVoxelBox(0, FVector(Data->Size))
		.ShiftBy(FVector(Data->Origin))
		.Scale(Data->VoxelSize)
		.ToFBox();

	DrawWireBox(PDI, Box, FLinearColor(0.0f, 0.48828125f, 0.0f), SDPG_World);
}

TOptional<float> FVoxelStaticMeshToolkit::GetInitialViewDistance() const
{
	const TSharedPtr<const FVoxelStaticMeshData> Data = Asset->GetData_NoPrepare();
	if (!Data)
	{
		return {};
	}

	return Data->Size.Size() * Asset->VoxelSize;
}

void FVoxelStaticMeshToolkit::PopulateToolBar(const TSharedRef<SHorizontalBox>& ToolbarBox, const TSharedPtr<SViewportToolBar>& ParentToolBarPtr)
{
	Super::PopulateToolBar(ToolbarBox, ParentToolBarPtr);

	ToolbarBox->AddSlot()
	.AutoWidth()
	[
		SNew(SEditorViewportToolbarMenu)
		.ToolTipText(INVTEXT("Preview voxel size"))
		.ParentToolBar(ParentToolBarPtr)
		.Label_Lambda([this]
		{
			if (!Asset)
			{
				return INVTEXT("Invalid");
			}

			if (!Asset->bPreviewAssetVoxelSize)
			{
				return FText::FromString("Preview Voxel Size: " + FText::AsNumber(Asset->PreviewVoxelSize).ToString());
			}

			return FText::FromString("Asset Voxel Size: " + FText::AsNumber(Asset->VoxelSize).ToString());
		})
		.LabelIcon(FAppStyle::GetBrush("ClassIcon.Cube"))
		.OnGetMenuContent_Lambda([this]
		{
			FMenuBuilder InMenuBuilder(true, nullptr);
			InMenuBuilder.BeginSection("PreviewVoxelSize", INVTEXT("Preview Voxel Size"));
			{
				InMenuBuilder.AddMenuEntry(
					INVTEXT("Preview Asset Voxel Size"),
					INVTEXT("Will preview mesh using mesh voxelization size"),
					{},
					FUIAction(
						MakeWeakPtrDelegate(this, [this]
						{
							if (!Asset)
							{
								return;
							}

							FVoxelTransaction Transaction(Asset, "Preview Voxel Size");
							Asset->bPreviewAssetVoxelSize = !Asset->bPreviewAssetVoxelSize;
						}),
						MakeWeakPtrDelegate(this, []
						{
							return true;
						}),
						MakeWeakPtrDelegate(this, [this]
						{
							if (!Asset)
							{
								return true;
							}

							return Asset->bPreviewAssetVoxelSize;
						})),
						{},
						EUserInterfaceActionType::ToggleButton);
				InMenuBuilder.AddMenuEntry(
					FUIAction(
						{},
						MakeWeakPtrDelegate(this, [this]
						{
							if (!Asset)
							{
								return false;
							}

							return !Asset->bPreviewAssetVoxelSize;
						})),
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(2.f, 0.f, 6.f, 0.f)
					[
						SNew(STextBlock)
						.TextStyle(FAppStyle::Get(), "Menu.Label")
						.Text(INVTEXT("Preview Voxel Size: "))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNew(SBox)
						.MinDesiredWidth(100.f)
						[
							SNew(SSpinBox<int32>)
							.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
							.MinValue(1)
							.Value_Lambda([this]
							{
								if (!Asset)
								{
									return 100;
								}

								return Asset->PreviewVoxelSize;
							})
							.OnValueChanged_Lambda([this](const int32 NewValue)
							{
								if (!Asset)
								{
									return;
								}

								FVoxelTransaction Transaction(Asset, "Preview Voxel Size");
								Asset->PreviewVoxelSize = NewValue;
							})
						]
					]);
			}
			InMenuBuilder.EndSection();

			return InMenuBuilder.MakeWidget();
		})
	];
}

void FVoxelStaticMeshToolkit::PostEditChange(const FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChange(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty != &FindFPropertyChecked(UVoxelStaticMesh, PreviewMaterial))
	{
		return;
	}

	if (!StampActor)
	{
		return;
	}

	FVoxelMeshStamp* Stamp = StampActor->GetStamp().As<FVoxelMeshStamp>();
	if (!Stamp)
	{
		return;
	}

	Stamp->SurfaceType = Asset->PreviewMaterial;
	StampActor->UpdateStamp();
}

void FVoxelStaticMeshToolkit::PostUndo()
{
	Super::PostUndo();
	if (!StampActor)
	{
		return;
	}

	FVoxelMeshStamp* Stamp = StampActor->GetStamp().As<FVoxelMeshStamp>();
	if (!Stamp)
	{
		return;
	}

	Stamp->SurfaceType = Asset->PreviewMaterial;
	StampActor->UpdateStamp();
}