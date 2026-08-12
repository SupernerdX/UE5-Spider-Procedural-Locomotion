// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNetworkMessage.h"
#include "Bulk/VoxelBulkHash.h"

TSharedPtr<FVoxelNetworkMessage> FVoxelNetworkMessage::FromBytes(
	const TConstVoxelArrayView<uint8> Bytes,
	TVoxelMap<int32, const UScriptStruct*>& IndexToStruct)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelNetworkMessageHeader Header;
	FMemory::Memzero(Header);

	FVoxelReader Reader(Bytes);
	Reader << ReinterpretCastRef<uint8>(Header);

	TVoxelOptional<FString> StructName;
	TVoxelOptional<FVoxelBulkHash> Hash;

	if (Header.bHasStructName)
	{
		Reader << StructName.Emplace();
	}
	if (Header.bHasHash)
	{
		Reader << Hash.Emplace();
	}

	if (!ensure(!Reader.HasError()))
	{
		return {};
	}

	if (Header.bHasStructName)
	{
		static const TVoxelMap<FName, const UScriptStruct*> NameToStruct = INLINE_LAMBDA
		{
			TVoxelMap<FName, const UScriptStruct*> Result;
			Result.Reserve(32);

			for (const UScriptStruct* Struct : GetDerivedStructs<FVoxelNetworkMessage>())
			{
				Result.Add_EnsureNew(Struct->GetFName(), Struct);
			}

			return Result;
		};

		const UScriptStruct* Struct = NameToStruct.FindRef(FName(StructName.GetValue()));
		if (!ensure(Struct))
		{
			LOG_VOXEL(Error, "Failed to find networkmessage struct %s", *StructName.GetValue());
			return {};
		}

		IndexToStruct.Add_EnsureNew(Header.Index, Struct);
	}

	const UScriptStruct* Struct = IndexToStruct.FindRef(Header.Index);
	if (!ensure(Struct))
	{
		LOG_VOXEL(Error, "Failed to find network message struct for index %d", Header.Index);
		return {};
	}

	const TConstVoxelArrayView<uint8> MessageData = Bytes.RightOf(Reader.Tell());

	if (Hash)
	{
		const FVoxelBulkHash ActualHash = FVoxelBulkHash::Create(MessageData);

		if (!ensure(Hash == ActualHash))
		{
			LOG_VOXEL(Error, "Message hash mismatch: expected %s, got %s", *Hash->ToString(), *ActualHash.ToString());
			return {};
		}
	}

	const TSharedRef<FVoxelNetworkMessage> Message = MakeSharedStruct<FVoxelNetworkMessage>(Struct);

	FVoxelReader MessageReader(MessageData);
	Message->Serialize(MessageReader);

	if (!ensure(MessageReader.IsAtEndWithoutError()))
	{
		LOG_VOXEL(Error, "Message failed to serialize: %s", *Struct->GetName());
		return {};
	}

	return Message;
}

TVoxelArray<uint8> FVoxelNetworkMessage::ToBytes(TVoxelMap<const UScriptStruct*, int32>& StructToIndex) const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelNetworkMessageHeader Header;
	FMemory::Memzero(Header);

	if (!StructToIndex.Contains(GetStruct()))
	{
		const int32 Index = StructToIndex.Num();

		if (VOXEL_DEBUG)
		{
			for (auto& It : StructToIndex)
			{
				check(It.Value != Index);
			}
		}

		StructToIndex.Add_EnsureNew(GetStruct(), Index);

		Header.bHasStructName = true;
	}

	Header.Index = FVoxelUtilities::CheckBits<6>(StructToIndex[GetStruct()]);
	Header.bHasHash = true;

	FVoxelWriter MessageWriter;
	ConstCast(*this).Serialize(MessageWriter);

	FVoxelWriter Writer;
	Writer << ReinterpretCastRef<uint8>(Header);

	if (Header.bHasStructName)
	{
		Writer << GetStruct()->GetName();
	}
	if (Header.bHasHash)
	{
		Writer << FVoxelBulkHash::Create(MessageWriter.Bytes);
	}

	Writer.Serialize(MessageWriter.Bytes.GetData(), MessageWriter.Bytes.Num());

	return TVoxelArray<uint8>(MoveTemp(Writer.Bytes));
}