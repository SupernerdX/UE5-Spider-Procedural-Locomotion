// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Heightmap/VoxelHeightmap_Weight.h"
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

		ForEachObjectOfClass_Copy<UVoxelHeightmap_Weight>([&](UVoxelHeightmap_Weight& Weight)
		{
			if (Weight.Texture == Texture)
			{
				Weight.OnReimport();
			}
		});
	});
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelHeightmap_Weight::SetWeights(
	const int32 SizeX,
	const int32 SizeY,
	const TConstVoxelArrayView<uint8> Weights)
{
	VOXEL_FUNCTION_COUNTER();
	check(Weights.Num() == SizeX * SizeY);

	const TSharedRef<FVoxelHeightmap_WeightData> NewData = MakeShared<FVoxelHeightmap_WeightData>();
#if WITH_EDITOR
	NewData->bCustomData = true;
#endif
	NewData->SizeX = SizeX;
	NewData->SizeY = SizeY;
	NewData->Weights = TVoxelArray<uint8>(Weights);
	Data = NewData;
}

void UVoxelHeightmap_Weight::SetWeights(
	const int32 SizeX,
	const int32 SizeY,
	const TConstVoxelArrayView<uint16> Weights)
{
	VOXEL_FUNCTION_COUNTER();
	check(Weights.Num() == SizeX * SizeY);

	const TSharedRef<FVoxelHeightmap_WeightData> NewData = MakeShared<FVoxelHeightmap_WeightData>();
#if WITH_EDITOR
	NewData->bCustomData = true;
#endif
	NewData->SizeX = SizeX;
	NewData->SizeY = SizeY;

	FVoxelUtilities::SetNumFast(NewData->Weights, SizeX * SizeY);
	for (int32 Index = 0; Index < SizeX * SizeY; Index++)
	{
		const float Value = Weights[Index] / float(MAX_uint16);
		NewData->Weights[Index] = FVoxelUtilities::ClampToUINT8(FMath::RoundToInt(MAX_uint8 * Value));
	}

	Data = NewData;
}

void UVoxelHeightmap_Weight::SetWeights(
	const int32 SizeX,
	const int32 SizeY,
	const TConstVoxelArrayView<float> Weights)
{
	VOXEL_FUNCTION_COUNTER();
	check(Weights.Num() == SizeX * SizeY);

	const TSharedRef<FVoxelHeightmap_WeightData> NewData = MakeShared<FVoxelHeightmap_WeightData>();
#if WITH_EDITOR
	NewData->bCustomData = true;
#endif
	NewData->SizeX = SizeX;
	NewData->SizeY = SizeY;

	FVoxelUtilities::SetNumFast(NewData->Weights, SizeX * SizeY);
	for (int32 Index = 0; Index < SizeX * SizeY; Index++)
	{
		NewData->Weights[Index] = FVoxelUtilities::ClampToUINT8(FMath::RoundToInt(MAX_uint8 * Weights[Index]));
	}

	Data = NewData;
}

#if WITH_EDITOR
void UVoxelHeightmap_Weight::OnReimport()
{
	VOXEL_FUNCTION_COUNTER();

	Data.Reset();
	(void)GetData();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<const FVoxelHeightmap_WeightData> UVoxelHeightmap_Weight::GetData() const
{
	VOXEL_FUNCTION_COUNTER();

#if WITH_EDITOR
	if (Data &&
		!Data->bCustomData)
	{
		if (Data->Texture != Texture ||
			Data->TextureChannel != TextureChannel)
		{
			Data.Reset();
		}
	}
#endif

	if (Data)
	{
		return Data.ToSharedRef();
	}

#if WITH_EDITOR
	Data = FVoxelHeightmap_WeightData::Create(*this);
	return Data;
#else
	ensure(false);
	return nullptr;
#endif
}

void UVoxelHeightmap_Weight::Serialize(FArchive& Ar)
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
		const TSharedRef<FVoxelHeightmap_WeightData> NewData = MakeShared<FVoxelHeightmap_WeightData>();
		NewData->Serialize(Ar, this);
		Data = NewData;
	}
	else
	{
#if WITH_EDITOR
		(void)GetData();

		if (!Data)
		{
			Data = MakeShared<FVoxelHeightmap_WeightData>();
		}

		ConstCast(*Data).Serialize(Ar, this);
#else
		ensure(false);
#endif
	}
}

#if WITH_EDITOR
void UVoxelHeightmap_Weight::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		GetOuterUVoxelHeightmap()->OnChanged_EditorOnly.Broadcast();
	}
}
#endif