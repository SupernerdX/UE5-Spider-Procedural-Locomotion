// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelLayer.h"
#include "VoxelLayerStack.h"
#include "VoxelEditorSettings.h"
#include "VoxelLayerFactory.generated.h"

UCLASS()
class VOXELEDITOR_API UVoxelHeightLayerFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelHeightLayerFactory()
	{
		SupportedClass = UVoxelHeightLayer::StaticClass();

		bCreateNew = true;
		bEditAfterNew = true;
	}

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, const FName Name, const EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		UVoxelHeightLayer* LayerAsset = NewObject<UVoxelHeightLayer>(InParent, Class, Name, Flags);
		if (!LayerAsset)
		{
			return nullptr;
		}

		const FLinearColor Color = UVoxelEditorSettings::GetRandomLayerColor();
		if (Color != FVoxelAssetIcon().Color)
		{
			LayerAsset->AssetIcon.bCustomIcon = true;
			LayerAsset->AssetIcon.Color = Color;
		}

		return LayerAsset;
	}
	//~ End UFactory Interface
};

UCLASS()
class VOXELEDITOR_API UVoxelVolumeLayerFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelVolumeLayerFactory()
	{
		SupportedClass = UVoxelVolumeLayer::StaticClass();

		bCreateNew = true;
		bEditAfterNew = true;
	}

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, const FName Name, const EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		UVoxelVolumeLayer* LayerAsset = NewObject<UVoxelVolumeLayer>(InParent, Class, Name, Flags);
		if (!LayerAsset)
		{
			return nullptr;
		}

		const FLinearColor Color = UVoxelEditorSettings::GetRandomLayerColor();
		if (Color != FVoxelAssetIcon().Color)
		{
			LayerAsset->AssetIcon.bCustomIcon = true;
			LayerAsset->AssetIcon.Color = Color;
		}

		return LayerAsset;
	}
	//~ End UFactory Interface
};

UCLASS()
class VOXELEDITOR_API UVoxelLayerStackFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelLayerStackFactory()
	{
		SupportedClass = UVoxelLayerStack::StaticClass();

		bCreateNew = true;
		bEditAfterNew = true;
	}

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, const FName Name, const EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		UVoxelLayerStack* LayerStackAsset = NewObject<UVoxelLayerStack>(InParent, Class, Name, Flags);
		if (!LayerStackAsset)
		{
			return nullptr;
		}

		const FLinearColor Color = UVoxelEditorSettings::GetRandomStackColor();
		if (Color != FVoxelAssetIcon().Color)
		{
			LayerStackAsset->AssetIcon.bCustomIcon = true;
			LayerStackAsset->AssetIcon.Color = Color;
		}

		return LayerStackAsset;
	}
	//~ End UFactory Interface
};