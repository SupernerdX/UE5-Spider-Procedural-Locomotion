// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "ToolMenus.h"
#include "DetailRowMenuContext.h"
#include "Styling/SlateIconFinder.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "Subsystems/AssetEditorSubsystem.h"

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(UE::PropertyEditor::RowContextMenuName);

	FToolMenuSection& EditSection = Menu->FindOrAddSection("Edit");
	EditSection.AddDynamicEntry("OpenSurfaceTypeMaterial", MakeLambdaDelegate([](FToolMenuSection& Section)
	{
		const UDetailRowMenuContext* Context = Section.FindContext<UDetailRowMenuContext>();
		if (Context->PropertyHandles.Num() != 1)
		{
			return;
		}

		TSharedPtr<IPropertyHandle> PropertyHandle = Context->PropertyHandles[0];
		if (!PropertyHandle ||
			!PropertyHandle->IsValidHandle())
		{
			return;
		}

		const bool bIsSurfaceType = INLINE_LAMBDA
		{
			const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(PropertyHandle->GetProperty());
			if (!ObjectProperty ||
				!ObjectProperty->PropertyClass)
			{
				return false;
			}

			if (ObjectProperty->PropertyClass->IsChildOf<UVoxelSurfaceTypeInterface>())
			{
				return true;
			}

			if (!PropertyHandle->HasMetaData("AllowedClasses"))
			{
				return false;
			}

			const FString AllowedClasses = PropertyHandle->GetMetaData("AllowedClasses");
			TArray<FString> AllowedClassesFilterNames;
			AllowedClasses.ParseIntoArrayWS(AllowedClassesFilterNames, TEXT(","), true);

			for (const FString& ClassPath : AllowedClassesFilterNames)
			{
				const UStruct* Struct = FindObject<UStruct>(nullptr, *ClassPath);
				if (!Struct)
				{
					continue;
				}

				if (Struct->IsChildOf(UVoxelSurfaceTypeInterface::StaticClass()))
				{
					return true;
				}
			}

			return false;
		};

		if (!bIsSurfaceType)
		{
			return;
		}

		{
			UObject* Object = nullptr;
			if (PropertyHandle->GetValue(Object) != FPropertyAccess::Success)
			{
				return;
			}

			const UVoxelSurfaceTypeAsset* Asset = Cast<UVoxelSurfaceTypeAsset>(Object);
			if (!Asset)
			{
				return;
			}
		}

		Section.AddMenuEntry(
			"OpenMaterial",
			INVTEXT("Open Surface Type Material"),
			INVTEXT(""),
			FSlateIconFinder::FindIconForClass(UVoxelSurfaceTypeAsset::StaticClass()),
			MakeLambdaDelegate([WeakHandle = MakeWeakPtr(PropertyHandle)]
			{
				const TSharedPtr<IPropertyHandle> Handle = WeakHandle.Pin();
				if (!Handle ||
					!Handle->IsValidHandle())
				{
					return;
				}

				UObject* Object = nullptr;
				if (Handle->GetValue(Object) != FPropertyAccess::Success)
				{
					return;
				}

				const UVoxelSurfaceTypeAsset* Asset = Cast<UVoxelSurfaceTypeAsset>(Object);
				if (!Asset)
				{
					return;
				}

				UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
				if (!AssetEditorSubsystem)
				{
					return;
				}

				FText ErrorMsg;
				if (AssetEditorSubsystem->CanOpenEditorForAsset(Asset->Material, EAssetTypeActivationOpenedMethod::Edit, &ErrorMsg))
				{
					AssetEditorSubsystem->OpenEditorForAsset(Asset->Material);
				}
			}));
	}));
}