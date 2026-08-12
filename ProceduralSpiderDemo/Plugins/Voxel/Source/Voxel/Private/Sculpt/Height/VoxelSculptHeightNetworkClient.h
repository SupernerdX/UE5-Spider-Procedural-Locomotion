// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkHint.h"
#include "Bulk/VoxelBulkLoader.h"
#include "Sculpt/ENET/VoxelNetworkClient.h"
#include "Sculpt/Height/VoxelSculptHeightDataSource.h"

class FVoxelSculptHeightDiff;
class IVoxelHeightModifierRuntime;
class FVoxelSculptHeightNetworkClient;
struct FVoxelHeightModifier;
struct FVoxelSculptHeightServerToClientMessage_ApplyModifier;

class FVoxelSculptHeightNetworkClientBulkLoader : public IVoxelBulkLoader
{
public:
	TWeakPtr<FVoxelSculptHeightNetworkClient> WeakClient;

	FVoxelCriticalSection CriticalSection;
	struct FChunkData
	{
		FVoxelBulkHash Hash;
		FVoxelBulkHint Hint;
	};
	TVoxelArray<FChunkData> ChunksToRequest_RequiresLock;
	TVoxelMap<FVoxelBulkHash, TVoxelPromise<TSharedPtr<const TVoxelArray64<uint8>>>> HashToPromise_RequiresLock;

	FVoxelSculptHeightNetworkClientBulkLoader() = default;
	virtual ~FVoxelSculptHeightNetworkClientBulkLoader() override;

	//~ Begin IVoxelBulkLoader Interface
	virtual TVoxelFuture<TSharedPtr<const TVoxelArray64<uint8>>> LoadBulkDataImpl(const FVoxelBulkHash& Hash, const FVoxelBulkHint& Hint) override;
	virtual TSharedPtr<const TVoxelArray64<uint8>> LoadBulkDataSyncImpl(const FVoxelBulkHash& Hash) override;
	//~ End IVoxelBulkLoader Interface
};

class FVoxelSculptHeightNetworkClient
	: public IVoxelNetworkClient
	, public IVoxelSculptHeightDataSource
{
public:
	explicit FVoxelSculptHeightNetworkClient(AVoxelSculptHeight& SculptActor);
	virtual ~FVoxelSculptHeightNetworkClient() override;

	//~ Begin IVoxelNetworkClient Interface
	virtual void OnRegistered() override;
	virtual void OnMessageReceived(const TSharedRef<FVoxelNetworkServerToClientMessage>& Message) override;
	//~ End IVoxelNetworkClient Interface

	//~ Begin IVoxelSculptHeightDataSource Interface
	virtual TSharedRef<IVoxelBulkLoader> GetBulkLoader() const override;
	virtual FVoxelFuture ApplyModifier(const TSharedRef<FVoxelHeightModifier>& Modifier) override;

	virtual void SetSculptData(
		const TVoxelBulkRef<FVoxelSculptHeightData>& NewData,
		const TSharedRef<IVoxelBulkLoader>& NewBulkLoader) override;
	//~ End IVoxelSculptHeightDataSource Interface

	void ProcessBulkRequests();

private:
	const TSharedRef<FVoxelSculptHeightNetworkClientBulkLoader> BulkLoader = MakeShared<FVoxelSculptHeightNetworkClientBulkLoader>();

	TVoxelBulkPtr<FVoxelSculptHeightData> ServerData;

	struct FPredictedData
	{
		FGuid ModifierGuid;
		TSharedPtr<const FVoxelSculptHeightDiff> Diff;
		TVoxelBulkRef<FVoxelSculptHeightData> NewData;
	};
	TVoxelArray<FPredictedData> PredictedDatas;

	struct FPendingServerModifier
	{
		double QueueTime = 0;
		double StartTime = 0;
		FGuid ModifierGuid;
		FVoxelBulkHash ExpectedHash;
		TVoxelFuture<const FVoxelSculptHeightData> Future;
	};
	TVoxelOptional<FPendingServerModifier> PendingServerModifier;

	struct FPendingPredictedModifier
	{
		double QueueTime = 0;
		double StartTime = 0;
		FGuid ModifierGuid;
		TVoxelBulkRef<FVoxelSculptHeightData> OldData;
		FVoxelPromise Promise;
		TVoxelFuture<const FVoxelSculptHeightData> Future;
	};
	TVoxelOptional<FPendingPredictedModifier> PendingPredictedModifier;

	struct FQueuedServerModifier
	{
		double QueueTime = 0;
		TSharedPtr<const FVoxelSculptHeightServerToClientMessage_ApplyModifier> Message;
	};
	TVoxelArray<FQueuedServerModifier> QueuedServerModifiers;

	struct FPredictedModifier
	{
		double QueueTime = 0;
		FGuid ModifierGuid;
		FVoxelPromise Promise;
		TSharedPtr<IVoxelHeightModifierRuntime> ModifierRuntime;
	};
	TVoxelArray<TSharedPtr<FPredictedModifier>> QueuedPredictedModifiers;

private:
	void UpdateData();
	void ClearQueue();
	void ProcessQueue();
	bool ProcessQueue_ShouldContinue();
	void ApplyServerModifier(const FQueuedServerModifier& Modifier);
};