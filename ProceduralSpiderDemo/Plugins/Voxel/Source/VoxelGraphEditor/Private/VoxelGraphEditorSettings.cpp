// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphEditorSettings.h"
#include "VoxelNodeLibrary.h"
#include "VoxelTerminalGraph.h"
#include "VoxelGraphSchemaAction.h"
#include "VoxelFunctionLibraryAsset.h"

FString FVoxelGraphEditorInputBinding::GetActionName() const
{
	switch (Type)
	{
	case EVoxelGraphEditorInputType::Struct:
	{
		if (Struct.IsNull())
		{
			return "INVALID";
		}

		const TSharedPtr<const FVoxelNode> Node = GVoxelNodeLibrary->FindNode(Struct.LoadSynchronous());
		if (!Node)
		{
			return "INVALID";
		}

		return Node->GetDisplayName();
	}
	case EVoxelGraphEditorInputType::Function:
	{
		if (Function.IsNull())
		{
			return "INVALID";
		}

		const TSharedPtr<const FVoxelNode> Node = GVoxelNodeLibrary->FindNode(Function.LoadSynchronous());
		if (!Node)
		{
			return "INVALID";
		}

		return Node->GetDisplayName();
	}
	case EVoxelGraphEditorInputType::FunctionLibrary:
	{
		if (FunctionLibrary.IsNull())
		{
			return "INVALID";
		}

		UVoxelFunctionLibraryAsset* Asset = FunctionLibrary.LoadSynchronous();
		const UVoxelTerminalGraph* TerminalGraph = Asset->GetGraph().FindTerminalGraph(FunctionGuid);
		if (!TerminalGraph)
		{
			return "INVALID";
		}

		return TerminalGraph->GetDisplayName();
	}
	case EVoxelGraphEditorInputType::None: return "None";
	default: ensure(false); return "INVALID";
	}
}

bool FVoxelGraphEditorInputBinding::IsActionValid(const bool bNoneValid) const
{
	switch (Type)
	{
	case EVoxelGraphEditorInputType::Struct:
	{
		return !Struct.IsNull();
	}
	case EVoxelGraphEditorInputType::Function:
	{
		return !Function.IsNull();
	}
	case EVoxelGraphEditorInputType::FunctionLibrary:
	{
		if (FunctionLibrary.IsNull())
		{
			return false;
		}

		UVoxelFunctionLibraryAsset* Asset = FunctionLibrary.LoadSynchronous();
		const UVoxelTerminalGraph* TerminalGraph = Asset->GetGraph().FindTerminalGraph(FunctionGuid);
		if (!TerminalGraph)
		{
			return false;
		}

		return true;
	}
	case EVoxelGraphEditorInputType::None: return bNoneValid;
	default: ensure(false); return false;
	}
}

TSharedPtr<FEdGraphSchemaAction> FVoxelGraphEditorInputBinding::CreateAction() const
{
	switch (Type)
	{
	case EVoxelGraphEditorInputType::Struct:
	{
		if (Struct.IsNull())
		{
			return nullptr;
		}

		const TSharedPtr<const FVoxelNode> Node = GVoxelNodeLibrary->FindNode(Struct.LoadSynchronous());
		if (!Node)
		{
			return nullptr;
		}

		const TSharedRef<FVoxelGraphSchemaAction_NewStructNode> Action = MakeShared<FVoxelGraphSchemaAction_NewStructNode>();
		Action->Node = Node;
		return Action;
	}
	case EVoxelGraphEditorInputType::Function:
	{
		if (Function.IsNull())
		{
			return nullptr;
		}

		const TSharedPtr<const FVoxelNode> Node = GVoxelNodeLibrary->FindNode(Function.LoadSynchronous());
		if (!Node)
		{
			return nullptr;
		}

		const TSharedRef<FVoxelGraphSchemaAction_NewStructNode> Action = MakeShared<FVoxelGraphSchemaAction_NewStructNode>();
		Action->Node = Node;
		return Action;
	}
	case EVoxelGraphEditorInputType::FunctionLibrary:
	{
		if (FunctionLibrary.IsNull())
		{
			return nullptr;
		}

		UVoxelFunctionLibraryAsset* Asset = FunctionLibrary.LoadSynchronous();
		if (!Asset->GetGraph().FindTerminalGraph(FunctionGuid))
		{
			return nullptr;
		}

		const TSharedRef<FVoxelGraphSchemaAction_NewCallExternalFunctionNode> Action = MakeShared<FVoxelGraphSchemaAction_NewCallExternalFunctionNode>();
		Action->FunctionLibrary = Asset;
		Action->Guid = FunctionGuid;

		return Action;
	}
	case EVoxelGraphEditorInputType::None: return nullptr;
	default: ensure(false); return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FName UVoxelGraphEditorSettings::GetContainerName() const
{
	return "Editor";
}