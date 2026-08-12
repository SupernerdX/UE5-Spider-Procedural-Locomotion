// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/ENET/VoxelNetworkServer.h"
#include "Sculpt/Height/VoxelSculptHeightDataSource.h"

class IVoxelHeightModifierRuntime;
struct FVoxelHeightModifier;

class FVoxelSculptHeightNetworkServer
	: public IVoxelNetworkServer
	, public IVoxelSculptHeightDataSource
	, public FVoxelTicker
{
public:
	explicit FVoxelSculptHeightNetworkServer(AVoxelSculptHeight& SculptActor);
	virtual ~FVoxelSculptHeightNetworkServer() override;

public:
	FVoxelFuture ApplyModifier(
		FGuid ModifierGuid,
		const TSharedRef<FVoxelHeightModifier>& Modifier);

	//~ Begin IVoxelSculptHeightDataSource Interface
	virtual TSharedRef<IVoxelBulkLoader> GetBulkLoader() const override;
	virtual FVoxelFuture ApplyModifier(const TSharedRef<FVoxelHeightModifier>& Modifier) override;

	virtual void SetSculptData(
		const TVoxelBulkRef<FVoxelSculptHeightData>& NewData,
		const TSharedRef<IVoxelBulkLoader>& NewBulkLoader) override;

	virtual void SetData(const TVoxelBulkRef<FVoxelSculptHeightData>& Data) override;
	//~ End IVoxelSculptHeightDataSource Interface

	//~ Begin IVoxelNetworkServer Interface
	virtual TSharedRef<IVoxelNetworkRemoteClient> CreateClientImpl() override;
	//~ End IVoxelNetworkServer Interface

	//~ Begin FVoxelTicker Interface
	virtual void Tick() override;
	//~ End FVoxelTicker Interface

private:
	TSharedRef<IVoxelBulkLoader> BulkLoader;

	struct FPendingModifier
	{
		double QueueTime = 0;
		double StartTime = 0;
		FGuid ModifierGuid;
		TVoxelArray<uint8> ModifierData;
		FVoxelPromise Promise;
		TVoxelFuture<const FVoxelSculptHeightData> Future;
	};
	TVoxelOptional<FPendingModifier> PendingModifier;

	struct FQueuedModifier
	{
		double QueueTime = 0;
		FGuid ModifierGuid;
		TVoxelArray<uint8> ModifierData;
		FVoxelPromise Promise;
		TSharedPtr<IVoxelHeightModifierRuntime> ModifierRuntime;
	};
	TVoxelArray<FQueuedModifier> QueuedModifiers;

	struct FOldBulkData
	{
		TVoxelBulkRef<FVoxelSculptHeightData> Data;
		double RemoveAt = 0.;
	};
	TVoxelMap<FVoxelBulkHash, FOldBulkData> HashToOldBulkData;

	void ClearQueue();
	void ProcessQueue();
	bool ProcessQueue_ShouldContinue();

	TVoxelBulkPtr<FVoxelSculptHeightData> FindSculptData(const FVoxelBulkHash& RootHash) const;

	friend class FVoxelSculptHeightNetworkRemoteClient;
};

class FVoxelSculptHeightNetworkRemoteClient : public IVoxelNetworkRemoteClient
{
public:
	const TSharedRef<FVoxelSculptHeightNetworkServer> Server;

	explicit FVoxelSculptHeightNetworkRemoteClient(const TSharedRef<FVoxelSculptHeightNetworkServer>& Server);

public:
	//~ Begin IVoxelNetworkRemoteClient Interface
	virtual void OnConnect() override;
	virtual void OnMessageReceived(const TSharedRef<FVoxelNetworkClientToServerMessage>& Message) override;
	//~ End IVoxelNetworkRemoteClient Interface
};
