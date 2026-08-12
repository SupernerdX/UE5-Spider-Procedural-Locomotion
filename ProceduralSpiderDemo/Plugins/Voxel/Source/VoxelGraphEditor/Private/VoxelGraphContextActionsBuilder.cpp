// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphContextActionsBuilder.h"
#include "VoxelNodeLibrary.h"
#include "VoxelGraphToolkit.h"
#include "VoxelTerminalGraph.h"
#include "Nodes/VoxelGraphNode.h"
#include "Nodes/VoxelOutputNode.h"
#include "VoxelGraphSchemaAction.h"
#include "Nodes/VoxelOperatorNodes.h"
#include "VoxelGraphEditorSettings.h"
#include "VoxelFunctionLibraryAsset.h"
#include "Nodes/VoxelNode_CustomizeParameter.h"

TSharedPtr<FVoxelGraphContextActionsBuilder> FVoxelGraphContextActionsBuilder::Build(const TSharedPtr<FGraphContextMenuBuilder>& MenuBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(MenuBuilder->CurrentGraph))
	{
		return nullptr;
	}

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = FVoxelGraphToolkit::Get(MenuBuilder->CurrentGraph);
	UVoxelTerminalGraph* TerminalGraph = MenuBuilder->CurrentGraph->GetTypedOuter<UVoxelTerminalGraph>();

	if (!ensure(Toolkit))
	{
		return nullptr;
	}

	if (!TerminalGraph)
	{
		ensure(MenuBuilder->CurrentGraph->HasAnyFlags(RF_Transient));
		return nullptr;
	}

	TSharedPtr<FVoxelGraphContextActionsBuilder> ContextActions = MakeShareable(new FVoxelGraphContextActionsBuilder(MenuBuilder, Toolkit, TerminalGraph));
	ContextActions->Build();
	return ContextActions;
}

