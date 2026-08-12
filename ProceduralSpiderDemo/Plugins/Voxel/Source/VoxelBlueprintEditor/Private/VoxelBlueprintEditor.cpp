// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelStampRef.h"
#include "SVoxelGraphPinSeed.h"
#include "SVoxelGraphPinStampRef.h"
#include "SVoxelBlueprintGraphNode.h"
#include "SVoxelGraphPinStackLayer.h"

VOXEL_DEFAULT_MODULE(VoxelBlueprintEditor);

class FVoxelBlueprintGraphNodeFactory : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* InNode) const override
	{
		VOXEL_FUNCTION_COUNTER();

		if (UK2Node_VoxelBaseNode* Node = Cast<UK2Node_VoxelBaseNode>(InNode))
		{
			return SNew(SVoxelBlueprintGraphNode, Node);
		}

		return nullptr;
	}
};

class FVoxelBlueprintGraphPinFactory : public FGraphPanelPinFactory
{
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* Pin) const override
	{
		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct)
		{
			if (Pin->PinType.PinSubCategoryObject == FVoxelStackLayer::StaticStruct() ||
				Pin->PinType.PinSubCategoryObject == FVoxelStackHeightLayer::StaticStruct() ||
				Pin->PinType.PinSubCategoryObject == FVoxelStackVolumeLayer::StaticStruct())
			{
				return SNew(SVoxelGraphPinStackLayer, Pin);
			}
			if (const UStruct* Struct = Cast<UStruct>(Pin->PinType.PinSubCategoryObject))
			{
				if (Struct->IsChildOf(FVoxelStampRef::StaticStruct()))
				{
					return SNew(SVoxelGraphPinStampRef, Pin);
				}
			}
			if (Pin->PinType.PinSubCategoryObject == FVoxelExposedSeed::StaticStruct())
			{
				return SNew(SVoxelGraphPinSeed, Pin);
			}
		}

		return nullptr;
	}
};

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FEdGraphUtilities::RegisterVisualNodeFactory(MakeShared<FVoxelBlueprintGraphNodeFactory>());
	FEdGraphUtilities::RegisterVisualPinFactory(MakeShared<FVoxelBlueprintGraphPinFactory>());
}