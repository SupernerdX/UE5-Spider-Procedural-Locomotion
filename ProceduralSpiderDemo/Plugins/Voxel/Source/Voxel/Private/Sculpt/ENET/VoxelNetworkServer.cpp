// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNetworkServer.h"
#include "VoxelChannel.h"
#include "Sculpt/ENET/VoxelNetworkLog.h"
#include "Sculpt/ENET/VoxelNetworkMessage.h"

DEFINE_VOXEL_INSTANCE_COUNTER(IVoxelNetworkServer);
DEFINE_VOXEL_INSTANCE_COUNTER(IVoxelNetworkRemoteClient);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IVoxelNetworkServer::~IVoxelNetworkServer()
{
	ensureMsgf(PrivateNetworkId.IsNone(), TEXT("IVoxelNetworkServer not unregistered: %s"), *PrivateNetworkId.ToString());
}

void IVoxelNetworkServer::Register(
	const UWorld& World,
	const FName NetworkId)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(!NetworkId.IsNone());

	const TSharedPtr<FVoxelNetworkManager> Manager = FVoxelNetworkManager::Get(World);
	if (!ensure(Manager))
	{
		return;
	}

	ensure(PrivateNetworkId.IsNone());
	PrivateNetworkId = NetworkId;

	Manager->RegisterServer(AsShared());
}

void IVoxelNetworkServer::Unregister()
{
	VOXEL_FUNCTION_COUNTER();

	ensure(!PrivateNetworkId.IsNone());
	OnUnregister.Broadcast();
	PrivateNetworkId = {};
}

TSharedRef<IVoxelNetworkRemoteClient> IVoxelNetworkServer::CreateClient()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	ensure(!PrivateNetworkId.IsNone());

	const TSharedRef<IVoxelNetworkRemoteClient> Client = CreateClientImpl();
	Client->PrivateNetworkId = PrivateNetworkId;
	WeakClients.Add(Client);
	return Client;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void IVoxelNetworkServer::ForeachClient(const TVoxelFunctionRef<void(IVoxelNetworkRemoteClient& Client)> Lambda)
{
	for (auto It = WeakClients.CreateIterator(); It; ++It)
	{
		const TSharedPtr<IVoxelNetworkRemoteClient> Client = It->Pin();
		if (!Client)
		{
			It.RemoveCurrentSwap();
			continue;
		}

		Lambda(*Client);
	}
}

void IVoxelNetworkServer::BroadcastMessage(const TSharedRef<const FVoxelNetworkServerToClientMessage>& Message)
{
	VOXEL_FUNCTION_COUNTER();

	ForeachClient([&](IVoxelNetworkRemoteClient& Client)
	{
		Client.SendMessage(Message);
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void IVoxelNetworkRemoteClient::SendMessage(const TSharedRef<const FVoxelNetworkServerToClientMessage>& Message)
{
	if (!ensure(SendMessageBytes))
	{
		return;
	}

	LOG_VOXEL_NET(Verbose, "IVoxelNetworkRemoteClient: %s: Sent %s", *GetNetworkId().ToString(), *Message->GetStruct()->GetName());

	SendMessageBytes(Message->ToBytes(ServerToClientMessage_StructToIndex));
}

void IVoxelNetworkRemoteClient::OnMessageBytesReceived(const TConstVoxelArrayView<uint8> Bytes)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedPtr<FVoxelNetworkMessage> Message = FVoxelNetworkMessage::FromBytes(Bytes, ClientToServerMessage_IndexToStruct);
	if (!ensureVoxelSlow(Message) ||
		!ensure(Message->IsA<FVoxelNetworkClientToServerMessage>()))
	{
		return;
	}

	LOG_VOXEL_NET(Verbose, "IVoxelNetworkRemoteClient: %s: Received %s", *GetNetworkId().ToString(), *Message->GetStruct()->GetName());

	OnMessageReceived(CastStructChecked<FVoxelNetworkClientToServerMessage>(Message.ToSharedRef()));
}