TSharedPtr<FVoxelGraphContextActionsBuilder> FVoxelGraphContextActionsBuilder::BuildNoGraph(const TSharedPtr<FGraphContextMenuBuilder>& MenuBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	TSharedPtr<FVoxelGraphContextActionsBuilder> ContextActions = MakeShareable(new FVoxelGraphContextActionsBuilder(MenuBuilder, nullptr, nullptr));
	ContextActions->Build();
	return ContextActions;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelGraphContextActionsBuilder::Build()
{
	VOXEL_FUNCTION_COUNTER();

	if (State == EState::Completed)
	{
		return false;
	}

	const TSharedPtr<FGraphContextMenuBuilder> MenuBuilder = WeakMenuBuilder.Pin();
	if (!MenuBuilder)
	{
		return true;
	}

	EndTime = FPlatformTime::Seconds() + GetDefault<UVoxelGraphEditorSettings>()->ContextMenuTimeSlicingThresholdMs / 1000.f;
	if (!GetDefault<UVoxelGraphEditorSettings>()->bEnableContextMenuTimeSlicing)
	{
		EndTime = FLT_MAX;
	}

	while (State != EState::Completed)
	{
		const EVoxelIterate Iterate = INLINE_LAMBDA -> EVoxelIterate
		{
			switch (State)
			{
			case EState::Initial: return BuildInitialState(*MenuBuilder);
			case EState::FunctionLibraries: return BuildFunctionLibraries(*MenuBuilder);
			case EState::MemberFunctions: return BuildMemberFunctions(*MenuBuilder);
			case EState::Nodes: return BuildNodes(*MenuBuilder);
			case EState::Parameters: return BuildParameters(*MenuBuilder);
			case EState::Inputs: return BuildInputs(*MenuBuilder);
			case EState::Outputs: return BuildOutputs(*MenuBuilder);
			case EState::LocalVariables: return BuildLocalVariables(*MenuBuilder);
			case EState::Promotions: return BuildPromotions(*MenuBuilder);
			case EState::CustomizeParameters: return BuildCustomizeParameters(*MenuBuilder);
			case EState::ShortList: return BuildShortList(*MenuBuilder);
			case EState::Completed: return EVoxelIterate::Stop;
			}
			return EVoxelIterate::Continue;
		};

		if (Iterate == EVoxelIterate::Stop)
		{
			Progress = float(State) / float(EState::Completed);
			Progress += (1.f / double(EState::Completed)) * (double(NumStepCompletedActions) / double(FMath::Max(NumStepActions, 1)));
			return true;
		}

		State = EState(int32(State) + 1);
		Progress = float(State) / float(EState::Completed);
		NumStepCompletedActions = 0;
	}

	return true;
}

float FVoxelGraphContextActionsBuilder::GetProgress() const
{
	return Progress;
}

bool FVoxelGraphContextActionsBuilder::IsCompleted() const
{
	return State == EState::Completed;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildInitialState(FGraphContextMenuBuilder& MenuBuilder) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Toolkit)
	{
		return EVoxelIterate::Continue;
	}

	// Add comment
	if (!MenuBuilder.FromPin)
	{
		bool bCreateFromSelection = false;
		if (MenuBuilder.CurrentGraph)
		{
			const TSet<UEdGraphNode*> SelectedNodes = Toolkit->GetSelectedNodes();

			for (const auto* Node : SelectedNodes)
			{
				if (Node->IsA<UVoxelGraphNode>())
				{
					bCreateFromSelection = true;
					break;
				}
			}
		}

		MenuBuilder.AddAction(MakeShared<FVoxelGraphSchemaAction_NewComment>(
			FText(),
			bCreateFromSelection ? INVTEXT("Create Comment from Selection") : INVTEXT("Add Comment"),
			INVTEXT("Creates a comment"),
			0));
	}

	// Paste here
	if (!MenuBuilder.FromPin &&
		Toolkit->GetCommands()->CanExecuteAction(FGenericCommands::Get().Paste.ToSharedRef()))
	{
		MenuBuilder.AddAction(MakeShared<FVoxelGraphSchemaAction_Paste>(
			FText(),
			INVTEXT("Paste here"),
			FText(),
			0));
	}

	// Add reroute node
	MenuBuilder.AddAction(MakeShared<FVoxelGraphSchemaAction_NewKnotNode>(
		FText(),
		INVTEXT("Add reroute node"),
		INVTEXT("Create a reroute node"),
		0));

	return EVoxelIterate::Continue;
}

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildMemberFunctions(FGraphContextMenuBuilder& MenuBuilder) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Toolkit)
	{
		return EVoxelIterate::Continue;
	}

	if (ActiveTerminalGraph &&
		ActiveTerminalGraph->IsMainTerminalGraph() &&
		Toolkit->Asset->GetBaseGraphs().Num() > 1)
	{
		if (!MenuBuilder.FromPin ||
			MenuBuilder.FromPin->Direction == EGPD_Input)
		{
			const TSharedRef<FVoxelGraphSchemaAction_NewCallParentMainGraphNode> Action = MakeShared<FVoxelGraphSchemaAction_NewCallParentMainGraphNode>(
				INVTEXT(""),
				FText::FromString("Call Parent Main Graph"),
				INVTEXT(""),
				GroupId_InlineFunctions);

				MenuBuilder.AddAction(Action);
			}
		}

	// Add member functions
	for (const FGuid& Guid : Toolkit->Asset->GetTerminalGraphs())
	{
		UVoxelTerminalGraph* TerminalGraph = Toolkit->Asset->FindTerminalGraph(Guid);
		if (!ensure(TerminalGraph) ||
			TerminalGraph->IsMainTerminalGraph() ||
			TerminalGraph->IsEditorTerminalGraph() ||
			!CanCallTerminalGraph(MenuBuilder, TerminalGraph))
		{
			continue;
		}
		const FVoxelGraphMetadata Metadata = TerminalGraph->GetMetadata();

		const TSharedRef<FVoxelGraphSchemaAction_NewCallFunctionNode> Action = MakeShared<FVoxelGraphSchemaAction_NewCallFunctionNode>(
			FText::FromString(Metadata.Category),
			FText::FromString(Metadata.DisplayName),
			FText::FromString(Metadata.Description),
			GroupId_InlineFunctions);

		Action->Guid = Guid;

		MenuBuilder.AddAction(Action);
	}

	// Add Call Parent
	for (const FGuid& Guid : Toolkit->Asset->GetTerminalGraphs())
	{
		UVoxelTerminalGraph* TerminalGraph = Toolkit->Asset->FindTerminalGraph(Guid);
		if (!ensure(TerminalGraph) ||
			TerminalGraph->IsMainTerminalGraph() ||
			TerminalGraph->IsEditorTerminalGraph() ||
			TerminalGraph->IsTopmostTerminalGraph() ||
			!CanCallTerminalGraph(MenuBuilder, TerminalGraph))
		{
			continue;
		}
		const FVoxelGraphMetadata Metadata = TerminalGraph->GetMetadata();

		const TSharedRef<FVoxelGraphSchemaAction_NewCallFunctionNode> Action = MakeShared<FVoxelGraphSchemaAction_NewCallFunctionNode>(
			FText::FromString(Metadata.Category),
			FText::FromString("Call Parent: " + Metadata.DisplayName),
			FText::FromString(Metadata.Description),
			GroupId_InlineFunctions);

		Action->Guid = Guid;
		Action->bCallParent = true;

		MenuBuilder.AddAction(Action);
	}

	return EVoxelIterate::Continue;
}

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildParameters(FGraphContextMenuBuilder& MenuBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Toolkit)
	{
		return EVoxelIterate::Continue;
	}

	if (MenuBuilder.FromPin &&
		MenuBuilder.FromPin->Direction == EGPD_Output)
	{
		return EVoxelIterate::Continue;
	}

	for (const FGuid& Guid : Toolkit->Asset->GetParameters())
	{
		const FVoxelParameter& Parameter = Toolkit->Asset->FindParameterChecked(Guid);
		if (MenuBuilder.FromPin &&
			!Parameter.Type.CanBeCastedTo_Schema(MenuBuilder.FromPin->PinType))
		{
			continue;
		}

		const TSharedRef<FVoxelGraphSchemaAction_NewParameterUsage> Action = MakeShared<FVoxelGraphSchemaAction_NewParameterUsage>(
			FText::FromString("Parameters"),
			FText::FromString("Get " + Parameter.Name.ToString()),
			FText::FromString(Parameter.Description),
			GroupId_Parameters);

		Action->Guid = Guid;
		Action->Type = Parameter.Type;

		MenuBuilder.AddAction(Action);

		if (MenuBuilder.FromPin)
		{
			ShortListActions_Parameters.Add(MakeShortListAction(Action));
		}
	}

	return EVoxelIterate::Continue;
}

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildInputs(FGraphContextMenuBuilder& MenuBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Toolkit ||
		!ActiveTerminalGraph ||
		!ActiveTerminalGraph->IsFunction())
	{
		return EVoxelIterate::Continue;
	}

	for (const FGuid& Guid : ActiveTerminalGraph->GetFunctionInputs())
	{
		const FVoxelGraphFunctionInput& Input = ActiveTerminalGraph->FindInputChecked(Guid);
		if (MenuBuilder.FromPin &&
			!Input.Type.CanBeCastedTo_Schema(MenuBuilder.FromPin->PinType))
		{
			continue;
		}

		if (!MenuBuilder.FromPin ||
			MenuBuilder.FromPin->Direction == EGPD_Input)
		{
			const TSharedRef<FVoxelGraphSchemaAction_NewFunctionInputUsage> Action = MakeShared<FVoxelGraphSchemaAction_NewFunctionInputUsage>(
				FText::FromString("Function Inputs"),
				FText::FromString("Get " + Input.Name.ToString()),
				FText::FromString(Input.Description),
				GroupId_Parameters);

			Action->Guid = Guid;
			Action->Type = Input.Type;

			MenuBuilder.AddAction(Action);

			if (MenuBuilder.FromPin)
			{
				ShortListActions_Parameters.Add(MakeShortListAction(Action));
			}
		}

		if (!MenuBuilder.FromPin ||
			MenuBuilder.FromPin->Direction == EGPD_Output)
		{
			{
				const TSharedRef<FVoxelGraphSchemaAction_NewFunctionInputDefaultUsage> Action = MakeShared<FVoxelGraphSchemaAction_NewFunctionInputDefaultUsage>(
					FText::FromString("Function Inputs"),
					FText::FromString("Set " + Input.Name.ToString() + " Default Value"),
					FText::FromString(Input.Description),
					GroupId_Parameters);

				Action->Guid = Guid;
				Action->Type = Input.Type;

				MenuBuilder.AddAction(Action);
			}
			{
				const TSharedRef<FVoxelGraphSchemaAction_NewFunctionInputPreviewUsage> Action = MakeShared<FVoxelGraphSchemaAction_NewFunctionInputPreviewUsage>(
					FText::FromString("Function Inputs"),
					FText::FromString("Set " + Input.Name.ToString() + " Preview"),
					FText::FromString(Input.Description),
					GroupId_Parameters);

				Action->Guid = Guid;
				Action->Type = Input.Type;

				MenuBuilder.AddAction(Action);
			}
		}
	}

	return EVoxelIterate::Continue;
}

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildOutputs(FGraphContextMenuBuilder& MenuBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Toolkit ||
		!ActiveTerminalGraph ||
		!ActiveTerminalGraph->IsFunction())
	{
		return EVoxelIterate::Continue;
	}

	if (MenuBuilder.FromPin &&
		MenuBuilder.FromPin->Direction == EGPD_Input)
	{
		return EVoxelIterate::Continue;
	}

	for (const FGuid& Guid : ActiveTerminalGraph->GetFunctionOutputs())
	{
		const FVoxelGraphFunctionOutput& Output = ActiveTerminalGraph->FindOutputChecked(Guid);
		if (MenuBuilder.FromPin &&
			!FVoxelPinType(MenuBuilder.FromPin->PinType).CanBeCastedTo_Schema(Output.Type))
		{
			continue;
		}

		const TSharedRef<FVoxelGraphSchemaAction_NewOutputUsage> Action = MakeShared<FVoxelGraphSchemaAction_NewOutputUsage>(
			FText::FromString("Function Outputs"),
			FText::FromString("Set " + Output.Name.ToString()),
			FText::FromString(Output.Description),
			GroupId_Parameters);

		Action->Guid = Guid;
		Action->Type = Output.Type;

		MenuBuilder.AddAction(Action);

		if (MenuBuilder.FromPin)
		{
			ShortListActions_Parameters.Add(MakeShortListAction(Action));
		}
	}

	return EVoxelIterate::Continue;
}

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildLocalVariables(FGraphContextMenuBuilder& MenuBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Toolkit ||
		!ActiveTerminalGraph)
	{
		return EVoxelIterate::Continue;
	}

	for (const FGuid& Guid : ActiveTerminalGraph->GetLocalVariables())
	{
		const FVoxelGraphLocalVariable& LocalVariable = ActiveTerminalGraph->FindLocalVariableChecked(Guid);

		for (const bool bIsDeclaration : TArray<bool>{ false, true })
		{
			if (MenuBuilder.FromPin)
			{
				if (MenuBuilder.FromPin->Direction == EGPD_Input)
				{
					if (bIsDeclaration)
					{
						continue;
					}

					if (!LocalVariable.Type.CanBeCastedTo_Schema(MenuBuilder.FromPin->PinType))
					{
						continue;
					}
				}
				else
				{
					check(MenuBuilder.FromPin->Direction == EGPD_Output);

					if (!bIsDeclaration)
					{
						continue;
					}

					if (!FVoxelPinType(MenuBuilder.FromPin->PinType).CanBeCastedTo_Schema(LocalVariable.Type))
					{
						continue;
					}
				}
			}

			const TSharedRef<FVoxelGraphSchemaAction_NewLocalVariableUsage> Action = MakeShared<FVoxelGraphSchemaAction_NewLocalVariableUsage>(
				FText::FromString("Local Variables"),
				FText::FromString((bIsDeclaration ? "Set " : "Get ") + LocalVariable.Name.ToString()),
				FText::FromString(LocalVariable.Description),
				GroupId_Parameters);

			Action->Guid = Guid;
			Action->Type = LocalVariable.Type;
			Action->bIsDeclaration = bIsDeclaration;

			MenuBuilder.AddAction(Action);

			if (MenuBuilder.FromPin)
			{
				ShortListActions_Parameters.Add(MakeShortListAction(Action));
			}
		}
	}

	return EVoxelIterate::Continue;
}

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildPromotions(FGraphContextMenuBuilder& MenuBuilder) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Toolkit ||
		!ActiveTerminalGraph)
	{
		return EVoxelIterate::Continue;
	}

	if (!MenuBuilder.FromPin ||
		MenuBuilder.FromPin->Direction == EGPD_Input)
	{
		MenuBuilder.AddAction(MakeShared<FVoxelGraphSchemaAction_NewParameter>(
			INVTEXT(""),
			FText::FromString(MenuBuilder.FromPin ? "Promote to parameter" : "Create new parameter"),
			FText::FromString(MenuBuilder.FromPin ? "Promote to parameter" : "Create new parameter"),
			GroupId_Parameters));
	}

	// Add Promote to input/output
	if (ActiveTerminalGraph->IsFunction())
	{
		if (!MenuBuilder.FromPin ||
			MenuBuilder.FromPin->Direction == EGPD_Input)
		{
			MenuBuilder.AddAction(MakeShared<FVoxelGraphSchemaAction_NewFunctionInput>(
				INVTEXT(""),
				FText::FromString(MenuBuilder.FromPin ? "Promote to function input" : "Create new function input"),
				FText::FromString(MenuBuilder.FromPin ? "Promote to function input" : "Create new function input"),
				GroupId_Parameters));
		}

		if (!MenuBuilder.FromPin ||
			MenuBuilder.FromPin->Direction == EGPD_Output)
		{
			MenuBuilder.AddAction(MakeShared<FVoxelGraphSchemaAction_NewFunctionOutput>(
				INVTEXT(""),
				FText::FromString(MenuBuilder.FromPin ? "Promote to function output" : "Create new function output"),
				FText::FromString(MenuBuilder.FromPin ? "Promote to function output" : "Create new function output"),
				GroupId_Parameters));
		}
	}

	// Add Promote to local variable
	MenuBuilder.AddAction(MakeShared<FVoxelGraphSchemaAction_NewLocalVariable>(
		INVTEXT(""),
		FText::FromString(MenuBuilder.FromPin ? "Promote to local variable" : "Create new local variable"),
		FText::FromString(MenuBuilder.FromPin ? "Promote to local variable" : "Create new local variable"),
		GroupId_Parameters));

	return EVoxelIterate::Continue;
}

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildCustomizeParameters(FGraphContextMenuBuilder& MenuBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Toolkit ||
		!ActiveTerminalGraph ||
		!ActiveTerminalGraph->IsEditorTerminalGraph())
	{
		return EVoxelIterate::Continue;
	}

	FVoxelNode_CustomizeParameter Template;

	const bool bHasPinMatch = !MenuBuilder.FromPin || (MenuBuilder.FromPin->Direction == EGPD_Output && INLINE_LAMBDA
	{
		const UEdGraphPin& FromPin = *MenuBuilder.FromPin;
		const FVoxelPinType FromType = FVoxelPinType(FromPin.PinType);

		for (const FVoxelPin& ToPin : Template.GetPins())
		{
			ensure(ToPin.bIsInput);

			if (FromType.CanBeCastedTo_Schema(ToPin.GetType()))
			{
				return true;
			}
		}

		return false;
	});

	if (!bHasPinMatch)
	{
		return EVoxelIterate::Continue;
	}

	for (const auto& Guid : Toolkit->Asset->GetParameters())
	{
		const FVoxelParameter& Parameter = Toolkit->Asset->FindParameterChecked(Guid);

		const TSharedRef<FVoxelGraphSchemaAction_NewCustomizeParameter> Action = MakeShared<FVoxelGraphSchemaAction_NewCustomizeParameter>(
			FText::FromString("Parameters"),
			FText::FromString("Customize " + Parameter.Name.ToString()),
			FText::FromString(Parameter.Description),
			GroupId_Parameters);

		Action->Guid = Guid;
		Action->Type = Parameter.Type;

		MenuBuilder.AddAction(Action);

		if (MenuBuilder.FromPin)
		{
			ShortListActions_Parameters.Add(MakeShortListAction(Action));
		}
	}

	return EVoxelIterate::Continue;
}

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildNodes(FGraphContextMenuBuilder& MenuBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	TConstVoxelArrayView<TSharedRef<const FVoxelNode>> Nodes = GVoxelNodeLibrary->GetNodes();

	NumStepActions = Nodes.Num();

	for (; NodeIndexToBuild < Nodes.Num(); NodeIndexToBuild++)
	{
		if (ShouldStop())
		{
			return EVoxelIterate::Stop;
		}

		NumStepCompletedActions++;

		const TSharedRef<const FVoxelNode>& Node = Nodes[NodeIndexToBuild];
		if (Toolkit)
		{
			if (!Node->CanPasteHere(*Toolkit->Asset, ActiveTerminalGraph))
			{
				continue;
			}

			if (Toolkit->IsGraphSensitive() &&
				!Node->IsDesignedForGraph(*Toolkit->Asset))
			{
				continue;
			}
		}

		if (Node->GetStruct()->IsChildOf<FVoxelOutputNode>() &&
			!Node->GetMetadataContainer().GetStringMetaDataHierarchical("Placeable"))
		{
			continue;
		}

		bool bIsExactMatch = true;
		FVoxelPinTypeSet PromotionTypes;

		const bool bHasPinMatch = !MenuBuilder.FromPin || INLINE_LAMBDA
		{
			const UEdGraphPin& FromPin = *MenuBuilder.FromPin;

			const FVoxelPinType FromPinType(FromPin.PinType);
			FVoxelPinTypeSet FromPinPromotionTypes;
			if (FromPinType.IsWildcard())
			{
				if (const UVoxelGraphNode* ThisNode = Cast<UVoxelGraphNode>(FromPin.GetOwningNode()))
				{
					if (!ThisNode->CanPromotePin(FromPin, FromPinPromotionTypes))
					{
						FromPinPromotionTypes.Add(FromPin.PinType);
					}
				}
			}
			else
			{
				FromPinPromotionTypes.Add(FromPinType);
			}

			FString PinTypeAliasesString;
			if (Node->GetMetadataContainer().GetStringMetaDataHierarchical(STATIC_FNAME("PinTypeAliases"), &PinTypeAliasesString))
			{
				TArray<FString> PinTypeAliases;
				PinTypeAliasesString.ParseIntoArray(PinTypeAliases, TEXT(","));

				for (const FString& PinTypeAlias : PinTypeAliases)
				{
					FVoxelPinType Type;
					if (!ensure(FVoxelPinType::TryParse(PinTypeAlias, Type)))
					{
						continue;
					}

					if ((FromPin.Direction == EGPD_Input && Type.CanBeCastedTo_Schema(FromPin.PinType)) ||
						(FromPin.Direction == EGPD_Output && FVoxelPinType(FromPin.PinType).CanBeCastedTo_Schema(Type)))
					{
						return true;
					}
				}
			}

			for (const FVoxelPin& ToPin : Node->GetPins())
			{
				if (FromPin.Direction == (ToPin.bIsInput ? EGPD_Input : EGPD_Output))
				{
					continue;
				}

				FVoxelPinTypeSet ToPinPromotionTypes;
				if (ToPin.IsPromotable())
				{
					ToPinPromotionTypes = Node->GetPromotionTypes(ToPin);
				}
				else
				{
					ToPinPromotionTypes.Add(ToPin.GetType());
				}

				if (ToPinPromotionTypes.IsInfinite())
				{
					PromotionTypes = ToPinPromotionTypes;
					bIsExactMatch = false;
					return true;
				}

				bool bFoundMatch = false;
				ToPinPromotionTypes.Iterate([&](const FVoxelPinType& Type)
				{
					return FromPinPromotionTypes.Iterate([&](const FVoxelPinType& FromType)
					{
						if (FromPin.Direction == EGPD_Input && Type.CanBeCastedTo_Schema(FromType))
						{
							bFoundMatch = true;
							PromotionTypes = ToPinPromotionTypes;
							return EVoxelIterate::Stop;
						}
						if (FromPin.Direction == EGPD_Output && FromType.CanBeCastedTo_Schema(Type))
						{
							bFoundMatch = true;
							PromotionTypes = ToPinPromotionTypes;
							return EVoxelIterate::Stop;
						}

						return EVoxelIterate::Continue;
					});
				});

				if (bFoundMatch)
				{
					return true;
				}
			}

			return false;
		};

		if (!bHasPinMatch)
		{
			continue;
		}

		FString Keywords;
		Node->GetMetadataContainer().GetStringMetaDataHierarchical(STATIC_FNAME("Keywords"), &Keywords);
		Keywords += Node->GetMetadataContainer().GetMetaData(STATIC_FNAME("CompactNodeTitle"));

		if (Node->GetMetadataContainer().HasMetaDataHierarchical(STATIC_FNAME("NativeMakeFunc")))
		{
			Keywords += "construct build";
		}
		if (Node->GetMetadataContainer().HasMetaDataHierarchical(STATIC_FNAME("Autocast")))
		{
			Keywords += "cast convert";
		}

		FString DisplayName = Node->GetDisplayName();
		Keywords += DisplayName;
		DisplayName.ReplaceInline(TEXT("\n"), TEXT(" "));

		bool bPermutationsAdded = false;
		if (MenuBuilder.FromPin &&
			Node->GetMetadataContainer().HasMetaData(STATIC_FNAME("Operator")))
		{
			const FString Operator = Node->GetMetadataContainer().GetMetaData(STATIC_FNAME("Operator"));

			TMap<FVoxelPinType, TSet<FVoxelPinType>> Permutations = CollectOperatorPermutations(*Node, *MenuBuilder.FromPin, PromotionTypes);
			for (const auto& It : Permutations)
			{
				FVoxelPinType InnerType = It.Key.GetInnerType();
				for (const FVoxelPinType& SecondType : It.Value)
				{
					FVoxelPinType SecondInnerType = SecondType.GetInnerType();

					const TSharedRef<FVoxelGraphSchemaAction_NewPromotableStructNode> Action = MakeShared<FVoxelGraphSchemaAction_NewPromotableStructNode>(
						FText::FromString(Node->GetCategory()),
						FText::FromString(InnerType.ToString() + " " + Operator + " " + SecondInnerType.ToString()),
						FText::FromString(Node->GetTooltip()),
						GroupId_Operators,
						FText::FromString(Keywords + " " + InnerType.ToString() + " " + SecondInnerType.ToString()));

					Action->PinTypes = { SecondType, It.Key };
					Action->Node = Node;

					MenuBuilder.AddAction(Action);
					bPermutationsAdded = true;
				}
			}
		}

		if (!bPermutationsAdded)
		{
			const TSharedRef<FVoxelGraphSchemaAction_NewStructNode> Action = MakeShared<FVoxelGraphSchemaAction_NewStructNode>(
				FText::FromString(Node->GetCategory()),
				FText::FromString(DisplayName),
				FText::FromString(Node->GetTooltip()),
				GroupId_StructNodes,
				FText::FromString(Keywords));

			Action->Node = Node;

			MenuBuilder.AddAction(Action);

			const bool bShowInGraphShortList = INLINE_LAMBDA
			{
				if (!Toolkit)
				{
					return false;
				}

				FString GraphShortListString;
				if (!Node->GetMetadataContainer().GetStringMetaDataHierarchical(STATIC_FNAME("ShowInGraphShortList"), &GraphShortListString))
				{
					return false;
				}

				if (Toolkit->Asset->IsFunctionLibrary())
				{
					return false;
				}

				const FString GraphType = Toolkit->Asset->GetGraphTypeName();
				TArray<FString> ShortList;
				GraphShortListString.ParseIntoArray(ShortList, TEXT(","));

				for (FString Type : ShortList)
				{
					Type = Type.TrimStartAndEnd().ToLower();
					if (Type == "all" ||
						Type == GraphType)
					{
						return true;
					}
				}

				return false;
			};

			if (bShowInGraphShortList ||
				(MenuBuilder.FromPin && Node->GetMetadataContainer().HasMetaDataHierarchical(STATIC_FNAME("ShowInPinShortList"))))
			{
				ShortListActions_Forced.Add(MakeShortListAction(Action));
			}
			else if (MenuBuilder.FromPin && bIsExactMatch)
			{
				ShortListActions_ExactMatches.Add(MakeShortListAction(Action));
			}
		}
	}

	return EVoxelIterate::Continue;
}

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildFunctionLibraries(FGraphContextMenuBuilder& MenuBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	if (AssetsToLoad.Num() == 0)
	{
		ForEachAssetDataOfClass<UVoxelFunctionLibraryAsset>([&](const FAssetData& AssetData)
		{
			AssetsToLoad.Add(AssetData);
		});

		NumStepActions = AssetsToLoad.Num();
	}

	while (AssetsToLoad.Num() > 0)
	{
		if (ShouldStop())
		{
			return EVoxelIterate::Stop;
		}

		NumStepCompletedActions++;

		FAssetData AssetData = AssetsToLoad.Pop();
		UVoxelFunctionLibraryAsset* FunctionLibrary = Cast<UVoxelFunctionLibraryAsset>(AssetData.GetAsset());
		if (!FunctionLibrary)
		{
			continue;
		}

		for (const FGuid& Guid : FunctionLibrary->GetGraph().GetTerminalGraphs())
		{
			const UVoxelTerminalGraph* TerminalGraph = FunctionLibrary->GetGraph().FindTerminalGraph(Guid);
			if (!ensure(TerminalGraph) ||
				TerminalGraph->IsMainTerminalGraph() ||
				TerminalGraph->IsEditorTerminalGraph() ||
				!TerminalGraph->bExposeToLibrary ||
				!CanCallTerminalGraph(MenuBuilder, TerminalGraph))
			{
				continue;
			}

			if (Toolkit &&
				!TerminalGraph->CanBePlaced(*Toolkit->Asset))
			{
				continue;
			}

			const FVoxelGraphMetadata Metadata = TerminalGraph->GetMetadata();

			const TSharedRef<FVoxelGraphSchemaAction_NewCallExternalFunctionNode> Action = MakeShared<FVoxelGraphSchemaAction_NewCallExternalFunctionNode>(
				FText::FromString(Metadata.Category),
				FText::FromString(Metadata.DisplayName),
				FText::FromString(Metadata.Description),
				GroupId_Functions,
				FText::FromString(TerminalGraph->Keywords.ToLower()));

			Action->FunctionLibrary = FunctionLibrary;
			Action->Guid = Guid;

			MenuBuilder.AddAction(Action);
		}
	}

	return EVoxelIterate::Continue;
}

