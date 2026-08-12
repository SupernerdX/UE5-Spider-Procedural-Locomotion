// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Preview/VoxelPreviewHandler.h"
#include "Buffer/VoxelIntegerBuffers.h"
#include "VoxelEnumPreviewHandler.generated.h"

USTRUCT(DisplayName = "Enum")
struct VOXELGRAPH_API FVoxelPreviewHandler_Enum : public FVoxelPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		if (!Type.Is<uint8>() &&
			!Type.Is<FVoxelByteBuffer>())
		{
			return false;
		}

		return Type.GetInnerType().GetEnum() != nullptr;
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void BuildStats(const FAddStat& AddStat) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelByteBuffer> Buffer = FVoxelByteBuffer::MakeSharedDefault();

	UPROPERTY(Transient)
	FVoxelPinType EnumType;
};