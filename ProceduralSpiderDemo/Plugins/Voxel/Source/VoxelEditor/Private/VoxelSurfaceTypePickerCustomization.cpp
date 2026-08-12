// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelComponentSettings.h"
#include "VoxelSurfacePickerEdMode.h"
#include "Surface/VoxelSurfaceTypePicker.h"

class SVoxelSurfacePickerButton : public SButton
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_EVENT(TDelegate<void(UVoxelSurfaceTypeInterface*)>, OnValueChanged)
	};

	void Construct(const FArguments& InArgs)
	{
		SButton::Construct(
			SButton::FArguments()
			.ButtonStyle( FAppStyle::Get(), "HoverHintOnly" )
			.OnClicked_Lambda([Delegate = InArgs._OnValueChanged]
			{
				if (FVoxelSurfacePickerEdMode::IsSurfaceTypePickerActive())
				{
					FVoxelSurfacePickerEdMode::EndSurfaceTypePicker();
				}
				else
				{
					FVoxelSurfacePickerEdMode::BeginSurfaceTypePicker(Delegate);
				}
				return FReply::Handled();
			})
			.ContentPadding(4.0f)
			.ForegroundColor( FSlateColor::UseForeground() )
			.IsFocusable(false)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.EyeDropper"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		);
	}

public:
	//~ Begin SButton Interface
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			if (FVoxelSurfacePickerEdMode::IsSurfaceTypePickerActive())
			{
				FVoxelSurfacePickerEdMode::EndSurfaceTypePicker();
				return FReply::Handled();
			}
		}

		return FReply::Unhandled();
	}
	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}
	virtual ~SVoxelSurfacePickerButton() override
	{
		// Delay
		FVoxelUtilities::DelayedCall([]
		{
			if (FVoxelSurfacePickerEdMode::IsSurfaceTypePickerActive())
			{
				FVoxelSurfacePickerEdMode::EndSurfaceTypePicker();
			}
		});
	}
	//~ End SButton Interface
};

class FVoxelSurfaceTypePickerCustomization : public FVoxelPropertyTypeCustomizationBase
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		const TSharedRef<IPropertyHandle> SurfaceTypeHandle = PropertyHandle->GetChildHandleStatic(FVoxelSurfaceTypePicker, SurfaceType);
		HeaderRow
		.NameContent()
		[
			SurfaceTypeHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(250.f)
		.MaxDesiredWidth(250.f)
		[
			SNew(SObjectPropertyEntryBox)
			.PropertyHandle(SurfaceTypeHandle)
			.DisplayThumbnail(true)
			.ThumbnailPool(CustomizationUtils.GetThumbnailPool())
			.AllowedClass(UVoxelSurfaceTypeInterface::StaticClass())
			.AllowClear(true)
			.CustomContentSlot()
			[
				SNew(SBox)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(SVoxelSurfacePickerButton)
					.ToolTipText(INVTEXT("Pick Voxel Surface Type from scene"))
					.OnValueChanged_Lambda([SurfaceTypeHandle](UVoxelSurfaceTypeInterface* NewSurfaceType)
					{
						SurfaceTypeHandle->SetValue(NewSurfaceType);
					})
				]
			]
		];
	}

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
	}
};

DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelSurfaceTypePicker, FVoxelSurfaceTypePickerCustomization);