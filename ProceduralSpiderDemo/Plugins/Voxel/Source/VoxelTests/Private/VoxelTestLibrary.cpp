// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelTestLibrary.h"
#include "NavigationSystem.h"
#include "Misc/ConfigCacheIni.h"
#include "NavMesh/RecastNavMesh.h"
#include "Kismet/KismetSystemLibrary.h"

FVoxelTestHandle UVoxelTestLibrary::StartTest(const FString& Name)
{
	const FVoxelTestId Id = FVoxelTestId::New();

	FVoxelTestManager::FTestData& Data = GVoxelTestManager->MapData.IdToData.Add_EnsureNew(Id);
	Data.Name = Name;
	Data.State = EVoxelTestState::Started;

	return { Id };
}

void UVoxelTestLibrary::PassTest(const FVoxelTestHandle& Handle)
{
	if (!GVoxelTestManager->MapData.IdToData.Contains(Handle.Id))
	{
		if (GVoxelTestManager->bIsRunningTests)
		{
			LOG_VOXEL(Fatal, "Invalid handle passed to PassTest");
		}

		VOXEL_MESSAGE(Error, "Invalid handle passed to PassTest");
		return;
	}

	FVoxelTestManager::FTestData& Data = GVoxelTestManager->MapData.IdToData[Handle.Id];
	if (Data.State != EVoxelTestState::Started)
	{
		if (GVoxelTestManager->bIsRunningTests)
		{
			LOG_VOXEL(Fatal, "Already completed handle passed to PassTest");
		}

		VOXEL_MESSAGE(Error, "Already completed handle passed to PassTest");
		return;
	}

	Data.State = EVoxelTestState::Succeeded;
}

void UVoxelTestLibrary::FailTest(
	const FVoxelTestHandle& Handle,
	const FString& Reason)
{
	// TODO Capture BP callstack

	if (!GVoxelTestManager->MapData.IdToData.Contains(Handle.Id))
	{
		if (GVoxelTestManager->bIsRunningTests)
		{
			LOG_VOXEL(Fatal, "Invalid handle passed to FailTest");
		}

		VOXEL_MESSAGE(Error, "Invalid handle passed to FailTest");
		return;
	}

	FVoxelTestManager::FTestData& Data = GVoxelTestManager->MapData.IdToData[Handle.Id];
	if (Data.State != EVoxelTestState::Started)
	{
		if (GVoxelTestManager->bIsRunningTests)
		{
			LOG_VOXEL(Fatal, "Already completed handle passed to FailTest");
		}

		VOXEL_MESSAGE(Error, "Already completed handle passed to FailTest");
		return;
	}

	Data.State = EVoxelTestState::Failed;
	Data.FailureReason = Reason;
}

void UVoxelTestLibrary::TakeScreenshot(const FGuid& Guid)
{
	if (!Guid.IsValid())
	{
		if (GVoxelTestManager->bIsRunningTests)
		{
			LOG_VOXEL(Fatal, "Invalid screenshot GUID");
		}

		VOXEL_MESSAGE(Error, "Invalid screenshot GUID");
		return;
	}

	if (GVoxelTestManager->MapData.ScreenshotIds.Contains(Guid))
	{
		if (GVoxelTestManager->bIsRunningTests)
		{
			LOG_VOXEL(Fatal, "Duplicate screenshot GUID: %s", *Guid.ToString());
		}

		VOXEL_MESSAGE(Error, "Duplicate screenshot GUID: {0}", Guid.ToString());
		return;
	}

	GVoxelTestManager->MapData.ScreenshotIds.Add(Guid);

	const FString Command = FString::Printf(
		TEXT("HighResShot 1920x1080 filename=\"%s\""),
		*FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / "VoxelTests" / GVoxelTestManager->MapData.MapName / Guid.ToString() + ".png"));

	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Taking screenshot");
	UKismetSystemLibrary::ExecuteConsoleCommand(GWorld, *Command);
}

DEFINE_PRIVATE_ACCESS(ARecastNavMesh, RuntimeGeneration)
DEFINE_PRIVATE_ACCESS_FUNCTION(UNavigationSystemV1, RebuildAll)
DEFINE_PRIVATE_ACCESS_FUNCTION(UNavigationSystemV1, OnReloadComplete)

void UVoxelTestLibrary::SetNavMeshGeneration(UObject* WorldContextObject, const bool bRuntime)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(WorldContextObject);
	if (!NavSys)
	{
		return;
	}

	class UNavigationAccessor : public UNavigationSystemV1
	{
	public:
		void SetGenerateOnlyAroundNavigationInvokers(const bool bValue)
		{
			bGenerateNavigationOnlyAroundNavigationInvokers = bValue;
			OnGenerateNavigationOnlyAroundNavigationInvokersChanged();
		}
	};
	reinterpret_cast<UNavigationAccessor*>(NavSys)->SetGenerateOnlyAroundNavigationInvokers(bRuntime);

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return;
	}

	for (TActorIterator<ARecastNavMesh> It(World); It; ++It)
	{
		ARecastNavMesh* Actor = *It;
		PrivateAccess::RuntimeGeneration(*Actor) =
			bRuntime
			? ERuntimeGenerationType::Dynamic
			: ERuntimeGenerationType::Static;
		// Actor->RebuildAll();
	}

	if (!bRuntime)
	{
		const FString EngineIniFilename = FPaths::ConvertRelativePathToFull(GetDefault<UEngine>()->GetDefaultConfigFilename());
		GConfig->SetString(TEXT("/Script/NavigationSystem.RecastNavMesh"), TEXT("RuntimeGeneration"), TEXT("Static"), *EngineIniFilename);
		GConfig->Flush(false);
	}

	PrivateAccess::RebuildAll(*NavSys)(false);
	PrivateAccess::OnReloadComplete(*NavSys)(EReloadCompleteReason::None);
}