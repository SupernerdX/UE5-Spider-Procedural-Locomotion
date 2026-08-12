// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

DECLARE_UNIQUE_VOXEL_ID(FVoxelTestId);

enum class EVoxelTestState : uint8
{
	Started,
	Succeeded,
	Failed
};

FORCEINLINE FString LexToString(const EVoxelTestState State)
{
	switch (State)
	{
	default: ensure(false);
	case EVoxelTestState::Started: return "Started";
	case EVoxelTestState::Succeeded: return "Succeeded";
	case EVoxelTestState::Failed: return "Failed";
	}
}

class VOXELTESTS_API FVoxelTestManager : public FVoxelSingleton
{
public:
	bool bIsRunningTests = false;

	struct FTestData
	{
		FString Name;
		EVoxelTestState State = {};
		FString FailureReason;
	};
	struct FMapData
	{
		FString MapName;
		TVoxelMap<FVoxelTestId, FTestData> IdToData;
		TVoxelSet<FGuid> ScreenshotIds;
	};
	FMapData MapData;

public:
	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override;
	virtual void Tick() override;
	//~ End FVoxelSingleton Interface

public:
	TDelegate<void(const TVoxelArray<FMapData>&)> OnComplete;

	void StartTests();

private:
	bool bIsWaitingForTests = false;
	TVoxelArray<FString> MapsToTest;
	TVoxelArray<FMapData> MapDatas;

	void GoToNextTestMap();
};
extern FVoxelTestManager* GVoxelTestManager;