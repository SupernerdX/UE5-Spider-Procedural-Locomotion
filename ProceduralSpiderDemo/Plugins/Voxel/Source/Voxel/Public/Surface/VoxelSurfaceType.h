// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSurfaceType.generated.h"

class UVoxelSurfaceTypeAsset;
class UVoxelSmartSurfaceType;
class UVoxelSurfaceTypeInterface;
class UVoxelCubicSurfaceTypeAsset;
struct FVoxelSurfaceTypeImpl;

template<typename T>
struct TVoxelSurfaceTypeImpl;

#if !UE_BUILD_SHIPPING
extern VOXEL_API TVoxelArray<FVoxelObjectPtr> GVoxelDebugSurfaceTypes;
#endif

USTRUCT()
struct VOXEL_API FVoxelSurfaceType
{
	GENERATED_BODY()

public:
	enum class EClass : uint8
	{
		SurfaceTypeAsset,
		SmartSurfaceType,
		CubicSurfaceType
	};

public:
	FVoxelSurfaceType() = default;
	explicit FVoxelSurfaceType(UVoxelSurfaceTypeInterface* SurfaceTypeInterface);

private:
	TVoxelSurfaceTypeImpl<UVoxelSurfaceTypeAsset>& GetSurfaceTypeAssetImpl() const;
	TVoxelSurfaceTypeImpl<UVoxelSmartSurfaceType>& GetSmartSurfaceTypeImpl() const;
	TVoxelSurfaceTypeImpl<UVoxelCubicSurfaceTypeAsset>& GetCubicSurfaceTypeImpl() const;

public:
	TVoxelObjectPtr<UVoxelSurfaceTypeAsset> GetSurfaceTypeAsset() const;
	TVoxelObjectPtr<UVoxelSmartSurfaceType> GetSmartSurfaceType() const;
	TVoxelObjectPtr<UVoxelCubicSurfaceTypeAsset> GetCubicSurfaceTypeAsset() const;
	TVoxelObjectPtr<UVoxelSurfaceTypeInterface> GetSurfaceTypeInterface() const;

	FName GetFName() const;
	FString GetName() const;
	FLinearColor GetDebugColor() const;
	FGuid GetGuid() const;

	friend void operator<<(FArchive& Ar, FVoxelSurfaceType& SurfaceType);

public:
	void UpdateFromSourceObject() const;

public:
	static void ForeachSurfaceType(TFunctionRef<void(FVoxelSurfaceType)> Lambda);
	static bool FindFromGuid(
		FGuid Guid,
		FVoxelSurfaceType& OutSurfaceType);

public:
	FORCEINLINE bool IsNull() const
	{
		return InternalIndex == 0;
	}
	FORCEINLINE EClass GetClass() const
	{
		return EClass(InternalType);
	}

public:
	FORCEINLINE bool operator==(const FVoxelSurfaceType& Other) const
	{
		return RawValue == Other.RawValue;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelSurfaceType& Index)
	{
		return Index.RawValue;
	}

	FORCEINLINE bool operator<(const FVoxelSurfaceType& Other) const
	{
		return RawValue < Other.RawValue;
	}
	FORCEINLINE bool operator>(const FVoxelSurfaceType& Other) const
	{
		return RawValue > Other.RawValue;
	}

private:
	static constexpr int32 MaxIndex = WITH_EDITOR ? (1 << 14) : (1 << 10);

	union
	{
		uint16 RawValue = 0;

		struct
		{
			uint16 InternalType : 2;
			uint16 InternalIndex : 14;
		};
	};

#if !CPP
	UPROPERTY()
	uint16 RawValue = 0;
#endif

	friend class FVoxelSurfaceTypeManager;
};
checkStatic(sizeof(FVoxelSurfaceType) == sizeof(uint16));