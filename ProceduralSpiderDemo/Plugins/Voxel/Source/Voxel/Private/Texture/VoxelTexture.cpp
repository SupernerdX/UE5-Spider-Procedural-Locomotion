// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Texture/VoxelTexture.h"
#include "Texture/VoxelTextureData.h"
#include "VoxelDependency.h"
#include "Engine/Texture2D.h"
#if WITH_EDITOR
#include "EditorReimportHandler.h"
#endif

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP_GAME()
{
	FReimportManager::Instance()->OnPostReimport().AddLambda([](const UObject* Asset, bool bSuccess)
	{
		const UTexture2D* Texture2D = Cast<UTexture2D>(Asset);
		if (!Texture2D)
		{
			return;
		}

		ForEachObjectOfClass_Copy<UVoxelTexture>([&](UVoxelTexture& Texture)
		{
			if (Texture.Texture == Texture2D)
			{
				Texture.OnReimport();
			}
		});
	});
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DEFINE_VOXEL_FACTORY(UVoxelTexture);

UVoxelTexture* UVoxelTexture::Create(const TSharedRef<const FVoxelTextureData>& Data)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	UVoxelTexture* Texture = NewObject<UVoxelTexture>();
	Texture->PrivateData = Data;
	return Texture;
}

#if WITH_EDITOR
void UVoxelTexture::OnReimport()
{
	VOXEL_FUNCTION_COUNTER();

	PrivateData.Reset();
	(void)GetData();
}
#endif

TSharedPtr<const FVoxelTextureData> UVoxelTexture::GetData() const
{
	VOXEL_FUNCTION_COUNTER();

#if WITH_EDITOR
	if (PrivateData)
	{
		if (PrivateData->Texture != Texture)
		{
			if (Dependency)
			{
				Dependency->Invalidate();
			}

			PrivateData.Reset();
		}
	}
#endif

	if (!PrivateData)
	{
#if WITH_EDITOR
		PrivateData = FVoxelTextureData::Create_EditorOnly(*this);
#else
		ensure(false);
#endif
	}

	ensure(!PrivateData || PrivateData->Buffer);
	return PrivateData;
}

void UVoxelTexture::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);

	int32 Version = 0;
	Ar << Version;
	ensure(Version == 0);

	bool bHasEditorOnlyData = !Ar.IsCooking();
	Ar << bHasEditorOnlyData;

	if (bHasEditorOnlyData)
	{
		ensure(!FPlatformProperties::RequiresCookedData());
		return;
	}

	if (Ar.IsLoading())
	{
		const TSharedRef<FVoxelTextureData> NewData = MakeShared<FVoxelTextureData>();
		NewData->Serialize(Ar, this);
		PrivateData = NewData;
	}
	else
	{
#if WITH_EDITOR
		(void)GetData();

		if (!PrivateData)
		{
			PrivateData = MakeShared<FVoxelTextureData>();
		}

		ConstCast(*PrivateData).Serialize(Ar, this);
#else
		ensure(false);
#endif
	}
}

#if WITH_EDITOR
void UVoxelTexture::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		(void)GetData();
	}
}
#endif