// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Nodes/VoxelGraphNode.h"
#include "VoxelGraphNode_CallFunction.generated.h"

class UVoxelGraph;
class UVoxelTerminalGraph;
class UVoxelFunctionLibraryAsset;

UCLASS(Abstract)
class UVoxelGraphNode_CallFunctionBase : public UVoxelGraphNode
{
	GENERATED_BODY()

public:
	virtual void AddPins() {}
	virtual const UVoxelTerminalGraph* GetBaseTerminalGraph() const VOXEL_PURE_VIRTUAL({});

	//~ Begin UVoxelGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual bool ShowPaletteIconOnNode() const override { return true; }

	virtual bool IsPinOptional(const UEdGraphPin& Pin) const override;
	virtual bool ShouldHidePinDefaultValue(const UEdGraphPin& Pin) const override;

	virtual bool CanJumpToDefinition() const override { return true; }
	virtual void JumpToDefinition() const override;

	virtual void PostReconstructNode() override;

	virtual FName GetPinCategory(const UEdGraphPin& Pin) const override;
	virtual TSharedRef<IVoxelNodeDefinition> GetNodeDefinition() override;
	//~ End UVoxelGraphNode Interface

private:
	TVoxelMap<FName, bool> PinNameToIsOptional;
	FSharedVoidPtr OnChangedPtr;
};

class FVoxelNodeDefinition_CallFunctionBase : public IVoxelNodeDefinition
{
public:
	UVoxelGraphNode_CallFunctionBase& Node;

	explicit FVoxelNodeDefinition_CallFunctionBase(UVoxelGraphNode_CallFunctionBase& Node)
		: Node(Node)
	{
	}

	//~ Begin IVoxelNodeDefinition Interface
	virtual TSharedPtr<const FNode> GetInputs() const override;
	virtual TSharedPtr<const FNode> GetOutputs() const override;

	virtual bool OverridePinsOrder() const override
	{
		return true;
	}
	//~ End IVoxelNodeDefinition Interface
};

UCLASS()
class UVoxelGraphNode_CallMemberFunction : public UVoxelGraphNode_CallFunctionBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGuid Guid;

	UPROPERTY()
	bool bCallParent = false;

	UPROPERTY()
	FString CachedName;

	//~ Begin UVoxelGraphNode_CallTerminalGraph Interface
	virtual const UVoxelTerminalGraph* GetBaseTerminalGraph() const override;
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;

	virtual void PrepareForCopying() override;
	virtual void PostPasteNode() override;
	//~ End UVoxelGraphNode_CallTerminalGraph Interface

private:
	FSharedVoidPtr OnFunctionMetaDataChangedPtr;
};

UCLASS()
class UVoxelGraphNode_CallExternalFunction : public UVoxelGraphNode_CallFunctionBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UVoxelFunctionLibraryAsset> FunctionLibrary;

	UPROPERTY()
	FGuid Guid;

	UPROPERTY()
	FString CachedName;

	//~ Begin UVoxelGraphNode_CallTerminalGraph Interface
	virtual const UVoxelTerminalGraph* GetBaseTerminalGraph() const override;
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool CanPasteHere(const UEdGraph* TargetGraph) const override;
	//~ End UVoxelGraphNode_CallTerminalGraph Interface

private:
	FSharedVoidPtr OnFunctionMetaDataChangedPtr;
};