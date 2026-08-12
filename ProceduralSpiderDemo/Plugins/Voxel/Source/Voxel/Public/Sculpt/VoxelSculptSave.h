// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSculptSave.generated.h"

struct VOXEL_API FVoxelSculptSaveBase
{
public:
	bool IsValid() const
	{
		return Data != nullptr;
	}
	bool IsCompressed() const
	{
		return Data->bIsCompressed;
	}
	int64 GetSize() const
	{
		return Data->Data.Num();
	}

	//~ Begin TStructOpsTypeTraits Interface
	bool Serialize(FArchive& Ar);
	bool Identical(const FVoxelSculptSaveBase* Other, uint32 PortFlags) const;
	//~ End TStructOpsTypeTraits Interface

protected:
	struct FData
	{
		bool bIsCompressed = false;
		TVoxelArray64<uint8> Data;
	};
	TSharedPtr<FData> Data;
};

template<typename T>
requires std::derived_from<T, FVoxelSculptSaveBase>
struct TStructOpsTypeTraits<T> : TStructOpsTypeTraitsBase2<T>
{
	enum
	{
		WithSerializer = true,
		WithIdentical = true,
	};
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelHeightSculptSave
#if CPP
	: public FVoxelSculptSaveBase
#endif
{
	GENERATED_BODY()

	friend class AVoxelSculptHeight;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelVolumeSculptSave
#if CPP
	: public FVoxelSculptSaveBase
#endif
{
	GENERATED_BODY()

	friend class AVoxelSculptVolume;
};