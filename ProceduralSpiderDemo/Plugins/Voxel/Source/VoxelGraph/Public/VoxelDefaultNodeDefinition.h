// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNodeDefinition.h"

struct FVoxelNode;

#if WITH_EDITOR
class VOXELGRAPH_API FVoxelDefaultNodeDefinition : public IVoxelNodeDefinition
{
public:
	FVoxelNode& Node;

	explicit FVoxelDefaultNodeDefinition(FVoxelNode& Node)
		: Node(Node)
	{
	}

	virtual void Initialize(UEdGraphNode& EdGraphNode) {}

	virtual TSharedPtr<const FNode> GetInputs() const override;
	virtual TSharedPtr<const FNode> GetOutputs() const override;
	TSharedPtr<const FNode> GetPins(bool bIsInput) const;

	virtual FString GetAddPinLabel() const override;
	virtual FString GetAddPinTooltip() const override;
	virtual FString GetRemovePinTooltip() const override;

	virtual bool Variadic_CanAddPinTo(FName VariadicPinName) const override;
	virtual FName Variadic_AddPinTo(FName VariadicPinName) override;

	virtual bool Variadic_CanRemovePinFrom(FName VariadicPinName) const override;
	virtual void Variadic_RemovePinFrom(FName VariadicPinName) override;

	virtual bool CanRemoveSelectedPin(FName PinName) const override;
	virtual void RemoveSelectedPin(FName PinName) override;

	virtual void InsertPinBefore(FName PinName) override;
	virtual void DuplicatePin(FName PinName) override;

	virtual void ExposePin(FName PinName) override;

	virtual bool ShouldPromptRenameOnSpawn(FName PinName) const override;
};
#endif