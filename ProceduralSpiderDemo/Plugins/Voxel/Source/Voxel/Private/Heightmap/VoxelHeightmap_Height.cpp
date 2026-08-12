// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Heightmap/VoxelHeightmap_Height.h"
#include "Heightmap/VoxelHeightmap.h"
#include "Engine/Texture2D.h"
#if WITH_EDITOR
#include "EditorReimportHandler.h"
#endif

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP_GAME()
{
	FReimportManager::Instance()->OnPostReimport().AddLambda([](const UObject* Asset, bool bSuccess)
	{
		const UTexture2D* Texture = Cast<UTexture2D>(Asset);
		if (!Texture)
		{
			return;
		}

		ForEachObjectOfClass_Copy<UVoxelHeightmap_Height>([&](UVoxelHeightmap_Height& Height)
		{
			if (Height.Texture == Texture)
			{
				Height.OnReimport();
			}
		});
	});
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelHeightmap_Height::SetHeights(
	const int32 SizeX,
	const int32 SizeY,
	const TConstVoxelArrayView<uint16> Heights,
	const float InternalScaleZ,
	const float InternalOffsetZ)
{
	VOXEL_FUNCTION_COUNTER();
	check(Heights.Num() == SizeX * SizeY);

	const TSharedRef<FVoxelHeightmap_HeightData> NewData = MakeShared<FVoxelHeightmap_HeightData>();
#if WITH_EDITOR
	NewData->bCustomData = true;
#endif
	NewData->SizeX = SizeX;
	NewData->SizeY = SizeY;
	NewData->bIsUINT16 = true;
	NewData->InternalScaleZ = InternalScaleZ;
	NewData->InternalOffsetZ = InternalOffsetZ;
	NewData->DataMin = FVoxelUtilities::GetMin(Heights);
	NewData->DataMax = FVoxelUtilities::GetMax(Heights);
	NewData->RawData = TVoxelArray<uint8>(Heights.ReinterpretAs<uint8>());
	Data = NewData;
}

void UVoxelHeightmap_Height::SetHeights(
	const int32 SizeX,
	const int32 SizeY,
	const TConstVoxelArrayView<float> Heights,
	const float InternalScaleZ,
	const float InternalOffsetZ)
{
	VOXEL_FUNCTION_COUNTER();
	check(Heights.Num() == SizeX * SizeY);

	const TSharedRef<FVoxelHeightmap_HeightData> NewData = MakeShared<FVoxelHeightmap_HeightData>();
#if WITH_EDITOR
	NewData->bCustomData = true;
#endif
	NewData->SizeX = SizeX;
	NewData->SizeY = SizeY;
	NewData->bIsUINT16 = false;
	NewData->InternalScaleZ = InternalScaleZ;
	NewData->InternalOffsetZ = InternalOffsetZ;
	NewData->DataMin = FVoxelUtilities::GetMin(Heights);
	NewData->DataMax = FVoxelUtilities::GetMax(Heights);
	NewData->RawData = TVoxelArray<uint8>(Heights.ReinterpretAs<uint8>());
	Data = NewData;
}

#if WITH_EDITOR
void UVoxelHeightmap_Height::OnReimport()
{
	VOXEL_FUNCTION_COUNTER();

	Data.Reset();
	(void)GetData();
}
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TSharedPtr<const FVoxelHeightmap_HeightData> UVoxelHeightmap_Height::GetData() const
{
	VOXEL_FUNCTION_COUNTER();

#if WITH_EDITOR
	if (Data &&
		bAllowUpdate &&
		!Data->bCustomData)
	{
		if (Data->Texture != Texture ||
			Data->TextureChannel != TextureChannel ||
			Data->bCompressTo16Bits != bCompressTo16Bits ||
			Data->bEnableMinHeight != bEnableMinHeight ||
			Data->MinHeight != MinHeight ||
			Data->MinHeightSlope != MinHeightSlope)
		{
			Data.Reset();
		}
	}
#endif

	if (!Data)
	{
#if WITH_EDITOR
		Data = FVoxelHeightmap_HeightData::Create(*this);
#else
		ensure(false);
#endif
	}

	return Data;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelHeightmap_Height::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);

	int32 Version = 0;
	Ar << Version;
	ensure(Version == 0);

	bool bHasEditorOnlyData = !Ar.IsCooking();
	Ar << bHasEditorOnlyData;

	if (Ar.IsLoading())
	{
		if (bHasEditorOnlyData)
		{
			// No serialized data in editor
			ensure(!FPlatformProperties::RequiresCookedData());
			return;
		}

		// We are cooked, use the serialized data

		const TSharedRef<FVoxelHeightmap_HeightData> NewData = MakeShared<FVoxelHeightmap_HeightData>();
		NewData->Serialize(Ar, this);
		Data = NewData;
		return;
	}

	if (Ar.IsSaving())
	{
		if (Ar.IsCooking())
		{
			// Serialize data if we are cooking

			(void)GetData();

			if (!Data)
			{
				Data = MakeShared<FVoxelHeightmap_HeightData>();
			}

			ConstCast(*Data).Serialize(Ar, this);
		}
		return;
	}

	if (Ar.IsCountingMemory() &&
		GetData())
	{
		ConstCast(*GetData()).Serialize(Ar, this);
	}
}

#if WITH_EDITOR
void UVoxelHeightmap_Height::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		GetOuterUVoxelHeightmap()->OnChanged_EditorOnly.Broadcast();
	}
}
#endif