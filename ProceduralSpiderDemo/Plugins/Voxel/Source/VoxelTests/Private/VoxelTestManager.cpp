// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelTestManager.h"
#include "Dom/JsonObject.h"
#include "PipelineFileCache.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/GameViewportClient.h"

DEFINE_UNIQUE_VOXEL_ID(FVoxelTestId);

VOXEL_CONSOLE_COMMAND(
	"voxel.tests.Start",
	"Start voxel tests")
{
	GVoxelTestManager->StartTests();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelTestsOutputDevice : public FOutputDevice
{
public:
	FVoxelCriticalSection CriticalSection;

	struct FEntry
	{
		FString Line;
		TVoxelArray<FString> Stack;

		bool operator==(const FEntry& Other) const
		{
			return
				Line == Other.Line &&
				Stack == Other.Stack;
		}
	};
	TVoxelArray<FEntry> Warnings;
	TVoxelArray<FEntry> Errors;

	//~ Begin FOutputDevice Interface
	virtual bool CanBeUsedOnAnyThread() const override
	{
		return true;
	}
	virtual bool CanBeUsedOnPanicThread() const override
	{
		return true;
	}
	virtual bool CanBeUsedOnMultipleThreads() const override
	{
		return true;
	}
	virtual void Serialize(const TCHAR* Data, const ELogVerbosity::Type Verbosity, const FName& Category) override
	{
		if (Verbosity != ELogVerbosity::Warning &&
			Verbosity != ELogVerbosity::Error)
		{
			return;
		}

		if (Category == "LogAudio" ||
			Category == "LogAudioMixer")
		{
			// Ignore
			return;
		}

		if (!FString(Data).Contains("ensure"))
		{
			UE_DEBUG_BREAK();
		}

		FEntry Entry;
		Entry.Line = FString::Printf(TEXT("%s: %s"), *Category.ToString(), Data);
		Entry.Stack = FVoxelUtilities::StackFramesToString_NoStats(FVoxelUtilities::GetStackFrames_NoStats(8));

		if (Entry.Line.Contains("Changing default audio render device to new device") ||
			Entry.Line.Contains("Unable to find RecastNavMesh instance while trying to create UCrowdManager instance") ||
			Entry.Line.Contains("Recreating dtNavMesh instance due mismatch in number of bytes required to store serialized maxTiles"))
		{
			// Ignore
			return;
		}

		VOXEL_SCOPE_LOCK(CriticalSection);

		if (Verbosity == ELogVerbosity::Warning)
		{
			Warnings.AddUnique(Entry);
		}
		else
		{
			check(Verbosity == ELogVerbosity::Error);
			Errors.AddUnique(Entry);
		}
	}
	//~ End FOutputDevice Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VOXEL_RUN_ON_STARTUP_GAME()
{
	if (!FParse::Param(FCommandLine::Get(), TEXT("RunVoxelTests")))
	{
		return;
	}

	check(!GIsEditor);

	// Spammy
	FPipelineFileCacheManager::SetNewPSOConsoleAndCSVLogging(false);

	FVoxelUtilities::DelayedCall([]
	{
		FVoxelTestsOutputDevice* OutputDevice = new FVoxelTestsOutputDevice();
		GLog->AddOutputDevice(OutputDevice);

		GVoxelTestManager->OnComplete = MakeLambdaDelegate([=](const TVoxelArray<FVoxelTestManager::FMapData>& MapDatas)
		{
			LOG_VOXEL(Display, "Tests complete");

			const TSharedRef<FJsonObject> ResultJson = MakeShared<FJsonObject>();

			TVoxelArray<FVoxelTestManager::FTestData> FailedTests;
			for (const FVoxelTestManager::FMapData& MapData : MapDatas)
			{
				LOG_VOXEL(Display, "%s", *MapData.MapName);

				const TSharedRef<FJsonObject> MapJson = MakeShared<FJsonObject>();

				{
					TArray<TSharedPtr<FJsonValue>> JsonArray;

					for (const auto& It : MapData.IdToData)
					{
						const FVoxelTestManager::FTestData& TestData = It.Value;
						check(TestData.State != EVoxelTestState::Started);

						LOG_VOXEL(Display, "\t%s %s %s",
							*TestData.Name,
							*LexToString(TestData.State),
							*TestData.FailureReason);

						if (TestData.State == EVoxelTestState::Failed)
						{
							FailedTests.Add(TestData);
						}

						const TSharedRef<FJsonObject> EntryJson = MakeShared<FJsonObject>();
						EntryJson->SetStringField("name", TestData.Name);
						EntryJson->SetBoolField("success", TestData.State == EVoxelTestState::Succeeded);

						if (TestData.State == EVoxelTestState::Failed)
						{
							EntryJson->SetStringField("reason", TestData.FailureReason);
						}

						JsonArray.Add(MakeShared<FJsonValueObject>(EntryJson));
					}

					MapJson->SetArrayField("tests", JsonArray);
				}

				{
					TArray<TSharedPtr<FJsonValue>> JsonArray;

					for (const FGuid& Id : MapData.ScreenshotIds)
					{
						JsonArray.Add(MakeShared<FJsonValueString>(Id.ToString()));
					}

					MapJson->SetArrayField("screenshots", JsonArray);
				}

				ResultJson->SetObjectField(MapData.MapName, MapJson);
			}

			const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
			Json->SetObjectField("results", ResultJson);
			Json->SetBoolField("has_raytracing", IsRayTracingAllowed());

			{
				VOXEL_SCOPE_LOCK(OutputDevice->CriticalSection);

				TArray<TSharedPtr<FJsonValue>> WarningArray;
				for (const FVoxelTestsOutputDevice::FEntry& Entry : OutputDevice->Warnings)
				{
					const TSharedRef<FJsonObject> EntryJson = MakeShared<FJsonObject>();
					EntryJson->SetStringField("line", Entry.Line);

					TArray<TSharedPtr<FJsonValue>> JsonArray;
					for (const FString& Stack : Entry.Stack)
					{
						JsonArray.Add(MakeShared<FJsonValueString>(Stack));
					}
					EntryJson->SetArrayField("stack", JsonArray);

					WarningArray.Add(MakeShared<FJsonValueObject>(EntryJson));
				}
				Json->SetArrayField("warnings", WarningArray);

				TArray<TSharedPtr<FJsonValue>> ErrorArray;
				for (const FVoxelTestsOutputDevice::FEntry& Entry : OutputDevice->Errors)
				{
					const TSharedRef<FJsonObject> EntryJson = MakeShared<FJsonObject>();
					EntryJson->SetStringField("line", Entry.Line);

					TArray<TSharedPtr<FJsonValue>> JsonArray;
					for (const FString& Stack : Entry.Stack)
					{
						JsonArray.Add(MakeShared<FJsonValueString>(Stack));
					}
					EntryJson->SetArrayField("stack", JsonArray);

					ErrorArray.Add(MakeShared<FJsonValueObject>(EntryJson));
				}
				Json->SetArrayField("errors", ErrorArray);
			}

			const FString JsonText = FVoxelUtilities::JsonToString(Json, true);
			verify(FFileHelper::SaveStringToFile(JsonText, *(FPaths::ProjectSavedDir() / "VoxelTests" / "VoxelTests.json")));

			if (FailedTests.Num() > 0)
			{
				LOG_VOXEL(Error, "%d tests failed:", FailedTests.Num());

				for (const FVoxelTestManager::FTestData& Test : FailedTests)
				{
					LOG_VOXEL(Error, "\t%s %s %s",
						*Test.Name,
						*LexToString(Test.State),
						*Test.FailureReason);
				}
			}

			ForEachObjectOfClass_Copy<AActor>([&](AActor& Actor)
			{
				if (!Actor.GetWorld())
				{
					return;
				}

				Actor.Destroy();
			});

			FPlatformMisc::RequestExit(false);
		});

		FTSTicker::GetCoreTicker().AddTicker(MakeLambdaDelegate([=](float)
		{
			for (const auto& It : GVoxelTestManager->MapData.IdToData)
			{
				if (It.Value.State == EVoxelTestState::Started)
				{
					LOG_VOXEL(Display, "Waiting for %s", *It.Value.Name);
				}
			}

			return true;
		}), 10);

		GVoxelTestManager->bIsRunningTests = true;
		GVoxelTestManager->StartTests();
	});
}

FVoxelTestManager* GVoxelTestManager = new FVoxelTestManager();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelTestManager::Initialize()
{
#if WITH_EDITOR
	FWorldDelegates::OnPIEStarted.AddLambda([this](UGameInstance*)
	{
		MapData = {};
	});
	FWorldDelegates::OnPIEEnded.AddLambda([this](UGameInstance*)
	{
		MapData = {};
	});
#endif
}

void FVoxelTestManager::Tick()
{
	INLINE_LAMBDA
	{
		if (!bIsWaitingForTests)
		{
			return;
		}

		for (const auto& It : MapData.IdToData)
		{
			if (It.Value.State == EVoxelTestState::Started)
			{
				return;
			}
		}
		bIsWaitingForTests = false;

		LOG_VOXEL(Display, "TEAMCITY_CLOSE[%s]", *MapData.MapName);

		MapDatas.Add(MapData);
		MapData = {};

		if (MapsToTest.Num() > 0)
		{
			GoToNextTestMap();
			return;
		}

		(void)OnComplete.ExecuteIfBound(MapDatas);
		OnComplete = {};

		MapDatas.Empty();
	};

	for (const auto& It : MapData.IdToData)
	{
		const uint64 Key = uint64(this + It.Key.GetId());

		switch (It.Value.State)
		{
		case EVoxelTestState::Started:
		{
			GEngine->AddOnScreenDebugMessage(Key, 0.1f, FColor::Yellow,  It.Value.Name + ": Running");
		}
		break;
		case EVoxelTestState::Succeeded:
		{
			GEngine->AddOnScreenDebugMessage(Key, 0.1f, FColor::Green,  It.Value.Name + ": Success");
		}
		break;
		case EVoxelTestState::Failed:
		{
			GEngine->AddOnScreenDebugMessage(Key, 0.1f, FColor::Red,  It.Value.Name + ": Failed: " + It.Value.FailureReason);
		}
		break;
		}
	}
}

void FVoxelTestManager::StartTests()
{
	VOXEL_FUNCTION_COUNTER();

	MapsToTest.Reset();

	ForEachAssetDataOfClass<UWorld>([&](const FAssetData& AssetData)
	{
		if (!AssetData.GetObjectPathString().StartsWith("/Game/VoxelTests/"))
		{
			return;
		}

		MapsToTest.Add(AssetData.AssetName.ToString());
	});

	if (MapsToTest.Num() == 0)
	{
		LOG_VOXEL(Fatal, "No test map found");
	}

	GoToNextTestMap();
}

void FVoxelTestManager::GoToNextTestMap()
{
	check(MapsToTest.Num() > 0);
	check(MapData.IdToData.Num() == 0);

	const FString MapName = MapsToTest.Pop();

	GEngine->SetClientTravel(
		GWorld,
		*MapName,
		TRAVEL_Absolute);

	check(MapData.MapName.IsEmpty());
	MapData.MapName = MapName;

	LOG_VOXEL(Display, "TEAMCITY_OPEN[%s]", *MapName);

	const TSharedRef<FSharedVoidPtr> SharedSharedPtr = MakeSharedCopy(MakeSharedVoid().ToSharedPtr());

	FCoreUObjectDelegates::PostLoadMapWithWorld.Add(MakeWeakPtrDelegate(*SharedSharedPtr, [=, this](UWorld*)
	{
		check(SharedSharedPtr->IsValid());
		SharedSharedPtr->Reset();

		if (MapData.IdToData.Num() == 0)
		{
			LOG_VOXEL(Fatal, "Map %s did not start any test", *MapName);
		}

		if (ensureVoxelSlow(GEngine) &&
			ensureVoxelSlow(GEngine->GameViewport))
		{
			FEngineShowFlags* EngineShowFlags = GEngine->GameViewport->GetEngineShowFlags();
			if (ensureVoxelSlow(EngineShowFlags))
			{
				EngineShowFlags->SetCloud(false);
			}
		}

		check(!bIsWaitingForTests);
		bIsWaitingForTests = true;
	}));
}