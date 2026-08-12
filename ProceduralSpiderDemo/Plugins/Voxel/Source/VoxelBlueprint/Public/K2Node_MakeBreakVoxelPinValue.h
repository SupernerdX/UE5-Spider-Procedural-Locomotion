// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "K2Node_VoxelBaseNode.h"
#include "VoxelPinType.h"
#include "K2Node_MakeBreakVoxelPinValue.generated.h"

UCLASS(Abstract)
class VOXELBLUEPRINT_API UK2Node_MakeBreakVoxelPinValueBase : public UK2Node_VoxelBaseNode
{
	GENERATED_BODY()

public:
	UK2Node_MakeBreakVoxelPinValueBase();

	//~ Begin UEdGraphNode Interface
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	//~ End UEdGraphNode Interface

	//~ Begin UK2Node Interface
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void PostReconstructNode() override;
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;
	//~ End UK2Node Interface

	//~ Begin UK2Node_VoxelBaseNode Interface
	virtual bool IsPinWildcard(const UEdGraphPin& Pin) const override;
	virtual void OnPinTypeChange(UEdGraphPin& Pin, const FVoxelPinType& NewType) override;
	//~ End UK2Node_VoxelBaseNode Interface

	virtual FName GetValuePinName() const VOXEL_PURE_VIRTUAL({});

private:
	UPROPERTY()
	FVoxelPinType CachedType;
};

UCLASS()
class VOXELBLUEPRINT_API UK2Node_MakeVoxelPinValueBase : public UK2Node_MakeBreakVoxelPinValueBase
{
	GENERATED_BODY()

public:
	UK2Node_MakeVoxelPinValueBase();

	//~ Begin UK2Node_MakeBreakVoxelPinValueBase Interface
	virtual FName GetValuePinName() const override;
	//~ End UK2Node_MakeBreakVoxelPinValueBase Interface
};

UCLASS()
class VOXELBLUEPRINT_API UK2Node_BreakVoxelPinValueBase : public UK2Node_MakeBreakVoxelPinValueBase
{
	GENERATED_BODY()

public:
	UK2Node_BreakVoxelPinValueBase();

	//~ Begin UK2Node_MakeBreakVoxelPinValueBase Interface
	virtual FName GetValuePinName() const override;
	//~ End UK2Node_MakeBreakVoxelPinValueBase Interface
};