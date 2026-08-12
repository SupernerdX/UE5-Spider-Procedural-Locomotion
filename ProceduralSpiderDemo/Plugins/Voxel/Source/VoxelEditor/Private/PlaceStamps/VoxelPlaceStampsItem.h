// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "AssetSelection.h"
#include "VoxelActorFactories.h"

struct FVoxelPlaceStampsItem
{
	FString Name;
	FString Description;
	FString Type;
	FString BrushName;
	TSharedPtr<FAssetThumbnail> AssetThumbnail;
	TSet<FName> UserTags;
	TSet<FName> SystemTags;

	FAssetData Asset;
	TScriptInterface<IAssetFactoryInterface> ActorFactory = nullptr;
	bool bIsFavorite = false;

	TDelegate<TSet<FName>()> GetTags;
	TDelegate<FString()> GetDescription;

	void Setup()
	{
		if (GetDescription.IsBound())
		{
			Description = GetDescription.Execute();
		}

		if (GetTags.IsBound())
		{
			UserTags = GetTags.Execute();
		}

		if (ActorFactory)
		{
			return;
		}

		if (Asset.GetClass()->IsChildOf<UScriptStruct>())
		{
			ActorFactory = UActorFactory_VoxelPlaceStampActor::StaticClass()->GetDefaultObject<UActorFactory>();
			return;
		}

		const UClass* TargetClass = Cast<UClass>(Asset.GetAsset());
		if (!TargetClass)
		{
			TargetClass = Asset.GetClass();
		}

		ActorFactory = GEditor->FindActorFactoryByClass(TargetClass);
		if (ActorFactory)
		{
			return;
		}

		ActorFactory = GEditor->FindActorFactoryForActorClass(TargetClass);
		if (ActorFactory)
		{
			return;
		}

		UObject* ClassObject = TargetClass->GetDefaultObject();
		ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(ClassObject);
	}
};