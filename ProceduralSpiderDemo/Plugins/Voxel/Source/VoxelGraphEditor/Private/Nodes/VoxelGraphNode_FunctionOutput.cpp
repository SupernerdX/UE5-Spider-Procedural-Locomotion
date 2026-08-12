// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphNode_FunctionOutput.h"
#include "VoxelGraphToolkit.h"
#include "VoxelGraphTracker.h"
#include "VoxelGraphVisuals.h"

const FVoxelGraphFunctionOutput* UVoxelGraphNode_FunctionOutput::GetOutput() const
{
	const UVoxelTerminalGraph* TerminalGraph = GetTypedOuter<UVoxelTerminalGraph>();
	if (!ensure(TerminalGraph))
	{
		return nullptr;
	}

	const FVoxelGraphFunctionOutput* Output = TerminalGraph->FindOutput(Guid);
	if (!Output)
	{
		return nullptr;
	}

	ConstCast(this)->CachedOutput = *Output;

	return Output;
}

FVoxelGraphFunctionOutput UVoxelGraphNode_FunctionOutput::GetOutputSafe() const
{
	if (const FVoxelGraphFunctionOutput* Output = GetOutput())
	{
		return *Output;
	}

	return CachedOutput;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGraphNode_FunctionOutput::AllocateDefaultPins()
{
	const UVoxelTerminalGraph* TerminalGraph = GetTypedOuter<UVoxelTerminalGraph>();
	if (ensure(TerminalGraph))
	{
		OnOutputChangedPtr = MakeSharedVoid();
		GVoxelGraphTracker->OnOutputChanged(*TerminalGraph).Add(FOnVoxelGraphChanged::Make(OnOutputChangedPtr, this, [this]
		{
			ReconstructNode();
		}));
	}

	const FVoxelGraphFunctionOutput Output = GetOutputSafe();

	UEdGraphPin* Pin = CreatePin(
		EGPD_Input,
		Output.Type.GetEdGraphPinType(),
		STATIC_FNAME("Value"));

	Pin->PinFriendlyName = FText::FromName(Output.Name);

	Super::AllocateDefaultPins();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGraphNode_FunctionOutput::PrepareForCopying()
{
	Super::PrepareForCopying();

	// Updated CachedOutput
	(void)GetOutput();
}

void UVoxelGraphNode_FunctionOutput::PostPasteNode()
{
	Super::PostPasteNode();

	UVoxelTerminalGraph* TerminalGraph = GetTypedOuter<UVoxelTerminalGraph>();
	if (!ensure(TerminalGraph))
	{
		return;
	}

	if (TerminalGraph->FindOutput(Guid))
	{
		return;
	}

	for (const FGuid& OutputGuid : TerminalGraph->GetFunctionOutputs())
	{
		const FVoxelGraphFunctionOutput& Output = TerminalGraph->FindOutputChecked(OutputGuid);
		if (Output.Name != CachedOutput.Name ||
			Output.Type != CachedOutput.Type)
		{
			continue;
		}

		Guid = OutputGuid;
		CachedOutput = Output;
		return;
	}

	// Add a new output
	// Regenerate guid to be safe
	Guid = FGuid::NewGuid();

	TerminalGraph->AddFunctionOutput(Guid, CachedOutput);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FText UVoxelGraphNode_FunctionOutput::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	if (TitleType != ENodeTitleType::FullTitle)
	{
		return INVTEXT("OUTPUT");
	}

	return FText::FromString("Output " + CachedOutput.Name.ToString());
}

FLinearColor UVoxelGraphNode_FunctionOutput::GetNodeTitleColor() const
{
	return FVoxelGraphVisuals::GetPinColor(GetOutputSafe().Type);
}

FString UVoxelGraphNode_FunctionOutput::GetSearchTerms() const
{
	return Guid.ToString();
}