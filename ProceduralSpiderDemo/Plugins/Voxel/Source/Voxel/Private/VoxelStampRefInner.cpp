// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampRefInner.h"
#include "VoxelStamp.h"
#include "VoxelStampManager.h"
#include "Net/RepLayout.h"
#include "Engine/NetConnection.h"
#include "Engine/PackageMapClient.h"
#include "UObject/CoreRedirects.h"

void FVoxelStampRefInner::Load(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	int32 Version = 0;
	Ar << Version;
	check(Version == FVersion::LatestVersion);

	int32 SerializedSize = 0;
	Ar << SerializedSize;

	const int64 SeekPosition = Ar.Tell() + SerializedSize;
	ON_SCOPE_EXIT
	{
		Ar.Seek(SeekPosition);
	};

	if (SerializedSize == 0)
	{
		return;
	}

	FString StructPath;
	Ar << StructPath;

	UScriptStruct* Struct = nullptr;
	Ar << Struct;

	if (!Struct)
	{
		VOXEL_SCOPE_COUNTER("FindObject");

		// Serializing structs directly doesn't seem to handle redirects properly
		const FCoreRedirectObjectName RedirectedName = FCoreRedirects::GetRedirectedName(
			ECoreRedirectFlags::Type_Struct,
			FCoreRedirectObjectName(StructPath));

		Struct = FindObject<UScriptStruct>(nullptr, *RedirectedName.ToString());
	}

	if (!ensureVoxelSlow(Struct))
	{
		LOG_VOXEL(Warning, "FVoxelStampRef: Failed to find struct %s. Archive: %s Callstack: \n%s",
			*StructPath,
			*FVoxelUtilities::GetArchivePath(Ar),
			*FVoxelUtilities::GetPrettyCallstack_WithStats());

		return;
	}

	{
		VOXEL_SCOPE_COUNTER("Preload");
		Ar.Preload(Struct);
	}

	// Reuse stamps as much as possible to avoid issues when undoing
	if (!Stamp ||
		Stamp->GetStruct() != Struct)
	{
#if WITH_EDITOR
		if (Stamp)
		{
			// Back up the old struct
			StructToStamp_Editor.FindOrAdd(Stamp->GetStruct()) = Stamp;
		}
#endif

		Stamp = MakeSharedStruct<FVoxelStamp>(Struct);
	}

	{
		VOXEL_SCOPE_COUNTER("SerializeItem");
		VOXEL_SCOPE_COUNTER_FNAME(Stamp->GetStruct()->GetFName());

		Stamp->GetStruct()->SerializeItem(Ar, Stamp.Get(), nullptr);
	}

	ensure(Ar.Tell() == SeekPosition);
}

void FVoxelStampRefInner::Save(FArchive& Ar) const
{
	VOXEL_FUNCTION_COUNTER();

	int32 Version = FVersion::LatestVersion;
	Ar << Version;

	const int64 SerializedSizePosition = Ar.Tell();
	{
		int32 SerializedSize = 0;
		Ar << SerializedSize;
	}
	const int64 Start = Ar.Tell();

	ON_SCOPE_EXIT
	{
		const int64 End = Ar.Tell();

		Ar.Seek(SerializedSizePosition);
		{
			int32 SerializedSize = End - Start;
			Ar << SerializedSize;
		}
		Ar.Seek(End);
	};

	if (!Stamp)
	{
		return;
	}

	FString StructPath = Stamp->GetStruct()->GetPathName();
	Ar << StructPath;

	UScriptStruct* Struct = Stamp->GetStruct();
	Ar << Struct;

	Stamp->GetStruct()->SerializeItem(Ar, Stamp.Get(), nullptr);
}

void FVoxelStampRefInner::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint8 bValidData = Ar.IsSaving() ? Stamp != nullptr : 0;
	Ar.SerializeBits(&bValidData, 1);

	if (!bValidData)
	{
		if (Ar.IsLoading())
		{
			Stamp = {};
		}

		bOutSuccess = true;
		return;
	}

	if (Ar.IsLoading())
	{
		UScriptStruct* NewScriptStruct = nullptr;
		Ar << NewScriptStruct;

		if (!Stamp ||
			Stamp->GetStruct() != NewScriptStruct)
		{
			Stamp = MakeSharedStruct<FVoxelStamp>(NewScriptStruct);
		}
	}
	else
	{
		UScriptStruct* Struct = Stamp ? Stamp->GetStruct() : nullptr;
		Ar << Struct;
	}

	if (!Stamp)
	{
		return;
	}

	if (EnumHasAllFlags(Stamp->GetStruct()->StructFlags, STRUCT_NetSerializeNative))
	{
		Stamp->GetStruct()->GetCppStructOps()->NetSerialize(Ar, Map, bOutSuccess, Stamp.Get());
	}
	else
	{
		bOutSuccess = INLINE_LAMBDA
		{
			UPackageMapClient* MapClient = Cast<UPackageMapClient>(Map);
			check(::IsValid(MapClient));

			UNetConnection* NetConnection = MapClient->GetConnection();
			check(::IsValid(NetConnection));
			check(::IsValid(NetConnection->GetDriver()));

			const TSharedPtr<FRepLayout> RepLayout = NetConnection->GetDriver()->GetStructRepLayout(Stamp->GetStruct());
			check(RepLayout.IsValid());

			bool bHasUnmapped = false;
			RepLayout->SerializePropertiesForStruct(Stamp->GetStruct(), static_cast<FBitArchive&>(Ar), Map, Stamp.Get(), bHasUnmapped);
			return true;
		};
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStampRefInner::~FVoxelStampRefInner()
{
	if (!Index.IsValid())
	{
		return;
	}

	const TSharedPtr<FVoxelStampManager> StampManager = Index.GetWeakStampManager().Pin();
	if (!ensureVoxelSlow(StampManager))
	{
		return;
	}

	FVoxelInvalidationScope Scope("FVoxelStampRefInner::~FVoxelStampRefInner");

	StampManager->UnregisterStamps(MakeVoxelArrayView(Index));
}