// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "K2Node_SetVoxelGraphParameter.h"
#include "K2Node_GetVoxelGraphParameter.h"
#include "Graphs/VoxelHeightGraph.h"
#include "Graphs/VoxelVolumeGraph.h"
#include "Sculpt/Height/VoxelHeightSculptGraph.h"
#include "Sculpt/Volume/VoxelVolumeSculptGraph.h"
#include "Graphs/VoxelParameterBlueprintLibrary.h"

#include "KismetCompiler.h"
#include "K2Node_MakeArray.h"
#include "K2Node_GetArrayItem.h"

void UK2Node_SetVoxelGraphParameterBase::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	if (UEdGraphPin* OutValuePin = FindPin(TEXT("OutValue")))
	{
		OutValuePin->PinFriendlyName = INVTEXT("Value");
	}

	{
		const int32 ValuePinIndex = Pins.IndexOfByPredicate([](const UEdGraphPin* Pin)
		{
			return Pin->PinName == "Value";
		});
		const int32 AssetPinIndex = Pins.IndexOfByPredicate([this](const UEdGraphPin* Pin)
		{
			return Pin->PinName == GetGraphPinName();
		});

		if (!ensure(ValuePinIndex != -1) ||
			!ensure(AssetPinIndex != -1))
		{
			return;
		}

		Pins.Swap(ValuePinIndex, AssetPinIndex);
	}

	{
		const int32 ValuePinIndex = Pins.IndexOfByPredicate([](const UEdGraphPin* Pin)
		{
			return Pin->PinName == "Value";
		});
		const int32 ParameterPinIndex = Pins.IndexOfByPredicate([](const UEdGraphPin* Pin)
		{
			return Pin->PinName == ParameterPinName;
		});

		if (!ensure(ValuePinIndex != -1) ||
			!ensure(ParameterPinIndex != -1))
		{
			return;
		}

		Pins.Swap(ValuePinIndex, ParameterPinIndex);
	}
}

void UK2Node_SetVoxelGraphParameterBase::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin* NamePin = FindPin(TEXT("Name"));
	UEdGraphPin* OutValuePin = FindPin(TEXT("OutValue"));
	UEdGraphPin* OwnerPin = FindPin(GetOwnerPinName());

	if (!ensure(NamePin) ||
		!ensure(OutValuePin) ||
		!ensure(OwnerPin) ||
		OutValuePin->LinkedTo.Num() == 0)
	{
		return;
	}

	UK2Node_VoxelGraphParameterBase* GetterNode = SpawnGetterNode(CompilerContext, SourceGraph);
	GetterNode->AllocateDefaultPins();

	UEdGraphPin* FunctionNamePin = GetterNode->FindPin(TEXT("Name"));
	UEdGraphPin* FunctionValuePin = GetterNode->FindPin(TEXT("Value"));
	UEdGraphPin* FunctionOwnerPin = GetterNode->FindPin(GetOwnerPinName());

	if (!ensure(FunctionNamePin) ||
		!ensure(FunctionValuePin) ||
		!ensure(FunctionOwnerPin))
	{
		return;
	}

	GetterNode->CachedParameter = CachedParameter;
	FunctionNamePin->DefaultValue = NamePin->DefaultValue;

	GetterNode->PostReconstructNode();

	// Will be null if pure
	if (GetterNode->GetThenPin())
	{
		CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *GetterNode->GetThenPin());
	}

	CompilerContext.CopyPinLinksToIntermediate(*OwnerPin, *FunctionOwnerPin);
	CompilerContext.CopyPinLinksToIntermediate(*NamePin, *FunctionNamePin);
	CompilerContext.MovePinLinksToIntermediate(*OutValuePin, *FunctionValuePin);

	GetThenPin()->MakeLinkTo(GetterNode->GetExecPin());

	GetterNode->PostReconstructNode();

	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(GetterNode, this);
}

bool UK2Node_SetVoxelGraphParameterBase::IsPinWildcard(const UEdGraphPin& Pin) const
{
	return
		Pin.PinName == "Value" ||
		Pin.PinName == "OutValue";
}

