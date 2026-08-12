// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelMessage.h"
#include "VoxelPCGCallstack.h"

#include "PCGGraph.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "Editor/EditorEngine.h"
#include "Toolkits/ToolkitManager.h"
#if VOXEL_ENGINE_VERSION >= 507
#include "PCGEditor.h"
#else
#include "PCGEditor/Private/PCGEditor.h"
#endif
#if VOXEL_ENGINE_VERSION >= 506
#include "Editor/IPCGEditorModule.h"
#endif
#include "Subsystems/AssetEditorSubsystem.h"

DEFINE_PRIVATE_ACCESS(FPCGEditor, PCGEditorGraph);
DEFINE_PRIVATE_ACCESS(FPCGEditor, GraphEditorWidget);

void JumpToPCGNode(
	const FPCGEditor& Editor,
	const UPCGNode* TargetNode)
{
	VOXEL_FUNCTION_COUNTER();

	if (!TargetNode)
	{
		return;
	}

	UEdGraph* EdGraph = reinterpret_cast<UEdGraph*>(PrivateAccess::PCGEditorGraph(Editor));
	const TSharedPtr<SGraphEditor> GraphEditor = PrivateAccess::GraphEditorWidget(Editor);
	if (!ensure(EdGraph) ||
		!ensure(GraphEditor))
	{
		return;
	}

	for (UEdGraphNode* EdGraphNode : EdGraph->Nodes)
	{
		UClass* Class = FindObjectChecked<UClass>(nullptr, TEXT("/Script/PCGEditor.PCGEditorGraphNodeBase"));
		if (!EdGraphNode ||
			!EdGraphNode->IsA(Class))
		{
			continue;
		}

		const FProperty* Property = FindFProperty<FObjectProperty>(Class, "PCGNode");
		if (!ensure(Property))
		{
			continue;
		}

		const TObjectPtr<UPCGNode> PCGNode = *Property->ContainerPtrToValuePtr<TObjectPtr<UPCGNode>>(EdGraphNode);
		if (PCGNode == TargetNode)
		{
			GraphEditor->JumpToNode(EdGraphNode);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	GVoxelMessageManager->OnMessageLogged.AddLambda([](const TSharedRef<FVoxelMessage>& Message)
	{
		check(IsInGameThread());

		for (const TSharedRef<FVoxelMessageToken>& Token : Message->GetTokens())
		{
			const TSharedPtr<FVoxelMessageToken_PCGCallstack> CallstackToken = CastStruct<FVoxelMessageToken_PCGCallstack>(Token);
			if (!CallstackToken)
			{
				continue;
			}

			const UPCGComponent* Component = CallstackToken->Callstack->Component.Resolve();
			if (!ensureVoxelSlow(Component))
			{
				continue;
			}

			UPCGSubsystem* Subsystem = UPCGSubsystem::GetInstance(Component->GetWorld());
			if (!ensureVoxelSlow(Subsystem))
			{
				continue;
			}

			const ELogVerbosity::Type Verbosity = INLINE_LAMBDA
			{
				switch (Message->GetSeverity())
				{
				default: ensure(false);
				case EVoxelMessageSeverity::Info: return ELogVerbosity::Log;
				case EVoxelMessageSeverity::Warning: return ELogVerbosity::Warning;
				case EVoxelMessageSeverity::Error: return ELogVerbosity::Error;
				}
			};

#if VOXEL_ENGINE_VERSION >= 506
			if (IPCGEditorModule* PCGEditorModule = IPCGEditorModule::Get())
			{
				PCGEditorModule->GetNodeVisualLogsMutable().Log(
					CallstackToken->Callstack->Stack,
					Verbosity,
					FText::FromString(Message->ToString()));
			}
#else
			Subsystem->GetNodeVisualLogsMutable().Log(
				CallstackToken->Callstack->Stack,
				Verbosity,
				FText::FromString(Message->ToString()));
#endif
		}
	});

	GVoxelTryFocusObjectFunctions.Add([](const UObject& Object)
	{
		const UPCGNode* Node = Cast<UPCGNode>(&Object);
		if (!Node)
		{
			return false;
		}

		const UPCGGraph* Graph = Node->GetTypedOuter<UPCGGraph>();
		if (!ensureVoxelSlow(Graph))
		{
			return false;
		}

		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Graph);

		const TSharedPtr<IToolkit> AssetEditor = FToolkitManager::Get().FindEditorForAsset(Graph);
		if (!ensureVoxelSlow(AssetEditor))
		{
			return false;
		}

		JumpToPCGNode(
			static_cast<FPCGEditor&>(*AssetEditor),
			Node);

		return true;
	});

	GVoxelTryGetObjectNameFunctions.Add([](const UObject& Object) -> FString
	{
		const UPCGNode* Node = Cast<UPCGNode>(&Object);
		if (!Node)
		{
			return {};
		}

		const UPCGSettings* Settings = Node->GetSettings();
		if (!Settings)
		{
			return {};
		}

		return Settings->GetDefaultNodeTitle().ToString();
	});
}