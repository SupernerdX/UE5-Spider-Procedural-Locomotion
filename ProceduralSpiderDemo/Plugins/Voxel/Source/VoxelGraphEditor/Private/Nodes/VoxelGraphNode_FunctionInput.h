// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelTerminalGraph.h"
#include "Nodes/VoxelGraphNode.h"
#include "VoxelGraphNode_FunctionInput.generated.h"

UCLASS(Abstract)
class UVoxelGraphNode_FunctionInputBase : public UVoxelGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGuid Guid;

	UPROPERTY()
	FVoxelGraphFunctionInput CachedInput;

	UVoxelTerminalGraph* GetTerminalGraph() const;
	const FVoxelGraphFunctionInput* GetInput() const;
	FVoxelGraphFunctionInput GetInputSafe() const;

	//~ Begin UVoxelGraphNode Interface
	virtual void AllocateDefaultPins() override;

	virtual void PrepareForCopying() override;
	virtual void PostPasteNode() override;

	virtual FString GetSearchTerms() const override;
	//~ End UVoxelGraphNode Interface

private:
	FSharedVoidPtr OnInputChangedPtr;
};

UCLASS()
class UVoxelGraphNode_FunctionInput : public UVoxelGraphNode_FunctionInputBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bExposeDefaultPin = false;

	//~ Begin UVoxelGraphNode Interface
	virtual bool IsVariable() const override { return true; }
	virtual void AllocateDefaultPins() override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	//~ End UVoxelGraphNode Interface
};

UCLASS(Abstract)
class UVoxelGraphNode_FunctionInputDefaultsBase : public UVoxelGraphNode_FunctionInputBase
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelGraphNode Interface
	virtual void AllocateDefaultPins() override;

	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual bool ShowPaletteIconOnNode() const override { return true; }
	//~ End UVoxelGraphNode Interface

	virtual FName GetInputPinName() const VOXEL_PURE_VIRTUAL({});
};

UCLASS()
class UVoxelGraphNode_FunctionInputDefault : public UVoxelGraphNode_FunctionInputDefaultsBase
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelGraphNode Interface
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void PostPlacedNewNode() override;
	virtual void DestroyNode() override;
	virtual FName GetInputPinName() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ End UVoxelGraphNode Interface
};

UCLASS()
class UVoxelGraphNode_FunctionInputPreview : public UVoxelGraphNode_FunctionInputDefaultsBase
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelGraphNode Interface
	virtual FName GetInputPinName() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ End UVoxelGraphNode Interface
};