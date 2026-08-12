// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphNode_FunctionInput.h"
#include "VoxelGraph.h"
#include "VoxelGraphTracker.h"
#include "VoxelGraphVisuals.h"

UVoxelTerminalGraph* UVoxelGraphNode_FunctionInputBase::GetTerminalGraph() const
{
	UVoxelTerminalGraph* TerminalGraph = GetTypedOuter<UVoxelTerminalGraph>();
	if (!ensure(TerminalGraph))
	{
		return nullptr;
	}

	return TerminalGraph;
}

const FVoxelGraphFunctionInput* UVoxelGraphNode_FunctionInputBase::GetInput() const
{
	const UVoxelTerminalGraph* TerminalGraph = GetTerminalGraph();
	if (!ensure(TerminalGraph))
	{
		return nullptr;
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(Guid);
	if (!Input)
	{
		return nullptr;
	}

	ConstCast(this)->CachedInput = *Input;

	return Input;
}

FVoxelGraphFunctionInput UVoxelGraphNode_FunctionInputBase::GetInputSafe() const
{
	if (const FVoxelGraphFunctionInput* Input = GetInput())
	{
		return *Input;
	}

	return CachedInput;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGraphNode_FunctionInputBase::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	const UVoxelTerminalGraph* TerminalGraph = GetTerminalGraph();
	if (ensure(TerminalGraph))
	{
		OnInputChangedPtr = MakeSharedVoid();
		GVoxelGraphTracker->OnInputChanged(*TerminalGraph).Add(FOnVoxelGraphChanged::Make(OnInputChangedPtr, this, [=, this]
		{
			ReconstructNode();
		}));
	}
}

void UVoxelGraphNode_FunctionInputBase::PrepareForCopying()
{
	Super::PrepareForCopying();

	// Updated CachedInput
	(void)GetInput();
}

void UVoxelGraphNode_FunctionInputBase::PostPasteNode()
{
	Super::PostPasteNode();

	UVoxelTerminalGraph* TerminalGraph = GetTerminalGraph();
	if (!ensure(TerminalGraph))
	{
		return;
	}

	if (TerminalGraph->FindInput(Guid))
	{
		return;
	}

	for (const FGuid& InputGuid : TerminalGraph->GetFunctionInputs())
	{
		const FVoxelGraphFunctionInput& Input = TerminalGraph->FindInputChecked(InputGuid);
		if (Input.Name != CachedInput.Name ||
			Input.Type != CachedInput.Type)
		{
			continue;
		}

		Guid = InputGuid;
		CachedInput = Input;
		return;
	}

	// Add a new input
	// Regenerate guid to be safe
	Guid = FGuid::NewGuid();

	TerminalGraph->AddFunctionInput(Guid, CachedInput);
}

FString UVoxelGraphNode_FunctionInputBase::GetSearchTerms() const
{
	return Guid.ToString();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGraphNode_FunctionInput::AllocateDefaultPins()
{
	const FVoxelGraphFunctionInput Input = GetInputSafe();

	UEdGraphPin* Pin = CreatePin(
		EGPD_Output,
		CachedInput.Type.GetEdGraphPinType(),
		STATIC_FNAME("Value"));

	Pin->PinFriendlyName = FText::FromName(Input.Name);

	Super::AllocateDefaultPins();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FText UVoxelGraphNode_FunctionInput::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	if (TitleType != ENodeTitleType::FullTitle)
	{
		return {};
	}

	return FText::FromString("Input " + CachedInput.Name.ToString());
}

FLinearColor UVoxelGraphNode_FunctionInput::GetNodeTitleColor() const
{
	return FVoxelGraphVisuals::GetPinColor(GetInputSafe().Type);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGraphNode_FunctionInputDefaultsBase::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	const FVoxelGraphFunctionInput Input = GetInputSafe();

	UEdGraphPin* Pin = CreatePin(
		EGPD_Input,
		Input.Type.GetEdGraphPinType(),
		GetInputPinName());

	Pin->PinFriendlyName = FText::FromName(Input.Name);
}

FLinearColor UVoxelGraphNode_FunctionInputDefaultsBase::GetNodeTitleColor() const
{
	return FLinearColor::Red;
}

FSlateIcon UVoxelGraphNode_FunctionInputDefaultsBase::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = FVoxelGraphVisuals::GetPinColor(GetInputSafe().Type);

	return FVoxelGraphVisuals::GetPinIcon(GetInputSafe().Type);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGraphNode_FunctionInputDefault::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	if (UVoxelTerminalGraph* TerminalGraph = GetTerminalGraph())
	{
		GVoxelGraphTracker->NotifyInputChanged(*TerminalGraph);
	}
}

void UVoxelGraphNode_FunctionInputDefault::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	if (UVoxelTerminalGraph* TerminalGraph = GetTerminalGraph())
	{
		GVoxelGraphTracker->NotifyInputChanged(*TerminalGraph);
	}
}

void UVoxelGraphNode_FunctionInputDefault::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();

	if (UVoxelTerminalGraph* TerminalGraph = GetTerminalGraph())
	{
		GVoxelGraphTracker->NotifyInputChanged(*TerminalGraph);
	}
}

void UVoxelGraphNode_FunctionInputDefault::DestroyNode()
{
	Super::DestroyNode();

	if (UVoxelTerminalGraph* TerminalGraph = GetTerminalGraph())
	{
		GVoxelGraphTracker->NotifyInputChanged(*TerminalGraph);
	}
}

FName UVoxelGraphNode_FunctionInputDefault::GetInputPinName() const
{
	return STATIC_FNAME("Default");
}

FText UVoxelGraphNode_FunctionInputDefault::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType != ENodeTitleType::FullTitle)
	{
		return {};
	}

	return FText::FromString("Set " + GetInputSafe().Name.ToString() + " Default Value");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FName UVoxelGraphNode_FunctionInputPreview::GetInputPinName() const
{
	return STATIC_FNAME("Preview");
}

FText UVoxelGraphNode_FunctionInputPreview::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType != ENodeTitleType::FullTitle)
	{
		return {};
	}

	return FText::FromString("Set " + GetInputSafe().Name.ToString() + " Preview");
}