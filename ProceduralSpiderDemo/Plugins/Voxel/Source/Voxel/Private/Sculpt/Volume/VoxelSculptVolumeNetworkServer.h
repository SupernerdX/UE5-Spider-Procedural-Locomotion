// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/ENET/VoxelNetworkServer.h"
#include "Sculpt/Volume/VoxelSculptVolumeDataSource.h"

class IVoxelVolumeModifierRuntime;
struct FVoxelVolumeModifier;

class FVoxelSculptVolumeNetworkServer
	: public IVoxelNetworkServer
	, public IVoxelSculptVolumeDataSource
	, public FVoxelTicker
{
public:
	explicit FVoxelSculptVolumeNetworkServer(AVoxelSculptVolume& SculptActor);
	virtual ~FVoxelSculptVolumeNetworkServer() override;

public:
	FVoxelFuture ApplyModifier(
		FGuid ModifierGuid,
		const TSharedRef<FVoxelVolumeModifier>& Modifier);

	//~ Begin IVoxelSculptVolumeDataSource Interface
	virtual TSharedRef<IVoxelBulkLoader> GetBulkLoader() const override;
	virtual FVoxelFuture ApplyModifier(const TSharedRef<FVoxelVolumeModifier>& Modifier) override;

	virtual void SetSculptData(
		const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData,
		const TSharedRef<IVoxelBulkLoader>& NewBulkLoader) override;

	virtual void SetData(const TVoxelBulkRef<FVoxelSculptVolumeData>& Data) override;
	//~ End IVoxelSculptVolumeDataSource Interface

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
		TVoxelFuture<const FVoxelSculptVolumeData> Future;
	};
	TVoxelOptional<FPendingModifier> PendingModifier;

	struct FQueuedModifier
	{
		double QueueTime = 0;
		FGuid ModifierGuid;
		TVoxelArray<uint8> ModifierData;
		FVoxelPromise Promise;
		TSharedPtr<IVoxelVolumeModifierRuntime> ModifierRuntime;
	};
	TVoxelArray<FQueuedModifier> QueuedModifiers;

	struct FOldBulkData
	{
		TVoxelBulkRef<FVoxelSculptVolumeData> Data;
		double RemoveAt = 0.;
	};
	TVoxelMap<FVoxelBulkHash, FOldBulkData> HashToOldBulkData;

	void ClearQueue();
	void ProcessQueue();
	bool ProcessQueue_ShouldContinue();

	TVoxelBulkPtr<FVoxelSculptVolumeData> FindSculptData(const FVoxelBulkHash& RootHash) const;

	friend class FVoxelSculptVolumeNetworkRemoteClient;
};

class FVoxelSculptVolumeNetworkRemoteClient : public IVoxelNetworkRemoteClient
{
public:
	const TSharedRef<FVoxelSculptVolumeNetworkServer> Server;

	explicit FVoxelSculptVolumeNetworkRemoteClient(const TSharedRef<FVoxelSculptVolumeNetworkServer>& Server);

public:
	//~ Begin IVoxelNetworkRemoteClient Interface
	virtual void OnConnect() override;
	virtual void OnMessageReceived(const TSharedRef<FVoxelNetworkClientToServerMessage>& Message) override;
	//~ End IVoxelNetworkRemoteClient Interface
};