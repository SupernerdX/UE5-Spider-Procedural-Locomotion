// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraphNode.h"
#include "VoxelTerminalGraph.h"
#include "VoxelGraphNode_FunctionOutput.generated.h"

UCLASS()
class UVoxelGraphNode_FunctionOutput : public UVoxelGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGuid Guid;

	UPROPERTY()
	FVoxelGraphFunctionOutput CachedOutput;

	const FVoxelGraphFunctionOutput* GetOutput() const;
	FVoxelGraphFunctionOutput GetOutputSafe() const;

	//~ Begin UVoxelGraphNode Interface
	virtual bool IsVariable() const override { return true; }
	virtual void AllocateDefaultPins() override;

	virtual void PrepareForCopying() override;
	virtual void PostPasteNode() override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FString GetSearchTerms() const override;
	//~ End UVoxelGraphNode Interface

private:
	FSharedVoidPtr OnOutputChangedPtr;
};