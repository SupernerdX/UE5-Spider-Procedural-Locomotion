// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinType.h"
#include "VoxelAssetIcon.h"
#include "VoxelMetadataRef.h"
#include "VoxelMetadataMaterialType.h"
#include "VoxelMetadata.generated.h"

struct FVoxelMetadataRefStatics
{
	void (*Blend)(
		const FVoxelBuffer& Value,
		const FVoxelFloatBuffer& Alpha,
		FVoxelBuffer& InOutResult) = nullptr;

	void (*IndirectBlend)(
		TConstVoxelArrayView<int32> IndexToResult,
		const FVoxelBuffer& Value,
		const FVoxelFloatBuffer& Alpha,
		FVoxelBuffer& InOutResult) = nullptr;

	void (*AddToPCG)(
		UPCGMetadata& PCGMetadata,
		TConstVoxelArrayView<FPCGPoint> Points,
		FName Name,
		const FVoxelBuffer& Values) = nullptr;

	void (*WriteMaterialData)(
		const FVoxelBuffer& Values,
		TVoxelArrayView<uint8> OutBytes,
		EVoxelMetadataMaterialType MaterialType) = nullptr;

	FVoxelPinValue (*GetValue)(
		const FVoxelBuffer& Buffer,
		int32 Index) = nullptr;

	FVoxelFloatBuffer (*GetChannel)(
		const FVoxelBuffer& Buffer,
		EVoxelTextureChannel Channel) = nullptr;
};

#define GENERATED_VOXEL_METADATA_BODY(InRefType) \
	public: \
		using RefType = InRefType; \
		virtual UScriptStruct* GetMetadataRefStruct() const override \
		{ \
			return InRefType::StaticStruct(); \
		} \
		virtual FVoxelPinType GetInnerType() const override \
		{ \
			return FVoxelPinType::Make<InRefType::BufferType::UniformType>(); \
		} \
		virtual FVoxelMetadataRefStatics GetStatics() const override \
		{ \
			return FVoxelMetadataRefStatics \
			{ \
				&InRefType::Impl_Blend, \
				&InRefType::Impl_IndirectBlend, \
				&InRefType::Impl_AddToPCG, \
				&InRefType::Impl_WriteMaterialData, \
				&InRefType::Impl_GetValue, \
				&InRefType::Impl_GetChannel, \
			}; \
		}

UCLASS(BlueprintType, Abstract, meta = (VoxelAssetType, AssetColor=Red, AssetSubMenu = "Metadata"))
class VOXEL_API UVoxelMetadata : public UVoxelAsset
{
	GENERATED_BODY()

public:
	using RefType = FVoxelMetadataRef;

	virtual UScriptStruct* GetMetadataRefStruct() const
	{
		return FVoxelMetadataRef::StaticStruct();
	}
	virtual FVoxelPinType GetInnerType() const VOXEL_PURE_VIRTUAL({});
	virtual FVoxelMetadataRefStatics GetStatics() const VOXEL_PURE_VIRTUAL({});

public:
	virtual FVoxelPinValue GetDefaultValue() const;

	virtual TVoxelOptional<EVoxelMetadataMaterialType> GetMaterialType() const
	{
		return {};
	}

public:
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	FGuid Guid;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Editor")
	FVoxelAssetIcon AssetIcon;
#endif

public:
#if WITH_EDITOR
	static bool CanMigrate();
	static UVoxelMetadata* Migrate(FName LegacyName);

	static void Migrate(
		TArray<FName>& LegacyNames,
		TArray<TObjectPtr<UVoxelMetadata>>& NewMetadatas);
#endif

public:
	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostRename(UObject* OldOuter, const FName OldName) override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
#endif
	//~ End UObject Interface
};