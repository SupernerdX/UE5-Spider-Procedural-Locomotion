// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelPinTypeSet.h"
#include "Nodes/VoxelGraphNode_Struct.h"
#include "Nodes/VoxelNode_MakeStruct.h"
#include "Nodes/VoxelNode_BreakStruct.h"

class FVoxelUserTypesHandler : public FVoxelEditorSingleton
{
public:
	//~ Begin FVoxelEditorSingleton Interface
	virtual void Initialize() override
	{
		FVoxelPinTypeSet::OnUserTypeChanged.Add(MakeLambdaDelegate([this](const FVoxelPinType& Type)
		{
			if (Type.IsUserDefinedEnum())
			{
				OnEnumChanged(Type);
			}
			else if (Type.IsUserDefinedStruct())
			{
				OnStructChanged(Type);
			}
			else
			{
				ensure(false);
			}
		}));
	}
	//~ End FVoxelEditorSingleton Interface

private:
	static void OnEnumChanged(const FVoxelPinType& Type)
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());
		ensure(!Type.IsBuffer());
		ensure(Type.IsUserDefinedEnum());

		ForEachObjectOfClass_Copy<UVoxelGraphNode>([&](UVoxelGraphNode& Node)
		{
			for (const UEdGraphPin* Pin : Node.GetAllPins())
			{
				if (FVoxelPinType(Pin->PinType).GetInnerType() != Type)
				{
					continue;
				}

				Node.ReconstructNode();
				return;
			}
		});
	}
	static void OnStructChanged(const FVoxelPinType& Type)
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());
		ensure(!Type.IsBuffer());
		ensure(Type.IsUserDefinedStruct());

		ForEachObjectOfClass_Copy<UVoxelGraphNode_Struct>([&](UVoxelGraphNode_Struct& Node)
		{
			const TSharedPtr<FVoxelNode_MakeStruct> MakeStruct = Node.Struct.ToSharedPtr<FVoxelNode_MakeStruct>();
			if (MakeStruct &&
				MakeStruct->GetPin(MakeStruct->StructPin).GetType().GetInnerType() == Type)
			{
				MakeStruct->FixupInputPins();
				Node.ReconstructNode();
			}

			const TSharedPtr<FVoxelNode_BreakStruct> BreakStruct = Node.Struct.ToSharedPtr<FVoxelNode_BreakStruct>();
			if (BreakStruct &&
				BreakStruct->GetPin(BreakStruct->StructPin).GetType().GetInnerType() == Type)
			{
				BreakStruct->FixupOutputPins();
				Node.ReconstructNode();
			}
		});
	}
};
FVoxelUserTypesHandler* GVoxelUserTypesHandler = new FVoxelUserTypesHandler();