EVoxelIterate FVoxelGraphContextActionsBuilder::BuildShortList(FGraphContextMenuBuilder& MenuBuilder)
{
	VOXEL_FUNCTION_COUNTER();

	for (const TSharedPtr<FEdGraphSchemaAction>& Action : ShortListActions_Forced)
	{
		MenuBuilder.AddAction(Action);
	}

	if (ShortListActions_ExactMatches.Num() < 10)
	{
		for (const TSharedPtr<FEdGraphSchemaAction>& Action : ShortListActions_ExactMatches)
		{
			MenuBuilder.AddAction(Action);
		}
	}

	for (const TSharedPtr<FEdGraphSchemaAction>& Action : ShortListActions_Parameters)
	{
		MenuBuilder.AddAction(Action);
	}

	return EVoxelIterate::Continue;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelGraphContextActionsBuilder::CanCallTerminalGraph(const FGraphContextMenuBuilder& MenuBuilder, const UVoxelTerminalGraph* TerminalGraph)
{
	if (!ensure(TerminalGraph))
	{
		return false;
	}

	if (!MenuBuilder.FromPin)
	{
		return true;
	}

	if (MenuBuilder.FromPin->Direction == EGPD_Input)
	{
		for (const FGuid& Guid : TerminalGraph->GetFunctionOutputs())
		{
			if (TerminalGraph->FindOutputChecked(Guid).Type.CanBeCastedTo_Schema(MenuBuilder.FromPin->PinType))
			{
				return true;
			}
		}
	}
	else
	{
		check(MenuBuilder.FromPin->Direction == EGPD_Output);

		for (const FGuid& Guid : TerminalGraph->GetFunctionInputs())
		{
			if (FVoxelPinType(MenuBuilder.FromPin->PinType).CanBeCastedTo_Schema(TerminalGraph->FindInputChecked(Guid).Type))
			{
				return true;
			}
		}
	}

	return false;
}

bool FVoxelGraphContextActionsBuilder::ShouldStop() const
{
	return FPlatformTime::Seconds() > EndTime;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TMap<FVoxelPinType, TSet<FVoxelPinType>> FVoxelGraphContextActionsBuilder::CollectOperatorPermutations(
	const FVoxelNode& Node,
	const UEdGraphPin& FromPin,
	const FVoxelPinTypeSet& PromotionTypes)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelPinType FromPinType = FVoxelPinType(FromPin.PinType);
	const bool bIsBuffer = FromPinType.IsBuffer();

	if (!PromotionTypes.Contains(FromPinType))
	{
		return {};
	}

	const bool bIsCommutativeOperator = Node.IsA<FVoxelTemplateNode_CommutativeAssociativeOperator>();

	TMap<FVoxelPinType, TSet<FVoxelPinType>> Result;
	if (FromPin.Direction == EGPD_Output)
	{
		PromotionTypes.Iterate([&](const FVoxelPinType& Type)
		{
			if (Type.IsBuffer() != bIsBuffer)
			{
				return;
			}

			Result.FindOrAdd(FromPinType, {}).Add(Type);

			if (!bIsCommutativeOperator)
			{
				Result.FindOrAdd(Type, {}).Add(FromPinType);
			}
		});
	}
	else
	{
		const int32 FromDimension = FVoxelTemplateNodeUtilities::GetDimension(FromPinType);

		PromotionTypes.Iterate([&](const FVoxelPinType& Type)
		{
			if (Type.IsBuffer() != bIsBuffer ||
				FromDimension < FVoxelTemplateNodeUtilities::GetDimension(Type))
			{
				return;
			}

			if (bIsCommutativeOperator)
			{
				Result.FindOrAdd(FromPinType, {}).Add(Type);
				return;
			}

			PromotionTypes.Iterate([&](const FVoxelPinType& SecondType)
			{
				if (SecondType.IsBuffer() != bIsBuffer ||
					FromDimension < FVoxelTemplateNodeUtilities::GetDimension(SecondType))
				{
					return;
				}

				if (Type != FromPinType &&
					SecondType != FromPinType)
				{
					return;
				}

				Result.FindOrAdd(Type, {}).Add(SecondType);
			});
		});
	}

	return Result;
}