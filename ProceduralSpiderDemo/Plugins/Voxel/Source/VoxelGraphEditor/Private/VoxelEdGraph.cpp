// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEdGraph.h"
#include "VoxelVersion.h"
#include "VoxelGraphSchema.h"
#include "VoxelGraphTracker.h"
#include "VoxelGraphToolkit.h"
#include "VoxelGraphMigration.h"
#include "Nodes/VoxelNode_UFunction.h"
#include "Nodes/VoxelGradientNodes.h"
#include "Nodes/VoxelGraphNode_Struct.h"
#include "Nodes/VoxelGraphNode_FunctionInput.h"
#include "Nodes/VoxelGraphNode_FunctionOutput.h"
#include "Graphs/VoxelOutputNode_OutputHeightBase.h"
#include "Surface/VoxelSurfaceTypeBlend.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "GraphEditAction.h"

void UVoxelEdGraph::SetLatestVersion()
{
	Version = FVersion::LatestVersion;
}

void UVoxelEdGraph::SetToolkit(const TSharedRef<FVoxelGraphToolkit>& Toolkit)
{
	ensure(!WeakToolkit.IsValid() || WeakToolkit == Toolkit);
	WeakToolkit = Toolkit;
}

TSharedPtr<FVoxelGraphToolkit> UVoxelEdGraph::GetGraphToolkit() const
{
	return WeakToolkit.Pin();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelEdGraph::MigrateIfNeeded()
{
	VOXEL_FUNCTION_COUNTER();

	if (Version != FVersion::LatestVersion)
	{
		MigrateAndReconstructAll();
	}

	ensure(Version == FVersion::LatestVersion);
}

void UVoxelEdGraph::MigrateAndReconstructAll()
{
	VOXEL_FUNCTION_COUNTER();

	ensure(Version != -1);

	// Disable SetDirty
	const bool bOldIsEditorLoadingPackage = UE::GetIsEditorLoadingPackage();
	UE::SetIsEditorLoadingPackage(true);

	ON_SCOPE_EXIT
	{
		UE::SetIsEditorLoadingPackage(bOldIsEditorLoadingPackage);
	};

	UVoxelTerminalGraph* OuterTerminalGraph = GetTypedOuter<UVoxelTerminalGraph>();
	if (!ensure(OuterTerminalGraph))
	{
		return;
	}

	ON_SCOPE_EXIT
	{
		// Migrate struct nodes
		for (UEdGraphNode* Node : Nodes)
		{
			UVoxelGraphNode_Struct* StructNode = Cast<UVoxelGraphNode_Struct>(Node);
			if (!StructNode ||
				StructNode->Struct.IsValid())
			{
				continue;
			}

			const FName CachedName(StructNode->CachedName);

			UFunction* NewFunction = GVoxelGraphMigration->FindNewFunction(CachedName);
			if (!NewFunction)
			{
				continue;
			}

			StructNode->Struct = FVoxelNode_UFunction::StaticStruct();
			StructNode->Struct.Get<FVoxelNode_UFunction>().SetFunction_EditorOnly(NewFunction);
			StructNode->ReconstructNode();

			GVoxelGraphMigration->OnNodeMigrated(CachedName, StructNode);
		}

		// Migrate function nodes
		for (UEdGraphNode* Node : Nodes)
		{
			UVoxelGraphNode_Struct* StructNode = Cast<UVoxelGraphNode_Struct>(Node);
			if (!StructNode ||
				!StructNode->Struct.IsA<FVoxelNode_UFunction>())
			{
				continue;
			}

			const FVoxelNode_UFunction& FunctionNode = StructNode->Struct->AsChecked<FVoxelNode_UFunction>();
			if (FunctionNode.GetFunction())
			{
				continue;
			}

			const FName CachedName = FunctionNode.GetCachedName();

			UScriptStruct* NewNode = GVoxelGraphMigration->FindNewNode(CachedName);
			if (!NewNode)
			{
				continue;
			}

			StructNode->Struct = NewNode;
			StructNode->ReconstructNode();

			GVoxelGraphMigration->OnNodeMigrated(CachedName, StructNode);
		}

		for (UEdGraphNode* Node : Nodes)
		{
			Node->ReconstructNode();
		}

		Version = FVersion::LatestVersion;
	};

	if (Version == FVersion::LatestVersion)
	{
		return;
	}

	if (Version < FVersion::AddNormalizeToGradientNodes)
	{
		TArray<UVoxelGraphNode_Struct*> StructNodes;
		GetNodesOfClass<UVoxelGraphNode_Struct>(StructNodes);

		for (UVoxelGraphNode_Struct* Node : StructNodes)
		{
			if (!Node->Struct ||
				!Node->Struct->IsA<FVoxelNode_GetGradient3D>())
			{
				continue;
			}

			Node->ReconstructNode();

			UEdGraphPin* NormalizePin = Node->FindPin(TEXT("Normalize"), EGPD_Input);
			if (!ensure(NormalizePin))
			{
				continue;
			}

			FVoxelPinValue::Make(false).ApplyToPinDefaultValue(*NormalizePin);
		}
	}

	if (Version < FVersion::AddVoxelMaterial)
	{
		for (UEdGraphNode* Node : Nodes)
		{
			if (!Node)
			{
				continue;
			}

			for (UEdGraphPin* Pin : Node->Pins)
			{
				if (!Pin ||
					Pin->Direction != EGPD_Input)
				{
					continue;
				}

				if (!FVoxelPinType(Pin->PinType).GetInnerType().Is<FVoxelSurfaceType>() &&
					!FVoxelPinType(Pin->PinType).GetInnerType().Is<FVoxelSurfaceTypeBlend>())
				{
					continue;
				}

				UMaterialInterface* Material = Cast<UMaterialInterface>(Pin->DefaultObject);
				if (!Material)
				{
					ensureVoxelSlow(!Pin->DefaultObject || Pin->DefaultObject->IsA<UVoxelSurfaceTypeAsset>());
					continue;
				}

				Pin->DefaultObject = UVoxelSurfaceTypeInterface::Migrate(Material);
			}
		}
	}

	if (Version < FVersion::RemoveFunctionInputDefaultPin)
	{
		TArray<UVoxelGraphNode_FunctionInput*> InputNodes;
		GetNodesOfClass<UVoxelGraphNode_FunctionInput>(InputNodes);

		for (UVoxelGraphNode_FunctionInput* InputNode : InputNodes)
		{
			if (!InputNode->bExposeDefaultPin)
			{
				continue;
			}

			FGraphNodeCreator<UVoxelGraphNode_FunctionInputDefault> NodeCreator(*this);
			UVoxelGraphNode_FunctionInputDefault* InputDefaultNode = NodeCreator.CreateNode(false);
			InputDefaultNode->Guid = InputNode->Guid;
			InputDefaultNode->CachedInput = InputNode->CachedInput;
			InputDefaultNode->NodePosX = InputNode->NodePosX;
			InputDefaultNode->NodePosY = InputNode->NodePosY + 100;
			NodeCreator.Finalize();

			UEdGraphPin* OldInputPin = InputNode->GetInputPin(0);
			UEdGraphPin* NewInputPin = InputDefaultNode->GetInputPin(0);

			if (ensure(OldInputPin) &&
				ensure(NewInputPin))
			{
				NewInputPin->CopyPersistentDataFromOldPin(*OldInputPin);
			}

			if (OldInputPin)
			{
				InputNode->RemovePin(OldInputPin);
			}
			InputNode->ReconstructNode();
		}
	}

	if (Version < FVersion::UpdateOutputHeightRanges)
	{
		TArray<UVoxelGraphNode_Struct*> StructNodes;
		GetNodesOfClass<UVoxelGraphNode_Struct>(StructNodes);

		for (UVoxelGraphNode_Struct* StructNode : StructNodes)
		{
			if (!StructNode->Struct ||
				!StructNode->Struct->IsA<FVoxelOutputNode_OutputHeightBase>())
			{
				continue;
			}

			StructNode->ReconstructNode();

			UEdGraphPin* Pin = StructNode->FindPin(VOXEL_PIN_NAME(FVoxelOutputNode_OutputHeightBase, HeightRangePin), EGPD_Input);
			if (!Pin ||
				!Pin->DoesDefaultValueMatchAutogenerated() ||
				Pin->LinkedTo.Num() > 0)
			{
				continue;
			}

			FVoxelPinValue Value = FVoxelPinValue::Make(FVoxelFloatRange(-1000000.f, 1000000.f));
			Pin->GetSchema()->TrySetDefaultValue(*Pin, Value.ExportToString());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelEdGraph::NotifyGraphChanged(const FEdGraphEditAction& Action)
{
	VOXEL_FUNCTION_COUNTER();

	Super::NotifyGraphChanged(Action);

	if (Action.Action & (GRAPHACTION_AddNode | GRAPHACTION_RemoveNode))
	{
		GVoxelGraphTracker->NotifyEdGraphChanged(*this);
	}
}

void UVoxelEdGraph::PostEditUndo()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditUndo();

	GVoxelGraphTracker->NotifyEdGraphChanged(*this);
}

void UVoxelEdGraph::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	GVoxelGraphTracker->NotifyEdGraphChanged(*this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelEdGraph::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);

	if (Ar.CustomVer(GVoxelCustomVersionGUID) < FVoxelVersion::FixVoxelEdGraphMigration)
	{
		Version = FVersion::AddNormalizeToGradientNodes;
	}
}