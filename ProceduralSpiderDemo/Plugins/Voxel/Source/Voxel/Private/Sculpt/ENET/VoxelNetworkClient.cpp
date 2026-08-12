// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNetworkClient.h"
#include "VoxelChannel.h"
#include "Sculpt/ENET/VoxelNetworkLog.h"
#include "Sculpt/ENET/VoxelNetworkMessage.h"

DEFINE_VOXEL_INSTANCE_COUNTER(IVoxelNetworkClient);

void IVoxelNetworkClient::Register(
	const UWorld& World,
	const FName NetworkId)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedPtr<FVoxelNetworkManager> Manager = FVoxelNetworkManager::Get(World);
	if (!ensure(Manager))
	{
		return;
	}

	ensure(PrivateNetworkId.IsNone());
	PrivateNetworkId = NetworkId;

	Manager->RegisterClient(AsShared());

	OnRegistered();
}

void IVoxelNetworkClient::SendMessage(const TSharedRef<const FVoxelNetworkClientToServerMessage>& Message)
{
	if (!ensure(SendMessageBytes))
	{
		return;
	}

	LOG_VOXEL_NET(Verbose, "IVoxelNetworkClient: %s: Sent %s", *GetNetworkId().ToString(), *Message->GetStruct()->GetName());

	SendMessageBytes(Message->ToBytes(ClientToServerMessage_StructToIndex));
}

void IVoxelNetworkClient::OnMessageBytesReceived(const TConstVoxelArrayView<uint8> Bytes)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedPtr<FVoxelNetworkMessage> Message = FVoxelNetworkMessage::FromBytes(Bytes, ServerToClientMessage_IndexToStruct);
	if (!ensureVoxelSlow(Message) ||
		!ensure(Message->IsA<FVoxelNetworkServerToClientMessage>()))
	{
		return;
	}

	LOG_VOXEL_NET(Verbose, "IVoxelNetworkClient: %s: Received %s", *GetNetworkId().ToString(), *Message->GetStruct()->GetName());

	OnMessageReceived(CastStructChecked<FVoxelNetworkServerToClientMessage>(Message.ToSharedRef()));
}