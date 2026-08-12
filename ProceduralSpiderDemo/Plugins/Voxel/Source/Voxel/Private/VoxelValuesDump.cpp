// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelValuesDump.h"
#include "VoxelLayers.h"
#include "VoxelLayerStack.h"
#include "VoxelStampDelta.h"
#include "VoxelStampRuntime.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"

VOXEL_CONSOLE_WORLD_COMMAND(
	"DumpVoxelValues",
	"Dump voxel values at a location, use the context menu in editor to get the right command")
{
	if (Args.Num() != 5)
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Usage: DumpVoxelValues /Game/Stack /Game/Layer X=0 Y=0 Z=0"));
		return;
	}

	const FString StackPath = Args[0];
	const FString LayerPath = Args[1];
	const FString PositionString = Args[2] + " " + Args[3] + " " + Args[4];

	UVoxelLayerStack* Stack = LoadObject<UVoxelLayerStack>(nullptr, *StackPath);
	if (!Stack)
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to load %s"), *StackPath);
		return;
	}

	UVoxelLayer* Layer = LoadObject<UVoxelLayer>(nullptr, *LayerPath);
	if (!Layer)
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to load %s"), *LayerPath);
		return;
	}

	FVector Position = FVector(ForceInit);
	if (!Position.InitFromString(PositionString))
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to parse %s"), *PositionString);
		return;
	}

	FVoxelValuesDump::Log(
		World,
		Stack,
		Layer,
		Position);
}

void FVoxelValuesDump::Log(
	UWorld* World,
	UVoxelLayerStack* Stack,
	UVoxelLayer* Layer,
	const FVector& Position)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelLayers> Layers = FVoxelLayers::Get(World);

	const TVoxelArray<FVoxelStampDelta> StampDeltas = Layers->GetStampDeltas(
		FVoxelStackLayer(Stack, Layer),
		Position,
		0);

	LOG_VOXEL(Log, "%d stamps:", StampDeltas.Num());
	LOG_VOXEL(Log, "Height:");

	for (const FVoxelStampDelta& StampDelta : StampDeltas)
	{
		if (!StampDelta.Stamp->IsA<FVoxelHeightStampRuntime>())
		{
			continue;
		}
		const FVoxelHeightStamp& Stamp = StampDelta.Stamp->GetStamp().AsChecked<FVoxelHeightStamp>();

		FString PackageName;
#if WITH_EDITOR
		if (const AActor* Actor = StampDelta.Stamp->GetActor())
		{
			PackageName = Actor->GetPackage()->GetLoadedPath().GetPackageName();
		}
#endif

		LOG_VOXEL(Log, "\t%8.2f %-15s Smoothness=%8.1f HeightPaddingMultiplier=%2.1f %s %s %s",
			StampDelta.DistanceAfter,
			*GetEnumDisplayName(Stamp.BlendMode).ToString(),
			Stamp.Smoothness,
			Stamp.HeightPaddingMultiplier,
			*Stamp.GetStruct()->GetName(),
			*MakeVoxelObjectPtr(Stamp.GetAsset()).GetPathName(),
			*PackageName);
	}

	LOG_VOXEL(Log, "Volume:");

	for (const FVoxelStampDelta& StampDelta : StampDeltas)
	{
		if (!StampDelta.Stamp->IsA<FVoxelVolumeStampRuntime>())
		{
			continue;
		}
		const FVoxelVolumeStamp& Stamp = StampDelta.Stamp->GetStamp().AsChecked<FVoxelVolumeStamp>();

		FString PackageName;
#if WITH_EDITOR
		if (const AActor* Actor = StampDelta.Stamp->GetActor())
		{
			PackageName = Actor->GetPackage()->GetLoadedPath().GetPackageName();
		}
#endif

		LOG_VOXEL(Log, "\t%8.2f %-15s Smoothness=%8.1f BoundsExtensionMultiplier=%2.1f MaximumBoundsExtension=%f %s %s %s",
			StampDelta.DistanceAfter,
			*GetEnumDisplayName(Stamp.BlendMode).ToString(),
			Stamp.Smoothness,
			Stamp.BoundsExtensionMultiplier,
			Stamp.MaximumBoundsExtension,
			*Stamp.GetStruct()->GetName(),
			*MakeVoxelObjectPtr(Stamp.GetAsset()).GetPathName(),
			*PackageName);
	}

	LOG_VOXEL(Log, "DumpVoxelValues %s %s %s",
		*MakeVoxelObjectPtr(Stack).GetPathName(),
		*MakeVoxelObjectPtr(Layer).GetPathName(),
		*Position.ToString());
}