UEdGraphPin* UK2Node_SetVoxelGraphParameterBase::GetParameterNamePin() const
{
	return FindPin(TEXT("Name"));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UK2Node_SetVoxelHeightGraphParameter::UK2Node_SetVoxelHeightGraphParameter()
{
	FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UVoxelParameterBlueprintLibrary, K2_SetVoxelHeightGraphParameter),
		UVoxelParameterBlueprintLibrary::StaticClass());
}

UClass* UK2Node_SetVoxelHeightGraphParameter::GetGraphClassType() const
{
	return UVoxelHeightGraph::StaticClass();
}

FName UK2Node_SetVoxelHeightGraphParameter::GetGraphPinName() const
{
	return "Graph";
}

FName UK2Node_SetVoxelHeightGraphParameter::GetOwnerPinName() const
{
	return "Stamp";
}

UK2Node_VoxelGraphParameterBase* UK2Node_SetVoxelHeightGraphParameter::SpawnGetterNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	return CompilerContext.SpawnIntermediateNode<UK2Node_GetVoxelHeightGraphParameter>(this, SourceGraph);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UK2Node_SetVoxelVolumeGraphParameter::UK2Node_SetVoxelVolumeGraphParameter()
{
	FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UVoxelParameterBlueprintLibrary, K2_SetVoxelVolumeGraphParameter),
		UVoxelParameterBlueprintLibrary::StaticClass());
}

UClass* UK2Node_SetVoxelVolumeGraphParameter::GetGraphClassType() const
{
	return UVoxelVolumeGraph::StaticClass();
}

FName UK2Node_SetVoxelVolumeGraphParameter::GetGraphPinName() const
{
	return "Graph";
}

FName UK2Node_SetVoxelVolumeGraphParameter::GetOwnerPinName() const
{
	return "Stamp";
}

UK2Node_VoxelGraphParameterBase* UK2Node_SetVoxelVolumeGraphParameter::SpawnGetterNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	return CompilerContext.SpawnIntermediateNode<UK2Node_GetVoxelVolumeGraphParameter>(this, SourceGraph);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UK2Node_SetVoxelHeightSculptGraphParameter::UK2Node_SetVoxelHeightSculptGraphParameter()
{
	FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UVoxelParameterBlueprintLibrary, K2_SetVoxelHeightSculptGraphParameter),
		UVoxelParameterBlueprintLibrary::StaticClass());
}

UClass* UK2Node_SetVoxelHeightSculptGraphParameter::GetGraphClassType() const
{
	return UVoxelHeightSculptGraph::StaticClass();
}

FName UK2Node_SetVoxelHeightSculptGraphParameter::GetGraphPinName() const
{
	return "Graph";
}

FName UK2Node_SetVoxelHeightSculptGraphParameter::GetOwnerPinName() const
{
	return "SculptGraph";
}

UK2Node_VoxelGraphParameterBase* UK2Node_SetVoxelHeightSculptGraphParameter::SpawnGetterNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	return CompilerContext.SpawnIntermediateNode<UK2Node_GetVoxelHeightSculptGraphParameter>(this, SourceGraph);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UK2Node_SetVoxelVolumeSculptGraphParameter::UK2Node_SetVoxelVolumeSculptGraphParameter()
{
	FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UVoxelParameterBlueprintLibrary, K2_SetVoxelVolumeSculptGraphParameter),
		UVoxelParameterBlueprintLibrary::StaticClass());
}

UClass* UK2Node_SetVoxelVolumeSculptGraphParameter::GetGraphClassType() const
{
	return UVoxelVolumeSculptGraph::StaticClass();
}

FName UK2Node_SetVoxelVolumeSculptGraphParameter::GetGraphPinName() const
{
	return "Graph";
}

FName UK2Node_SetVoxelVolumeSculptGraphParameter::GetOwnerPinName() const
{
	return "SculptGraph";
}

UK2Node_VoxelGraphParameterBase* UK2Node_SetVoxelVolumeSculptGraphParameter::SpawnGetterNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	return CompilerContext.SpawnIntermediateNode<UK2Node_GetVoxelVolumeSculptGraphParameter>(this, SourceGraph);
}