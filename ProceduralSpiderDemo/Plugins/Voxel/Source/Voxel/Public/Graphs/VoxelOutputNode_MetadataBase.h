// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "Nodes/VoxelOutputNode.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelOutputNode_MetadataBase.generated.h"

USTRUCT(meta = (Abstract))
struct VOXEL_API FVoxelOutputNode_MetadataBase : public FVoxelOutputNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	//~ Begin FVoxelNode Interface
	virtual void Initialize(FInitializer& Initializer) override;
	virtual void PostSerialize() override;
#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
#endif
	//~ End FVoxelNode Interface

	struct FMetadataPin
	{
		TPinRef_Input<FVoxelMetadataRef> Metadata;
		TPinRef_Input<FVoxelBuffer> Value;
	};
	TVoxelArray<FMetadataPin> MetadataPins;

private:
	UPROPERTY()
	TArray<FName> PinNames_DEPRECATED;

	UPROPERTY()
	TArray<FName> MetadataNames;

	void FixupInputPins();

public:
#if WITH_EDITOR
	class FDefinition : public Super::FDefinition
	{
	public:
		GENERATED_VOXEL_NODE_DEFINITION_BODY(FVoxelOutputNode_MetadataBase);

		virtual FString GetAddPinLabel() const override;
		virtual FString GetAddPinTooltip() const override;

		virtual bool CanAddInputPin() const override { return true; }
		virtual void AddInputPin() override;

		virtual bool CanRemoveInputPin() const override;
		virtual void RemoveInputPin() override;

		virtual bool CanRemoveSelectedPin(FName PinName) const override;
		virtual void RemoveSelectedPin(FName PinName) override;

		virtual bool OnPinDefaultValueChanged(FName PinName, const FVoxelPinValue& NewDefaultValue) override;
	};
#endif
};