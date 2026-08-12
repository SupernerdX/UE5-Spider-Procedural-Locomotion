// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkLoader.h"
#include "Sculpt/ENET/VoxelNetworkClient.h"
#include "Sculpt/Volume/VoxelSculptVolumeDataSource.h"

class FVoxelSculptVolumeDiff;
class IVoxelVolumeModifierRuntime;
class FVoxelSculptVolumeNetworkClient;
struct FVoxelVolumeModifier;
struct FVoxelSculptVolumeServerToClientMessage_ApplyModifier;

class FVoxelSculptVolumeNetworkClientBulkLoader : public IVoxelBulkLoader
{
public:
	TWeakPtr<FVoxelSculptVolumeNetworkClient> WeakClient;

	FVoxelCriticalSection CriticalSection;
	struct FChunkData
	{
		FVoxelBulkHash Hash;
		FVoxelBulkHint Hint;
	};
	TVoxelArray<FChunkData> ChunksToRequest_RequiresLock;
	TVoxelMap<FVoxelBulkHash, TVoxelPromise<TSharedPtr<const TVoxelArray64<uint8>>>> HashToPromise_RequiresLock;

	FVoxelSculptVolumeNetworkClientBulkLoader() = default;
	virtual ~FVoxelSculptVolumeNetworkClientBulkLoader() override;

	//~ Begin IVoxelBulkLoader Interface
	virtual TVoxelFuture<TSharedPtr<const TVoxelArray64<uint8>>> LoadBulkDataImpl(const FVoxelBulkHash& Hash, const FVoxelBulkHint& Hint) override;
	virtual TSharedPtr<const TVoxelArray64<uint8>> LoadBulkDataSyncImpl(const FVoxelBulkHash& Hash) override;
	//~ End IVoxelBulkLoader Interface
};

class FVoxelSculptVolumeNetworkClient
	: public IVoxelNetworkClient
	, public IVoxelSculptVolumeDataSource
{
public:
	explicit FVoxelSculptVolumeNetworkClient(AVoxelSculptVolume& SculptActor);
	virtual ~FVoxelSculptVolumeNetworkClient() override;

	//~ Begin IVoxelNetworkClient Interface
	virtual void OnRegistered() override;
	virtual void OnMessageReceived(const TSharedRef<FVoxelNetworkServerToClientMessage>& Message) override;
	//~ End IVoxelNetworkClient Interface

	//~ Begin IVoxelSculptVolumeDataSource Interface
	virtual TSharedRef<IVoxelBulkLoader> GetBulkLoader() const override;
	virtual FVoxelFuture ApplyModifier(const TSharedRef<FVoxelVolumeModifier>& Modifier) override;

	virtual void SetSculptData(
		const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData,
		const TSharedRef<IVoxelBulkLoader>& NewBulkLoader) override;
	//~ End IVoxelSculptVolumeDataSource Interface

	void ProcessBulkRequests();

private:
	const TSharedRef<FVoxelSculptVolumeNetworkClientBulkLoader> BulkLoader = MakeShared<FVoxelSculptVolumeNetworkClientBulkLoader>();

	TVoxelBulkPtr<FVoxelSculptVolumeData> ServerData;

	struct FPredictedData
	{
		FGuid ModifierGuid;
		TSharedPtr<const FVoxelSculptVolumeDiff> Diff;
		TVoxelBulkRef<FVoxelSculptVolumeData> NewData;
	};
	TVoxelArray<FPredictedData> PredictedDatas;

	struct FPendingServerModifier
	{
		double QueueTime = 0;
		double StartTime = 0;
		FGuid ModifierGuid;
		FVoxelBulkHash ExpectedHash;
		TVoxelFuture<const FVoxelSculptVolumeData> Future;
	};
	TVoxelOptional<FPendingServerModifier> PendingServerModifier;

	struct FPendingPredictedModifier
	{
		double QueueTime = 0;
		double StartTime = 0;
		FGuid ModifierGuid;
		TVoxelBulkRef<FVoxelSculptVolumeData> OldData;
		FVoxelPromise Promise;
		TVoxelFuture<const FVoxelSculptVolumeData> Future;
	};
	TVoxelOptional<FPendingPredictedModifier> PendingPredictedModifier;

	struct FQueuedServerModifier
	{
		double QueueTime = 0;
		TSharedPtr<const FVoxelSculptVolumeServerToClientMessage_ApplyModifier> Message;
	};
	TVoxelArray<FQueuedServerModifier> QueuedServerModifiers;

	struct FPredictedModifier
	{
		double QueueTime = 0;
		FGuid ModifierGuid;
		FVoxelPromise Promise;
		TSharedPtr<IVoxelVolumeModifierRuntime> ModifierRuntime;
	};
	TVoxelArray<TSharedPtr<FPredictedModifier>> QueuedPredictedModifiers;

private:
	void UpdateData();
	void ClearQueue();
	void ProcessQueue();
	bool ProcessQueue_ShouldContinue();
	void ApplyServerModifier(const FQueuedServerModifier& Modifier);
};